#include "pch.h"
#include "Enemy.h"
#include <dinput.h>
#include "GameData.h"

Enemy::Enemy(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_pos = _pos;
	m_pitch = _pitch;
	m_yaw = _yaw;
	m_roll = _roll;

	SetScale(Vector3(1, 1, 1));

	speed = 5.0f;
	player_spotted = false;
}

Enemy::~Enemy()
{
}

void Enemy::Tick(GameData* _GD)
{
	for (int i = 0; i < m_ESensor.size(); i++)
	{
		m_ESensor[i]->SetScale(Vector3(10, 0.1f, 10));
		m_ESensor[i]->SetRendered(true);
		m_ESensor[i]->SetPos(Vector3(10,10,10));
		m_ESensor[i]->SetPitch(this->GetPitch());
		m_ESensor[i]->SetYaw(this->GetYaw());
	}

	CMOGO::Tick(_GD);
}
