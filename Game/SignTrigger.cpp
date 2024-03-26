#include "pch.h"
#include "SignTrigger.h"
#include <dinput.h>
#include "GameData.h"

SignTrigger::SignTrigger(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
	SetScale(Vector3(1.5f, 0.4f, 2.5f));
}

SignTrigger::~SignTrigger()
{
}

void SignTrigger::Draw(DrawData* _DD)
{
	if (false)
		CMOGO::Draw(_DD);
}
