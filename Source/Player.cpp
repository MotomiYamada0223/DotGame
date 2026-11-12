#include "Player.h"
#include "NeedleTrap.h"
#include "CrumbleFloor.h"
#include "DxLib.h"
#include "Texture.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameScene.h"
#include "ObjectManager.h"
#include "Enemy.h"
#include "Collision.h"
#include "GameConstants.h"
#include "BlockAction.h"
#include "InputManager.h"
#include "PlayerActionState.h"


namespace
{
	const int kScatterFrames = 150;      // 飛び散りの時間（この後GAME OVER）
	const int kGameOverMenuFrames = 25;  // GAME OVER後、この時間から入力を受け付ける
}

Player::Player(VECTOR initPos)
// TextureAnimationは使わず一枚絵としてロード
	: Object2D(CharacterGraphPath::PlayerAnimation, initPos)
	, mDeathReason(DeathReason::DeathType::None)
{
	SetTag(Object2D::Player2D);
	mbIsJumping = false;
	isGrounded = true;
	mForceGroundedThisFrame = false;
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
	alpha = 255.0f;
	mSpawnPos = initPos;
	mpBlockMap = nullptr;
	mfMoveDirection = 0.0f;
	mFallMode = StageFallMode::None;
	mWKeyHoldTimer = 0;
	mSKeyHoldTimer = 0;


	UpdateStatusByProgress(GameProgress::Tutorial1);
	mDeath.Reset();

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
	mGameProgress = progress;
	switch (progress)
	{
	case GameProgress::Tutorial1:
		mMaxHp = 15; mHp = 15;
		mAttack = 5000; mDefense = 5; mMagic = 5;
		mIsBlinkWallDeathImmune = false;
		break;
	case GameProgress::PostTutorial1:
	case GameProgress::Tutorial2:
		mMaxHp = 30000; mHp = 30000;
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
	mlives = Master::gPlayerLive;
}


void Player::PlayerMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap;
	// 死亡時は操作を受け付けない
	if (isDead)
	{
		return;
	}


	// Wキー/Sキー長押しによるステージ裏・表落下処理の判定
	if (mFallMode == StageFallMode::None && !isDead && !mbAutoWalking)
	{
		if (mvPosition.x <= 8000)
		{
			// Wキー: ステージ裏側（マップ奥）へ落下
			if (CheckHitKey(KEY_INPUT_W) == 1)
			{
				mWKeyHoldTimer++;
				if (mWKeyHoldTimer >= PlayerConstants::FallThroughHoldFrames)
				{
					mFallMode = StageFallMode::Back;
					isGrounded = false;
					mbIsJumping = true;
					if (velocityY < 2.0f) velocityY = 2.0f;
					mPlayerState.SetJump();
				}
			}
			else
			{
				mWKeyHoldTimer = 0;
			}

			// Sキー: ステージ表側（マップ手前）へ落下
			if (CheckHitKey(KEY_INPUT_S) == 1)
			{
				mSKeyHoldTimer++;
				if (mSKeyHoldTimer >= PlayerConstants::FallThroughHoldFrames)
				{
					mFallMode = StageFallMode::Front;
					isGrounded = false;
					mbIsJumping = true;
					if (velocityY < 2.0f) velocityY = 2.0f;
					mPlayerState.SetJump();
				}
			}
			else
			{
				mSKeyHoldTimer = 0;
			}
		}
	}

	// ステージ裏・表落下処理中（マップ当たり判定を無視して落下）
	if (mFallMode != StageFallMode::None)
	{
		velocityY += gravity;
		if (velocityY > 25.0f) velocityY = 25.0f;
		mvPosition.y += velocityY;
		isGrounded = false;
		mbIsJumping = true;
		mPlayerState.SetJump();

		if (!mbDeath && mvPosition.y >= PlayerConstants::PlayerDeathHeight)
		{
			// WSで落下したときにテキストを変更した
			mDeathReason = DeathReason::DeathType::FallSide;
			mHp = 0;
			mbDeath = true;
		}

		const int playerScreenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
		const int playerMapY = static_cast<int>(mvPosition.y);
		blockMap.Move(
			playerScreenX,
			playerMapY,
			0.0f,
			0.0f
		);

		return;
	}


	if (!mbDeath && mvPosition.y >= PlayerConstants::PlayerDeathHeight)
	{
		// 死因を設定してからHPを0にする
		mDeathReason = DeathReason::DeathType::Fall;
		mHp = 0;
		mbDeath = true;
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
	float currentSpeed = moveSpeed;

	if (mbIsEvading)
	{
		// 回避中は背中側（左: -1.0f）へ高速移動しつつ、向きは右向きを維持
		mfMoveDirection = -1.0f;
		currentSpeed = KingSlimeIntroConstants::EvasionSpeedX;
		isFacingRight = true;
	}
	else if (mbAutoWalking)
	{
		currentSpeed = BossIntroConstants::AutoWalkSpeed;
		float diffX = mAutoWalkTargetX - mvPosition.x;
		if (std::abs(diffX) <= currentSpeed)
		{
			mvPosition.x = mAutoWalkTargetX;
			mfMoveDirection = 0.0f;
			mbAutoWalking = false;
			mbAutoWalkArrived = true;
		}
		else
		{
			if (diffX > 0.0f)
			{
				mfMoveDirection = 1.0f;
				isFacingRight = true;
			}
			else
			{
				mfMoveDirection = -1.0f;
				isFacingRight = false;
			}
		}
	}
	else if (!mbAutoWalkArrived)
	{
		if (CheckHitKey(KEY_INPUT_A) == 1)
		{
			mActionState.move = true;
			mfMoveDirection = -1.0f;
			isFacingRight = false;
		}
		else if (CheckHitKey(KEY_INPUT_D) == 1)
		{
			mActionState.move = true;
			mfMoveDirection = 1.0f;
			isFacingRight = true;
		}
	}
	else
	{
		mfMoveDirection = 0.0f;
	}

	// キャラクターの物理処理
	float currentGravity = gravity;
	if (isBlinking)
	{
		currentSpeed = blinkSpeed;
		currentGravity = 0.0f;
		velocityY = 0.0f; // 落下を止める
		mfMoveDirection = blinkDirection; // ブリンク開始時の方向に固定
	}

	mfCurrentSpeed = currentSpeed; // 今の速さの代入
	float prevX = mvPosition.x;

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

	// ボスエリア接触判定の更新
	mbTouchingBossArea = (collisionType == BlockMap::CollisionType::BossArea);

	// 動く床などによって強制接地フラグが立っている場合は、地形の判定を上書きする
	if (mForceGroundedThisFrame) {
		isGrounded = true;
		mbIsJumping = false;
		velocityY = 0.0f;
		mForceGroundedThisFrame = false; // 消費
	}

	if (isBlinking && !mIsBlinkWallDeathImmune)
	{
		// X軸方向の移動がほとんどできなかった場合、壁に激突したとみなして即死
		if (abs(mvPosition.x - prevX) < 1.0f)
		{
			// 壁激突で即死とする
			mDeathReason = DeathReason::DeathType::WallHit;
			TakeDamage(mHp);
		}
	}

	// 当たり判定後にスクロール量を反映させるため、親クラスの共通関数を使用してプレイヤーのスクリーン座標を算出する
	// 横方向は画面座標（境界判定のため）、縦方向はマップ座標（カメラ目標位置の計算のため）
	const int playerScreenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
	const int playerMapY = static_cast<int>(mvPosition.y);

	blockMap.Move(
		playerScreenX,
		playerMapY,
		mfMoveDirection,
		mfCurrentSpeed
	);


	// 特殊地形の処理
	mBlockAction.SetCollisionType(collisionType);
	mBlockAction.ExecuteDeath(mHp);
	if (mBlockAction.GetHitDeathBlock())
	{
		// 死亡判定のアルブロックに重なったら
		mDeathReason = DeathReason::DeathType::DeathBlock;
	}
	mBlockAction.ExecuteGoal();



	// Blink (shiftキー)
	static bool eKeyWasDown = false;
	bool eKeyIsDown = ((CheckHitKey(KEY_INPUT_LSHIFT) == 1) || (CheckHitKey(KEY_INPUT_RSHIFT) == 1));
	if (!mbAutoWalking && !mbAutoWalkArrived && eKeyIsDown && !eKeyWasDown && !isBlinking && blinkCooldownTimer <= 0 && mfMoveDirection != 0.0f)
	{
		mActionState.blink = true;
		isBlinking = true;
		blinkTimer = blinkDuration;
		blinkDirection = isFacingRight ? 1.0f : -1.0f;
	}
	eKeyWasDown = eKeyIsDown;


	// ジャンプ開始
	if (mbInHotSpring)
	{
		mActionState.hotSpring = true;
		// 温泉のぬめり処理：
		// 1回のジャンプではぬめりで上がれず、重い湯に引きずり込まれる
		// スペースキーを連打（足掻き）すると少しずつ浮上・脱出可能
		bool spaceDown = (CheckHitKey(KEY_INPUT_SPACE) == 1);
		bool spacePressed = spaceDown && !mbSpaceWasPressed;
		mbSpaceWasPressed = spaceDown;

		if (spacePressed)
		{
			mbIsJumping = true;
			isGrounded = false;
			// ぬめりでもがく小キック
			velocityY = -4.6f;
		}

		// ぬめりによる強い抵抗と引きずり込み
		if (velocityY < 0.0f)
		{
			velocityY += 0.58f; // 上昇への強い抵抗
		}
		else
		{
			velocityY += 0.38f; // 底への引きずり込み
		}
	}
	else
	{
		// ジャンプ開始（着地後の再入力が必要なようにトリガー判定化）
		static bool spaceKeyWasDown = false;
		bool spaceKeyIsDown = (CheckHitKey(KEY_INPUT_SPACE) == 1);
		if (!mbAutoWalking && !mbAutoWalkArrived && spaceKeyIsDown && !spaceKeyWasDown &&
			isGrounded)
		{
			mActionState.jump = true;
			mbIsJumping = true;
			isGrounded = false;
			velocityY = jumpPower;
		}
		spaceKeyWasDown = spaceKeyIsDown;
	}




	// 攻撃 F
	if (!mbAutoWalking && !mbAutoWalkArrived && CheckHitKey(KEY_INPUT_F) == 1 &&
		!isAttacking)
	{
		mActionState.attack = true;
		isAttacking = true;
		attackTimer = attackDuration;
		mPlayerState.SetAttack();
		mHitEnemies.clear();
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
		if (mbIsEvading)
		{
			mPlayerState.SetJump();
		}
		else if (mbSwordReadyStance)
		{
			mPlayerState.SetSwordReady();
		}
		else if (mbIsJumping)
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

		if (mbIsEvading)
		{
			mPlayerState.SetJump();
		}
		else if (mbSwordReadyStance)
		{
			mPlayerState.SetSwordReady();
		}
		else if (mbIsJumping)
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
	if (isDead)
	{
		// フェーズ1：天使カットイン（時止め）
		if (mDeathPhase == 1)
		{
			GameScene* gs = nullptr;
			if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
			{
				gs = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
			}

			DeathController::FrameContext ctx;
			ctx.text = gs ? &gs->GetTutorialText() : nullptr;
			mDeath.Update(ctx);

			// 天使テキストと白フラッシュが終わったら飛び散りへ
			if (mDeath.IsFinished())
			{
				StartDeathScatter();
			}
			return;
		}

		// フェーズ2：飛び散り
		if (mDeathPhase == 2)
		{
			DeadProcess();
			Object2D::Update();

			if (deadTimer >= kScatterFrames)
			{
				mDeathPhase = 3;
				mGameOverFadeTimer = 0;
				mGameOverMenuIndex = 0;
			}
			return;
		}

		// フェーズ3：GAME OVER
		if (mDeathPhase == 3)
		{
			mGameOverFadeTimer++;
			UpdateGameOver();
		}
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
		}
		else {
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

	// 演出用回避中の残像生成と着地判定
	if (mbIsEvading)
	{
		mEvasionTimer--;
		if (mEvasionTimer % 2 == 0)
		{
			BlinkAfterimage img;
			img.pos = mvPosition;
		 img.facingRight = isFacingRight;
			img.alpha = 150.0f;
			mAfterimages.push_back(img);
		}

		if (mEvasionTimer <= 0 && isGrounded)
		{
			mbIsEvading = false;
			mfMoveDirection = 0.0f;
			mPlayerState.SetSwordReady();
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
		int halfSizeX = PlayerConstants::PlayerAttackDist;
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

	// 壁を壊す処理
	if (hasAttackRect && mpBlockMap != nullptr)
	{
		bool isWallBerak = mpBlockMap->BreakWallAtArea(
			atkPos.x,
			atkPos.y,
			atkPos.x + atkSize.x,
			atkPos.y + atkSize.y
			);

		if (isWallBerak)
		{
			mActionState.wallbreak = true;
		}
	}



	for (Object2D* obj : enemyList)
	{
		Enemy* enemy = dynamic_cast<Enemy*>(obj);
		if (!enemy) continue;

		// 敵の当たり判定
		float enemyWidth = enemy->GetCollisionWidth();
		float enemyHeight = enemy->GetCollisionHeight();
		int eneLeft = static_cast<int>(
			enemy->GetPosition().x - enemyWidth / 2.0f
			);
		int eneTop = static_cast<int>(
			enemy->GetPosition().y + enemy->GetCollisionOffsetY() - enemyHeight / 2.0f
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

		//  プレイヤー自身と敵の衝突判定 (ダメージで赤くする)
		bool canHitBody = enemy->IsActiveTarget();
		if ((canHitBody && Collision::CheckRectToRect(myPos, mySize, enePos, eneSize)) ||
			enemy->CheckAttackCollision(myPos, mySize))
		{
			UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
			bool ignoresInvincible = (enemyStatus && enemyStatus->mIgnoresInvincibility);

			if (ignoresInvincible)
			{
				int dmg = enemyStatus ? enemyStatus->mAttack : 0;
				if (enemyStatus && enemyStatus->mHasInstantKillAttack) dmg = mHp;
				if (mHp - dmg <= 0)
				{
					// ニードルに当たった時に表示するもの
					mDeathReason = DeathReason::DeathType::NeedleTrapHit;
				}
				TakeDamage(dmg);
			}
			else if (isBlinking)
			{
				if (!mIsBlinkWallDeathImmune)
				{
					TakeDamage(mHp);
					mDeathReason = DeathReason::DeathType::NormalDeath;
				}
			}
			else
			{
				isHitDamage = true;
				if (mInvincibleTimer <= 0 && mDeadState == 0)
				{
					mInvincibleTimer = 60;
					if (enemyStatus)
					{
						int dmg = enemyStatus->mAttack;
						if (enemyStatus->mHasInstantKillAttack) dmg = mHp;

						if (mHp - dmg <= 0)
						{
							if (dynamic_cast<EnemySlime*>(enemy) != nullptr)
							{
								mDeathReason = DeathReason::DeathType::EnemySlime;
							}
							else if (dynamic_cast<EnemySkeleton*>(enemy) != nullptr)
							{
								mDeathReason = DeathReason::DeathType::EnemySkeleton;
							}
							else
							{
								mDeathReason = DeathReason::DeathType::NormalDeath;
							}
						}
						TakeDamage(dmg);
					}
				}
			}
		}
		// 2. 攻撃判定と敵の衝突判定 (敵を赤く光らせる)
		if (hasAttackRect && enemy->IsActiveTarget())
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
				NeedleTrap* trap = dynamic_cast<NeedleTrap*>(enemy);
				if (trap != nullptr)
				{
					continue;
				}
				if (!alreadyHit)
				{
					enemy->OnDamaged();
					mHitEnemies.push_back(enemy);
					UnitStatus* enemyStatus = dynamic_cast<UnitStatus*>(enemy);
					if (enemyStatus)
					{
						enemyStatus->TakeDamage(mAttack);
						if (enemyStatus->mHp <= 0 && !enemy->HasCustomDeath())
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
		//isDead = true;
		//mfCurrentSpeed = 0;
		//Master::gPlayerLive -= 1;
		//mlives = Master::gPlayerLive; // 表示用に同期

		//const int stageIndex = Master::gCurrentStageIndex;
		//if (stageIndex >= 0 && stageIndex < Master::MaxStageCount)
		//{
		//	// 現在のステージでの死亡回数
		//	Master::gDeathCounts[stageIndex]++;

		//	// これまでの累計死亡回数
		//	Master::gTotalDeathCounts[stageIndex]++;
		//}


		BeginDeath();
	}

	Object2D::Update();
}

void Player::TakeBulletDamage(int damage)
{
	if (CheckHitKey(KEY_INPUT_6))
	{
		return;
	}

	TakeDamage(damage);
	isHitDamage = true;

	if (mHp <= 0 && !isDead)
	{
		/*isDead = true;
		mfCurrentSpeed = 0;
		Master::gPlayerLive -= 1;
		mlives = Master::gPlayerLive;

		const int stageIndex = Master::gCurrentStageIndex;
		if (stageIndex >= 0 && stageIndex < Master::MaxStageCount)
		{
			Master::gDeathCounts[stageIndex]++;
			Master::gTotalDeathCounts[stageIndex]++;
		}*/


		BeginDeath();
	}
}


void Player::BeginDeath()
{
	isDead = true;
	mfCurrentSpeed = 0;
	Master::gPlayerLive -= 1;
	mlives = Master::gPlayerLive;

	const int stageIndex = Master::gCurrentStageIndex;
	if (stageIndex >= 0 && stageIndex < Master::MaxStageCount)
	{
		Master::gDeathCounts[stageIndex]++;
		Master::gTotalDeathCounts[stageIndex]++;
	}

	mDeathPhase = 1;
	mGameOverFadeTimer = 0;
	mGameOverMenuIndex = 0;
	mFragments.clear();
	deadTimer = 0;
	mDeadState = 0;

	mDeath.Start(mDeathReason, GetAngelMotionForDeathType(mDeathReason));
}

void Player::StartDeathScatter()
{
	mDeathPhase = 2;
	deadTimer = 0;
	mFragments.clear();

	// 落下死は飛び散らず、穴の下へ落ちていく
	if (mDeathReason == DeathReason::DeathType::Fall ||
		mDeathReason == DeathReason::DeathType::FallSide)
	{
		mDeadState = 0;
		return;
	}

	mDeadState = 1;

	const int fragSize = 16;
	const int srcBaseX = mPlayerState.GetCurrentFrame() * PlayerAnimState::FRAME_WIDTH;
	const int srcBaseY = mPlayerState.GetSrcY();
	const float topLeftX = mvPosition.x - PlayerAnimState::FRAME_WIDTH / 2.0f;
	const float topLeftY = mvPosition.y - PlayerAnimState::FRAME_HEIGHT / 2.0f;

	for (int y = 0; y < PlayerAnimState::FRAME_HEIGHT; y += fragSize)
	{
		for (int x = 0; x < PlayerAnimState::FRAME_WIDTH; x += fragSize)
		{
			PlayerFragment frag;
			frag.pos = VGet(topLeftX + x, topLeftY + y, 0.0f);
			frag.srcX = srcBaseX + x;
			frag.srcY = srcBaseY + y;
			frag.width = fragSize;
			frag.height = fragSize;
			frag.vel = VGet(
				((float)GetRand(100) / 100.0f * 14.0f) - 7.0f,
				((float)GetRand(100) / 100.0f * -14.0f) - 4.0f,
				0.0f);

			mFragments.push_back(frag);
		}
	}
}


void Player::UpdateGameOver()
{
	if (mGameOverFadeTimer < kGameOverMenuFrames) return;

	if (InputManager::CheckDownKey(KEY_INPUT_W) || InputManager::CheckDownKey(KEY_INPUT_UP) ||
		InputManager::CheckDownKey(KEY_INPUT_A) || InputManager::CheckDownKey(KEY_INPUT_LEFT))
	{
		mGameOverMenuIndex = 0;
	}
	else if (InputManager::CheckDownKey(KEY_INPUT_S) || InputManager::CheckDownKey(KEY_INPUT_DOWN) ||
		InputManager::CheckDownKey(KEY_INPUT_D) || InputManager::CheckDownKey(KEY_INPUT_RIGHT))
	{
		mGameOverMenuIndex = 1;
	}

	const bool confirmPressed =
		InputManager::CheckDownKey(KEY_INPUT_RETURN) || InputManager::CheckDownKey(KEY_INPUT_SPACE);

	if (InputManager::CheckDownKey(KEY_INPUT_R) || (confirmPressed && mGameOverMenuIndex == 0))
	{
		Revive();
	}
	else if (InputManager::CheckDownKey(KEY_INPUT_Q) || (confirmPressed && mGameOverMenuIndex == 1))
	{
		Master::mpGameManager->GetSceneManager()->SetNextScene(SceneManager::SCENE_STAGESELECT);
	}
}



// GameSceneで呼び出し
void Player::DrawFallDeath()
{
	if (!isDead) return;

	if (mDeathPhase == 1)
	{
		GameScene* gs = nullptr;
		if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
		{
			gs = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
		}
		mDeath.Draw(gs ? &gs->GetTutorialText() : nullptr);
	}
	else if (mDeathPhase == 3)
	{
		DrawGameOver();
	}
}

void Player::Draw()
{
	// 表側落下中は、Scene::Draw()（マップ奥）での描画をスキップ（GameSceneでマップ手前に描画）
	if (mFallMode == StageFallMode::Front)
	{
		return;
	}

	DrawPlayerModel();
}

void Player::DrawFront()
{
	// 表側落下中のみ、マップの手前側に描画
	if (mFallMode == StageFallMode::Front)
	{
		DrawPlayerModel();
	}
}

void Player::DrawPlayerModel()
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
				int playerTop = static_cast<int>(ConvertToScreenY(frag.pos.y, mpBlockMap));


				DrawRectGraph(
					playerLeft,
					playerTop,
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

				int playerTopX = static_cast<int>(ConvertToScreenY(frag.pos.y, mpBlockMap));
				int playerDownX = static_cast<int>(ConvertToScreenY(frag.pos.y + frag.height, mpBlockMap));

				// テクスチャが無い場合の保険   
				DrawBox(
					playerLeftX,
					playerTopX,
					playerRigthtX,
					playerDownX,
					GetColor(255, 0, 0),
					TRUE
				);
			}
		}

		return;
	}

	if (mpTexture != nullptr)
	{
		const int srcX = mPlayerState.GetCurrentFrame() * PlayerAnimState::FRAME_WIDTH;
		// 残像の描画
		for (const auto& img : mAfterimages)
		{
			int ax = static_cast<int>(ConvertToScreenX(img.pos.x, mpBlockMap) - PlayerAnimState::FRAME_WIDTH / 2);
			int ay = static_cast<int>(ConvertToScreenY(img.pos.y, mpBlockMap) - PlayerAnimState::FRAME_HEIGHT / 2);
			int aSrcX = srcX;
			int aSrcY = mPlayerState.GetSrcY();
			bool aFileX = !img.facingRight;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)img.alpha);
			DrawRectGraph(
				ax,
				ay,
				aSrcX,
				aSrcY,
				PlayerAnimState::FRAME_WIDTH,
				PlayerAnimState::FRAME_HEIGHT,
				mpTexture->GetHandle(),
				true,
				aFileX
			);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


		// 現在の状態に応じた画像の切り出し位置を設定する   
		PlayerAnimState::State animState = mPlayerState.GetState();
		int srcY = PlayerAnimState::IDLE_POS;

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
		}
		else if (animState == PlayerAnimState::State::SwordReady)
		{
			srcY = PlayerAnimState::ATTACK_POS;
		}

	
		// ダメージ中は赤み、温泉に入っている間は赤く発光させる
		// ダメージ中、または温泉に入っている間は赤くする
		if (isHitDamage || mHotSpringHeat > 0.12f)
		{
			SetDrawBright(255, 100, 100);
		}
		else
		{
			SetDrawBright(255, 255, 255);
		}

		// 指定の場所だけ描画する
		// 親クラスの共通関数を使用してワールド座標からスクリーン座標へ変換する
		const int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap) - PlayerAnimState::FRAME_WIDTH / 2);
		const int screenY = static_cast<int>(ConvertToScreenY(mvPosition.y, mpBlockMap) - PlayerAnimState::FRAME_HEIGHT / 2);
		const bool flipX = !isFacingRight;

		DrawRectGraph(
			screenX,
			screenY,
			srcX,
			srcY,
			PlayerAnimState::FRAME_WIDTH,
			PlayerAnimState::FRAME_HEIGHT,
			mpTexture->GetHandle(),
			TRUE,
			flipX
		);

		// 色を元に戻す（以降の描画に影響しないように必ず初期値へ）
		SetDrawBright(255, 255, 255);
	}
	else
	{
		Object2D::Draw();
	}

	// 攻撃エフェクトの描画
	if (isAttacking)
	{
		int attackWidth = PlayerConstants::PlayerAttackWidth;
		int attackHeight = PlayerConstants::PlayerAttackHeight;
		int rectLeft, rectTop, rectRight, rectBottom;
		int halfSizeX = PlayerConstants::PlayerAttackDist;

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

		// 画面上の中心Y座標を算出して上下の枠を決める
		int screenCenterY = static_cast<int>(ConvertToScreenY(mvPosition.y, mpBlockMap));
		rectTop = screenCenterY - attackHeight / 2;
		rectBottom = rectTop + attackHeight;


		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
		DrawBox(rectLeft, rectTop, rectRight, rectBottom, GetColor(255, 50, 50), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

}


// 体力はあとの描画
void Player::DrawLife()
{
	// UI（ハート）の描画（右上）
	if (mHp >= 0)
	{
		int hpLevel = (mHp * 6) / mMaxHp;
		if (mHp > 0 && hpLevel == 0) hpLevel = 1;

		int drawX = UIGraphController::LifePosX; // 左上に配置
		int drawY = UIGraphController::LifePosY;
		int Distance = UIGraphController::LifeDistance; // ハートの間隔
		for (int i = 0; i < 3; i++)
		{
			int heartState = hpLevel - (i * 2);
			int graph = mHeartEmptyGraph;
			if (heartState >= 2) graph = mHeartFullGraph;
			else if (heartState == 1) graph = mHeartHalfGraph;
			

			if (isDead)
			{
				graph = mHeartEmptyGraph;
			}

			if (graph != -1)
			{
				// 拡大するために描画方法を変更した
				DrawRotaGraph(
					drawX + i * Distance,
					drawY,
					UIGraphController::LifeScale,
					0.0f,
					graph,
					TRUE);
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
	const int playerTop = static_cast<int>(ConvertToScreenY(mvPosition.y - mfPlayerHeight / 2.0f, mpBlockMap));
	const int playerRight = static_cast<int>(ConvertToScreenX(mvPosition.x + mfPlayerWidth / 2.0f, mpBlockMap));
	const int playerBottom = static_cast<int>(ConvertToScreenY(mvPosition.y + mfPlayerHeight / 2.0f, mpBlockMap));
	DrawBox(
		playerLeft,
		playerTop,
		playerRight,
		playerBottom,
		GetColor(0, 0, 255),
		FALSE
	);


	int playerY = static_cast<int>(ConvertToScreenY(mvPosition.y, mpBlockMap));
	// HPのデバッグ
	DrawFormatString(playerLeft, playerY + 10, ColorOption::White, "HP: %d\n残機: %d\nSP: %f", mHp, mlives, mfCurrentSpeed);
	DrawFormatString(0, 600, ColorOption::White, "X:%2f, Y:%2f\n L:%d", mvPosition.x, mvPosition.y, playerLeft);
	
	
	
	char deathText[64];
	int stageIndex = Master::gCurrentStageIndex;

	if (stageIndex >= 0 && stageIndex < Master::MaxStageCount)
	{
		if (Master::gDeathCounts[stageIndex] == -1)
		{
			sprintf_s(deathText, "死亡回数: - 回");
		}
		else
		{
			sprintf_s(
				deathText,
				"死亡回数: %d 回",
				Master::gDeathCounts[stageIndex]
			);
		}
		DrawFormatString(500, 300, GetColor(255, 0, 0), "%s", deathText);
		DrawFormatString(500, 340, GetColor(255, 0, 0), "累計死亡回数: %d回", Master::gTotalDeathCounts[stageIndex]);


	}


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
			float enemyWidth = enemy->GetCollisionWidth();
			float enemyHeight = enemy->GetCollisionHeight();

			float divide = 2.0f;

			float eneOffsetY = enemy->GetCollisionOffsetY();

			// 敵のデバッグ描画でも共通関数を利用してスクロールを正確に合わせる
			int eneLeft = static_cast<int>(ConvertToScreenX(enemy->GetPosition().x - enemyWidth / divide, mpBlockMap));
			int eneTop = static_cast<int>(ConvertToScreenY(enemy->GetPosition().y + eneOffsetY - enemyHeight / divide, mpBlockMap));
			int eneRight = static_cast<int>(ConvertToScreenX(enemy->GetPosition().x + enemyWidth / divide, mpBlockMap));
			int eneBottom = static_cast<int>(ConvertToScreenY(enemy->GetPosition().y + eneOffsetY + enemyHeight / divide, mpBlockMap));

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
	deadTimer++;

	// 落下死：穴の下へ加速しながら落ちていく
	if (mDeadState == 0)
	{
		velocityY += 0.65f;
		mvPosition.y += velocityY;
		return;
	}

	// 飛び散り直後の画面揺れ
	if (mpBlockMap != nullptr)
	{
		if (deadTimer < 16)
		{
			const int amp = 16 - deadTimer;
		}
	}

	// 破片の物理（重力・ブロック衝突）。元の位置へは戻らない
	for (auto& frag : mFragments)
	{
		frag.vel.y += 0.45f;

		frag.pos.x += frag.vel.x;
		if (mpBlockMap &&
			mCharacterPhysics.IsBlockCollision(*mpBlockMap, frag.pos.x, frag.pos.y,
				static_cast<float>(frag.width), static_cast<float>(frag.height)))
		{
			frag.pos.x -= frag.vel.x;
			frag.vel.x *= -0.6f;
		}

		frag.pos.y += frag.vel.y;
		if (mpBlockMap &&
			mCharacterPhysics.IsBlockCollision(*mpBlockMap, frag.pos.x, frag.pos.y,
				static_cast<float>(frag.width), static_cast<float>(frag.height)))
		{
			frag.pos.y -= frag.vel.y;
			frag.vel.y *= -0.4f;
			frag.vel.x *= 0.9f;
		}
	}
}


/*void Player::DeadProcess()
{
	if (mbDeath) return;

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
			float targetX =
				mSpawnPos.x -
				(PlayerAnimState::FRAME_WIDTH / 2.0f) +
				frag.srcX;

			float targetY =
				mSpawnPos.y -
				(PlayerAnimState::FRAME_HEIGHT / 2.0f) +
				(frag.srcY % PlayerAnimState::FRAME_HEIGHT);

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
	}
}*/

// 復活した時に位置とHPを戻す処理
void Player::Revive()
{
	mDeathReason = DeathReason::DeathType::None;
	mFallMode = StageFallMode::None;
	mWKeyHoldTimer = 0;
	mSKeyHoldTimer = 0;
	

	// 位置のリセット
	mvPosition = mSpawnPos;
	mvPosition = mSpawnPos;
	mHp = mMaxHp;

	// 死亡フェーズのリセット
	mDeathPhase = 0;
	mGameOverFadeTimer = 0;
	mGameOverMenuIndex = 0;

	// 残機を同期（死亡確定時に減算済みのgPlayerLiveと合わせる）
	mlives = Master::gPlayerLive;

	// ジャンプ状態をリセット
	mbIsJumping = false;
	isGrounded = true;
	velocityY = 0.0f;

	// ブリンク状態をリセット
	isBlinking = false;
	blinkTimer = 0;
	blinkCooldownTimer = 0;
	blinkDirection = 0.0f;
	mAfterimages.clear();

	// 攻撃状態をリセット
	isAttacking = false;
	attackTimer = 0;
	mHitEnemies.clear();

	// ダメージ状態をリセット
	isHitDamage = false;
	mInvincibleTimer = 0;

	// 移動状態をリセット
	mfMoveDirection = 0.0f;
	mfCurrentSpeed = moveSpeed;

	// 行動ステートのリセット
	mActionState.blink = false;
	mActionState.wallbreak = false;

	// マップのスクロール位置を一番左に戻す
	if (mpBlockMap != nullptr)
	{
		mpBlockMap->ResetScroll();
		mDeath.Reset();

		if (mpBlockMap != nullptr)
		{
			mpBlockMap->ResetScroll();
			mBlockAction.SetHitDeathBlock(false);
		}

		mBlockAction.SetHitDeathBlock(false);
	}

	// ボス演出・自動歩行状態をリセット
	mbAutoWalking = false;
	mbAutoWalkArrived = false;
	mAutoWalkTargetX = 0.0f;
	mbTouchingBossArea = false;
	mbIsEvading = false;
	mbEvadedBossSpawn = false;
	mEvasionTimer = 0;
	mbSwordReadyStance = false;

	// 崩落床のリセット
	CrumbleFloor::ResetAll();

	isDead = false;
	mbDeath = false;
}

void Player::StartAutoWalk(float targetX)
{
	mbAutoWalking = true;
	mbAutoWalkArrived = false;
	mAutoWalkTargetX = targetX;
}

void Player::StopAutoWalk()
{
	mbAutoWalking = false;
	mbAutoWalkArrived = false;
	mfMoveDirection = 0.0f;
}

void Player::TriggerCinematicEvasion()
{
	if (mbEvadedBossSpawn) return;
	mbEvadedBossSpawn = true;
	mbIsEvading = true;
	mEvasionTimer = KingSlimeIntroConstants::EvasionDurationFrames;
	mActionState.blink = true;

	// 右向きのまま背中側（左）へジャンプ飛び退き
	isFacingRight = true;
	velocityY = KingSlimeIntroConstants::EvasionJumpSpeedY;
	isGrounded = false;
	mbIsJumping = true;
	mPlayerState.SetJump();

	// 残像を即座に1つ生成
	BlinkAfterimage img;
	img.pos = mvPosition;
	img.facingRight = isFacingRight;
	img.alpha = 150.0f;
	mAfterimages.push_back(img);
}

void Player::ResetBossIntroState()
{
	mbAutoWalking = false;
	mbAutoWalkArrived = false;
	mAutoWalkTargetX = 0.0f;
	mbTouchingBossArea = false;
	mbIsEvading = false;
	mbEvadedBossSpawn = false;
	mEvasionTimer = 0;
	mbSwordReadyStance = false;
	mfMoveDirection = 0.0f;
	mPlayerState.SetIdle();
}

void Player::EndBossIntroLock()
{
	mbAutoWalking = false;
	mbAutoWalkArrived = false;
	mbSwordReadyStance = false;
	mbIsEvading = false;
	mPlayerState.SetIdle();
}


DeathController::AngelMotion Player::GetAngelMotionForDeathType(DeathReason::DeathType type) const
{
	using DT = DeathReason::DeathType;
	using AM = DeathController::AngelMotion;

	switch (type)
	{
	case DT::FallSide:
	case DT::WallHit:
		return AM::Laugh;   // 爆笑

	case DT::Fall:
	case DT::NeedleTrapHit:
		return AM::Point;   // 指さし

	case DT::DeathBlock:
		return AM::Hide;    // 隠れ（あきれ）

	case DT::None:
	case DT::NormalDeath:
	case DT::EnemySlime:
	case DT::EnemySkeleton:
	default:
		return AM::Normal;  // ノーマル
	}
}


void Player::DrawGameOver()
{
	int fadeAlpha = (mGameOverFadeTimer * 255) / 40;
	if (fadeAlpha > 255) fadeAlpha = 255;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
	DrawBox(0, 0, ScreenSize::ScrrenWidth, ScreenSize::ScrrenHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	if (mGameOverFadeTimer < 20) return;

	int uiAlpha = ((mGameOverFadeTimer - 20) * 255) / 25;
	if (uiAlpha > 255) uiAlpha = 255;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, uiAlpha);

	auto* font = Master::mpGameManager->GetFontManager();

	// 1. GAME OVER 大見出し
	const int titleX = ScreenSize::CenterX - 380;
	const int titleY = 180;
	font->DrawDotString(titleX + 6, titleY + 6, 140, GetColor(75, 0, 0), "GAME OVER");
	font->DrawDotString(titleX, titleY, 140, GetColor(235, 35, 35), "GAME OVER");

	// 2. 残機アイコン
	if (mpTexture != nullptr && mpTexture->GetHandle() != -1)
	{
		DrawRectGraph(ScreenSize::CenterX - 120, 360, 0, 0,
			PlayerAnimState::FRAME_WIDTH, PlayerAnimState::FRAME_HEIGHT,
			mpTexture->GetHandle(), TRUE, FALSE);
	}
	font->DrawDotString(ScreenSize::CenterX + 20, 400, 68, GetColor(255, 255, 255), "x %d", mlives);

	// 3. メニュー
	const int menuBoxW = 720;
	const int menuBoxH = 74;
	const int menuLeft = ScreenSize::CenterX - menuBoxW / 2;
	const int opt0Y = 560;
	const int opt1Y = 660;

	const int selY = (mGameOverMenuIndex == 0) ? opt0Y : opt1Y;
	DrawBox(menuLeft, selY - 10, menuLeft + menuBoxW, selY + menuBoxH - 10, GetColor(60, 45, 15), TRUE);
	DrawBox(menuLeft, selY - 10, menuLeft + menuBoxW, selY + menuBoxH - 10, GetColor(255, 215, 60), FALSE);

	const int bobX = static_cast<int>(sinf(mGameOverFadeTimer * 0.15f) * 6.0f);
	const unsigned int col0 = (mGameOverMenuIndex == 0) ? GetColor(255, 240, 70) : GetColor(165, 165, 175);
	const unsigned int col1 = (mGameOverMenuIndex == 1) ? GetColor(255, 240, 70) : GetColor(165, 165, 175);

	if (mGameOverMenuIndex == 0)
	{
		font->DrawDotString(menuLeft + 24 + bobX, opt0Y, 48, col0, ">");
	}
	font->DrawDotString(menuLeft + 76, opt0Y, 48, col0, "コンテニュー / リトライ  [Enter / R]");

	if (mGameOverMenuIndex == 1)
	{
		font->DrawDotString(menuLeft + 24 + bobX, opt1Y, 48, col1, ">");
	}
	font->DrawDotString(menuLeft + 76, opt1Y, 48, col1, "ステージ選択へ戻る        [Q]");

	font->DrawDotString(ScreenSize::CenterX - 280, 810, 32, GetColor(170, 170, 170),
		"W/S または ↑/↓ で選択 ・ SPACE / ENTER で決定");

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}