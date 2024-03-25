#include "pch.h"
#include "Player.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Player::Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	//any special set up for Player goes here
	m_fudge = Matrix::CreateRotationY(XM_PI);

	m_pos.y = 5.0f;

	SetDrag(0.7);
	SetPhysicsOn(true);
	SetScale(Vector3(1,2,1));
}

Player::~Player()
{
	//tidy up anything I've created
}

void Player::Tick(GameData* _GD)
{
	spawn = Vector3(0.15f, -15.0f, 0.15f);

	switch (_GD->m_GS)
	{
	case GS_PLAY_MAIN_CAM:
	{
		{
			//MOUSE CONTROL SCHEME HERE
			float speed = 10.0f;
			if (!is_attacking)
			{
				m_acc.x += speed * _GD->m_MS.x;
				m_acc.z += speed * _GD->m_MS.y;
			}
			else
			{
				m_acc.x = 0;
				m_acc.z = 0;
			}

			break;
		}
	}
	case GS_GAME:
	{
		//MOVEMENT CONTROL HERE

		Vector3 forwardMove = 40.0f * Vector3::Forward;
		Vector3 leftMove = 40.0f * Vector3::Left;
		Matrix rotMove = Matrix::CreateRotationY(m_yaw);
		forwardMove = Vector3::Transform(forwardMove, rotMove);
		leftMove = Vector3::Transform(leftMove, rotMove);
		if (!is_attacking)
		{
			if (_GD->m_KBS.W)
			{
				m_acc += forwardMove;
				if (_GD->m_KBS.LeftShift)
					m_acc += (forwardMove * 1.1f);
			}
			if (_GD->m_KBS.S)
			{
				m_acc -= forwardMove;
				if (_GD->m_KBS.LeftShift)
					m_acc -= (forwardMove * 1.1f);
			}
			if (_GD->m_KBS.A)
			{
				m_acc += leftMove;
				if (_GD->m_KBS.LeftShift)
					m_acc += (leftMove * 1.1f);
			}
			if (_GD->m_KBS.D)
			{
				m_acc -= leftMove;
				if (_GD->m_KBS.LeftShift)
					m_acc -= (leftMove * 1.1f);
			}
			break;
		}
	}
	}

	//change orientation of player
	float rotSpeed = 0.5f * _GD->m_dt;
	if (!is_attacking)
	{
		if (_GD->m_MS.x)
		{
			m_yaw -= rotSpeed * (_GD->m_MS.x / 1.1f);
		}
		if (_GD->m_MS.y)
		{
			m_pitch -= rotSpeed * (_GD->m_MS.y / 1.1f);
			if (m_pitch <= -XM_PI / 4)
				m_pitch = -XM_PI / 4;
			if (m_pitch >= XM_PI / 2 - 0.05)
				m_pitch = XM_PI / 2 - 0.05;
		}
	}
	else
	{
		if (_GD->m_MS.x)
		{
			m_yaw -= rotSpeed * (_GD->m_MS.x / 4);
		}
		if (_GD->m_MS.y)
		{
			m_pitch -= rotSpeed * (_GD->m_MS.y / 4);
			if (m_pitch <= -XM_PI / 4)
				m_pitch = -XM_PI / 4;
			if (m_pitch >= XM_PI / 2 - 0.05)
				m_pitch = XM_PI / 2 - 0.05;
		}
	}

	//jumping code
	if (_GD->m_KBS.Space && is_grounded && !is_attacking)
	{
		m_acc.y += 200.0f;
		is_grounded = false;
	}

	for (size_t i = 0; i < m_SwordTrigger.size(); i++)
	{
		//checks if sword bounds are active
		if (m_SwordTrigger[i]->isRendered())
			is_attacking = true;
		else
			is_attacking = false;

		//creating sword bounds
		if (_GD->m_MS.leftButton)
		{
			if (!m_SwordTrigger[i]->isRendered() && is_grounded)
			{
				Vector3 forwardMove = 40.0f * Vector3::Forward;
				Matrix rotMove = Matrix::CreateRotationY(m_yaw);
				forwardMove = Vector3::Transform(forwardMove, rotMove);
				m_SwordTrigger[i]->SetPos(this->GetPos() + forwardMove * spawn);
				m_SwordTrigger[i]->SetRendered(true);
				m_SwordTrigger[i]->SetYaw(this->GetYaw());
				m_vel *= 0;
			}
		}
	}

	//limit motion of the player
	float length = m_pos.Length();
	float maxLength = 500.0f;
	if (length > maxLength)
	{
		m_pos.Normalize();
		m_pos *= maxLength;
		m_vel *= -0.9; //VERY simple bounce back
	}

	//apply my base behaviour
	CMOGO::Tick(_GD);
}

void Player::Draw(DrawData* _DD)
{
	if (true)
	{
		CMOGO::Draw(_DD);
	}
}
