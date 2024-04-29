#include "pch.h"
#include "MovingPlatform.h"
#include "GameData.h"
#include <iostream>

MovingPlatform::MovingPlatform(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll, Vector3 _scale) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	m_pos = _pos;
	m_pitch = _pitch;
	m_roll = _roll;
	m_yaw = _yaw;
	m_scale = _scale;

	GroundCheck = std::make_shared<CMOGO>("GreenCube", _pd3dDevice, _EF);
	GroundCheck->SetScale(this->GetScale());
}

MovingPlatform::~MovingPlatform()
{
}

void MovingPlatform::Tick(GameData* _GD)
{
	GroundCheck->SetPos(Vector3(this->GetPos().x, this->GetPos().y + 0.5f, this->GetPos().z));
	GroundCheck->SetYaw(this->GetYaw());

	//it takes 5 seconds for the platform to move there and back
	//when moving the platform, increase the others' lifetime
	//once that lifetime reaches 5, swap movement!
	if (back_lifetime > 5.0f)
		going = true;
	if (direction_lifetime > 5.0f)
		going = false;

	switch (this->Moving)
	{
	case ROTATEANTICLOCKWISE:
		m_yaw -= _GD->m_dt / rotate_speed;
		break;
	case ROTATECLOCKWISE:
		m_yaw += _GD->m_dt / rotate_speed;
		break;
	case MOVEUP:
		if (going)
		{
			back_lifetime = 0;
			m_pos.y += _GD->m_dt * updown_speed;
			direction_lifetime += _GD->m_dt;
		}
		else
		{
			direction_lifetime = 0;
			m_pos.y -= _GD->m_dt * updown_speed;
			back_lifetime += _GD->m_dt;
		}
		break;
	case MOVEDOWN:
		if (going)
		{
			back_lifetime = 0;
			m_pos.y -= _GD->m_dt * updown_speed;
			direction_lifetime += _GD->m_dt;
		}
		else
		{
			direction_lifetime = 0;
			m_pos.y += _GD->m_dt * updown_speed;
			back_lifetime += _GD->m_dt;
		}
		break;
	case MOVELEFTX:
		if (going)
		{
			back_lifetime = 0;
			m_pos.x -= _GD->m_dt * leftright_speed;
			direction_lifetime += _GD->m_dt;
		}
		else
		{
			direction_lifetime = 0;
			m_pos.x += _GD->m_dt * leftright_speed;
			back_lifetime += _GD->m_dt;
		}
		break;
	case MOVEFORWARDZ:
		if (going)
		{
			back_lifetime = 0;
			m_pos.z -= _GD->m_dt * leftright_speed;
			direction_lifetime += _GD->m_dt;
		}
		else
		{
			direction_lifetime = 0;
			m_pos.z += _GD->m_dt * leftright_speed;
			back_lifetime += _GD->m_dt;
		}
		break;
	default:
		break;
	}

	CMOGO::Tick(_GD);
}
