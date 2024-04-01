#pragma once
#include "CMOGO.h"
#include "Player.h"

class Enemy : public CMOGO
{
public:
	Enemy(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll);
	~Enemy();

	virtual void Tick(GameData* _GD) override;
	virtual void Draw(DrawData* _DD) override;
	void EnemyAI(GameData* _GD);

	float speed;
	bool player_spotted;
	CMOGO* EnemySensor;
	float player_facing;
};

