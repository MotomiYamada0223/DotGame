#include "CrumbleFloor.h"
#include "GameConstants.h"
#include "Player.h"
#include "BlockMap.h"
#include <cmath>
#include <algorithm>

std::vector<CrumbleFloor*> CrumbleFloor::s_AllCrumbleFloors;

CrumbleFloor::CrumbleFloor(VECTOR startPos, Player* player, BlockMap* blockMap, std::string imagePath)
    : Object2D(startPos)
    , mpPlayer(player)
    , mpBlockMap(blockMap)
    , mStartPosition(startPos)
    , mState(State::Waiting)
    , mVelocityY(0.0f)
    , mIsPlayerRiding(false)
{
    s_AllCrumbleFloors.push_back(this);

    // 画像の読み込み（デフォルトはFeintFloor画像）
    std::string path = imagePath.empty() ? CharacterGraphPath::FeintFloor : imagePath;
    mImageHandle = LoadGraph(path.c_str());
    int w = 0, h = 0;
    GetGraphSize(mImageHandle, &w, &h);
    mWidth = static_cast<float>(w);
    mHeight = static_cast<float>(h);

    InitFragments();
}

CrumbleFloor::~CrumbleFloor()
{
    auto it = std::find(s_AllCrumbleFloors.begin(), s_AllCrumbleFloors.end(), this);
    if (it != s_AllCrumbleFloors.end())
    {
        s_AllCrumbleFloors.erase(it);
    }

    if (mImageHandle != -1)
    {
        DeleteGraph(mImageHandle);
        mImageHandle = -1;
    }
}

void CrumbleFloor::InitFragments()
{
    mFragments.clear();

    const int cols = 3;
    const int rows = 4;
    const int colW = 30; // 30 * 3 = 90
    const int rowH[4] = { 30, 30, 30, 32 }; // sum = 122
    int currentSrcY = 0;

    for (int r = 0; r < rows; ++r)
    {
        int h = rowH[r];
        for (int c = 0; c < cols; ++c)
        {
            Fragment f;
            f.srcX = c * colW;
            f.srcY = currentSrcY;
            f.width = colW;
            f.height = h;

            // 中心からのオフセット
            f.offsetX = (-mWidth / 2.0f) + f.srcX + f.width / 2.0f;
            f.offsetY = (-mHeight / 2.0f) + f.srcY + f.height / 2.0f;

            f.posX = mvPosition.x + f.offsetX;
            f.posY = mvPosition.y + f.offsetY;
            f.vx = 0.0f;
            f.vy = 0.0f;
            f.gravity = 0.0f;
            f.angle = 0.0f;
            f.rotSpeed = 0.0f;
            f.alpha = 255.0f;

            mFragments.push_back(f);
        }
        currentSrcY += h;
    }
}

void CrumbleFloor::StartCrumbling()
{
    if (mState != State::Waiting) return;

    mState = State::Falling;
    mIsPlayerRiding = false;

    const int cols = 3;
    const int rows = 4;

    for (size_t i = 0; i < mFragments.size(); ++i)
    {
        auto& f = mFragments[i];
        int c = static_cast<int>(i % cols);
        int r = static_cast<int>(i / cols);

        // 初期位置を中心＋オフセットに再設定
        f.posX = mvPosition.x + f.offsetX;
        f.posY = mvPosition.y + f.offsetY;
        f.angle = 0.0f;
        f.alpha = 255.0f;

        // 横方向への拡散初速（左右へばらばらに散る）
        if (c == 0)
        {
            f.vx = -3.0f - 1.2f * static_cast<float>(rows - 1 - r);
            f.rotSpeed = -0.06f - 0.02f * static_cast<float>(r);
        }
        else if (c == 1)
        {
            f.vx = (r % 2 == 0) ? -1.0f : 1.0f;
            f.rotSpeed = (r % 2 == 0) ? -0.04f : 0.04f;
        }
        else
        {
            f.vx = 3.0f + 1.2f * static_cast<float>(rows - 1 - r);
            f.rotSpeed = 0.06f + 0.02f * static_cast<float>(r);
        }

        // 縦方向の初速（上部の破片は少し上空に跳ね上がり、下部は下へ素早く落ちる）
        float baseVy = -4.5f + static_cast<float>(r) * 2.0f;
        if (c == 1)
        {
            baseVy -= 1.0f;
        }
        f.vy = baseVy;

        // 重力加速度（下部ほど重力で先に落ちていく）
        f.gravity = 0.7f + 0.06f * static_cast<float>(r);
    }
}

