#include "pch.h"
#include "Boss.h"
#include <dinput.h>
#include "GameData.h"

Boss::Boss(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_filename, _pd3dDevice, _EF)
{
    SetPos(Vector3(0, 350, -25));
    SetScale(1.25f);
    player_yaw = 0.0f;
    player_pitch = 0.0f;
    is_talking = true;
    is_dying = false;
}

Boss::~Boss()
{
}

void Boss::Tick(GameData* _GD)
{
    BossAI(_GD);
    if (is_talking)
        BossIntroduction(_GD);
    else 
    {
        
    }

	CMOGO::Tick(_GD);
}

void Boss::Draw(DrawData* _DD)
{
    if (this->isRendered())
        CMOGO::Draw(_DD);
}

void Boss::BossAI(GameData* _GD)
{
    if (_GD->m_GS == GS_BOSS)
    {
        this->SetYaw(player_yaw / 2);
        this->SetPitch(player_pitch);
    }
}

void Boss::BossIntroduction(GameData* _GD)
{
    if (m_pos.y > 70)
        m_pos.y -= _GD->m_dt * 5;
    else
        m_pos.y = 70;
}
