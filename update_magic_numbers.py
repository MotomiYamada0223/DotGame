import sys
import re

# 1. GameConstants.h の更新
with open('Source/GameConstants.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()

constants_code = """
// 動く床関連の定数
namespace MovingFloorConstants
{
	// プレイヤーが床に乗る判定の「Y軸（高さ）」のマージン
	static constexpr float RideMarginTop = 10.0f;    // 上側の許容範囲（めり込み許容）
	static constexpr float RideMarginBottom = 30.0f; // 下側の許容範囲（高速落下時のすり抜け防止）

	// FloorFeature::SpeedUpOnRide の際に上昇する速度の倍率
	static constexpr float SpeedUpMultiplier = 3.0f;

	// MovePattern::OneWayDestroy で画面外と判定して破棄するまでのマージン（ピクセル）
	static constexpr float DestroyMarginY = 200.0f;
}
"""

if 'namespace MovingFloorConstants' not in content:
    content = content + '\n' + constants_code

with open('Source/GameConstants.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)

# 2. MovingFloor.cpp の更新
with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# OneWayDestroy の 200.0f 置換
content = re.sub(
    r"if \(nextPos\.y < -200\.0f \|\| nextPos\.y > ScreenSize::ScrrenHeight \+ 200\.0f\) \{",
    "if (nextPos.y < -MovingFloorConstants::DestroyMarginY || nextPos.y > ScreenSize::ScrrenHeight + MovingFloorConstants::DestroyMarginY) {",
    content
)

# inY の 10.0f と 30.0f 置換
content = re.sub(
    r"bool inY = \(pyBottom >= myTop - 10\.0f\) && \(pyBottom <= myTop \+ 30\.0f\);",
    "bool inY = (pyBottom >= myTop - MovingFloorConstants::RideMarginTop) && (pyBottom <= myTop + MovingFloorConstants::RideMarginBottom);",
    content
)

# mBaseSpeed * 3.0f 置換
content = re.sub(
    r"mCurrentSpeed = mBaseSpeed \* 3\.0f;",
    "mCurrentSpeed = mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;",
    content
)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

