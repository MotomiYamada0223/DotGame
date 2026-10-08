import sys
import re

with open('Source/FloorSpawner.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

content = content.replace(
    'MovePattern::OneWayDestroy, \n            FloorFeature::Normal, \n            mpPlayer,',
    'MovePattern::OneWayDestroy, \n            mFeature, \n            mpPlayer,'
)

with open('Source/FloorSpawner.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

