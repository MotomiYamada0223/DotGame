#include "EnemySkeleton.h"
#include "DxLib.h"
#include "BlockMap.h"
#include "Master.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "Player.h"
#include "Collision.h"
#include "GameConstants.h"
#include <cmath>
#include <algorithm>

// 敵キャラクター（スケルトン）の初期化処理
EnemySkeleton::EnemySkeleton(VECTOR initPos)
    : Enemy(CharacterGraphPath::Skeleton, initPos)
    , mAnimFrame(0)
    , mAnimTimer(0)
    , mMoveDirection(-1.0f)
    , mFacingDirection(-1.0f)
    , mActionTimer(0)
    , mAttackCooldown(30)
    , mDrawScale(0.75f)
    , mArrowHandle(-1)
    , mbArrowShotInThisAnim(false)
    , mState(State::Idle)
    , mIdleSETimer(0)
{
    // 進行度に応じたステータス（HP・攻撃力など）を設定する
    UpdateStatusByProgress(GameProgress::Tutorial2);

    // 待機アニメーション用の画像を連続して読み込む
    for (int i = 1; i <= 6; ++i)
    {
        char buf[256];
        sprintf_s(buf, "Resource/Enemy/skeleton_animation/skeleton_animation/skeleton_idle/skeleton_idle%d.png", i);
        mIdleAnim.push_back(LoadGraph(buf));
    }

    // 移動アニメーション用の画像を連続して読み込む
    for (int i = 1; i <= 7; ++i)
    {
        char buf[256];
        sprintf_s(buf, "Resource/Enemy/skeleton_animation/skeleton_animation/skeleton_walk/skeleton_walk%d.png", i);
        mMoveAnim.push_back(LoadGraph(buf));
    }

    // 攻撃アニメーション用の画像を連続して読み込む
    for (int i = 1; i <= 7; ++i)
    {
        char buf[256];
        sprintf_s(buf, "Resource/Enemy/skeleton_animation/skeleton_animation/skeleton_attack/skeleton_attack%d.png", i);
        mAttackAnim.push_back(LoadGraph(buf));
    }

    // 攻撃用の矢のグラフィックが未読込の場合にのみ読み込む
    if (mArrowHandle == -1)
    {
        mArrowHandle = LoadGraph("Resource/Enemy/skeleton_animation/skeleton_arrow.png");
    }

    // Scale-adjusted collision box (scale: 0.75)
    // スケール0.75を考慮した当たり判定サイズ
    mfEnemyWidth = 42.0f;
    mfEnemyHeight = 105.0f;
    moveSpeed = 0.8f;
    gravity = 0.5f;
}

// 敵キャラクターの破棄処理とリソース解放
EnemySkeleton::~EnemySkeleton()
{
    // 保持しているアニメーション用ハンドルを解放する
    for (int h : mIdleAnim) DeleteGraph(h);
    for (int h : mMoveAnim) DeleteGraph(h);
    for (int h : mAttackAnim) DeleteGraph(h);

    // 矢のグラフィックハンドルが存在する場合は解放する
    if (mArrowHandle != -1) {
        DeleteGraph(mArrowHandle);
        mArrowHandle = -1;
    }
}

// ゲームの進行度に応じてステータスを更新する
void EnemySkeleton::UpdateStatusByProgress(GameProgress progress)
{
    // 進行度ごとにHP・攻撃力・即死攻撃の有無を設定する
    switch (progress)
    {
    case GameProgress::Tutorial1:
    case GameProgress::PostTutorial1:
    case GameProgress::Tutorial2:
        // 序盤の進行度では同じステータスを設定する
        mMaxHp = 25; mHp = 25;
        mAttack = 15;
        mHasInstantKillAttack = false;
        mIsGiantInvincible = false;
        break;
    default:
        // それ以外の進行度でも現状は同じステータスを設定する
        mMaxHp = 25; mHp = 25;
        mAttack = 15;
        mHasInstantKillAttack = false;
        mIsGiantInvincible = false;
        break;
    }
}

