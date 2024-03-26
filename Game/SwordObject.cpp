#include "pch.h"
#include "SwordObject.h"
#include <dinput.h>
#include "GameData.h"

SwordObject::SwordObject(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_fudge = Matrix::CreateRotationY(XM_PI);
	SetScale(Vector3(0.15f, 0.2f, 0.2f));
}

SwordObject::~SwordObject()
{
}

void SwordObject::Tick(GameData* _GD)
{
	CMOGO::Tick(_GD);
}
