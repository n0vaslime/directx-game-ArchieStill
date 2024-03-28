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

    EnemySensor1 = new CMOGO("Enemy", _pd3dDevice, _EF);
    EnemySensor1->SetScale(10, 0.01f, 10);
    EnemySensor1->SetRendered(true);
    EnemySensor1->SetPos(this->GetPos());
    // SET IT TO GAMEOBJECT/render it in some way
}

Enemy::~Enemy()
{
}

void Enemy::Tick(GameData* _GD)
{
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
        if (!player_spotted)
        {
            this->SetYaw(player_facing);
            // PASS IN THE PLAYER!!!
            Vector3 forwardMove = 0.2f * Vector3::Forward;
            Matrix rotMove = Matrix::CreateRotationY(this->GetYaw());
            forwardMove = Vector3::Transform(forwardMove, rotMove);
            this->SetPos(this->GetPos() - forwardMove);
        }

        // if (m_Enemies[i]->player_spotted)
        // {
        //     m_EnemySensors.clear();
        //     m_Enemies[i]->SetYaw(pPlayer->GetYaw());
        //     Vector3 forwardMove = 0.2f * Vector3::Forward;
        //     Matrix rotMove = Matrix::CreateRotationY(m_Enemies[i]->GetYaw());
        //     forwardMove = Vector3::Transform(forwardMove, rotMove);
        //     m_Enemies[i]->SetPos(m_Enemies[i]->GetPos() - forwardMove);
        //     m_EnemySensors.push_back(EnemySensor);
        // }

    }
}