// 毎フレームの更新処理
void EnemySkeleton::Update()
{
    // 基底クラス（Enemy）の共通更新処理を実行する
    Enemy::Update();

    // スケルトンの状態に関係なく、発射済みの矢の更新を行う
    GameScene* scene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
    Player* player = scene ? scene->GetPlayer() : nullptr;
    BlockMap* blockMap = mpBlockMap;
    // マップ情報が未設定の場合はシーンから取得する
    if (!blockMap && scene) blockMap = &scene->GetBlockMap();

    // 発射済みのすべての矢の位置や判定を更新する
    for (auto& arr : mArrows) {
        if (!arr.active) continue;

        // 矢の位置を速度分だけ移動させ、寿命を1減らす
        arr.x += arr.vx;
        arr.y += arr.vy;
        arr.life--;

        // 寿命が尽きた矢は非アクティブにする
        if (arr.life <= 0) {
            arr.active = false;
            continue;
        }

        // 地形ブロックとの衝突判定を行い、壁等に当たったら矢を消滅させる
        if (blockMap) {
            BlockMap::CollisionType cType = blockMap->GetCollisionType(static_cast<int>(arr.x), static_cast<int>(arr.y));
            if (cType == BlockMap::CollisionType::Block || cType == BlockMap::CollisionType::BreakableBlock) {
                arr.active = false;
                continue;
            }
        }

        // プレイヤーとの衝突判定を行い、当たった場合はダメージを与える
        if (player && !player->GetIsDead()) {
            VECTOR pPos = player->GetPosition();
            const float pW = static_cast<float>(PlayerConstants::PlayerCollisionWidth);
            const float pH = static_cast<float>(PlayerConstants::PlayerCollisionHeight);

            // プレイヤーの当たり判定の左上座標とサイズを求める（位置は中心基準）
            VECTOR myPos = VGet(pPos.x - pW * 0.5f, pPos.y - pH * 0.5f, 0.0f);
            VECTOR mySize = VGet(pW, pH, 0.0f);

            // Arrow collision box
            // 矢の当たり判定（矢の中心から左右20px、上下7pxの矩形）
            VECTOR arrPos = VGet(arr.x - 20.0f, arr.y - 7.0f, 0.0f);
            VECTOR arrSize = VGet(40.0f, 14.0f, 0.0f);

            // 矢がプレイヤーに当たった場合、ダメージと死因を設定して矢を消す
            if (Collision::CheckRectToRect(myPos, mySize, arrPos, arrSize)) {
                player->TakeDamage(5);
                player->SetDeathReason(DeathReason::DeathType::EnemySkeleton);
                arr.active = false;
                continue;
            }
        }
    }

    if (mHp > 0)
    {
        // プレイヤーとの距離を求める
        bool isPlayerNear = false;
        if (player && !player->GetIsDead())
        {
            float dx = player->GetPosition().x - mvPosition.x;
            float dy = player->GetPosition().y - mvPosition.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            isPlayerNear = (dist < 500.0f);
        }

        if (isPlayerNear)
        {
            mIdleSETimer++;
            if (mIdleSETimer >= 600)
            {
                mIdleSETimer = 0;
                Master::mpGameManager->GetSoundManager()->PlaySE(SoundManager::SE_SKELETON_IDEL, 60);
            }
        }
        else
        {
            // 離れている間はタイマーをリセットする
            mIdleSETimer = 0;
        }
    }
    else
    {
        mIdleSETimer = 0;
    }

    // 不要となった（非アクティブな）矢をコンテナから削除する
    mArrows.erase(std::remove_if(mArrows.begin(), mArrows.end(), [](const Arrow& a) { return !a.active; }), mArrows.end());
    if (mHp <= 0)
    {
        if (!isDeadBlinking)
        {
            // 死亡の最初のフレームだけ点滅と死亡SEを開始する
            isDeadBlinking = true;
            deadBlinkTimer = 60;
            Master::mpGameManager->GetSoundManager()->PlaySE(SoundManager::SE_SKELETON_DEATH);
        }

        deadBlinkTimer--;
        if (deadBlinkTimer <= 0)
        {
            SetDeleteFlag(true);
        }
        return;
    }

    // 状態に応じてアニメーションのフレーム更新間隔（ウェイト）を変更する
    int ticksPerFrame = 8;
    if (mState == State::Move) ticksPerFrame = 6;
    else if (mState == State::Attack) ticksPerFrame = 7;

    // 更新間隔に達したらアニメーションフレームを1つ進める
    mAnimTimer++;
    if (mAnimTimer >= ticksPerFrame) {
        mAnimTimer = 0;
        mAnimFrame++;

        // At frame 5, release the arrow!
        // Height is aligned exactly with the bow position in the animation:
        // In the 300x300 image, feet are at Y=242, arrow center is at Y=110.5.
        // Distance above feet = 131.5px. Scaled = 131.5f * mDrawScale.
        // 攻撃アニメーションの特定のタイミングで新規の矢を生成し発射する
        if (mState == State::Attack && mAnimFrame == 5 && !mbArrowShotInThisAnim) {
            mbArrowShotInThisAnim = true;
            Arrow arr;
            arr.facing = mFacingDirection;
            // 弓の位置（スケルトンの向いている側・足元からの高さ）に矢を配置する
            arr.x = mvPosition.x + mFacingDirection * (30.0f * mDrawScale);
            const float feetY = mvPosition.y + mfEnemyHeight * 0.5f;
            arr.y = feetY - (131.5f * mDrawScale);
            // 向いている方向へ一定速度で飛ばす
            arr.vx = mFacingDirection * 8.0f;
            arr.vy = 0.0f;
            arr.active = true;
            arr.life = 180;
            mArrows.push_back(arr);
        }

        // 攻撃アニメーションが終了したら待機状態に戻し、次の攻撃までのクールダウンを設定する
        if (mState == State::Attack && mAnimFrame >= static_cast<int>(mAttackAnim.size())) {
            mState = State::Idle;
            mAnimFrame = 0;
            mAnimTimer = 0;
            mActionTimer = 0;
            mbArrowShotInThisAnim = false;
            mAttackCooldown = 90; // 1.5s cooldown between bow shots
        }
    }
}

