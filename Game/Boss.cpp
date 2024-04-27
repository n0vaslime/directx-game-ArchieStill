#include "pch.h"
#include "Boss.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Boss::Boss(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
    m_fudge = Matrix::CreateRotationY(XM_PI);
    SetPos(Vector3(0, 375, 0));
    SetScale(1.25f);
    player_pitch = 0.0f;
    is_talking = true;
    is_dying = false;

    pBossProjectile = new CMOGO("Projectile", _pd3dDevice, _EF);
    pBossProjectile->SetPos(Vector3::One * 1000000);
    pBossProjectile->SetScale(Vector3::One * 2.5f);
    pBossProjectile->SetRendered(false);
}

Boss::~Boss()
{
}

void Boss::Tick(GameData* _GD)
{
    if (_GD->m_GS == GS_BOSS)
    {
        BossFacing();
        if (is_talking)
            BossIntroduction(_GD);
        else
        {
            if (!is_dying)
            {
                BossHealth();

                //prevents overlapping voicelines
                if (play_hurt_sfx)
                {
                    hurt_lifetime += _GD->m_dt;
                    if (hurt_lifetime >= 5)
                    {
                        play_hurt_sfx = false;
                        hurt_lifetime = 0;
                    }
                }

                //attacks every 4.5 seconds
                projectile_timer += _GD->m_dt;
                if (projectile_timer >= 4.5f)
                {
                    BossAttacking();
                    projectile_timer = 0;
                }

                //projectiles disappear after 2 seconds
                if (pBossProjectile->isRendered())
                {
                    projectile_lifetime += _GD->m_dt;
                    if (projectile_lifetime >= 2)
                    {
                        pBossProjectile->SetRendered(false);
                        projectile_lifetime = 0;
                    }
                }
            }
            else
                BossEnding(_GD);
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
    float angleLookAt = atan2(player_adjacent, player_opposite);
    this->SetYaw(angleLookAt + 3.1415f);
    this->SetPitch(-player_pitch * 1.1f);

}

void Boss::BossIntroduction(GameData* _GD) 
{
    if (m_pos.y > 100)
        m_pos.y -= _GD->m_dt * 5;
    else
        m_pos.y = 100;

    intro_talk += _GD->m_dt;
    if (intro_talk >= 66)
    {
        intro_talk = 0;
        is_talking = false;
    }
}
void Boss::BossEnding(GameData* _GD)
{
    m_pitch = -5;
    m_pos.y += _GD->m_dt * 2.5f;
    
    if (m_scale.x > 0)
        m_scale.x -= _GD->m_dt / 35;
    else
        m_scale.x = 0;
    if (m_scale.y > 0)
        m_scale.y -= _GD->m_dt / 35;
    else
        m_scale.y = 0;
    if (m_scale.z > 0)
        m_scale.z -= _GD->m_dt / 35;
    else
        m_scale.z = 0;

    dying_time += _GD->m_dt;
}

void Boss::BossHealth()
{
    //as more cores are destroyed, he gets closer to the ground
    if (boss_health == 2)
        m_pos.y = 80;
    if (boss_health == 1)
        m_pos.y = 60;
    if (boss_health == 0)
        m_pos.y = 40;
}
void Boss::BossAttacking()
{
    play_combat_sfx = true;
    Vector3 forwardMove = 40.0f * Vector3::Forward;
    Matrix rotMove = Matrix::CreateRotationY(m_yaw);
    Matrix pitchMove = Matrix::CreateRotationX(m_pitch);
    forwardMove = Vector3::Transform(forwardMove, pitchMove);
    forwardMove = Vector3::Transform(forwardMove, rotMove);
    pBossProjectile->SetPos(this->GetPos() + forwardMove);
    pBossProjectile->SetPhysicsOn(true);
    pBossProjectile->SetRendered(true);
    pBossProjectile->SetAcceleration(forwardMove * 600);
}
