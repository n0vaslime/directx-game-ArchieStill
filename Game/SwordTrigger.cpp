#include "pch.h"
#include "SwordTrigger.h"
#include <dinput.h>
#include "GameData.h"

SwordTrigger::SwordTrigger(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_fudge = Matrix::CreateRotationY(XM_PI);
	SetScale(Vector3(0.075f,-0.075f,0.05f));
}

SwordTrigger::~SwordTrigger()
{
}

void SwordTrigger::Tick(GameData* _GD)
{
	if (isRendered())
	{
		lifetime += _GD->m_dt;
		if (lifetime > 0.4f)
		{
			SetRendered(false);
			lifetime = 0;
		}
	}


	CMOGO::Tick(_GD);

}

void SwordTrigger::Draw(DrawData* _DD)
{
	// keep false - sword bounds table isn't drawn!
	if (false)
	{
		CMOGO::Draw(_DD);
	}
}