void CrumbleFloor::Update()
{
    if (IsDeleteFlag()) return;

    // プレイヤーが死亡している場合は初期状態へ自動リセット
    if (mpPlayer != nullptr && mpPlayer->GetIsDead())
    {
        Reset();
        return;
    }

    if (mState == State::Waiting)
    {
        CheckPlayerRiding();
    }
    else if (mState == State::Falling)
    {
        bool anyVisible = false;

        for (auto& f : mFragments)
        {
            // 物理挙動の更新
            f.vx *= 0.99f; // 空気抵抗
            f.vy += f.gravity;
            if (f.vy > CrumbleFloorConstants::MaxFallSpeed)
            {
                f.vy = CrumbleFloorConstants::MaxFallSpeed;
            }

            f.posX += f.vx;
            f.posY += f.vy;
            f.angle += f.rotSpeed;

            // 一定距離落下したらフェードアウト
            float fallenDistance = f.posY - (mStartPosition.y + f.offsetY);
            if (fallenDistance > CrumbleFloorConstants::DestroyDistanceY * 0.7f)
            {
                float fadeRatio = (fallenDistance - CrumbleFloorConstants::DestroyDistanceY * 0.7f) /
                                  (CrumbleFloorConstants::DestroyDistanceY * 0.3f);
                f.alpha = 255.0f * (1.0f - (std::min)(1.0f, (std::max)(0.0f, fadeRatio)));
            }

            if (fallenDistance < CrumbleFloorConstants::DestroyDistanceY && f.alpha > 0.0f)
            {
                anyVisible = true;
            }
        }

        if (!anyVisible)
        {
            mState = State::Destroyed;
        }
    }
}

