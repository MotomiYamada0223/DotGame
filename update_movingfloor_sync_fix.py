import sys

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    lines = f.readlines()

new_lines = []
skip = False
for i, line in enumerate(lines):
    if skip:
        # else { の後、次の } までスキップする等の処理をしたいが
        # 行単位で安全に置換する
        pass

# 一旦全体を文字列として処理する
content = "".join(lines)

import re

# 加速部分の置換
old_accel_pattern = r"\s*//[^\n]*\n\s*if \(mFeature == FloorFeature::SpeedUpOnRide\) \{\s*mCurrentSpeed = mBaseSpeed \* MovingFloorConstants::SpeedUpMultiplier;[^\n]*\n\s*\}"
accel_code = """
        // 乗った時に加速する機能（一度加速したら降りても戻らない、かつ同じ動きの他の床も連動する）
        if (mFeature == FloorFeature::SpeedUpOnRide && !mIsSpedUp) {
            mIsSpedUp = true;
            mCurrentSpeed = mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
            
            // 同じ方向・同じ速度の他の床も一斉に加速させる
            for (MovingFloor* floor : s_AllMovingFloors) {
                if (floor != this && floor->mFeature == FloorFeature::SpeedUpOnRide && 
                    floor->mVelocity.x == this->mVelocity.x && floor->mVelocity.y == this->mVelocity.y) {
                    floor->mCurrentSpeed = floor->mBaseSpeed * MovingFloorConstants::SpeedUpMultiplier;
                    floor->mIsSpedUp = true;
                }
            }
        }"""
content = re.sub(old_accel_pattern, accel_code, content)

# 降りたときのリセットを削除
old_reset_pattern = r"\s*//[^\n]*\n\s*if \(mFeature == FloorFeature::SpeedUpOnRide\) \{\s*mCurrentSpeed = mBaseSpeed;\s*\}"
content = re.sub(old_reset_pattern, "", content)

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

