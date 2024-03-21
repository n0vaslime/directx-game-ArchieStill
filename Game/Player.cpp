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
	SetScale(Vector3(2.5f,2.5f,2.5f));

}

Player::~Player()
{
	//tidy up anything I've created
}


void Player::Tick(GameData* _GD)
{
	switch (_GD->m_GS)
	{
	case GS_PLAY_MAIN_CAM:
	{
		{
			//MOUSE CONTROL SCHEME HERE
			float speed = 10.0f;
			m_acc.x += speed * _GD->m_MS.x;
			m_acc.z += speed * _GD->m_MS.y;
			break;
		}
	}
	case GS_GAME:
	{
		//TURN AND FORWARD CONTROL HERE

		Vector3 forwardMove = 40.0f * Vector3::Forward;
		Matrix rotMove = Matrix::CreateRotationY(m_yaw);
		forwardMove = Vector3::Transform(forwardMove, rotMove);
		if (_GD->m_KBS.W)
		{
			m_acc += forwardMove;
		}
		if (_GD->m_KBS.S)
		{
			m_acc -= forwardMove;
		}
		break;
	}
	}

	//change orientation of player
	float rotSpeed = 0.5f * _GD->m_dt;
	if (_GD->m_MS.x)
	{
		m_yaw -= rotSpeed * _GD->m_MS.x;
	}
	if (_GD->m_MS.y)
	{
		m_pitch -= rotSpeed * _GD->m_MS.y;
		if (m_pitch <= -XM_PI / 4)
			m_pitch = -XM_PI / 4;
		if (m_pitch >= XM_PI / 2 - 0.05)
			m_pitch = XM_PI / 2 - 0.05;
	}

	//move player up and down
	if (_GD->m_KBS.Space && is_grounded)
	{
		m_acc.y += 200.0f;
		is_grounded = false;
	}
	
	if (_GD->m_KBS.F)
	{
		m_acc.y -= 40.0f;
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
	if (false)
	{
		CMOGO::Draw(_DD);
	}
}