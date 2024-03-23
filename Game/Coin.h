#pragma once
#include "CMOGO.h"

class Coin : public CMOGO
{
public:
	Coin(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, Vector3 _scale);
	~Coin();

	virtual void Tick(GameData* _GD) override;
};

