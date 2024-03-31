#include "pch.h"
#include "Player.h"
#include <dinput.h>
#include "GameData.h"
#include <iostream>

Player::Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	//any special set up for Player goes here
	m_fudge = Matrix::CreateRotationY(XM_PI);

	m_pos.y = 1.0f;

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
	if (_GD->m_GS == GS_INTRO or _GD->m_GS == GS_GAME) 
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
			}
			if (_GD->m_KBS.S)
			{
				m_acc -= forwardMove;
			}
			if (_GD->m_KBS.A)
			{
				m_acc += leftMove;
			}
			if (_GD->m_KBS.D)
			{
				m_acc -= leftMove;
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
			//m_acc.y += 200.0f;
			m_vel.y = 30;
			is_grounded = false;
		}

		for (size_t i = 0; i < m_PSwordTrigger.size(); i++)
		{
			//checks if sword bounds are active
			if (m_PSwordTrigger[i]->isRendered())
				is_attacking = true;
			else
				is_attacking = false;

			//creating sword bounds
			if (_GD->m_MS.leftButton)
			{
				if (!m_PSwordTrigger[i]->isRendered() && is_grounded)
				{
					Vector3 spawn = Vector3(0.15f, -15.0f, 0.15f);
					Vector3 forwardMove = 40.0f * Vector3::Forward;
					Matrix rotMove = Matrix::CreateRotationY(m_yaw);
					forwardMove = Vector3::Transform(forwardMove, rotMove);
					m_PSwordTrigger[i]->SetPos(this->GetPos() + forwardMove * spawn);
					m_PSwordTrigger[i]->SetRendered(true);
					m_PSwordTrigger[i]->SetYaw(this->GetYaw());
					m_vel *= 0;
				}
			}
		}

		for (size_t i = 0; i < m_PSwordObject.size(); i++)
		{
			Vector3 objSpawn = Vector3(0, -2, 0);
			Vector3 forwardMove = 2.5f * Vector3::Forward;
			Matrix rotMove = Matrix::CreateRotationY(m_yaw);
			forwardMove = Vector3::Transform(forwardMove, rotMove);
			m_PSwordObject[i]->SetRendered(true);
			m_PSwordObject[i]->SetPos(this->GetPos() + (forwardMove + objSpawn));
			m_PSwordObject[i]->SetYaw(this->GetYaw());
			m_PSwordObject[i]->SetPitch(0);

			for (size_t j = 0; j < m_PSwordTrigger.size(); j++)
			{
				if (is_attacking)
				{
					m_PSwordObject[i]->SetPitch(m_PSwordObject[i]->GetPitch() - XM_PI / 4);
					if (m_PSwordTrigger[j]->lifetime < 0.1f)
						m_PSwordObject[i]->SetPitch(m_PSwordObject[i]->GetPitch() - XM_PI / 8);
					else if (m_PSwordTrigger[j]->lifetime < 0.2f)
						m_PSwordObject[i]->SetPitch(m_PSwordObject[i]->GetPitch() - XM_PI / 4);
					else if (m_PSwordTrigger[j]->lifetime < 0.3f)
						m_PSwordObject[i]->SetPitch(m_PSwordObject[i]->GetPitch() - XM_PI / 8);
					else if (m_PSwordTrigger[j]->lifetime < 0.4f)
						m_PSwordObject[i]->SetPitch(0);				
				}
				else
				{
					m_PSwordObject[i]->SetPitch(0);
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
			m_vel *= -0.9;
		}

		if (is_respawning)
		{
			this->SetPos(Vector3(0, 5, 0));
			this->SetPitch(0);
			this->SetYaw(0);
			m_vel.x = 0;
			m_vel.z = 0;
			is_respawning = false;
		}

		//apply my base behaviour
		CMOGO::Tick(_GD);
	}
}

void Player::Draw(DrawData* _DD)
{
	if (true)
		CMOGO::Draw(_DD);
}
