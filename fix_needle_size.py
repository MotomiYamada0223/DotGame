import sys

try:
    with open('Source/NeedleTrap.h', 'r', encoding='utf-8-sig') as f:
        content = f.read()
except UnicodeDecodeError:
    with open('Source/NeedleTrap.h', 'r', encoding='cp932') as f:
        content = f.read()

# GetSizeX と GetSizeY をオーバーライド
new_funcs = """    virtual void UpdateStatusByProgress(GameProgress progress) override;

    // Player側の当たり判定処理（GetSize() / 6.0f）に合わせるため
    // 本来の表示サイズ（32 * 3 = 96）になるように逆算して返す
    virtual int GetSizeX() { return 32 * 3 * 6; }
    virtual int GetSizeY() { return 32 * 3 * 6; }
"""
content = content.replace("    virtual void UpdateStatusByProgress(GameProgress progress) override;", new_funcs)

with open('Source/NeedleTrap.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)
