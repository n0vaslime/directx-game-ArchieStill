#pragma once
#include "CMOGO.h"

class Boss : public CMOGO
{
public:
	Boss(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);
	~Boss();

	virtual void Tick(GameData* _GD) override;
	virtual void Draw(DrawData* _DD) override;
	void BossFacing();
	void BossIntroduction(GameData* _GD);
	void BossHealth();
	void BossAttacking();

	CMOGO* pBossProjectile;

	float player_adjacent;
	float player_opposite;
	float player_pitch;

	int boss_health = 3;

	bool is_talking = true;
	bool play_combat_sfx = false;
	bool play_hurt_sfx = false;
	float hurt_lifetime = 0;

	float intro_talk = 0;
	float projectile_timer = 0;
	float projectile_lifetime = 0;

	bool is_dying;
};