// 敵キャラクターおよび発射された矢の描画処理
void EnemySkeleton::Draw()
{
    // 描画に使うマップ情報を取得する（未設定ならシーンから取得）
    BlockMap* blockMap = mpBlockMap;
    if (!blockMap) {
        Scene* scene = Master::mpGameManager->GetSceneManager()->GetCurrentScene();
        GameScene* gscene = dynamic_cast<GameScene*>(scene);
        if (gscene) blockMap = &gscene->GetBlockMap();
    }

    // 1. Draw Active Arrows
    // 画面上に存在するアクティブな矢を描画する
    for (const auto& arr : mArrows) {
        if (!arr.active) continue;
        // ワールド座標を画面座標に変換する
        const int sx = static_cast<int>(ConvertToScreenX(arr.x, blockMap));
        const int sy = static_cast<int>(ConvertToScreenY(arr.y, blockMap));
        bool reverse = (arr.facing > 0.0f); // Default 矢.png faces left; reverse when flying right

        if (mArrowHandle != -1) {
            // Center of arrow graphic in 300x300 canvas is at (148, 110)
            // 矢画像の中心点を基準に回転・拡大して描画する
            DrawRotaGraph2(sx, sy, 148, 110, mDrawScale, 0.0, mArrowHandle, TRUE, reverse);
        }
        else {
            // 画像が読み込めなかった場合は代わりに線を描画する
            DrawLine(sx - 15, sy, sx + 15, sy, GetColor(220, 220, 220), 3);
        }
    }

    // 2. Draw Skeleton
    // スケルトン本体の描画アニメーションデータを取得する
    // 状態に応じて待機・移動・攻撃のアニメーション画像リストを選ぶ
    const std::vector<int>* anim = &mIdleAnim;
    if (mState == State::Move) anim = &mMoveAnim;
    else if (mState == State::Attack) anim = &mAttackAnim;

    // 画像が無い場合は描画しない
    if (anim->empty()) return;

    // 現在のフレーム番号を求める（攻撃中は最後のフレームで止める）
    int frameIdx = mAnimFrame % anim->size();
    if (mState == State::Attack) {
        frameIdx = mAnimFrame;
        if (frameIdx >= static_cast<int>(anim->size())) frameIdx = static_cast<int>(anim->size()) - 1;
    }
    int handle = (*anim)[frameIdx];

    // 死亡時の点滅表現用フラグ制御
    bool shouldDraw = true;
    if (isDeadBlinking) {
        // 6フレームごとに表示・非表示を切り替える
        if ((deadBlinkTimer / 6) % 2 == 0) shouldDraw = false;
    }

    // 被ダメージ時や死亡時の点滅中は赤色で描画する
    if (isDamaged || isDeadBlinking) SetDrawBright(255, 0, 0);
    else SetDrawBright(255, 255, 255);

    // 本体の画面上のX座標を求める
    const int screenX = static_cast<int>(ConvertToScreenX(mvPosition.x, blockMap));
    // Feet screen Y is at bottom of collision box (mvPosition.y + mfEnemyHeight * 0.5f)
    // In 300x300 image, feet are at Y = 242 (+92px below center 150)
    // Scale adjusts this visual distance to 92.0f * mDrawScale
    // 足元の位置から画像の中心がずれる分を補正して、画面上のY座標を求める
    const float feetWorldY = mvPosition.y + mfEnemyHeight * 0.5f;
    const int screenY = static_cast<int>(ConvertToScreenY(feetWorldY - (92.0f * mDrawScale), blockMap));

    // Default image faces left; reverse when facing right
    // 元画像は左向きなので、右向きのときは反転して描画する
    bool reverse = (mFacingDirection > 0.0f);

    // 描画フラグが有効な場合のみスプライトを描画する
    if (shouldDraw) {
        DrawRotaGraph(screenX, screenY, mDrawScale, 0.0, handle, TRUE, reverse);
    }

    // 描画状態（ブレンド・明るさ）を元に戻す
    if (isDeadBlinking) SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    SetDrawBright(255, 255, 255);
}

