#pragma once
#include <DxLib.h>
#include <vector>

class BlockMap;
class Player;

class Minecart
{
public:
	Minecart();
	~Minecart();

	void Initialize();
	void Reset();
	void Update(BlockMap& block, Player* player);
	void Draw(int scrollX, int scrollY);

	//bool IsActive() const { return mbActive; }

private:

	enum class CartStagte {
		Waiting,
		RollingLeft,
		SupperBoostLeft,
		LurkingLeft,
		ReturningRight,
		Finished
	};
};