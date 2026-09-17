#include "BlockMap.h"
#include "Master.h"
#include "GameConstants.h"
#include <iostream>


BlockMap::BlockMap()
    : mnBackgroundGraph(-1)
    , mnCollisionSoftImage(-1)
    , mnCollisionWidth(0)
    , mnCollisionHeight(0)
    , mCollisionData()
    , mbIsLoaded(false)
{
}

BlockMap::~BlockMap()
{
}


// マップ読み込み
bool BlockMap::Load(
    const std::string& backgroundPath,
    const std::string& collisionPath)
{
    if (mbIsLoaded) { return true; }

    // 背景画像の読み込み
    mnBackgroundGraph =
        Master::mpGameManager
        ->GetResourceManager()
        ->LoadGraphics(backgroundPath);

    if (mnBackgroundGraph == -1)
    {
        std::cout<< "ブロックの背景画像が開けませんでした。" << std::endl;
        return false;
    }

    // 当たり判定画像を読み込む
    mnCollisionSoftImage =
        LoadSoftImage(
            collisionPath.c_str()
        );

    if (mnCollisionSoftImage == -1)
    {
        std::cout<< "ブロックの当たり判定画像が開けませんでした。" << std::endl;
        return false;
    }


    // 当たり判定画像のサイズを取得
    GetSoftImageSize(
        mnCollisionSoftImage,
        &mnCollisionWidth,
        &mnCollisionHeight
    );

    // CollisionTypeデータを確保 全てNONEにしている
    mCollisionData.resize(
        mnCollisionWidth * mnCollisionHeight,
        CollisionType::None
    );

    // 当たり判定画像を1ピクセルずつ調べる
    for (int y = 0; y < mnCollisionHeight; y++)
    {
        for (int x = 0; x < mnCollisionWidth; x++)
        {
            // RGBAを受け取る変数
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;

            // 現在のピクセルの色を取得
            int result =
                GetPixelSoftImage(
                    mnCollisionSoftImage,
                    x,
                    y,
                    &r,
                    &g,
                    &b,
                    &a
                );
            if (result != 0) { continue; }

            // 透明ならNone
            if (a == 0)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::None;
                continue;
            }


            // 赤ならBlock
            if (r == BlockCollisionColor::BLOCK_R &&
                g == BlockCollisionColor::BLOCK_G &&
                b == BlockCollisionColor::BLOCK_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Block;

                continue;
            }

            // 青ならDeath
            if (r == BlockCollisionColor::DEATH_R &&
                g == BlockCollisionColor::DEATH_G &&
                b == BlockCollisionColor::DEATH_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Death;
                continue;
            }


            // 緑ならGoal
            if (r == BlockCollisionColor::GOAL_R &&
                g == BlockCollisionColor::GOAL_G &&
                b == BlockCollisionColor::GOAL_B)
            {
                mCollisionData[
                    GetCollisionIndex(x, y)
                ] = CollisionType::Goal;
                continue;
            }

            // それ以外はNone
            mCollisionData[
                GetCollisionIndex(x, y)
            ] = CollisionType::None;
        }
    }

    // 元の画像データはもう必要ない
    DeleteSoftImage(
        mnCollisionSoftImage
    );

    mnCollisionSoftImage = -1;
    mbIsLoaded = true;
    return true;
}


// 背景描画
void BlockMap::Draw()
{
    if (!mbIsLoaded) { return; }

    // 背景画像を描画 今は00だが将来的には変更したい
    // 今のメイン画像の前後だけ描画など (画像は4毎ぐらい用意する予定)
    DrawGraph(
        0,
        0,
        mnBackgroundGraph,
        TRUE
    );
}


// CollisionType取得
BlockMap::CollisionType BlockMap::GetCollisionType(
    int x,
    int y) const
{
    // マップ外
    if (x < 0 || x >= mnCollisionWidth ||
        y < 0 || y >= mnCollisionHeight)
    {
        // マップ外は何もない扱い
        return CollisionType::None;
    }

<<<<<<< HEAD
    // 指定座標のCollisionTypeを返す
    return mCollisionData[ GetCollisionIndex(x, y)];
=======
	// プレイヤーが重なっているブロック範囲を計算
	// ワールド座標をマップの配列インデックスに変換するため
	int left = static_cast<int>(x) / CHIP_SIZE;
	int right = static_cast<int>(x + width - 1) / CHIP_SIZE;
	int top = static_cast<int>(y) / CHIP_SIZE;
	int bottom = static_cast<int>(y + height - 1) / CHIP_SIZE;

	// マップ範囲を調整
	if (left < 0) {left = 0;}
	if (right >= MAX_MAP_WIDTH) {right = MAX_MAP_WIDTH - 1;}
	if (top < 0) {top = 0;}
	if (bottom >= MAX_MAP_HEIGHT) {bottom = MAX_MAP_HEIGHT - 1;}


	// 周囲のブロックを1個ずつ調べる
	// マップ全体を毎フレーム調べると重いので、プレイヤーが実際に重なっている可能性のある近傍のチップだけを効率よく探すため
	for (int mapY = top; mapY <= bottom; mapY++)
	{
		for (int mapX = left; mapX <= right; mapX++)
		{
			int chipID = mnMapData[mapY][mapX];

			if (!IsSolidChip(chipID)) { continue; }

			// ブロックの矩形
			VECTOR blockPos = VGet(
				static_cast<float>(mapX * CHIP_SIZE),
				static_cast<float>(mapY * CHIP_SIZE),
				0.0f
			);

			VECTOR blockSize = VGet(
				static_cast<float>(CHIP_SIZE),
				static_cast<float>(CHIP_SIZE),
				0.0f
			);

			// プレイヤーとブロックの当たり判定
			if (Collision::CheckRectToRect(
				playerPos,
				playerSize,
				blockPos,
				blockSize))
			{
				if (chipID >= BlockCollision::MinDamageBlock &&
					chipID <= BlockCollision::MaxDamageBlock)
				{
					DrawFormatString(600, 600, GetColor(255, 255, 255), "針にあたった");

					auto getPlayer = Master::mpGameManager->GetSceneManager()->GetCurrentScene()
						->GetObjectManager()->GetObject2DByTag(Object2D::Player2D);

					Player* player = dynamic_cast<Player*>(getPlayer);
					if(player) { player->TakeDamage(SetDamage::SpikeBlock); }
				}

				// 格納先が指定されている場合のみブロック座標を代入する
				if (blockX != nullptr)
				{
					*blockX = mapX;
				}
				if (blockY != nullptr)
				{
					*blockY = mapY;
				}
				return true;
			}
		}
	}
	return false;
>>>>>>> Yamada
}