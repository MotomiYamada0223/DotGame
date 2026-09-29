#include "Player.h"   
#include "DxLib.h"   
#include "Texture.h"   
#include "Master.h"   
#include "SceneManager.h"   
#include "Scene.h"   
#include "ObjectManager.h"   
#include "Enemy.h"   
#include "Collision.h"   
#include "GameConstants.h"   
#include "BlockAction.h"   
#include "InputManager.h"   

Player::Player(VECTOR initPos)
// TextureAnimationは使わず一枚絵としてロード   
	: Object2D(CharacterGraphPath::PlayerAnimation, initPos)
{
	SetTag(Object2D::Player2D);
	mbIsJumping = false;
	isGrounded = true;
	velocityY = 0.0f;
	isFacingRight = true;
	isAttacking = false;
	attackTimer = 0;
	isHitDamage = false;
	mInvincibleTimer = 0;
	isDead = false;
	isBlinking = false;
	blinkTimer = 0;
	blinkCooldownTimer = 0;
	blinkDirection = 0.0f;
	deadTimer = 0;
	alpha = 255.0f;
	mSpawnPos = initPos;
	mDeadState = 0;
	mpBlockMap = nullptr;
	mfMoveDirection = 0.0f;


	UpdateStatusByProgress(GameProgress::Tutorial1);
	mFallDeath.Reset();

	// 画像の読み込み   
	mHeartFullGraph = LoadGraph(CharacterGraphPath::HeartFull.c_str());
	if (mHeartFullGraph == -1) { printfDx("体力MAXの画像がない。"); }
	mHeartHalfGraph = LoadGraph(CharacterGraphPath::HeartHalf.c_str());
	if (mHeartHalfGraph == -1) { printfDx("体力半分の画像がない。"); }
	mHeartEmptyGraph = LoadGraph(CharacterGraphPath::HeartEmpty.c_str());
	if (mHeartEmptyGraph == -1) { printfDx("体力0の画像がない。"); }


	// プレイヤー当たり判定サイズ   
	// Width...幅   
	// Height...足元の位置   
	mfPlayerWidth = PlayerConstants::PlayerCollisionWidth;
	mfPlayerHeight = PlayerConstants::PlayerCollisionHeight;
}

Player::~Player()
{
	DeleteGraph(mHeartFullGraph);
	DeleteGraph(mHeartHalfGraph);
	DeleteGraph(mHeartEmptyGraph);
}

void Player::UpdateStatusByProgress(GameProgress progress)
{
	// なぜそうしたか・何のための処理か（意図）: 進行度ごとのパラメータ設定を明確に分離し、マジックナンバーの散逸を防ぐため   
	switch (progress)
	{
	case GameProgress::Tutorial1:
		mMaxHp = 15; mHp = 15;
		mAttack = 5; mDefense = 5; mMagic = 5;
		mIsBlinkWallDeathImmune = false;
		break;
	case GameProgress::PostTutorial1:
	case GameProgress::Tutorial2:
		mMaxHp = 30; mHp = 30;
		mAttack = 15; mDefense = 15; mMagic = 15;
		mIsBlinkWallDeathImmune = true;
		break;
	case GameProgress::PostTutorial2:
	case GameProgress::Tutorial3:
		mMaxHp = 90; mHp = 90;
		mAttack = 45; mDefense = 45; mMagic = 45;
		mIsBlinkWallDeathImmune = true;
		mAttackReachLevel = 1;
		mIsAttackFlashy = true;
		mBlinkCooldownLevel = 1;
		mIsPoisonImmune = true;
		mIsPetrificationImmune = true;
		break;
	case GameProgress::PostTutorial3:
	case GameProgress::BossTree:
	case GameProgress::BossSnake:
	case GameProgress::BossDragon:
		mMaxHp = 450; mHp = 450;
		mAttack = 250; mDefense = 250; mMagic = 250;
		mIsBlinkWallDeathImmune = true;
		mAttackReachLevel = 2;
		mIsAttackFlashy = true;
		mBlinkCooldownLevel = 1;
		mIsPoisonImmune = true;
		mIsPetrificationImmune = true;
		mIsFireImmune = true;
		break;
	}

	// 残機の代入   
	mlives = PlayerConstants::MaxLive;
}


