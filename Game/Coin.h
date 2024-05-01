#pragma once
#include "CMOGO.h"

class Coin : public CMOGO
{
public:
	Coin(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos);
	~Coin();

	virtual void Tick(std::shared_ptr<GameData> _GD) override;
};
