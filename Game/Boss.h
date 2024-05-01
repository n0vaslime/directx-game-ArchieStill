#pragma once
#include "CMOGO.h"

class Boss : public CMOGO
{
public:
	Boss(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);
	~Boss();

	virtual void Tick(std::shared_ptr<GameData> _GD) override;
	virtual void Draw(std::shared_ptr<DrawData> _DD) override;
	void BossFacing();
	void BossIntroduction(std::shared_ptr<GameData> _GD);
	void BossEnding(std::shared_ptr<GameData> _GD);

	void BossHealth();
	void BossAttacking();

	std::shared_ptr<CMOGO> pBossProjectile;

	float player_adjacent;
	float player_opposite;
	float player_pitch;

	int boss_health = 3;

	bool play_combat_sfx = false;
	bool play_hurt_sfx = false;
	float hurt_lifetime = 0;

	bool is_talking = true;
	float intro_talk = 0;

	bool is_dying = false;
	bool dying_words = false;
	float dying_time = 0;

	float projectile_timer = 0;
	float projectile_lifetime = 0;
};
