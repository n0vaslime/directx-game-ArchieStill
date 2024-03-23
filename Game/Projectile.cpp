#include "pch.h"
#include "Projectile.h"
#include <dinput.h>
#include "GameData.h"

Projectile::Projectile(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_fudge = Matrix::CreateRotationY(XM_PI);

	SetRendered(false);
	SetScale(Vector3::One * 0.05f);
	// SetDrag(0.7f);
	// SetPhysicsOn(true);
}

Projectile::~Projectile()
{
}

void Projectile::Tick(GameData* _GD)
{
	if (isRendered())
	{
		lifetime += _GD->m_dt;
		if (lifetime > 2.5f)
		{
			SetRendered(false);
			lifetime = 0;
		}
	}


	CMOGO::Tick(_GD);

}