void Player::PlayerMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap;
	// 死亡時は操作を受け付けない   
	if (isDead)
	{
		return;
	}

	if (!mbFallDeath && mvPosition.y >= PlayerConstants::PlayerDeathHeight)
	{
		mHp = 0;
		mbFallDeath = true;
	}

	// 移動領域の設定   
	if (mvPosition.x <= static_cast<float>(PlayerConstants::PlayerCollisionWidth))
	{
		mvPosition.x = static_cast<float>(PlayerConstants::PlayerCollisionWidth);
	}
	if (mvPosition.x >= static_cast<float>(mpBlockMap->GetCurrentWidth() - PlayerConstants::PlayerCollisionWidth))
	{
		mvPosition.x = static_cast<float>(mpBlockMap->GetCurrentWidth() - PlayerConstants::PlayerCollisionWidth);
	}


	// 左右移動   
	mfMoveDirection = 0.0f;
	if (CheckHitKey(KEY_INPUT_A) == 1)
	{
		mfMoveDirection = -1.0f;
		isFacingRight = false;
	}
	else if (CheckHitKey(KEY_INPUT_D) == 1)
	{
		mfMoveDirection = 1.0f;
		isFacingRight = true;
	}


<<<<<<< HEAD
	// キャラクターの物理処理
		float currentSpeed = moveSpeed;
	float currentGravity = gravity;
	if (isBlinking)
	{
		currentSpeed = blinkSpeed;
		currentGravity = 0.0f;
		velocityY = 0.0f; // 落下を止める
		mfMoveDirection = blinkDirection; // ブリンク開始時の方向に固定
	}

	float prevX = mvPosition.x;

=======
	// キャラクターの物理処理   
>>>>>>> AyameTest
	BlockMap::CollisionType collisionType =
		mCharacterPhysics.UpdateMoveAndCollision(
			mvPosition,
			velocityY,
			isGrounded,
			mbIsJumping,
			blockMap,
			mfPlayerWidth,
			mfPlayerHeight,
			currentGravity,
			currentSpeed,
			mfMoveDirection
		);

<<<<<<< HEAD
	if (isBlinking && !mIsBlinkWallDeathImmune)
	{
		// X軸方向の移動がほとんどできなかった場合、壁に激突したとみなして即死
		if (abs(mvPosition.x - prevX) < 1.0f)
		{
			TakeDamage(mHp);
		}
	}

	// 当たり判定後にスクロール量を反映させるため、親クラスの共通関数を使用してプレイヤーのスクリーン座標を算出する
=======
	// 当たり判定後にスクロール量を反映させるため、親クラスの共通関数を使用してプレイヤーのスクリーン座標を算出する   
>>>>>>> AyameTest
	const int playerScreenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));

	blockMap.Move(
		playerScreenX,
		mfMoveDirection
	);

	// 特殊地形の処理   
	mBlockAction.SetCollisionType(collisionType);
	mBlockAction.ExecuteDeath(mHp);
	mBlockAction.ExecuteGoal();



	// ジャンプ開始   
	if (CheckHitKey(KEY_INPUT_SPACE) == 1 &&
		isGrounded)
	{
		mbIsJumping = true;
		isGrounded = false;
		velocityY = jumpPower;
	}


<<<<<<< HEAD
		// Blink (Eキー)
	static bool eKeyWasDown = false;
	bool eKeyIsDown = (CheckHitKey(KEY_INPUT_E) == 1);
	if (eKeyIsDown && !eKeyWasDown && !isBlinking && blinkCooldownTimer <= 0 && mfMoveDirection != 0.0f)
	{
		isBlinking = true;
		blinkTimer = blinkDuration;
		blinkDirection = isFacingRight ? 1.0f : -1.0f;
	}
	eKeyWasDown = eKeyIsDown;

	// U F
=======
	// 攻撃 F   
