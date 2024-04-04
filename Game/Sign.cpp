#include "pch.h"
#include "Sign.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Sign::Sign(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	m_pos = _pos;
	SetScale(Vector3(0.45f, 0.4f, 0.45f));

	pSignTrigger = new CMOGO("Sign", _pd3dDevice, _EF);
	pSignTrigger->SetPos(this->GetPos());
	pSignTrigger->SetScale(Vector3(0.65f, 0.4f, 2.5f));

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
		pSignTrigger->SetRendered(true);
		pSignTrigger->SetPos(this->GetPos() - forwardMove);
		pSignTrigger->SetPitch(0);
	}
}

void Sign::CreateText()
{
	pReadText = new TextGO2D("Press 'E' to read!");
	pReadText->SetPos(Vector2(275, 550));
	pReadText->SetColour(Color((float*)&Colors::WhiteSmoke));
	pReadText->SetScale(0.75f);
}
