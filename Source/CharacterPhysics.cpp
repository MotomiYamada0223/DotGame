#include "CharacterPhysics.h"
#include "GameConstants.h"
#include <algorithm>
#include <cmath>

namespace
{
	bool IsSolidCollisionType(BlockMap::CollisionType type)
	{
		return type == BlockMap::CollisionType::Block ||
			type == BlockMap::CollisionType::BossArea ||
			type == BlockMap::CollisionType::BreakableBlock;
	}

	void AccumulateSpecialCollisionType(
		BlockMap::CollisionType& currentSpecial,
		BlockMap::CollisionType foundType)
	{
		if (foundType == BlockMap::CollisionType::Death ||
			foundType == BlockMap::CollisionType::Goal)
		{
			currentSpecial = foundType;
		}
		else if (foundType == BlockMap::CollisionType::BossArea &&
			currentSpecial != BlockMap::CollisionType::Death &&
			currentSpecial != BlockMap::CollisionType::Goal)
		{
			currentSpecial = foundType;
		}
	}

	// 指定矩形に固体ブロックが含まれるかを調べ、特殊タイル(Death/Goal/BossArea)もあわせて記録する
	bool HasSolidCollision(
		const BlockMap& blockMap,
		float x,
		float y,
		float width,
		float height,
		BlockMap::CollisionType* outSpecialType = nullptr)
	{
		const int left = static_cast<int>(x);
		const int right = static_cast<int>(x + width - 1.0f);
		const int top = static_cast<int>(y);
		const int bottom = static_cast<int>(y + height - 1.0f);

		bool hitSolid = false;
		for (int mapY = top; mapY <= bottom; ++mapY)
		{
			for (int mapX = left; mapX <= right; ++mapX)
			{
				const BlockMap::CollisionType type = blockMap.GetCollisionType(mapX, mapY);
				if (type == BlockMap::CollisionType::None) { continue; }

				if (outSpecialType != nullptr)
				{
					AccumulateSpecialCollisionType(*outSpecialType, type);
				}

				if (IsSolidCollisionType(type))
				{
					hitSolid = true;
					if (outSpecialType == nullptr ||
						*outSpecialType == BlockMap::CollisionType::Death ||
						*outSpecialType == BlockMap::CollisionType::Goal)
					{
						return true;
					}
				}
			}
		}

		return hitSolid;
	}

	// 指定足元幅 [leftCol .. leftCol + widthCols - 1] を支える最上部の床面Yを探索する
	// 上端プローブ行に既に固体がある（壁に接している）場合や床が無い場合は false
	bool TrySampleFootSupportY(
		const BlockMap& blockMap,
		int leftCol,
		int widthCols,
		int footY,
		int probeRangeY,
		int& outSupportY)
	{
		const int probeTopY = footY - probeRangeY;
		const int probeBottomY = footY + probeRangeY;
		const int rightCol = leftCol + widthCols - 1;

		for (int mx = leftCol; mx <= rightCol; ++mx)
		{
			if (IsSolidCollisionType(blockMap.GetCollisionType(mx, probeTopY)))
			{
				return false;
			}
		}

		for (int sy = probeTopY + 1; sy <= probeBottomY; ++sy)
		{
			for (int mx = leftCol; mx <= rightCol; ++mx)
			{
				if (IsSolidCollisionType(blockMap.GetCollisionType(mx, sy)))
				{
					outSupportY = sy;
					return true;
				}
			}
		}

		return false;
	}

