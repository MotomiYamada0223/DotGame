import sys

try:
    with open('Source/EnemySlime.cpp', 'r', encoding='utf-8-sig') as f:
        lines = f.readlines()
except UnicodeDecodeError:
    with open('Source/EnemySlime.cpp', 'r', encoding='cp932') as f:
        lines = f.readlines()

new_lines = []
in_move = False

for line in lines:
    if "void EnemySlime::EnemyMove(BlockMap& blockMap)" in line:
        in_move = True
        new_lines.append(line)
        new_lines.append("""{
    mpBlockMap = &blockMap;
    
    ObjectManager* objManager = Master::mpGameManager->GetSceneManager()->GetCurrentScene()->GetObjectManager();
    Object2D* playerObj = objManager->GetObject2DByTag(Object2D::Tag::Player2D);
    
    mVisionLength = 600.0f;
    mVisionBaseHeight = 50.0f;
    mVisionAngle = 15.0f;
    
    if (playerObj != nullptr)
    {
        float playerX = playerObj->GetPosition().x;
        float playerY = playerObj->GetPosition().y;
        float enemyX = mvPosition.x;
        float enemyY = mvPosition.y;
        
        if (mIsChasing)
        {
            // --- 追跡中の処理 ---
            // 常にプレイヤーの方向に視界(自分の向き)と移動方向を合わせる（振り向く）
            if (playerX > enemyX) {
                mIsFacingRight = true;
                mCurrentMoveDirection = 1.0f;
            } else {
                mIsFacingRight = false;
                mCurrentMoveDirection = -1.0f;
            }
            
            // 逃げ切られたかの判定（距離か、扇状のY幅から外れたか）
            float dx = std::abs(playerX - enemyX);
            float dy = std::abs(playerY - enemyY);
            float maxDy = (mVisionBaseHeight / 2.0f) + dx * std::tan(mVisionAngle * 0.01745329f);
            
            // 視界の長さから逃げ切られたか、高さ(ジャンプ等)で視界から外れたら見失う
            if (dx > mVisionLength || dy > maxDy)
            {
                mIsChasing = false;
            }
        }
        else
        {
            // --- 未発見（徘徊中）の処理 ---
            // 扇状の視界判定
            float dx = mIsFacingRight ? (playerX - enemyX) : (enemyX - playerX);
            float dy = std::abs(playerY - enemyY);
            
            // 向きの方向にいて、長さ以内か
            if (dx >= 0.0f && dx <= mVisionLength)
            {
                float maxDy = (mVisionBaseHeight / 2.0f) + dx * std::tan(mVisionAngle * 0.01745329f);
                
                if (dy <= maxDy)
                {
                    mIsChasing = true;
                    // 発見した瞬間に向きを更新
                    mCurrentMoveDirection = mIsFacingRight ? 1.0f : -1.0f;
                }
            }
        }
    }
    
    if (mIsChasing)
    {
        mMoveSpeed = 2.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(7);
    }
    else
    {
        mMoveSpeed = 1.0f;
        if (mpAnimMove) mpAnimMove->SetInterval(8);
    }

    // 追跡中でない場合はランダム徘徊
    if (!mIsChasing)
    {
        mActionTimer--;
        if (mActionTimer <= 0)
        {
            int randAction = GetRand(2); // 0, 1, 2
            if (randAction == 0) mCurrentMoveDirection = 1.0f;
            else if (randAction == 1) mCurrentMoveDirection = -1.0f;
            else mCurrentMoveDirection = 0.0f; // 停止
            
            int loopCount = 1 + GetRand(2); // 1, 2, 3
            mActionTimer = loopCount * 48;
        }
        
        // 向きの保存
        if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
        else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;
    }
    else
    {
        // 追跡中も向きを更新（プレイヤーを追い越した時のため）
        if (mCurrentMoveDirection > 0.0f) mIsFacingRight = true;
        else if (mCurrentMoveDirection < 0.0f) mIsFacingRight = false;
    }

    mCharacterPhysics.UpdateMoveAndCollision(
        mvPosition,
        velocityY,
        isGrounded,
        mbIsJumping,
        blockMap,
        mfEnemyWidth,
        mfEnemyHeight,
        gravity,
        mMoveSpeed,
        mCurrentMoveDirection
    );
}\n""")
        continue

    if in_move and line.startswith("void EnemySlime::UpdateStatusByProgress"):
        in_move = False
        
    if not in_move:
        new_lines.append(line)

with open('Source/EnemySlime.cpp', 'w', encoding='utf-8-sig') as f:
    f.writelines(new_lines)

