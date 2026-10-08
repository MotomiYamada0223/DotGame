import sys
import re

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

old_move_logic = """    if (mIsPlayerRiding && mpPlayer) {
        VECTOR moveDelta = VSub(nextPos, prevPos);
        VECTOR playerPos = mpPlayer->GetPosition(); // GetPosition() \x81BPlayer.h\x9am\x81F\x8aK\x97v\x82\xc5\x82\xb7\x82\xaa\x81A\x91\xd1\x82\xe8 Object2D \x9cp\x82\xc8\x82\xcc\x82\xc5 GetPosition() / SetPosition() \x82\xcd\x82\xa0\x82\xe8\x81B
        playerPos = VAdd(playerPos, moveDelta);
        mpPlayer->SetPosition(playerPos);
    }"""

# コメント部分が文字化けしてうまく置換できないかもしれないので、ブロック全体を正規表現で探して置換する
new_move_logic = """    if (mIsPlayerRiding && mpPlayer) {
        VECTOR moveDelta = VSub(nextPos, prevPos);
        VECTOR playerPos = mpPlayer->GetPosition();
        
        // X座標は床の移動量分だけ追従させる
        playerPos.x += moveDelta.x;
        
        // Y座標は毎フレームの重力加算による「めり込み」を防止するため、床の上にぴったり強制配置する
        playerPos.y = nextPos.y - mHeight / 2.0f - PlayerConstants::PlayerCollisionHeight / 2.0f;
        
        mpPlayer->SetPosition(playerPos);
    }"""

# 実際はre.subで置換
content = re.sub(r"    if \(mIsPlayerRiding && mpPlayer\) \{.*?\n    \}", new_move_logic, content, flags=re.DOTALL)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