	// 坂道沿いの実移動速度が平地と同じになるよう、水平速度への補正倍率 (cosθ) を求める
	float MeasureSlopeSpeedScale(
		const BlockMap& blockMap,
		float posX,
		float footY,
		float width)
	{
		const float halfWidth = width * 0.5f;
		const int baseLeft = static_cast<int>(std::floor(posX - halfWidth));
		const int widthCols = (std::max)(1, static_cast<int>(width));
		const int refFootY = static_cast<int>(std::round(footY));

		int yLeft = 0;
		int yRight = 0;
		const int span = PlayerBlockCollision::SlopeMeasureHalfSpan;
		if (!TrySampleFootSupportY(blockMap, baseLeft - span, widthCols, refFootY, PlayerBlockCollision::SlopeProbeRangeY, yLeft) ||
			!TrySampleFootSupportY(blockMap, baseLeft + span, widthCols, refFootY, PlayerBlockCollision::SlopeProbeRangeY, yRight))
		{
			return 1.0f;
		}

		const float dy = static_cast<float>(yRight - yLeft);
		if (std::abs(dy) <= static_cast<float>(PlayerBlockCollision::SlopeFlatIgnoreDeltaY))
		{
			return 1.0f;
		}

		const float slope = dy / static_cast<float>(span * 2);
		return 1.0f / std::sqrt(1.0f + slope * slope);
	}

	// 1px単位の小刻み移動で坂道の上り段差・下り吸着と本物の壁の停止を正確に判定する
	void ResolveHorizontalMove(
		const BlockMap& blockMap,
		VECTOR& position,
		float velocityY,
		bool wasGrounded,
		float width,
		float height,
		float moveSpeed,
		float moveDirection,
		BlockMap::CollisionType& outSpecialType)
	{
		float totalDx = moveSpeed * moveDirection;
		if (totalDx == 0.0f) { return; }

		if (wasGrounded && velocityY >= 0.0f)
		{
			totalDx *= MeasureSlopeSpeedScale(blockMap, position.x, position.y + height * 0.5f, width);
		}

		const float halfWidth = width * 0.5f;
		const float halfHeight = height * 0.5f;
		const int steps = (std::max)(
			1,
			static_cast<int>(std::ceil(std::abs(totalDx) / PlayerBlockCollision::SlopeSubstepSize)));
		const float stepDx = totalDx / static_cast<float>(steps);

		for (int i = 0; i < steps; ++i)
		{
			const float trialX = position.x + stepDx;
			const float edgeX = (stepDx < 0.0f)
				? (trialX - halfWidth)
				: (trialX + halfWidth - PlayerBlockCollision::CollisionThickness);
			const float curTop = position.y - halfHeight;

			if (!HasSolidCollision(
				blockMap,
				edgeX,
				curTop,
				PlayerBlockCollision::CollisionThickness,
				height,
				&outSpecialType))
			{
				position.x = trialX;

				// 接地中（かつ非ジャンプ上昇時）に下り坂を降りる際は、微細な段差へ足元を追従させる
				if (wasGrounded && velocityY >= 0.0f)
				{
					const float boxLeft = position.x - halfWidth;
					const float footY = position.y + halfHeight;
					if (!HasSolidCollision(
						blockMap,
						boxLeft,
						footY,
						width,
						PlayerBlockCollision::CollisionThickness))
					{
						for (int drop = 1; drop <= PlayerBlockCollision::SlopeMaxStepDownPerPixel; ++drop)
						{
							if (HasSolidCollision(
								blockMap,
								boxLeft,
								footY + static_cast<float>(drop),
								width,
								PlayerBlockCollision::CollisionThickness))
							{
								position.y += static_cast<float>(drop);
								break;
							}
						}
					}
				}
				continue;
			}

			// 細かな階段状坂道の上り段差であれば、頭上の天井に当たらない範囲で上り上げる
			bool climbed = false;
			for (int step = 1; step <= PlayerBlockCollision::SlopeMaxStepUpPerPixel; ++step)
			{
				const float candTop = curTop - static_cast<float>(step);
				const bool edgeClear = !HasSolidCollision(
					blockMap,
					edgeX,
					candTop,
					PlayerBlockCollision::CollisionThickness,
					height,
					&outSpecialType);
				const bool ceilingClear = !HasSolidCollision(
					blockMap,
					trialX - halfWidth,
					candTop,
					width,
					static_cast<float>(step),
					&outSpecialType);

				if (edgeClear && ceilingClear)
				{
					position.x = trialX;
					position.y -= static_cast<float>(step);
					climbed = true;
					break;
				}
			}

			if (!climbed)
			{
				break;
			}
		}
	}

