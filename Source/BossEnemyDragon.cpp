#include "BossEnemyDragon.h"
#include "GameConstants.h"
#include "FontManager.h"
#include "DxLib.h"
#include "BlockMap.h"
#include "Master.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "Player.h"
#include <cmath>
#include <cstdio>

BossEnemyDragon::BossEnemyDragon(VECTOR initPos)
	: Enemy("",initPos)
	, mState(State::Idle)
	, mAnimFrame(0)
	, mAnimTimer(0)
	, mStateTimer(0)
	, mAttackPhase(0)
	, mAttackCooldown(60)
	, mFlightCooldown(180)
	, mEntranceTimer(0)
	, mPostEntranceTimer(0)
	, mHasBrokenCeiling(false)
	, mEntranceLandAnimDone(false)
	, mFacingDirection(-1.0f)
	, mDrawScale(1.6f)
	, mHoverTargetY(1480.0f)
	, mSmoothHp(80.0f)
	, mHpShakeTimer(0)
{
	UpdateStatusByProgress(GameProgress::Tutorial1);

	// 1. Idle (4��: sprite_idle1.png ~ sprite_idle4.png)
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_idle/sprite_idle%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mIdleAnim.push_back(h);
	}

	// 2. Move (4��: sprite_move1.png ~ sprite_move4.png)
	for (int i = 1; i <= 4; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_move/sprite_move%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mMoveAnim.push_back(h);
	}

	// 3. Attack (9��: sprite_attack1.png ~ sprite_attack9.png)
	for (int i = 1; i <= 9; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_attack/sprite_attack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAttackAnim.push_back(h);
	}

	// 4. �����A�j���[�V���� (7��: sprite_takeoff1.png ~ sprite_takeoff7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_takeoff/sprite_takeoff%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mTakeoffAnim.push_back(h);
	}

	// 5. ���ł���A�j���[�V���� (7��: sprite_fly1.png ~ sprite_fly7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_flying/sprite_fly%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mFlyAnim.push_back(h);
	}

	// 6. �󒆍U�� (7��: sprite_airAttack1.png ~ sprite_airAttack7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_airborne/sprite_airAttack%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mAirAttackAnim.push_back(h);
	}

	// 7. �����A�j���[�V���� (7��: sprite_landing1.png ~ sprite_landing7.png)
	for (int i = 1; i <= 7; ++i)
	{
		char path[256];
		sprintf_s(path, "Resource/Enemy/Dragon/animation_dragon/new_landing/sprite_landing%d.png", i);
		int h = LoadGraph(path);
		if (h != -1) mLandingAnim.push_back(h);
	}

	// �����E�����蔻��T�C�Y�̐ݒ�
	mfEnemyWidth = 220.0f;
	mfEnemyHeight = 200.0f;
	moveSpeed = 0.0f;
}

BossEnemyDragon::~BossEnemyDragon()
{
	for (int h : mIdleAnim) if (h != -1) DeleteGraph(h);
	for (int h : mMoveAnim) if (h != -1) DeleteGraph(h);
	for (int h : mAttackAnim) if (h != -1) DeleteGraph(h);
	for (int h : mTakeoffAnim) if (h != -1) DeleteGraph(h);
	for (int h : mFlyAnim) if (h != -1) DeleteGraph(h);
	for (int h : mAirAttackAnim) if (h != -1) DeleteGraph(h);
	for (int h : mLandingAnim) if (h != -1) DeleteGraph(h);
}

void BossEnemyDragon::UpdateStatusByProgress(GameProgress progress)
{
	mMaxHp = 80;
	mHp = 80;
	mAttack = 2;
	mDefense = 5;
	mMagic = 10;
	mSmoothHp = 80.0f;
}

void BossEnemyDragon::StartEntrance()
{
	mState = State::EntranceDrop;
	mEntranceTimer = 0;
	mPostEntranceTimer = 0;
	mHasBrokenCeiling = false;
	mEntranceLandAnimDone = false;
	mAnimFrame = 0;
	mAnimTimer = 0;
	isGrounded = false;
	velocityY = 6.0f;
	mFireProjectiles.clear();
	mBurningGrounds.clear();
}

void BossEnemyDragon::TakeDamage(int damage)
{
	if (mState == State::EntranceDrop || mState == State::EntranceWait) return;
	mHp -= damage;
	if (mHp < 0) mHp = 0;
	mHpShakeTimer = 15;
}

bool BossEnemyDragon::IsAttacking() const
{
	if (mState == State::Attack)
	{
		return ((mAttackPhase == 2 || mAttackPhase == 4) && mAnimFrame >= 3 && mAnimFrame <= 6);
	}
	return false;
}

