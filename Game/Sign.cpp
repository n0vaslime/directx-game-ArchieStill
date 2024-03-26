#include "pch.h"
#include "Sign.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Sign::Sign(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	m_pos = _pos;
	SetScale(Vector3(0.45f, 0.4f, 0.45f));
}

Sign::~Sign()
{
}

void Sign::Draw(DrawData* _DD)
{
	if (true)
		CMOGO::Draw(_DD);
}

void Sign::Tick(GameData* _GD)
{
	if (_GD->m_GS == GS_GAME)
	{
		for (size_t i = 0; i < m_ReadingTrigger.size(); i++)
		{
			Vector3 forwardMove = 1.0f * Vector3::Forward;
			Matrix rotMove = Matrix::CreateRotationY(m_yaw);
			forwardMove = Vector3::Transform(forwardMove, rotMove);
			m_ReadingTrigger[i]->SetRendered(true);
			m_ReadingTrigger[i]->SetPos(this->GetPos() - forwardMove);
			m_ReadingTrigger[i]->SetPitch(0);
		}
	}

	CMOGO::Tick(_GD);
}
