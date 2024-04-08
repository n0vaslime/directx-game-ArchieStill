#include "pch.h"
#include "MovingPlatform.h"
#include "GameData.h"

MovingPlatform::MovingPlatform(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll, Vector3 _scale) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	m_pos = _pos;
	m_pitch = _pitch;
	m_roll = _roll;
	m_yaw = _yaw;
	m_scale = _scale;

	GroundCheck = new CMOGO("GreenCube", _pd3dDevice, _EF);
	GroundCheck->SetScale(this->GetScale());
	GroundCheck->SetPos(Vector3(this->GetPos().x, this->GetPos().y + 0.25f, this->GetPos().z));
}

MovingPlatform::~MovingPlatform()
{
}

void MovingPlatform::Tick(GameData* _GD)
{
	switch (this->Moving)
	{
	case ROTATELEFT:
		m_yaw -= _GD->m_dt / 4;
		break;
	case ROTATERIGHT:
		m_yaw += _GD->m_dt / 4;
		break;
	case MOVEUP:
		m_pos.y += _GD->m_dt / 4;
		break;
	case MOVELEFT:
		break;
	case MOVERIGHT:
		break;
	default:
		break;
	}
	CMOGO::Tick(_GD);
}