	void ResolveVerticalMove(
		const BlockMap& blockMap,
		VECTOR& position,
		float& velocityY,
		bool& isGrounded,
		bool& isJumping,
		bool wasGrounded,
		float width,
		float height,
		BlockMap::CollisionType& outSpecialType)
	{
		const float halfWidth = width * 0.5f;
		const float halfHeight = height * 0.5f;
		const float boxLeft = position.x - halfWidth;
		const float oldTop = position.y - halfHeight;
		const float oldBottom = position.y + halfHeight;
		const float nextY = position.y + velocityY;
		const float nextTop = nextY - halfHeight;
		const float nextBottom = nextY + halfHeight;

		isGrounded = false;

		if (velocityY > 0.0f)
		{
			const int startY = static_cast<int>(oldBottom);
			const int endY = static_cast<int>(nextBottom);

			for (int sy = startY; sy <= endY; ++sy)
			{
				if (HasSolidCollision(
					blockMap,
					boxLeft,
					static_cast<float>(sy),
					width,
					PlayerBlockCollision::CollisionThickness,
					&outSpecialType))
				{
					position.y = static_cast<float>(sy) - halfHeight;
					velocityY = 0.0f;
					isGrounded = true;
					isJumping = false;
					return;
				}
			}

			position.y = nextY;
			isGrounded = false;
			isJumping = true;
		}
		else if (velocityY < 0.0f)
		{
			const int startY = static_cast<int>(oldTop);
			const int endY = static_cast<int>(nextTop);

			for (int sy = startY; sy >= endY; --sy)
			{
				if (HasSolidCollision(
					blockMap,
					boxLeft,
					static_cast<float>(sy),
					width,
					PlayerBlockCollision::CollisionThickness,
					&outSpecialType))
				{
					position.y = static_cast<float>(sy) + PlayerBlockCollision::CollisionThickness + halfHeight;
					velocityY = 0.0f;
					isGrounded = false;
					isJumping = true;
					return;
				}
			}

			position.y = nextY;
			isGrounded = false;
			isJumping = true;
		}
		else
		{
			position.y = nextY;
			if (wasGrounded && HasSolidCollision(
				blockMap,
				boxLeft,
				position.y + halfHeight,
				width,
				PlayerBlockCollision::CollisionThickness,
				&outSpecialType))
			{
				isGrounded = true;
				isJumping = false;
			}
			else
			{
				isGrounded = false;
				isJumping = true;
			}
		}
	}
}

// クラスの初期化を行う
CharacterPhysics::CharacterPhysics()
{
}

// クラスの終了処理を行う
CharacterPhysics::~CharacterPhysics()
{
}

BlockMap::CollisionType CharacterPhysics::CheckCollision(
	const BlockMap& blockMap,
	float x,
	float y,
	float width,
	float height,
	int* blockX,
	int* blockY) const
{
	// 引数はすべてマップ座標（mvPosition と同じ座標系）
	const int left = static_cast<int>(x);
	const int right = static_cast<int>(x + width - 1.0f);
	const int top = static_cast<int>(y);
	const int bottom = static_cast<int>(y + height - 1.0f);

	for (int mapY = top; mapY <= bottom; ++mapY)
	{
		for (int mapX = left; mapX <= right; ++mapX)
		{
			const BlockMap::CollisionType type =
				blockMap.GetCollisionType(mapX, mapY);

			if (type == BlockMap::CollisionType::None) { continue; }
			if (blockX != nullptr) { *blockX = mapX; }
			if (blockY != nullptr) { *blockY = mapY; }
			return type;
		}
	}
	return BlockMap::CollisionType::None;
}



