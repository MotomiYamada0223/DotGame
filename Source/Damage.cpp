#include "Damage.h"

void Damage::ApplyDamage(int& hp, int damage)
{
	if (hp <= 0)
		return;

	hp -= damage;

	if (hp < 0)
		hp = 0;
}