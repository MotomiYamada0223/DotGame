import sys

with open('Source/TestScene.cpp', 'r', encoding='utf-8-sig') as f:
    lines = f.readlines()

insert_idx = -1
for i, line in enumerate(lines):
    if "MovingFloor* floor1 = new MovingFloor(" in line:
        insert_idx = i
        break

if insert_idx != -1:
    spawner_code = [
        "\t// 上から下へ無限に降ってくる足場のスポナーを配置\n",
        "\t// 始点: 6300, -100\n",
        "\t// 速度: Y軸に3.0\n",
        "\t// 間隔: 5秒 = 300フレーム (60fps想定)\n",
        "\tnew FloorSpawner(\n",
        "\t\tVGet(6300.0f, -100.0f, 0.0f),\n",
        "\t\tVGet(0.0f, 3.0f, 0.0f),\n",
        "\t\t300,\n",
        "\t\tmpPlayer,\n",
        "\t\t&mBlockMap\n",
        "\t);\n\n"
    ]
    for line in reversed(spawner_code):
        lines.insert(insert_idx, line)

with open('Source/TestScene.cpp', 'w', encoding='utf-8-sig') as f:
    f.writelines(lines)
