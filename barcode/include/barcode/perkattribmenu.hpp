#include <barcode/menustate.hpp>
#include <hydra/world/world.hpp>
#include <imgui/imgui.h>
#include <fstream>
#include <iostream>

class PerkAttribMenu
{
public:
	PerkAttribMenu();
	~PerkAttribMenu();
	void render(bool& thing, Hydra::Renderer::Batch* renderBatch,float delta);
	
	struct BulletInfo
	{
		float dmg = 0.0f;
		float recoil = 0.0f;
		float ammoCap = 0.0f;
		float bulletSpread = 0.0f;
		float bulletSize = 0.5f;
		float roundsPerMinute = 0.0f;
		float reloadTime = 0.0f;
		float glowIntensity = 0.0f;
		int mesh = 0;

		float bulletColor[4] = { 1.0f };
		int currentMagAmmo = 0;
		int bulletPerShot = 0;
		int ammoPerShot = 0;

		char perkDescription[128] = "";
		char perkName[128] = "";

		//if true = multiplier, if false add
		bool Multiplier = true;
		bool Adder = false;
		bool glow = false;

	}Bullet;
	


	bool openPopup = false;

	void updateBullet(BulletInfo &bullet);

private:
	
	void writeToFile(const char* fileName);

};