import sys

with open('Source/Player.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()

target = "void SetVelocityY(float vy) { velocityY = vy; }"
replacement = """void SetVelocityY(float vy) { velocityY = vy; }

	// 動く床などのオブジェクトに乗った際に接地状態を強制するためのSetter
	void SetGrounded(bool grounded) { 
		isGrounded = grounded; 
		if (grounded) {
			mbIsJumping = false;
		}
	}"""

content = content.replace(target, replacement)

with open('Source/Player.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)
