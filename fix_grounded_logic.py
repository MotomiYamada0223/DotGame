import sys
import re

with open('Source/Player.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()

setter_old = """	// 動く床などのオブジェクトに乗った際に接地状態を強制するためのSetter
	void SetGrounded(bool grounded) { 
		isGrounded = grounded; 
		if (grounded) {
			mbIsJumping = false;
		}
	}"""

setter_new = """	// 動く床などのオブジェクトに乗った際に接地状態を強制するためのSetter
	void SetForceGrounded() { 
		mForceGroundedThisFrame = true;
	}"""

content = content.replace(setter_old, setter_new)

if 'bool mForceGroundedThisFrame;' not in content:
    content = content.replace('bool isGrounded; // nʂɐڒnĂ邩ǂ', 'bool isGrounded;\n\tbool mForceGroundedThisFrame; // 外部オブジェクトにより強制接地させるフラグ')

with open('Source/Player.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)

with open('Source/Player.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

# コンストラクタで初期化
if 'mForceGroundedThisFrame = false;' not in content:
    content = content.replace('isGrounded = true;', 'isGrounded = true;\n\tmForceGroundedThisFrame = false;')

# UpdateMoveAndCollisionの直後でフラグを消費して上書き
update_call = """		);

	if (isBlinking && !mIsBlinkWallDeathImmune)"""

update_call_new = """		);

	// 動く床などによって強制接地フラグが立っている場合は、地形の判定を上書きする
	if (mForceGroundedThisFrame) {
		isGrounded = true;
		mbIsJumping = false;
		velocityY = 0.0f;
		mForceGroundedThisFrame = false; // 消費
	}

	if (isBlinking && !mIsBlinkWallDeathImmune)"""

content = content.replace(update_call, update_call_new)

with open('Source/Player.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

with open('Source/MovingFloor.cpp', 'r', encoding='utf-8-sig') as f:
    content = f.read()

content = content.replace('mpPlayer->SetGrounded(true); // 接地フラグを強制的にTrueにしてジャンプや歩行を可能にする', 'mpPlayer->SetForceGrounded(); // 接地フラグを強制的にTrueにする予約を入れる')
content = content.replace('mpPlayer->SetVelocityY(0.0f); // 重力リセット', '// Player側で処理されるため削除')

with open('Source/MovingFloor.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(content)