>>>>>>> AyameTest
	if (CheckHitKey(KEY_INPUT_F) == 1 &&
		!isAttacking)
	{
		isAttacking = true;
		attackTimer = attackDuration;
<<<<<<< HEAD
		mHitEnemies.clear();
=======
		mPlayerState.SetAttack();
>>>>>>> AyameTest
	}


	// 攻撃エフェクト更新   
	if (isAttacking)
	{
		attackTimer--;

		if (attackTimer <= 0)
		{
			attackTimer = 0;
		}
	}


	// アニメーション状態を更新   
	if (!isAttacking)
	{
		if (mbIsJumping)
		{
			mPlayerState.SetJump();
		}
		else if (mfMoveDirection != 0.0f)
		{
			mPlayerState.SetWalk();
		}
		else
		{
			mPlayerState.SetIdle();
		}
	}

	mPlayerState.Update();

	if (isAttacking && mPlayerState.IsFinished())
	{
		isAttacking = false;
		attackTimer = 0;

		if (mbIsJumping)
		{
			mPlayerState.SetJump();
		}
		else if (mfMoveDirection != 0.0f)
		{
			mPlayerState.SetWalk();
		}
		else
		{
			mPlayerState.SetIdle();
		}
	}
}

void Player::Update()
{
	if (mbFallDeath)
	{
		mFallDeath.Update(mvPosition, mlives);
		// タイマーが0かつエンターが押されていたらの判定の可否をとる   
		if (mFallDeath.IsReviveFinished()) { Revive(); }
	}

	if (isDead)
	{
		DeadProcess();
		Object2D::Update();
		return;
	}

	// --- 当たり判定処理 ---   
	isHitDamage = false;
	if (mInvincibleTimer > 0) mInvincibleTimer--;
	if (blinkCooldownTimer > 0) blinkCooldownTimer--;

	// ブリンクと残像の更新
	for (auto it = mAfterimages.begin(); it != mAfterimages.end(); )
	{
		it->alpha -= 15.0f;
		if (it->alpha <= 0.0f) {
			it = mAfterimages.erase(it);
		} else {
			++it;
		}
	}

	if (isBlinking)
	{
		blinkTimer--;
		if (blinkTimer <= 0)
		{
			isBlinking = false;
			blinkCooldownTimer = 60; // 60フレームのクールタイム
		}
		else if (blinkTimer % 2 == 0) // 2フレームに1回残像を生成
		{
			BlinkAfterimage img;
			img.pos = mvPosition;
			img.facingRight = isFacingRight;
			img.alpha = 150.0f; // 半透明からスタート
			mAfterimages.push_back(img);
		}
	}

	ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
	std::vector<Object2D*> enemyList = objManager->GetObject2DListByTag(Object2D::Enemy2D);

	// プレイヤーの当たり判定   
	VECTOR myPos = VGet(
		mvPosition.x - mfPlayerWidth / 2.0f,
		mvPosition.y - mfPlayerHeight / 2.0f,
		0.0f
	);
	VECTOR mySize = VGet(
		mfPlayerWidth,
		mfPlayerHeight,
		0.0f
	);


	VECTOR atkPos = VGet(0, 0, 0);
	VECTOR atkSize = VGet(0, 0, 0);
	bool hasAttackRect = false;

	if (isAttacking)
	{
		hasAttackRect = true;
		int attackWidth = PlayerConstants::PlayerAttackWidth;
		int attackHeight = PlayerConstants::PlayerAttackHeight;
		int halfSizeX = PlayerAnimState::FRAME_WIDTH / 2;
		float atkLeft;

		if (isFacingRight)
		{
			atkLeft = mvPosition.x + halfSizeX;
		}
		else
		{
			atkLeft = mvPosition.x - halfSizeX - attackWidth;
		}
		float atkTop = mvPosition.y - attackHeight / 2.0f;

		atkPos = VGet(atkLeft, atkTop, 0.0f);
		atkSize = VGet((float)attackWidth, (float)attackHeight, 0.0f);
	}

	for (Object2D* obj : enemyList)
	{
		Enemy* enemy = dynamic_cast<Enemy*>(obj);
		if (!enemy) continue;

		// 敵の当たり判定   
		float enemyWidth = enemy->GetSizeX() / 6.0f;
		float enemyHeight = enemy->GetSizeY() / 6.0f;
		int eneLeft = static_cast<int>(
			enemy->GetPosition().x - enemyWidth / 2.0f
			);
		int eneTop = static_cast<int>(
			enemy->GetPosition().y - enemyHeight / 2.0f
			);

		VECTOR enePos = VGet(
			(float)eneLeft,
			(float)eneTop,
			0.0f
		);
		VECTOR eneSize = VGet(
			enemyWidth,
			enemyHeight,
			0.0f
		);

<<<<<<< HEAD
				// 1. プレイヤー自身と敵の衝突判定 (ダメージで赤くする)
=======
		// 1. プレイヤー自身と敵の衝突判定 (ダメージで赤くする)   
>>>>>>> AyameTest
		if (Collision::CheckRectToRect(myPos, mySize, enePos, eneSize))
		{
			if (isBlinking)
			{
				if (!mIsBlinkWallDeathImmune)
				{
					// ブリンク中かつ未強化なら即死
					TakeDamage(mHp);
				}
				// 強化中なら何もしない（すり抜け）
			}
			else
			{
				isHitDamage = true;
				if (mInvincibleTimer <= 0 && mDeadState == 0)
				{
					mInvincibleTimer = 60; // 1秒無敵
					UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
					if (enemyStatus)
					{
						int dmg = enemyStatus->mAttack;
						if (enemyStatus->mHasInstantKillAttack) dmg = mHp;
						TakeDamage(dmg);
					}
				}
			}
		}

<<<<<<< HEAD
						// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
=======
		// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)   
>>>>>>> AyameTest
		if (hasAttackRect)
		{
			if (Collision::CheckRectToRect(atkPos, atkSize, enePos, eneSize))
			{
				bool alreadyHit = false;
				for (auto* hitEnemy : mHitEnemies)
				{
					if (hitEnemy == enemy)
					{
						alreadyHit = true;
						break;
					}
				}
				if (!alreadyHit)
				{
					enemy->OnDamaged();
					mHitEnemies.push_back(enemy);
					UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
					if (enemyStatus)
					{
						enemyStatus->TakeDamage(mAttack);
						if (enemyStatus->mHp <= 0)
						{
							enemy->SetDeleteFlag(true);
						}
					}
				}
			}
		}
	}
	// デバッグ用: Kキーで5ダメージ   
	static bool kKeyWasDown = false;
	bool kKeyIsDown = (CheckHitKey(KEY_INPUT_K) == 1);
	if (kKeyIsDown && !kKeyWasDown && !isDead)
	{
		TakeDamage(5);
	}

	kKeyWasDown = kKeyIsDown;

	if (mHp <= 0 && !isDead)
	{
		isDead = true;
		mlives -= 1;
		mDeadState = 1; // SCATTER   
		deadTimer = 0;
		mFragments.clear();

		int fragSize = 16;
		int srcBaseY = PlayerAnimState::IDLE_POS;

		if (isAttacking)
		{
			srcBaseY = PlayerAnimState::ATTACK_POS;
		}
		else if (mbIsJumping)
		{
			srcBaseY = PlayerAnimState::JUMP_POS;
		}
		else if (mfMoveDirection != 0.0f)
		{
			srcBaseY = PlayerAnimState::WALK_POS;
		}

		for (int y = 0; y < PlayerAnimState::FRAME_HEIGHT; y += fragSize)
		{
			for (int x = 0; x < PlayerAnimState::FRAME_WIDTH; x += fragSize)
			{
				PlayerFragment frag;
				frag.pos.x = mvPosition.x - (PlayerAnimState::FRAME_WIDTH / 2.0f) + (float)x;
				frag.pos.y = mvPosition.y - (PlayerAnimState::FRAME_HEIGHT / 2.0f) + (float)y;
				frag.srcX = x;
				frag.srcY = srcBaseY + y;
				frag.width = fragSize;
				frag.height = fragSize;

				frag.vel.x = ((float)GetRand(100) / 100.0f * 10.0f) - 5.0f;
				frag.vel.y = ((float)GetRand(100) / 100.0f * -15.0f) - 5.0f;

				mFragments.push_back(frag);
			}
		}
	}

	Object2D::Update();
}

