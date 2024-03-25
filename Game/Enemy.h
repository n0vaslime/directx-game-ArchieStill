#pragma once
#include "CMOGO.h"

class Enemy : public CMOGO
{
public:
	Enemy(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos);
	~Enemy();

	virtual void Tick(GameData* _GD) override;
};

