import sys

def modify():
    with open('Source/Player.cpp', 'r', encoding='cp932', errors='ignore') as f:
        content = f.read()

    # コンストラクタ側
    old_init = "isHitDamage = false;\n\tmInvincibleTimer = 0;\n\tisDead = false;"
    if old_init in content:
        # これはOK
        pass
        
    # Update側
    old_upd = """// --- 当たり判定 ---
	isHitDamage = false;
	if (mInvincibleTimer > 0) mInvincibleTimer--;
	mInvincibleTimer = 0;"""
    new_upd = """// --- 当たり判定 ---
	isHitDamage = false;
	if (mInvincibleTimer > 0) mInvincibleTimer--;"""
    content = content.replace(old_upd, new_upd)
    
    # 描画側で点滅処理を入れる（無敵中は半透明にするなど）
    # 元々の isHitDamage で赤くする処理に加えて、mInvincibleTimer > 0 のときも点滅させたい。
    # しかし指示には「攻撃判定とダメージ処理」としかないので、まずはタイマーバグだけ直す。

    with open('Source/Player.cpp', 'w', encoding='cp932') as f:
        f.write(content)

modify()