// キャラクターの移動、重力処理、マップタイルとの当たり判定を解決する
BlockMap::CollisionType CharacterPhysics::UpdateMoveAndCollision(
	VECTOR& position,
	float& velocityY,
	bool& isGrounded,
	bool& isJumping,
	const BlockMap& blockMap,
	float width,
	float height,
	float gravity,
	float moveSpeed,
	float moveDirection)
{
	BlockMap::CollisionType detectedSpecialType =
		BlockMap::CollisionType::None;

	const bool wasGrounded = isGrounded;
	velocityY += gravity;

	// ジャンプ上昇中はY軸移動を先に解決し、斜面に擦って不自然な追加上昇が起きないようにする
	if (velocityY < 0.0f)
	{
		ResolveVerticalMove(
			blockMap,
			position,
			velocityY,
			isGrounded,
			isJumping,
			wasGrounded,
			width,
			height,
			detectedSpecialType);
		ResolveHorizontalMove(
			blockMap,
			position,
			velocityY,
			wasGrounded,
			width,
			height,
			moveSpeed,
			moveDirection,
			detectedSpecialType);
	}
	else
	{
		ResolveHorizontalMove(
			blockMap,
			position,
			velocityY,
			wasGrounded,
			width,
			height,
			moveSpeed,
			moveDirection,
			detectedSpecialType);
		ResolveVerticalMove(
			blockMap,
			position,
			velocityY,
			isGrounded,
			isJumping,
			wasGrounded,
			width,
			height,
			detectedSpecialType);
	}

	return detectedSpecialType;
}



// 指定した矩形が何らかの地形に接触しているかどうかを判定する
bool CharacterPhysics::IsBlockCollision(
	const BlockMap& blockMap,
	float x,
	float y,
	float width,
	float height) const
{
	return CheckCollision(
		blockMap,
		x,
		y,
		width,
		height
	) != BlockMap::CollisionType::None;
}

float CharacterPhysics::CalcSmoothedSlopeOffsetY(
	const BlockMap& blockMap,
	const VECTOR& position,
	float width,
	float height) const
{
	const float halfWidth = width * 0.5f;
	const float footY = position.y + height * 0.5f;
	const float leftF = position.x - halfWidth;
	const int baseLeft = static_cast<int>(std::floor(leftF));
	const float frac = leftF - static_cast<float>(baseLeft);
	const int widthCols = (std::max)(1, static_cast<int>(width));
	const int refFootY = static_cast<int>(std::round(footY));

	constexpr int kRadius = PlayerBlockCollision::SlopeVisualSmoothRadius;
	constexpr int kWindowCount = kRadius * 2 + 1;
	constexpr int kTotalSamples = kRadius * 2 + 2;

	int samples[kTotalSamples] = {};
	for (int i = 0; i < kTotalSamples; ++i)
	{
		const int col = baseLeft - kRadius + i;
		if (!TrySampleFootSupportY(
			blockMap,
			col,
			widthCols,
			refFootY,
			PlayerBlockCollision::SlopeProbeRangeY,
			samples[i]))
		{
			return 0.0f;
		}
	}

	if (std::abs(samples[kTotalSamples - 1] - samples[0]) <= PlayerBlockCollision::SlopeFlatIgnoreDeltaY)
	{
		return 0.0f;
	}

	int sum0 = 0;
	int sum1 = 0;
	for (int i = 0; i < kWindowCount; ++i)
	{
		sum0 += samples[i];
		sum1 += samples[i + 1];
	}

	const float avg0 = static_cast<float>(sum0) / static_cast<float>(kWindowCount);
	const float avg1 = static_cast<float>(sum1) / static_cast<float>(kWindowCount);
	const float smoothSupportY = avg0 + (avg1 - avg0) * frac;
	return smoothSupportY - footY;
}