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
	// SetPhysicsOn(true);
}

Enemy::~Enemy()
{
}

void Enemy::Tick(GameData* _GD)
{
	CMOGO::Tick(_GD);
}
