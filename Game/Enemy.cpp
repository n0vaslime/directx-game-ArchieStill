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

	speed = 0.75f;
	player_spotted = false;
    player_facing = 0.0f;

    EnemySensor = new CMOGO("Enemy", _pd3dDevice, _EF);
    EnemySensor->SetScale(10, 1, 10);
    EnemySensor->SetPos(this->GetPos());
}

Enemy::~Enemy()
{
}

void Enemy::Tick(GameData* _GD)
{
    //idle??
    m_yaw -= _GD->m_dt / 4;

    EnemyAI(_GD);

	CMOGO::Tick(_GD);
}

void Enemy::Draw(DrawData* _DD)
{
    if (this->isRendered())
        CMOGO::Draw(_DD);
}

void Enemy::EnemyAI(GameData* _GD)
{
    if (_GD->m_GS == GS_GAME)
    {
        if (player_spotted)
        {
            this->SetYaw(player_facing);
            Vector3 forwardMove = 0.2f * Vector3::Forward;
            Matrix rotMove = Matrix::CreateRotationY(this->GetYaw());
            forwardMove = Vector3::Transform(forwardMove, rotMove);
            this->SetPos(this->GetPos() - forwardMove * speed);
            EnemySensor->SetPos(this->GetPos());
            EnemySensor->SetYaw(this->GetYaw());
        }
    }
}