void BossEnemyDragon::GetAttackHitbox(float& left, float& top, float& right, float& bottom) const
{
	float reach = (mState == State::AirAttack) ? 190.0f : 170.0f;
	if (mFacingDirection < 0.0f)
	{
		// �������U��
		right = mvPosition.x - 10.0f;
		left = mvPosition.x - (mfEnemyWidth * 0.5f + reach);
	}
	else
	{
		// �E�����U��
		left = mvPosition.x + 10.0f;
		right = mvPosition.x + (mfEnemyWidth * 0.5f + reach);
	}

	if (mState == State::AirAttack)
	{
		top = mvPosition.y - mfEnemyHeight * 0.3f;
		bottom = mvPosition.y + mfEnemyHeight * 0.65f;
	}
	else
	{
		top = mvPosition.y - mfEnemyHeight * 0.45f;
		bottom = mvPosition.y + mfEnemyHeight * 0.48f;
	}
}

void BossEnemyDragon::Update()
{
	Enemy::Update();

	if (mHpShakeTimer > 0) mHpShakeTimer--;
	if (mSmoothHp < 0.0f) mSmoothHp = static_cast<float>(mHp);
	if (mSmoothHp > mHp)
	{
		mSmoothHp -= 1.0f;
		if (mSmoothHp < mHp) mSmoothHp = static_cast<float>(mHp);
	}
	else if (mSmoothHp < mHp)
	{
		mSmoothHp = static_cast<float>(mHp);
	}

	GameScene* gameScene = nullptr;
	if (Master::mpGameManager && Master::mpGameManager->GetSceneManager())
	{
		gameScene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
	}

	// ���j���̓_�ŏ��ŏ���
	if (mHp <= 0)
	{
		if (!isDeadBlinking)
		{
			isDeadBlinking = true;
			deadBlinkTimer = 120;
			if (gameScene)
			{
			}
		}
		deadBlinkTimer--;
		if (deadBlinkTimer <= 0)
		{
			SetDeleteFlag(true);
		}
		return;
	}

	Player* player = (gameScene != nullptr) ? gameScene->GetPlayer() : nullptr;

	// =========================================================
	// �o�ꉉ�o1: �V���˂��j���ė��� (EntranceDrop)
	// =========================================================
	if (mState == State::EntranceDrop)
	{
		if (player)
		{
			float dx = player->GetPosition().x - mvPosition.x;
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		// �󒆍~���t���[���i�����A�j���[�V�����O�� 0..2�j
		mAnimTimer++;
		if (mAnimTimer >= 6)
		{
			mAnimTimer = 0;
			if (mAnimFrame < 2) mAnimFrame++;
		}

		//// �V��̍����iY >= 980�j�ɓ��B�����u�ԂɓV���j�󂵂Ċ�Ђ�~�点��
		//if (!mHasBrokenCeiling && mvPosition.y >= 980.0f && mpBlockMap != nullptr)
		//{
		//	mHasBrokenCeiling = true;
		//	mpBlockMap->BreakBossCeiling(static_cast<int>(mvPosition.x), 260);
		//	mpBlockMap->mShakeX = (GetRand(16) - 8);
		//	mpBlockMap->mShakeY = (GetRand(16) - 8);
		//}

		// ���ɒ��n������ EntranceWait �ֈڍs���A�����A�j���[�V�����㔼�i�����G�t�F�N�g 3..6�j��Đ�
		if (isGrounded)
		{
			mState = State::EntranceWait;
			mEntranceTimer = 240; // �L���O�X���C�~�[�Ɠ���4�b��
			mEntranceLandAnimDone = false;
			mAnimFrame = 3;
			mAnimTimer = 0;
			if (gameScene)
			{
			}
		}
		return;
	}

	// =========================================================
	// �o�ꉉ�o2: ���n��̙�K�E�C���g���\���ҋ@ (EntranceWait)
	// =========================================================
	if (mState == State::EntranceWait)
	{
		mAnimTimer++;
		if (!mEntranceLandAnimDone)
		{
			if (mAnimTimer >= 7)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= static_cast<int>(mLandingAnim.size()))
				{
					mEntranceLandAnimDone = true;
					mAnimFrame = 0;
				}
			}
		}
		else
		{
			if (mAnimTimer >= 10 && !mIdleAnim.empty())
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
			}
		}

		
		if (mEntranceTimer > 0) mEntranceTimer--;
		if (mEntranceTimer <= 0)
		{
			mState = State::Idle;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			mPostEntranceTimer = 60;
		}
		return;
	}

	if (mPostEntranceTimer > 0) mPostEntranceTimer--;
	if (mAttackCooldown > 0) mAttackCooldown--;
	if (mFlightCooldown > 0) mFlightCooldown--;

	// �Ή��u���X�e�ƒn�ʂ̉��āi8�b�ԃ_���[�W����j�̍X�V
	UpdateFireAndBurningGround(player);

	// �{�X�����̍s���͈͐��� (X: 8080 ~ 9680)
	const float roomMinX = 8080.0f;
	const float roomMaxX = 9680.0f;

	float px = (player != nullptr) ? player->GetPosition().x : mvPosition.x;
	float dx = px - mvPosition.x;
	float absDx = std::fabs(dx);

	switch (mState)
	{
	case State::Idle:
	{
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 10 && !mIdleAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
		}

		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;

			// ��s�N�[���_�E���������͗����A�j���[�V�����ֈڍs
			if (mFlightCooldown <= 0 && mStateTimer >= 35)
			{
				mState = State::Takeoff;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}

			// �ʏ�A�^�b�N�̊ԍ����Ȃ�n��A���U����
			if (absDx <= 360.0f && mAttackCooldown <= 0)
			{
				mState = State::Attack;
				mAttackPhase = 0; // Phase 0: ����Ɂu�I�v�}�[�N�\��
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}

			// ��莞�ԑҋ@��Ɉړ��J�n
			if (mStateTimer >= 40)
			{
				mState = State::Move;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
			}
		}
		break;
	}

	case State::Move:
	{
		// �ʏ�̈ړ��X�s�[�h�E�A�j���[�V�������x�͌��̂܂�
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 8 && !mMoveAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mMoveAnim.size());
		}

		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;

			// ��s�N�[���_�E�������܂����痣���iTakeoff�j��
			if (mFlightCooldown <= 0 && mStateTimer >= 35)
			{
				mState = State::Takeoff;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}

			// �U���ԍ����ɓ�������ʏ�A�^�b�N�iAttack�j��
			if (absDx <= 350.0f && mAttackCooldown <= 0)
			{
				mState = State::Attack;
				mAttackPhase = 0; // Phase 0: ����Ɂu�I�v�}�[�N�\��
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				break;
			}
		}

		float walkSpeed = 4.2f;
		mvPosition.x += mFacingDirection * walkSpeed;
		if (mvPosition.x < roomMinX) { mvPosition.x = roomMinX; mFacingDirection = 1.0f; }
		if (mvPosition.x > roomMaxX) { mvPosition.x = roomMaxX; mFacingDirection = -1.0f; }

		if (mStateTimer >= 110)
		{
			mState = State::Idle;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
		}
		break;
	}

	case State::Attack:
	{
		// �ʏ�A�^�b�N:
		// Phase 0: ���̏�Ɂu�I�v�}�[�N��o���ė\������
		// Phase 1: �r���őO�ɏ����O�i�i5�t���[���ŃX�b�Ɩ�75px�����O�֏o��j
		// Phase 2: 1��ڂ̍U���i���̏�őf�����a�����j
		// Phase 3: �Ăяr���őO�ɏ����O�i�i5�t���[���ŃX�b�Ɩ�75px�����O�֏o��j
		// Phase 4: 2��ڂ̍U��
		mStateTimer++;
		mAnimTimer++;

		if (mAttackPhase == 0)
		{
			// ����Ɂu�I�v�}�[�N��\�����Ȃ���\������
			if (mAnimTimer >= 8 && !mIdleAnim.empty())
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mIdleAnim.size());
			}
			if (mStateTimer >= 32)
			{
				mAttackPhase = 1;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
			}
		}
		else if (mAttackPhase == 1 || mAttackPhase == 3)
		{
			// �r���őO�ɏ��������O�i�i15px �~ 5�t���[�� = 75px ���u�ŃX�e�b�v�j
			if (mAnimTimer >= 2 && !mMoveAnim.empty())
			{
				mAnimTimer = 0;
				mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mMoveAnim.size());
			}

			mvPosition.x += mFacingDirection * 15.0f;
			if (mvPosition.x < roomMinX) mvPosition.x = roomMinX;
			if (mvPosition.x > roomMaxX) mvPosition.x = roomMaxX;

			if (mStateTimer >= 5)
			{
				mAttackPhase++; // Phase 1 -> 2 (1��ڍU��), Phase 3 -> 4 (2��ڍU��)
				mStateTimer = 0;
				mAnimFrame = 2; // ���߃t���[�����΂��Ă����U����
				mAnimTimer = 0;
			}
		}
		else if (mAttackPhase == 2 || mAttackPhase == 4)
		{
			// �U�����[�V�����Đ��i�O�i�������Ȃ��悤�A�U�����͂��̏�Ŏa�����j
			if (mAnimTimer >= 4)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				int endFrame = (mAttackPhase == 2) ? 7 : static_cast<int>(mAttackAnim.size());
				if (mAnimFrame >= endFrame)
				{
					mAnimFrame = 0;
					mStateTimer = 0;
					if (mAttackPhase == 2)
					{
						// 1��ڂ̍U������ -> Phase 3�i2��ڂ̏����O�i�j��
						mAttackPhase = 3;
					}
					else
					{
						// 2��ڂ̍U������ -> Idle�֖߂�
						mAttackPhase = 0;
						mState = State::Idle;
						mAttackCooldown = 70;
					}
				}
			}
		}
		break;
	}

	case State::Takeoff:
	{
		// �����A�j���[�V�����isprite_takeoff1 ~ 7�j
		mStateTimer++;
		mAnimTimer++;
		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		// �t���[��2�ȍ~�ŉH�΂����ċ󒆂֕���
		if (mAnimFrame >= 2)
		{
			mvPosition.y -= 6.5f;
			if (mvPosition.y < mHoverTargetY) mvPosition.y = mHoverTargetY;
		}

		if (mAnimTimer >= 6)
		{
			mAnimTimer = 0;
			mAnimFrame++;
			if (mAnimFrame >= static_cast<int>(mTakeoffAnim.size()))
			{
				mState = State::Fly;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
			}
		}
		break;
	}

	case State::Fly:
	{
		// ���ł���A�j���[�V�����isprite_fly1 ~ 7�j
		mStateTimer++;
		mAnimTimer++;
		if (mAnimTimer >= 6 && !mFlyAnim.empty())
		{
			mAnimTimer = 0;
			mAnimFrame = (mAnimFrame + 1) % static_cast<int>(mFlyAnim.size());
		}

		// �z�o�����O���x�֊��炩�Ɉێ����A�΂𕬂��₷���΂ߏ��̊ԍ����i��280�`420px�j�ֈړ�
		float hoverWave = std::sin(mStateTimer * 0.1f) * 2.5f;
		mvPosition.y += (mHoverTargetY - mvPosition.y) * 0.12f + hoverWave;

		if (player && !player->GetIsDead())
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			if (absDx > 400.0f)
			{
				mvPosition.x += mFacingDirection * 6.0f;
			}
			else if (absDx < 220.0f)
			{
				mvPosition.x -= mFacingDirection * 4.5f;
			}
		}
		if (mvPosition.x < roomMinX) mvPosition.x = roomMinX;
		if (mvPosition.x > roomMaxX) mvPosition.x = roomMaxX;

		// ��莞�Ԕ�s������ɋ󒆉Ή��u���X�U���iAirAttack�j�ֈڍs
		if (mStateTimer >= 65)
		{
			mState = State::AirAttack;
			mStateTimer = 0;
			mAnimFrame = 0;
			mAnimTimer = 0;
			if (player && !player->GetIsDead())
			{
				mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
			}
		}
		break;
	}

	case State::AirAttack:
	{
		// �󒆍U���A�j���[�V�����isprite_airAttack1 ~ 7�j�ŋ󒆂ɗ��܂�Ȃ���v���C���[�Ɍ������ĉ΂𕬂�
		mStateTimer++;
		mAnimTimer++;

		// �󒆂Ōy���z�o�����O�ێ�
		float hoverWave = std::sin(mStateTimer * 0.12f) * 1.5f;
		mvPosition.y += (mHoverTargetY - mvPosition.y) * 0.1f + hoverWave;

		if (player && !player->GetIsDead() && mAnimFrame <= 3)
		{
			mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;
		}

		// �t���[��3�`5�i����傫���J���ĉ΂�f�����[�V�����j�̊ԁA��������v���C���[�֌������ĉΉ��e�𕬎�
		if (mAnimFrame >= 3 && mAnimFrame <= 5)
		{
			if (mStateTimer % 4 == 0 && player)
			{
				float mouthX = mvPosition.x + mFacingDirection * (115.0f * mDrawScale);
				float mouthY = mvPosition.y - 110.0f;

				float targetX = player->GetPosition().x + static_cast<float>((GetRand(90) - 45));
				float targetY = 1903.0f; // �v���C���[�̑����̒n�ʂ܂ł�������͂��悤�ɑ_��
				float fdx = targetX - mouthX;
				float fdy = targetY - mouthY;
				if (fdy < 80.0f) fdy = 80.0f;
				float len = std::sqrt(fdx * fdx + fdy * fdy);
				if (len < 1.0f) len = 1.0f;

				float speed = 14.5f;
				FireProjectile fp;
				fp.x = mouthX;
				fp.y = mouthY;
				fp.vx = (fdx / len) * speed;
				fp.vy = (fdy / len) * speed;
				fp.radius = 24.0f;
				fp.animTick = 0;
				fp.active = true;
				mFireProjectiles.push_back(fp);
			}
		}

		// �΂𕬂��Ă���t���[���i3�`5�j�͏������߂Ɉێ����Ă�������Ή��u���X�������
		int frameDuration = (mAnimFrame >= 3 && mAnimFrame <= 5) ? 12 : 6;
		if (mAnimTimer >= frameDuration)
		{
			mAnimTimer = 0;
			mAnimFrame++;
			if (mAnimFrame >= static_cast<int>(mAirAttackAnim.size()))
			{
				mState = State::Landing;
				mStateTimer = 0;
				mAnimFrame = 0;
				mAnimTimer = 0;
				velocityY = 4.0f;
			}
		}
		break;
	}

	case State::Landing:
	{
		// �����A�j���[�V�����isprite_landing1 ~ 7�j
		mStateTimer++;
		mAnimTimer++;

		if (!isGrounded)
		{
			// ���n����܂ł̓t���[��0�`2�ō~��
			if (mAnimTimer >= 6)
			{
				mAnimTimer = 0;
				if (mAnimFrame < 2) mAnimFrame++;
			}
		}
		else
		{
			// ���n��̓t���[��3�`6�i�����G�t�F�N�g�t�������j��Đ�
			if (mAnimFrame < 3)
			{
				mAnimFrame = 3;
				mAnimTimer = 0;
			}
			if (mAnimTimer >= 6)
			{
				mAnimTimer = 0;
				mAnimFrame++;
				if (mAnimFrame >= static_cast<int>(mLandingAnim.size()))
				{
					mState = State::Idle;
					mStateTimer = 0;
					mAnimFrame = 0;
					mAnimTimer = 0;
					mFlightCooldown = 210;
					mAttackCooldown = 45;
				}
			}
		}
		break;
	}
	}
}

