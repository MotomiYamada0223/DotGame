#pragma once
#include <DxLib.h>
#include <vector>
#include "Player.h"
#include "BlockMap.h"

class LeafCutter
{

private:


public:

	struct LeafParticle
	{
		float x;
		float y;
		float fallSpeed; // —‰º‘¬“x
		float swaySpeed; // —h‚ê‚Ìi‚Ş‘¬“x
		float swayAmplitude; // ‰¡—h‚ê‚ÌU‚ê•
		float phase; // —h‚ê‚Ìƒ^ƒCƒ~ƒ“ƒO
		float rotation; // ‰ñ“]
		float rotationSpeed; // ‰ñ“]‘¬“x
		float scale;   // Šg‘åk¬
		bool isShadow; // ‰e‚È‚çtrue
	};

	LeafCutter();
	~LeafCutter();

	void LeafDraw(BlockMap& block);
	void LeafUpdate(BlockMap& block, Player* player);
	void LeafReset();

private:
	 // ƒƒ“ƒo‚ÌŠÖ”
	void LeafCollision(Player* player);
	void LeafDebug();


	int mnLeafHandle;
	int mnLeafShadowHandle;
	bool mbStopSpawning; // ¶¬‚ğ~‚ß‚éƒtƒ‰ƒO

	std::vector<LeafParticle> mLeaves;

};