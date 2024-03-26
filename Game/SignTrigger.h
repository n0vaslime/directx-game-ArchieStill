#pragma once
#include "CMOGO.h"

class SignTrigger : public CMOGO
{
public:
	SignTrigger(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);
	~SignTrigger();

	virtual void Draw(DrawData* _DD) override;
};