void BossEnemyDragon::EnemyMove(BlockMap& blockMap)
{
	mpBlockMap = &blockMap;

	// �����E��s�E�󒆍U�����͏d�͗����������󒆍��W��ێ�����
	if (mState == State::Takeoff || mState == State::Fly || mState == State::AirAttack)
	{
		isGrounded = false;
		velocityY = 0.0f;
		return;
	}

	Enemy::EnemyMove(blockMap);
}

void BossEnemyDragon::Draw()
{
	if (isDeadBlinking && (deadBlinkTimer / 4) % 2 == 0)
	{
		return;
	}

	int handle = -1;
	float footOffsetFromCenter = 79.0f; // 256x256�L�����o�X��̑����I�t�Z�b�g

	switch (mState)
	{
	case State::EntranceDrop:
		if (!mLandingAnim.empty())
		{
			int idx = mAnimFrame;
			if (idx >= static_cast<int>(mLandingAnim.size())) idx = static_cast<int>(mLandingAnim.size()) - 1;
			handle = mLandingAnim[idx];
			footOffsetFromCenter = 57.0f; // 400x400�L�����o�X�
		}
		break;

	case State::EntranceWait:
		if (!mEntranceLandAnimDone && !mLandingAnim.empty())
		{
			int idx = mAnimFrame;
			if (idx >= static_cast<int>(mLandingAnim.size())) idx = static_cast<int>(mLandingAnim.size()) - 1;
			handle = mLandingAnim[idx];
			footOffsetFromCenter = 57.0f;
		}
		else if (!mIdleAnim.empty())
		{
			handle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
			footOffsetFromCenter = 79.0f;
		}
		break;

	case State::Idle:
		if (!mIdleAnim.empty())
		{
			handle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
			footOffsetFromCenter = 79.0f;
		}
		break;

	case State::Move:
		if (!mMoveAnim.empty())
		{
			handle = mMoveAnim[mAnimFrame % mMoveAnim.size()];
			footOffsetFromCenter = 79.0f;
		}
		break;

	case State::Attack:
		footOffsetFromCenter = 79.0f;
		if (mAttackPhase == 0)
		{
			if (!mIdleAnim.empty()) handle = mIdleAnim[mAnimFrame % mIdleAnim.size()];
		}
		else if (mAttackPhase == 1 || mAttackPhase == 3)
		{
			if (!mMoveAnim.empty()) handle = mMoveAnim[mAnimFrame % mMoveAnim.size()];
		}
		else
		{
			if (!mAttackAnim.empty()) handle = mAttackAnim[mAnimFrame % mAttackAnim.size()];
		}
		break;

	case State::Takeoff:
		if (!mTakeoffAnim.empty())
		{
			handle = mTakeoffAnim[mAnimFrame % mTakeoffAnim.size()];
			footOffsetFromCenter = 80.0f; // 350x350�L�����o�X�
		}
		break;

	case State::Fly:
		if (!mFlyAnim.empty())
		{
			handle = mFlyAnim[mAnimFrame % mFlyAnim.size()];
			footOffsetFromCenter = 57.0f; // 400x400�L�����o�X�
		}
		break;

	case State::AirAttack:
		if (!mAirAttackAnim.empty())
		{
			handle = mAirAttackAnim[mAnimFrame % mAirAttackAnim.size()];
			footOffsetFromCenter = 57.0f; // 400x400�L�����o�X�
		}
		break;

	case State::Landing:
		if (!mLandingAnim.empty())
		{
			handle = mLandingAnim[mAnimFrame % mLandingAnim.size()];
			footOffsetFromCenter = 57.0f; // 400x400�L�����o�X�
		}
		break;
	}

	if (handle != -1)
	{
		if (isDamaged)
		{
			SetDrawBright(255, 100, 100);
		}

		int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, mpBlockMap));
		float feetWorldY = mvPosition.y + (mfEnemyHeight * 0.5f);
		int screenY = static_cast<int>(ConvertToScreenY(feetWorldY - (footOffsetFromCenter * mDrawScale), mpBlockMap));

		// ���X�v���C�g�͍������Ȃ̂ŁA�E����(mFacingDirection > 0)�̂Ƃ����E���]�`��
		bool reverse = (mFacingDirection > 0.0f);

		// �r���O�i���iPhase 1, 3 ����эU�����ݍ��ݒ��j�͍����ړ��̎c����`�悵�ăX�s�[�h�������
		if (mState == State::Attack && (mAttackPhase == 1 || mAttackPhase == 3))
		{
			int dirSign = (mFacingDirection > 0.0f) ? 1 : -1;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 75);
			SetDrawBright(120, 200, 255);
			DrawRotaGraph(screenX - dirSign * 55, screenY, mDrawScale, 0.0, handle, TRUE, reverse);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 130);
			DrawRotaGraph(screenX - dirSign * 26, screenY, mDrawScale, 0.0, handle, TRUE, reverse);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			if (isDamaged) SetDrawBright(255, 100, 100);
			else SetDrawBright(255, 255, 255);
		}

		DrawRotaGraph(screenX, screenY, mDrawScale, 0.0, handle, TRUE, reverse);

		if (isDamaged)
		{
			SetDrawBright(255, 255, 255);
		}

		// �ʏ�A�^�b�N�O�̗\������iPhase 0�j�F�h���S���̓��̏�Ɂu�I�v�}�[�N��\��
		if (mState == State::Attack && mAttackPhase == 0)
		{
			int exX = static_cast<int>(ConvertToScreenX(mvPosition.x + mFacingDirection * 85.0f, mpBlockMap));
			int exY = static_cast<int>(ConvertToScreenY(mvPosition.y - 165.0f, mpBlockMap));
			bool blinkOff = (mStateTimer / 4) % 2 != 0;
			if (!blinkOff)
			{
				// �������{�ԐF�́u�I�v�}�[�N
				DrawBox(exX - 10, exY - 62, exX + 10, exY - 13, GetColor(0, 0, 0), TRUE);
				DrawBox(exX - 10, exY - 7, exX + 10, exY + 12, GetColor(0, 0, 0), TRUE);
				DrawBox(exX - 8, exY - 60, exX + 8, exY - 15, GetColor(255, 20, 20), TRUE);
				DrawBox(exX - 8, exY - 5, exX + 8, exY + 10, GetColor(255, 20, 20), TRUE);
			}
		}
	}

	// �Ή��u���X��8�b�ԔR��������n�ʂ̕`��
	DrawFireAndBurningGround();
}

