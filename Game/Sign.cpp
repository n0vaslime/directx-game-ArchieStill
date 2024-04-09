#include "pch.h"
#include "Sign.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Sign::Sign(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _yaw) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	m_pos = _pos;
	m_yaw = _yaw;
	SetScale(Vector3(0.45f, 0.4f, 0.45f));

	SignTrigger = new CMOGO("Sign", _pd3dDevice, _EF);
	SignTrigger->SetPos(this->GetPos());
	SignTrigger->SetScale(Vector3(0.65f, 0.4f, 2.5f));

	CreateText();
}

Sign::~Sign()
{
}

void Sign::Draw(DrawData* _DD)
{
	if (this->isRendered())
		CMOGO::Draw(_DD);
}

void Sign::Tick(GameData* _GD)
{
	SetTriggerPos(_GD);

	CMOGO::Tick(_GD);
}

void Sign::SetTriggerPos(GameData* _GD)
{
	if (_GD->m_GS == GS_GAME || _GD->m_GS == GS_INTRO)
	{
		Vector3 forwardMove = 2.5f * Vector3::Forward;
		Matrix rotMove = Matrix::CreateRotationY(m_yaw);
		forwardMove = Vector3::Transform(forwardMove, rotMove);
		SignTrigger->SetRendered(true);
		SignTrigger->SetPos(this->GetPos() - forwardMove);
		SignTrigger->SetYaw(this->GetYaw());
		SignTrigger->SetPitch(0);
	}
}

void Sign::CreateText()
{
	pReadText = new TextGO2D("Press 'E' to read!");
	pReadText->SetPos(Vector2(275, 550));
	pReadText->SetColour(Color((float*)&Colors::WhiteSmoke));
	pReadText->SetScale(0.75f);
}
