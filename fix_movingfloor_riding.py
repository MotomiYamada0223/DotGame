import sys
import re

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

old_check = """    // vC[̉Ɏ܂Ă邩
    bool inX = (pxRight > myLeft) && (pxLeft < myRight);
    
    // vC[̑Ȁʕt߂ɂ邩
    bool inY = (pyBottom >= myTop - 5.0f) && (pyBottom <= myTop + 15.0f);

    if (inX && inY) {"""

new_check = """    // X軸の判定（プレイヤーが床の幅に収まっているか）
    bool inX = (pxRight > myLeft) && (pxLeft < myRight);
    
    // Y軸の判定（プレイヤーの足が床の上面付近にあるか）
    // 落下速度が大きくて突き抜けるのを防ぐため、下方向の許容範囲を広く取る(+30.0f)
    bool inY = (pyBottom >= myTop - 10.0f) && (pyBottom <= myTop + 30.0f);

    // 乗ったと判定する条件：
    // 1. X軸とY軸の範囲内にいる
    // 2. プレイヤーが「落下中（あるいは静止）」である（GetVelocityY() >= 0.0f）
    // これにより下からジャンプで突き抜ける時に吸い付くのを防ぐ
    if (inX && inY && mpPlayer->GetVelocityY() >= 0.0f) {"""

# 正規表現で一気に置き換える
# 元のコメントが文字化けしている可能性があるので、部分的に一致させる
content = re.sub(r"\s*bool inX = \(pxRight > myLeft\).*?if \(inX && inY\) \{", '\n' + new_check, content, flags=re.DOTALL)

# SetVelocityY の復活
old_riding = """    if (inX && inY && mpPlayer->GetVelocityY() >= 0.0f) {
        mIsPlayerRiding = true;
        mpPlayer->SetForceGrounded(); // 接地フラグを強制的にTrueにする予約を入れる
        // Player側で処理されるため削除"""

new_riding = """    if (inX && inY && mpPlayer->GetVelocityY() >= 0.0f) {
        mIsPlayerRiding = true;
        mpPlayer->SetForceGrounded(); // 接地フラグを強制的にTrueにする予約を入れる
        mpPlayer->SetVelocityY(0.0f); // ★着地時の落下速度の残存による次フレームのすり抜けを防ぐため必須"""

content = content.replace(old_riding, new_riding)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

