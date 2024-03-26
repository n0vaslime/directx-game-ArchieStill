#pragma once
#include "CMOGO.h"

class SwordObject : public CMOGO
{
public:
	SwordObject(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);
	~SwordObject();

	virtual void Tick(GameData* _GD) override;
};

