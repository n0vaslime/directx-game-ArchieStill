#pragma once
#include "CMOGO.h"

class Sign : public CMOGO
{
public:
	Sign(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos);
	~Sign();

	virtual void Draw(DrawData* _DD) override;
	virtual void Tick(GameData* _GD) override;

	vector<CMOGO*> m_ReadingTrigger;
};

