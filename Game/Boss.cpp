#include "pch.h"
#include "Boss.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Boss::Boss(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
    SetPos(Vector3(0, 350, -25));
    SetScale(1.25f);
    player_yaw = 0.0f;
    player_pitch = 0.0f;
    is_talking = true;
    is_dying = false;


    pBossProjectile = new CMOGO("Projectile", _pd3dDevice, _EF);
    pBossProjectile->SetScale(Vector3::One * 2);
    pBossProjectile->SetRendered(false);
}

Boss::~Boss()
{
}

void Boss::Tick(GameData* _GD)
{
    if (_GD->m_GS == GS_BOSS)
    {
        std::cout << projectile_timer << std::endl;

        BossFacing();
        if (is_talking)
            BossIntroduction(_GD);
        else
        {
            BossHealth();
            projectile_timer += _GD->m_dt;
            if (projectile_timer >= 5)
            {
                BossAttacking();
                projectile_timer = 0;
            }

            if (pBossProjectile->isRendered())
            {
                projectile_lifetime += _GD->m_dt;
                if (projectile_lifetime >= 3)
                {
                    pBossProjectile->SetRendered(false);
                    projectile_lifetime = 0;
                }
            }


        }
    }

	CMOGO::Tick(_GD);
}

void Boss::Draw(DrawData* _DD)
{
    if (this->isRendered())
        CMOGO::Draw(_DD);
}

void Boss::BossFacing()
{
    this->SetYaw(player_yaw);
    this->SetPitch(player_pitch);
}

void Boss::BossIntroduction(GameData* _GD)
{
    if (m_pos.y > 70)
        m_pos.y -= _GD->m_dt * 5;
    else
        m_pos.y = 70;

    intro_talk += _GD->m_dt;
    if (intro_talk >= 66)
    {
        intro_talk = 0;
        is_talking = false;
    }
}
void Boss::BossHealth()
{
    if (boss_health == 2)
        m_pos.y = 60;
    if (boss_health == 1)
        m_pos.y = 50;
    if (boss_health == 0)
        SetPos(Vector3(0, 40, -25));
}

void Boss::BossAttacking()
{
    std::cout << "SHOOT" << std::endl;
    Vector3 forwardMove = 40.0f * Vector3::Forward;
    Matrix rotMove = Matrix::CreateRotationY(m_yaw);
    Matrix pitchMove = Matrix::CreateRotationX(m_pitch);
    forwardMove = Vector3::Transform(forwardMove, pitchMove);
    forwardMove = Vector3::Transform(forwardMove, rotMove);
    pBossProjectile->SetPos(this->GetPos() + forwardMove);
    pBossProjectile->SetPhysicsOn(true);
    pBossProjectile->SetDrag(0.7f);
    pBossProjectile->SetRendered(true);
    pBossProjectile->SetAcceleration(forwardMove * -1000);
}
