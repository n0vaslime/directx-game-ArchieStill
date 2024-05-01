#pragma once
#include "CMOGO.h"

class Enemy : public CMOGO
{
public:
	Enemy(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos);
	~Enemy();

	virtual void Tick(std::shared_ptr<GameData>) override;
	virtual void Draw(std::shared_ptr<DrawData>) override;
	void EnemyAI(std::shared_ptr<GameData> _GD);

	Vector3 base_pos;
	float speed;
	bool player_spotted;
	std::shared_ptr<CMOGO> EnemySensor;
	float player_facing;
};