void BossEnemyDragon::UpdateFireAndBurningGround(Player* player)
{
	const float floorY = 1903.0f;

	// 1. �󒆂��畬���ꂽ�Ή��u���X�e�̈ړ��E�v���C���[�����蔻��E�n�ʒ��e����
	for (auto& fp : mFireProjectiles)
	{
		if (!fp.active) continue;

		fp.x += fp.vx;
		fp.y += fp.vy;
		fp.animTick++;

		// �v���C���[�ւ̒�������
		if (player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float pLeft = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop = py - 50.0f;
			float pBot = py + 50.0f;

			if (fp.x + fp.radius > pLeft && fp.x - fp.radius < pRight &&
				fp.y + fp.radius > pTop && fp.y - fp.radius < pBot)
			{
				player->TakeDamage(mAttack);
			}
		}

		// �n�ʁiY >= 1903�j�܂��̓u���b�N�ɒ��e������A���̏ꏊ�̒n�ʂ�8�b�ԁi480�t���[���j�R�₷
		bool hitGround = (fp.y >= floorY - 6.0f);
		if (!hitGround && mpBlockMap != nullptr && fp.y >= 1750.0f)
		{
			if (mpBlockMap->GetCollisionType(static_cast<int>(fp.x), static_cast<int>(fp.y)) == BlockMap::CollisionType::Block)
			{
				hitGround = true;
			}
		}

		if (hitGround)
		{
			fp.active = false;
			float burnX = fp.x;
			if (burnX < 7950.0f) burnX = 7950.0f;
			if (burnX > 9820.0f) burnX = 9820.0f;

			// ���ɂ����߂��i70px�ȓ�j�ɔR�ăG���A������ꍇ�̓^�C�}�[��8�b�i480�t���[���j�ɍX�V���ē���
			bool merged = false;
			for (auto& bg : mBurningGrounds)
			{
				if (std::fabs(bg.x - burnX) < 70.0f)
				{
					bg.timer = 480; // 8�b�� (60fps �~ 8s)
					merged = true;
					break;
				}
			}

			if (!merged)
			{
				BurningGround bg;
				bg.x = burnX;
				bg.y = floorY;
				bg.halfWidth = 55.0f;
				bg.timer = 480;    // 8�b�� (60fps �~ 8s)
				bg.maxTimer = 480;
				mBurningGrounds.push_back(bg);
			}
		}
		else if (fp.x < 7800.0f || fp.x > 9900.0f || fp.y > 2100.0f)
		{
			fp.active = false;
		}
	}

	// ���������ꂽ�Ή��e��폜
	for (auto it = mFireProjectiles.begin(); it != mFireProjectiles.end(); )
	{
		if (!it->active) it = mFireProjectiles.erase(it);
		else ++it;
	}

	// 2. �n�ʂ̔R�ăG���A�i8�b�Ԏ����{�_���[�W����j�̍X�V
	for (auto it = mBurningGrounds.begin(); it != mBurningGrounds.end(); )
	{
		it->timer--;
		if (it->timer <= 0)
		{
			it = mBurningGrounds.erase(it);
			continue;
		}

		// �R���Ă���n�ʂ̃_���[�W����
		if (player && !player->GetIsDead())
		{
			float px = player->GetPosition().x;
			float py = player->GetPosition().y;
			float pLeft = px - 16.0f;
			float pRight = px + 16.0f;
			float pTop = py - 50.0f;
			float pBot = py + 50.0f;

			float fireLeft = it->x - it->halfWidth;
			float fireRight = it->x + it->halfWidth;
			float fireTop = it->y - 46.0f;
			float fireBot = it->y + 12.0f;

			if (pRight > fireLeft && pLeft < fireRight && pBot > fireTop && pTop < fireBot)
			{
				player->TakeDamage(mAttack);
			}
		}

		++it;
	}
}