void CrumbleFloor::CheckPlayerRiding()
{
    if (!mpPlayer || mState != State::Waiting) return;

    VECTOR pPos = mpPlayer->GetPosition();
    float pVy = mpPlayer->GetVelocityY();

    // 当たり判定の領域（上面は削り値を反映）
    float myTop = mvPosition.y - mHeight / 2.0f + CrumbleFloorConstants::CollisionTopOffset;
    float myBottom = mvPosition.y + mHeight / 2.0f;
    float myLeft = mvPosition.x - mWidth / 2.0f;
    float myRight = mvPosition.x + mWidth / 2.0f;

    float pWidth = PlayerConstants::PlayerCollisionWidth;
    float pHeight = PlayerConstants::PlayerCollisionHeight;

    float pLeft = pPos.x - pWidth / 2.0f;
    float pRight = pPos.x + pWidth / 2.0f;
    float pTop = pPos.y - pHeight / 2.0f;
    float pBottom = pPos.y + pHeight / 2.0f;

    // AABB による四角形の重なり判定
    if (pRight > myLeft && pLeft < myRight && pBottom > myTop && pTop < myBottom)
    {
        float overlapLeft = pRight - myLeft;
        float overlapRight = myRight - pLeft;
        float overlapTop = pBottom - myTop;
        float overlapBottom = myBottom - pTop;

        float min1 = (std::min)(overlapLeft, overlapRight);
        float min2 = (std::min)(overlapTop, overlapBottom);
        float minOverlap = (std::min)(min1, min2);

        bool isBlinking = mpPlayer->IsBlinking();
        bool isImmune = mpPlayer->mIsBlinkWallDeathImmune;

        // 足場の上面に乗ったかの判定
        bool isRidingOnTop = (minOverlap == overlapTop) || (pVy >= 0.0f && pPos.y < myTop);

        if (isRidingOnTop && pVy >= 0.0f)
        {
            // 踏んだらプレイヤーの重力と落下速度をその瞬間だけリセット
            pPos.y = myTop - pHeight / 2.0f;
            mpPlayer->SetPosition(pPos);
            mpPlayer->SetVelocityY(0.0f);
            mpPlayer->SetForceGrounded();

            // 即座にばらばらに壊れながら落下開始！
            StartCrumbling();
        }
        else if (minOverlap == overlapBottom)
        {
            // 下から頭をぶつけた
            pPos.y = myBottom + pHeight / 2.0f;
            if (pVy < 0.0f)
            {
                mpPlayer->SetVelocityY(0.0f);
            }
            if (isBlinking && !isImmune)
            {
                mpPlayer->TakeDamage(mpPlayer->GetHp());
            }
            mpPlayer->SetPosition(pPos);
        }
        else if (minOverlap == overlapLeft)
        {
            // 左から壁としてぶつかった
            pPos.x = myLeft - pWidth / 2.0f;
            if (isBlinking && !isImmune)
            {
                mpPlayer->TakeDamage(mpPlayer->GetHp());
            }
            mpPlayer->SetPosition(pPos);
        }
        else if (minOverlap == overlapRight)
        {
            // 右から壁としてぶつかった
            pPos.x = myRight + pWidth / 2.0f;
            if (isBlinking && !isImmune)
            {
                mpPlayer->TakeDamage(mpPlayer->GetHp());
            }
            mpPlayer->SetPosition(pPos);
        }
        else
        {
            if (isBlinking)
            {
                if (pPos.x < mvPosition.x)
                {
                    pPos.x = myLeft - pWidth / 2.0f;
                }
                else
                {
                    pPos.x = myRight + pWidth / 2.0f;
                }
                if (!isImmune)
                {
                    mpPlayer->TakeDamage(mpPlayer->GetHp());
                }
                mpPlayer->SetPosition(pPos);
            }
            else
            {
                // 上面付近接触時はプレイヤーのリセットと崩落開始
                pPos.y = myTop - pHeight / 2.0f;
                mpPlayer->SetPosition(pPos);
                mpPlayer->SetVelocityY(0.0f);
                mpPlayer->SetForceGrounded();

                StartCrumbling();
            }
        }
    }
}

void CrumbleFloor::Draw()
{
    if (IsDeleteFlag() || mState == State::Destroyed) return;

    if (mpBlockMap == nullptr || mImageHandle == -1) return;

    if (mState == State::Waiting)
    {
        // 待機中は一枚絵として綺麗に描画
        float screenX = ConvertToScreenX(mvPosition.x, mpBlockMap);
        float screenY = ConvertToScreenY(mvPosition.y, mpBlockMap);
        DrawRotaGraph(static_cast<int>(screenX), static_cast<int>(screenY), 1.0, 0.0, mImageHandle, TRUE);
    }
    else if (mState == State::Falling)
    {
        // 崩壊中は破片を回転・フェードさせながら個別に描画
        for (const auto& f : mFragments)
        {
            if (f.alpha <= 0.0f) continue;

            float screenX = ConvertToScreenX(f.posX, mpBlockMap);
            float screenY = ConvertToScreenY(f.posY, mpBlockMap);

            if (f.alpha < 255.0f)
            {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(f.alpha));
            }

            DrawRectRotaGraphF(
                screenX, screenY,
                f.srcX, f.srcY,
                f.width, f.height,
                1.0,
                static_cast<double>(f.angle),
                mImageHandle,
                TRUE
            );

            if (f.alpha < 255.0f)
            {
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }
        }
    }
}

void CrumbleFloor::Reset()
{
    mvPosition = mStartPosition;
    mVelocityY = 0.0f;
    mState = State::Waiting;
    mIsPlayerRiding = false;
    InitFragments();
}

void CrumbleFloor::ResetAll()
{
    for (auto* floor : s_AllCrumbleFloors)
    {
        if (floor != nullptr)
        {
            floor->Reset();
        }
    }
}