// GameScereで呼び出し   
void Player::DrawFallDeath()
{
	mFallDeath.Draw(mlives);
}

void Player::Draw()
{
	if (isDead)
	{
		SetDrawBright(255, 255, 255); // 色をリセット   
		for (const auto& frag : mFragments)
		{
			if (mpTexture != nullptr)
			{
				// 死亡時の破片描画でも親クラスの共通関数を使用してスクロールを正確に反映させる   
				int playerLeft = static_cast<int>(ConvertToScreenX(frag.pos.x, mpBlockMap));

				DrawRectGraph(
					playerLeft,
					static_cast<int>(frag.pos.y),
					frag.srcX,
					frag.srcY,
					frag.width,
					frag.height,
					mpTexture->GetHandle(),
					true
				);
			}
			else
			{
				int playerLeftX = static_cast<int>(ConvertToScreenX(frag.pos.x, mpBlockMap));
				int playerRigthtX = static_cast<int>(ConvertToScreenX(frag.pos.x + frag.width, mpBlockMap));

				// テクスチャが無い場合の保険   
				DrawBox(
					playerLeftX,
					static_cast<int>(frag.pos.y),
					playerRigthtX,
					static_cast<int>(frag.pos.y + frag.height),
					GetColor(255, 0, 0),
					TRUE
				);
			}
		}
		return;
	}

	if (mpTexture != nullptr)
	{
<<<<<<< HEAD
		// 残像の描画
		for (const auto& img : mAfterimages)
		{
			int ax = static_cast<int>(ConvertToScreenX(img.pos.x, mpBlockMap) - FRAME_WIDTH / 2);
			int ay = static_cast<int>(img.pos.y - FRAME_HEIGHT / 2);
			int aSrcX = 128;
			int aSrcY = img.facingRight ? 256 : 384;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)img.alpha);
			DrawRectGraph(ax, ay, aSrcX, aSrcY, FRAME_WIDTH, FRAME_HEIGHT, mpTexture->GetHandle(), true);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		int srcX = mCurrentFrame * FRAME_WIDTH;
		int srcY = 256;
		if (!isFacingRight)
		{
			srcY = 384;
		}

		if (isBlinking)
		{
			srcX = 128;
=======
		const int srcX = mPlayerState.GetCurrentFrame() * PlayerAnimState::FRAME_WIDTH;

		// 現在の状態に応じた画像の切り出し位置を設定する   
		int srcY = PlayerAnimState::IDLE_POS;

		PlayerAnimState::State animState = mPlayerState.GetState();

		if (animState == PlayerAnimState::State::Attack)
		{
			srcY = PlayerAnimState::ATTACK_POS;
		}
		else if (animState == PlayerAnimState::State::Jump)
		{
			srcY = PlayerAnimState::JUMP_POS;
		}
		else if (animState == PlayerAnimState::State::Walk)
		{
			srcY = PlayerAnimState::WALK_POS;
>>>>>>> AyameTest
		}

		// ダメージ中なら赤く変色させる   
		if (isHitDamage)
		{
			SetDrawBright(255, 100, 100);
		}

		// 指定の場所だけ描画する   
		// 親クラスの共通関数を使用してワールド座標からスクリーン座標へ変換する   
		const int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap) - PlayerAnimState::FRAME_WIDTH / 2);
		const int screenY = static_cast<int>(mvPosition.y - PlayerAnimState::FRAME_HEIGHT / 2);

		// プレイヤーを描画    
		//    
		// 右向き：通常描画    
		// 左向き：左右反転して描画    
		if (isFacingRight)
		{
			DrawRectGraph(
				screenX,
				screenY,
				srcX,
				srcY,
				PlayerAnimState::FRAME_WIDTH,
				PlayerAnimState::FRAME_HEIGHT,
				mpTexture->GetHandle(),
				true,
				false
			);
		}
		else
		{
			DrawRectGraph(
				screenX,
				screenY,
				srcX,
				srcY,
				PlayerAnimState::FRAME_WIDTH,
				PlayerAnimState::FRAME_HEIGHT,
				mpTexture->GetHandle(),
				true,
				true
			);
		}

		// 色を元に戻す   
		if (isHitDamage)
		{
			SetDrawBright(255, 255, 255);
		}
	}
	else
	{
		Object2D::Draw();
	}

	// 攻撃エフェクトの描画   
	if (isAttacking)
	{
		int attackWidth = 60;
		int attackHeight = 40;
		int rectLeft, rectTop, rectRight, rectBottom;
		int halfSizeX = PlayerAnimState::FRAME_WIDTH / 2;

		if (isFacingRight)
		{
			rectLeft = static_cast<int>(ConvertToScreenX(mvPosition.x + halfSizeX, mpBlockMap));
			rectRight = rectLeft + attackWidth;
		}
		else
		{
			rectLeft = static_cast<int>(ConvertToScreenX(mvPosition.x - halfSizeX - attackWidth, mpBlockMap));
			rectRight = rectLeft + attackWidth;
		}

		rectTop = static_cast<int>(mvPosition.y) - attackHeight / 2;
		rectBottom = rectTop + attackHeight;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
		DrawBox(rectLeft, rectTop, rectRight, rectBottom, GetColor(255, 50, 50), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// UI（ハート）の描画（右上）   
	if (mMaxHp > 0)
	{
		int hpLevel = (mHp * 6) / mMaxHp;
		if (mHp > 0 && hpLevel == 0) hpLevel = 1;

		int drawX = ScreenSize::ScrrenWidth - 150; // 右上に配置   
		int drawY = 20;
		for (int i = 0; i < 3; i++)
		{
			int heartState = hpLevel - (i * 2);
			int graph = mHeartEmptyGraph;
			if (heartState >= 2) graph = mHeartFullGraph;
			else if (heartState == 1) graph = mHeartHalfGraph;

			if (graph != -1)
			{
				DrawGraph(drawX + i * 40, drawY, graph, TRUE);
			}
		}
	}
}

// デバッグ表示をしている関数   
// GameSceneで呼び出している   
void Player::DebugDraw()
{
	// プレイヤーの当たり判定デバッグ表示（共通関数を使ってスクロール位置を正しく反映）   
	const int playerLeft = static_cast<int>(ConvertToScreenX(mvPosition.x - mfPlayerWidth / 2.0f, mpBlockMap));
	const int playerTop = static_cast<int>(mvPosition.y - mfPlayerHeight / 2.0f);
	const int playerRight = static_cast<int>(ConvertToScreenX(mvPosition.x + mfPlayerWidth / 2.0f, mpBlockMap));
	const int playerBottom = static_cast<int>(mvPosition.y + mfPlayerHeight / 2.0f);
	DrawBox(
		playerLeft,
		playerTop,
		playerRight,
		playerBottom,
		GetColor(0, 0, 255),
		FALSE
	);


	// HPのデバッグ   
	DrawFormatString(playerLeft, (int)mvPosition.y + 10, ColorOption::White, "HP: %d\n残機: %d", mHp, mlives);
	DrawFormatString(0, 600, ColorOption::White, "X:%2f, Y:%2f\n L:%d", mvPosition.x, mvPosition.y, playerLeft);

	// シーン上のすべての敵の当たり判定をデバッグ表示（黄緑色）   
	ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();

	if (objManager != nullptr)
	{
		std::vector<Object2D*> enemyList =
			objManager->GetObject2DListByTag(Object2D::Enemy2D);

		for (Object2D* obj : enemyList)
		{
			Enemy* enemy = dynamic_cast<Enemy*>(obj);
			if (!enemy) continue;

			// 実際の当たり判定と同じサイズ   
			float enemyWidth = enemy->GetSizeX() / 6.0f;
			float enemyHeight = enemy->GetSizeY() / 6.0f;

			float divide = 2.0f;

			// 敵のデバッグ描画でも共通関数を利用してスクロールを正確に合わせる   
			int eneLeft = static_cast<int>(ConvertToScreenX(enemy->GetPosition().x - enemyWidth / divide, mpBlockMap));
			int eneTop = static_cast<int>(enemy->GetPosition().y - enemyHeight / divide);
			int eneRight = static_cast<int>(ConvertToScreenX(enemy->GetPosition().x + enemyWidth / divide, mpBlockMap));
			int eneBottom = static_cast<int>(enemy->GetPosition().y + enemyHeight / divide);

			DrawBox(
				eneLeft,
				eneTop,
				eneRight,
				eneBottom,
				GetColor(0, 255, 0),
				FALSE
			);
		}
	}
}


void Player::DeadProcess()
{
	if (mbFallDeath) return;

	deadTimer++;

	if (mDeadState == 1) // 飛び散り   
	{
		for (auto& frag : mFragments)
		{
			frag.pos.x += frag.vel.x;

			if (mpBlockMap &&
				mCharacterPhysics.IsBlockCollision(
					*mpBlockMap,
					frag.pos.x,
					frag.pos.y,
					static_cast<float>(frag.width),
					static_cast<float>(frag.height)))
			{
				frag.pos.x -= frag.vel.x;
				frag.vel.x *= -0.6f;
			}

			frag.pos.y += frag.vel.y;

			if (mpBlockMap &&
				mCharacterPhysics.IsBlockCollision(
					*mpBlockMap,
					frag.pos.x,
					frag.pos.y,
					static_cast<float>(frag.width),
					static_cast<float>(frag.height)))
			{
				frag.pos.y -= frag.vel.y;
				frag.vel.y *= -0.4f;
				frag.vel.x *= 0.9f; // 摩擦   
			}
		}

		if (deadTimer > 150) // 2.5秒経過   
		{
			mDeadState = 2; // 戻り   
			deadTimer = 0;
			for (auto& frag : mFragments)
			{
				frag.vel.y = -5.0f - ((float)GetRand(50) / 10.0f);
				frag.vel.x = ((float)GetRand(100) / 100.0f * 4.0f) - 2.0f;
			}
		}
	}
	else if (mDeadState == 2) // 戻り   
	{
		bool allReturned = true;

		for (auto& frag : mFragments)
		{
			float targetX = mSpawnPos.x - (PlayerAnimState::FRAME_WIDTH / 2.0f) + frag.srcX;
			float targetY = mSpawnPos.y - (PlayerAnimState::FRAME_HEIGHT / 2.0f) + (frag.srcY % PlayerAnimState::FRAME_HEIGHT);

			float dx = targetX - frag.pos.x;
			float dy = targetY - frag.pos.y;
			float dist = sqrtf(dx * dx + dy * dy);

			if (dist > 2.0f)
			{
				frag.pos.x += dx * 0.05f;
				frag.pos.y += dy * 0.05f;
				allReturned = false;
			}
			else
			{
				frag.pos.x = targetX;
				frag.pos.y = targetY;
			}
		}

		// 全て集まったか、タイムアウト（5秒）で強制復活   
		if ((allReturned && deadTimer > 60) || deadTimer > 300)
		{
			mDeadState = 0;
			deadTimer = 0;
			isFacingRight = true;
			mFragments.clear();

			Revive();
		}
	}
}


// 復活した時に位置とHPを戻す処理   
void Player::Revive()
{
	mvPosition = mSpawnPos;
	mvPosition = mSpawnPos;
	mHp = mMaxHp; // 復活時にHPをリセット   

	// マップのスクロール位置を一番左に戻す   
	if (mpBlockMap != nullptr)
	{
		mpBlockMap->ResetScroll();
		mFallDeath.Reset();
	}

	isDead = false;
	mbFallDeath = false;
}