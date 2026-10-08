import sys

with open('Source/TestScene.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

content = content.replace(
    '2.0f, MovePattern::Loop, FloorFeature::Normal, mpPlayer',
    '2.0f, MovePattern::Loop, FloorFeature::Normal, mpPlayer, &mBlockMap'
)
content = content.replace(
    '1.5f, MovePattern::Loop, FloorFeature::SpeedUpOnRide, mpPlayer',
    '1.5f, MovePattern::Loop, FloorFeature::SpeedUpOnRide, mpPlayer, &mBlockMap'
)

with open('Source/TestScene.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)
