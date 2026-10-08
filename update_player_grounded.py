import sys
import re

with open('Source/Player.h', 'r', encoding='utf-8-sig') as f:
    content = f.read()

setter_logic = """	void SetVelocityY(float vy) { velocityY = vy; }  // y@\z
	
	// 動く床などのオブジェクトに乗った際に接地状態を強制するためのSetter
	void SetGrounded(bool grounded) { 
		isGrounded = grounded; 
		if (grounded) {
			mbIsJumping = false;
		}
	}"""

content = re.sub(r"void SetVelocityY\(float vy\)\s*\{\s*velocityY = vy;\s*\}.*?\n", setter_logic + '\n', content)

with open('Source/Player.h', 'w', encoding='utf-8-sig') as f:
    f.write(content)