void BossEnemyDragon::DrawFireAndBurningGround()
{
	if (mpBlockMap == nullptr) return;

	int now = GetNowCount();

	// 1. �󒆉Ή��u���X�̌����G�t�F�N�g�i�\������̉΂̕������˒��̉Ή��}�Y���t���b�V���j
	if (mState == State::AirAttack)
	{
		int mouthSX = static_cast<int>(ConvertToScreenX(mvPosition.x + mFacingDirection * (115.0f * mDrawScale), mpBlockMap));
		int mouthSY = static_cast<int>(ConvertToScreenY(mvPosition.y - 110.0f, mpBlockMap));

		SetDrawBlendMode(DX_BLENDMODE_ADD, 210);
		if (mAnimFrame < 3)
		{
			// ����z������Ō����ɉ����W�܂�\��
			int r = 10 + mAnimFrame * 6 + (now / 40) % 6;
			DrawCircle(mouthSX, mouthSY, r, GetColor(255, 80, 20), TRUE);
			DrawCircle(mouthSX, mouthSY, r / 2, GetColor(255, 220, 100), TRUE);
		}
		else if (mAnimFrame >= 3 && mAnimFrame <= 5)
		{
			// �΂𕬂��Ă�������̌�������
			int r = 28 + (now / 30) % 10;
			DrawCircle(mouthSX, mouthSY, r, GetColor(255, 60, 10), TRUE);
			DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.7f), GetColor(255, 160, 30), TRUE);
			DrawCircle(mouthSX, mouthSY, static_cast<int>(r * 0.4f), GetColor(255, 255, 180), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 2. 8�b�ԔR��������n�ʂ̕`��
	SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
	for (const auto& bg : mBurningGrounds)
	{
		int cx = static_cast<int>(ConvertToScreenX(bg.x, mpBlockMap));
		int cy = static_cast<int>(ConvertToScreenY(bg.y, mpBlockMap));
		int hw = static_cast<int>(bg.halfWidth);

		// �c��1�b�i60�t���[�������j�ŏ��X�ɉ����������Ȃ��ď����Ă���
		float scale = (bg.timer < 60) ? (static_cast<float>(bg.timer) / 60.0f) : 1.0f;
		if (scale < 0.15f) scale = 0.15f;

		// �n�ʂɍL����ܔM�̃x�[�X��
		DrawOval(cx, cy - 2, hw + 10, static_cast<int>(10 * scale) + 3, GetColor(220, 45, 10), TRUE);
		DrawOval(cx, cy - 2, hw, static_cast<int>(6 * scale) + 2, GetColor(255, 140, 20), TRUE);

		// �h��߂����̐�i�����̎O�p�`��d�˂ĉ���\���j
		const int numFlames = 9;
		for (int i = 0; i < numFlames; ++i)
		{
			float ratio = static_cast<float>(i) / static_cast<float>(numFlames - 1) - 0.5f; // -0.5 ~ +0.5
			int fx = cx + static_cast<int>(ratio * (hw * 1.75f));
			float wave1 = std::sin(now * 0.022f + i * 1.7f + bg.x * 0.05f);
			float wave2 = std::cos(now * 0.031f + i * 2.3f);

			int flameH = static_cast<int>((38.0f + 16.0f * wave1) * scale);
			int flameW = static_cast<int>(16.0f * scale) + 5;
			int tipX = fx + static_cast<int>(wave2 * 7.0f);

			// �O���̐Ԃ���
			DrawTriangle(
				fx - flameW, cy,
				fx + flameW, cy,
				tipX, cy - flameH,
				GetColor(255, 50, 10), TRUE
			);
			// ���Ԃ̃I�����W�̉�
			DrawTriangle(
				fx - (flameW * 2) / 3, cy,
				fx + (flameW * 2) / 3, cy,
				tipX, cy - (flameH * 3) / 4,
				GetColor(255, 150, 20), TRUE
			);
			// ����̉��F�`���M�̉�
			DrawTriangle(
				fx - flameW / 3, cy,
				fx + flameW / 3, cy,
				tipX, cy - flameH / 2,
				GetColor(255, 240, 130), TRUE
			);
		}

		// ��������΂̕��p�[�e�B�N��
		for (int p = 0; p < 4; ++p)
		{
			int seed = static_cast<int>(bg.x) + p * 97;
			int cycle = (now / 12 + seed) % 45;
			int px = cx + ((seed * 31) % (hw * 2)) - hw + static_cast<int>(std::sin(cycle * 0.25f) * 6.0f);
			int py = cy - static_cast<int>(cycle * 1.4f * scale) - 6;
			int pr = (cycle < 30) ? 3 : 2;
			DrawCircle(px, py, pr, GetColor(255, 190, 60), TRUE);
		}
	}

	// 3. ���Ē��̉Ή��u���X�e�̕`��
	for (const auto& fp : mFireProjectiles)
	{
		if (!fp.active) continue;

		int sx = static_cast<int>(ConvertToScreenX(fp.x, mpBlockMap));
		int sy = static_cast<int>(ConvertToScreenY(fp.y, mpBlockMap));
		int r = static_cast<int>(fp.radius);

		// �i�s�����t���֐L�т�Ή��̔�
		int tailX1 = static_cast<int>(ConvertToScreenX(fp.x - fp.vx * 1.2f, mpBlockMap));
		int tailY1 = static_cast<int>(ConvertToScreenY(fp.y - fp.vy * 1.2f, mpBlockMap));
		int tailX2 = static_cast<int>(ConvertToScreenX(fp.x - fp.vx * 2.4f, mpBlockMap));
		int tailY2 = static_cast<int>(ConvertToScreenY(fp.y - fp.vy * 2.4f, mpBlockMap));

		DrawCircle(tailX2, tailY2, static_cast<int>(r * 0.55f), GetColor(220, 40, 10), TRUE);
		DrawCircle(tailX1, tailY1, static_cast<int>(r * 0.8f), GetColor(255, 100, 15), TRUE);

		// �Ή��e�{�́i�O���E����E���M�R�A�j
		DrawCircle(sx, sy, r, GetColor(255, 55, 10), TRUE);
		DrawCircle(sx, sy, static_cast<int>(r * 0.72f), GetColor(255, 165, 30), TRUE);
		DrawCircle(sx, sy, static_cast<int>(r * 0.42f), GetColor(255, 250, 170), TRUE);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BossEnemyDragon::DrawHpBar() const
{
	// EntranceDrop中はHPバー非表示
	if (mState == State::EntranceDrop) return;
	// HP0かつSmoothHp0なら非表示
	if (mHp <= 0 && mSmoothHp <= 0.0f) return;

	// EntranceWait中は徐々に表示（後半120フレームでフェードイン）
	float fillMultiplier = 1.0f;
	if (mState == State::EntranceWait)
	{
		// mEntranceTimer は 240 -> 0 とカウントダウン
		int elapsed = 240 - mEntranceTimer;
		if (elapsed < 120) return; // 前半は非表示
		fillMultiplier = static_cast<float>(elapsed - 120) / 120.0f;
		if (fillMultiplier > 1.0f) fillMultiplier = 1.0f;
	}

	const int barX = ScreenSize::CenterX - 600;
	const int barY = ScreenSize::ScrrenHeight - 120;
	const int barW = 1200;
	const int barH = 40;

	int shakeX = 0, shakeY = 0;
	if (mHpShakeTimer > 0)
	{
		shakeX = (rand() % 11) - 5;
		shakeY = (rand() % 11) - 5;
	}
	int bx = barX + shakeX;
	int by = barY + shakeY;

	// 外枠（黒）
	DrawBox(bx - 6, by - 6, bx + barW + 6, by + barH + 6, GetColor(0, 0, 0), TRUE);
	// 空バー（暗いグレー）
	DrawBox(bx, by, bx + barW, by + barH, GetColor(50, 50, 50), TRUE);

	// スムーズ（赤）
	float smoothRatio = (mSmoothHp / static_cast<float>(mMaxHp)) * fillMultiplier;
	if (smoothRatio > 1.0f) smoothRatio = 1.0f;
	if (smoothRatio > 0.0f)
	{
		DrawBox(bx, by, bx + static_cast<int>(barW * smoothRatio), by + barH, GetColor(220, 50, 50), TRUE);
	}

	// 実HPバー（緑）
	float actualRatio = (static_cast<float>(mHp) / static_cast<float>(mMaxHp)) * fillMultiplier;
	if (actualRatio > 1.0f) actualRatio = 1.0f;
	if (actualRatio > 0.0f)
	{
		DrawBox(bx, by, bx + static_cast<int>(barW * actualRatio), by + barH, GetColor(50, 220, 50), TRUE);
	}

	// 枠線（白）
	DrawBox(bx, by, bx + barW, by + barH, GetColor(255, 255, 255), FALSE);

	// 名前テキスト（バー上部）
	if (fillMultiplier > 0.0f && Master::mpGameManager)
	{
		int nameAlpha = static_cast<int>(255.0f * fillMultiplier);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, nameAlpha);
		const int nameFz = 48;
		Master::mpGameManager->GetFontManager()->DrawDotString(
			bx, by - 52, nameFz, GetColor(255, 100, 80), "%s", "\x83\x68\x83\x89\x83\x53\x83\x93"
		);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}