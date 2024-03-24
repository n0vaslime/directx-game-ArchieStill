#include "pch.h"
#include "SwordTrigger.h"
#include <dinput.h>
#include "GameData.h"

SwordTrigger::SwordTrigger(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_fudge = Matrix::CreateRotationY(XM_PI);

	SetRendered(false);
	SetScale(Vector3::One * 0.05f);
	// SetDrag(0.7f);
	// SetPhysicsOn(true);
}

SwordTrigger::~SwordTrigger()
{
}

void SwordTrigger::Tick(GameData* _GD)
{
	if (isRendered())
	{
		lifetime += _GD->m_dt;
		if (lifetime > 1.0f)
		{
			SetRendered(false);
			lifetime = 0;
		}
	}


	CMOGO::Tick(_GD);

}