// 敵キャラクターの移動およびAI思考処理
void EnemySkeleton::EnemyMove(BlockMap& blockMap)
{
    // 死亡後は移動やAI処理を行わない
    if (mHp <= 0) return;
    mpBlockMap = &blockMap;

    // 攻撃クールダウンタイマーを進める
    if (mAttackCooldown > 0) {
        mAttackCooldown--;
    }

    // 攻撃中は移動を行わず、重力計算のみ適用する
    if (mState == State::Attack) {
        // Stationary bow draw and release; still subject to gravity
        // その場で弓を構えて放つため水平方向の移動はせず、重力のみ適用する
        mCharacterPhysics.UpdateMoveAndCollision(mvPosition, velocityY, isGrounded, mbIsJumping, blockMap, mfEnemyWidth, mfEnemyHeight, gravity, 0.0f, 0.0f);
        return;
    }

    // 現在のシーンからプレイヤーを取得する
    GameScene* scene = dynamic_cast<GameScene*>(Master::mpGameManager->GetSceneManager()->GetCurrentScene());
    Player* player = scene ? scene->GetPlayer() : nullptr;

    bool seePlayer = false;
    bool inShootingRange = false;

    // プレイヤーの位置を観測し、射程範囲内かどうかの判定を行う
    if (player && !player->GetIsDead()) {
        float dx = player->GetPosition().x - mvPosition.x;
        float dy = player->GetPosition().y - mvPosition.y;

        // Archer detection: within 550px horizontal, 200px vertical
        // 視界範囲内であればプレイヤーを認識し、方向を向く
        if (std::abs(dy) <= 200.0f && std::abs(dx) <= 550.0f) {
            seePlayer = true;
            mFacingDirection = (dx >= 0.0f) ? 1.0f : -1.0f;

            // Bow shooting trigger range: between 30px and 480px
            // 攻撃可能な距離かつクールダウンが明けていれば射撃状態に移行する
            if (std::abs(dx) <= 480.0f && std::abs(dx) >= 30.0f && mAttackCooldown <= 0) {
                inShootingRange = true;
            }
        }
    }

    // 射撃範囲内であれば攻撃状態に移行する
    if (inShootingRange) {
        mState = State::Attack;
        mAnimFrame = 0;
        mAnimTimer = 0;
        mbArrowShotInThisAnim = false;
        return;
    }

    float currentMove = 0.0f;
    // プレイヤーを発見しているが射程外の場合は近づくように移動する
    if (seePlayer) {
        // If spotted player but not shooting yet, walk toward player
        // 射程外ならプレイヤーに近づくように移動する
        mState = State::Move;
        mMoveDirection = mFacingDirection;
        moveSpeed = 1.2f;
        currentMove = mMoveDirection;
    }
    // プレイヤー未発見時は定期的に待機と移動を切り替える
    else {
        moveSpeed = 0.7f;
        mActionTimer++;
        if (mActionTimer > 150) {
            // 一定時間ごとに待機と移動を切り替える
            mActionTimer = 0;
            if (mState == State::Idle) {
                // 待機中なら、ランダムな方向へ移動を開始する
                mState = State::Move;
                mMoveDirection = (GetRand(1) == 0) ? -1.0f : 1.0f;
                mFacingDirection = mMoveDirection;
            }
            else {
                // 移動中なら待機に戻る
                mState = State::Idle;
            }
        }
        if (mState == State::Move) {
            currentMove = mMoveDirection;
        }
    }

    // Cliff detection ahead: prevent stepping off ledges or falling into pits
    // 崖からの落下を防ぐため、進行方向の足元にブロックが存在するか確認する
    if (mState == State::Move && isGrounded) {
        // 進行方向の少し先・足元の位置を調べる
        float checkX = mvPosition.x + mMoveDirection * (mfEnemyWidth * 0.5f + 10.0f);
        float checkY = mvPosition.y + mfEnemyHeight * 0.5f + 10.0f;

        BlockMap::CollisionType cType = blockMap.GetCollisionType(static_cast<int>(checkX), static_cast<int>(checkY));
        if (cType == BlockMap::CollisionType::None) {
            if (!seePlayer) {
                // 崖なら進行方向を反転する
                mMoveDirection *= -1.0f;
                mFacingDirection = mMoveDirection;
                currentMove = mMoveDirection;
            }
            else {
                // プレイヤーを追いかけている場合は崖の手前で止まる
                currentMove = 0.0f;
                mState = State::Idle;
            }
        }
    }

    float oldX = mvPosition.x;
    // 物理演算を用いて移動とコリジョン判定を更新する
    mCharacterPhysics.UpdateMoveAndCollision(mvPosition, velocityY, isGrounded, mbIsJumping, blockMap, mfEnemyWidth, mfEnemyHeight, gravity, moveSpeed, currentMove);

    // Wall detection
    // 移動中で壁に阻まれて進めなくなった場合、進行方向を反転させる
    if (mState == State::Move && isGrounded && oldX == mvPosition.x && currentMove != 0.0f) {
        if (!seePlayer) {
            mMoveDirection *= -1.0f;
            mFacingDirection = mMoveDirection;
        }
    }
}

bool EnemySkeleton::IsAttacking() const
{
    return false; // Damage is dealt by arrows, not melee hitbox
}

void EnemySkeleton::GetAttackHitbox(float& left, float& top, float& right, float& bottom) const
{
    // 近接攻撃は行わないため、すべて0にして返す
    left = top = right = bottom = 0.0f;
}