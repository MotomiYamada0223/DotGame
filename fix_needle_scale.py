import sys

try:
    with open('Source/NeedleTrap.cpp', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.cpp', 'r', encoding='cp932') as f:
        content = f.read()

# スケールを 2.0 に変更
old_draw = "DrawRotaGraph(drawX, drawY, 1.0, 0.0, mGraphHandle, TRUE);"
new_draw = "DrawRotaGraph(drawX, drawY, 2.0, 0.0, mGraphHandle, TRUE);"
content = content.replace(old_draw, new_draw)

# もし画像がロードされていなかった場合のために、画像パスの再確認
# GameConstants などをいじらない

with open('Source/NeedleTrap.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)
