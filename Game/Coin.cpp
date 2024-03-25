#include "pch.h"
#include "Coin.h"
#include <dinput.h>
#include "GameData.h"

Coin::Coin(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_pos = _pos;
	SetScale(Vector3(0.15f, 0.2f, 0.2f));
}

Coin::~Coin()
{
}

void Coin::Tick(GameData* _GD)
{
	//spins!
	m_yaw += _GD->m_dt;

	CMOGO::Tick(_GD);
}
