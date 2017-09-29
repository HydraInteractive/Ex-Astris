#pragma once
#include <vector>

enum Buffs
{
	HEALING,
	DAMAGE
};

enum Debuffs
{
	POISON,
	BURNING
};

class BuffHandler
{
public:
	BuffHandler();
	~BuffHandler();
	void getActiveBuffs();
	void getActiveDebuffs();

private:
	std::vector<Buffs> activeBuffs;
	std::vector<Debuffs> activeDebuffs;
};