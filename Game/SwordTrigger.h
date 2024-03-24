#pragma once
#include "CMOGO.h"

class SwordTrigger : public CMOGO
{
public:
	SwordTrigger(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);
	~SwordTrigger();

	virtual void Tick(GameData* _GD) override;

protected:
	float lifetime = 0.0f;
};
