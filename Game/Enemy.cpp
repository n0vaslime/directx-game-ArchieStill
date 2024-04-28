#include "pch.h"
#include "Enemy.h"
#include <dinput.h>
#include "GameData.h"

Enemy::Enemy(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_pos = _pos;

	SetScale(Vector3(1, 1, 1));

    base_pos = GetPos();
	speed = 175;
	player_spotted = false;
    player_facing = 0.0f;

    EnemySensor = new CMOGO("Enemy", _pd3dDevice, _EF);
    EnemySensor->SetScale(10, 3, 10);
    EnemySensor->SetPos(GetPos());
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
    if (isRendered())
        CMOGO::Draw(_DD);
}

void Enemy::EnemyAI(GameData* _GD)
{
    if (_GD->m_GS == GS_GAME)
    {
        if (player_spotted)
        {
            //sets forward vector to face the player and moves towards that
            SetYaw(player_facing);
            Vector3 forwardMove = 0.2f * Vector3::Forward;
            Matrix rotMove = Matrix::CreateRotationY(GetYaw());
            forwardMove = Vector3::Transform(forwardMove, rotMove);
            SetPos(GetPos() - forwardMove * (speed * _GD->m_dt));
            EnemySensor->SetPos(GetPos());
            EnemySensor->SetYaw(GetYaw());
        }
        else
            SetPos(base_pos);
    }
}
