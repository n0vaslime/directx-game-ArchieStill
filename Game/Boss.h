#pragma once
#include "CMOGO.h"

class Boss : public CMOGO
{
public:
	Boss(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);
	~Boss();

	virtual void Tick(GameData* _GD) override;
	virtual void Draw(DrawData* _DD) override;
	void BossAI(GameData* _GD);
	void BossIntroduction(GameData* _GD);

	float player_yaw;
	float player_pitch;

	bool is_talking;
	bool is_dying;
};
