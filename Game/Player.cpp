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
	m_pos.z = -10.0f;
	SetDrag(1.5f);
	SetPhysicsOn(true);
	SetScale(Vector3(1, 2, 1));
	base_game_respawn = Vector3(0, 5, 75);

	//Set up sword trigger
	base_trigger_size = Vector3(0.075f, -0.1f, 0.075f);
	pSwordTrigger = std::make_shared<CMOGO>("table", _pd3dDevice, _EF);
	pSwordTrigger->SetScale(base_trigger_size);
	pSwordTrigger->SetRendered(false);

	//Set up sword object
	pSwordObject = std::make_shared<CMOGO>("Sword", _pd3dDevice, _EF);
	pSwordObject->SetScale(Vector3(0.15f, 0.2f, 0.2f));
}

Player::~Player()
{
}

void Player::Tick(std::shared_ptr<GameData> _GD)
{
	if (_GD->m_GS == GS_GAME || _GD->m_GS == GS_INTRO || _GD->m_GS == GS_BOSS)
	{
		PlayerMovement(_GD);
		if (has_sword)
		{
			SwordTriggers(_GD);
			SwordObjects();
		}

		//time that sword trigger is active (0.4s)
		if (pSwordTrigger->isRendered())
		{
			lifetime += _GD->m_dt;
			if (lifetime > 0.4f)
			{
				pSwordTrigger->SetRendered(false);
				lifetime = 0;
			}
		}

		//if the player dies, they respawn at the start/their last checkpoint
		if (is_respawning)
		{
			this->SetPos(Vector3(respawn_pos.x, respawn_pos.y + 5, respawn_pos.z));
			this->SetPitch(0);
			this->SetYaw(0);
			m_vel.x = 0;
			m_vel.y = 0;
			m_vel.z = 0;
			is_respawning = false;
		}

		CMOGO::Tick(_GD);
	}
}

void Player::Draw(std::shared_ptr<DrawData> _DD)
{
	//don't draw the player
	if (false)
		CMOGO::Draw(_DD);
}

void Player::PlayerMovement(std::shared_ptr<GameData> _GD)
{
	///MOVEMENT CONTROL HERE
	Vector3 forwardMove = 40.0f * Vector3::Forward;
	Vector3 leftMove = 40.0f * Vector3::Left;
	Matrix rotMove = Matrix::CreateRotationY(m_yaw);
	forwardMove = Vector3::Transform(forwardMove, rotMove);
	leftMove = Vector3::Transform(leftMove, rotMove);

	if (!is_attacking)
	{
		if (_GD->m_KBS.W)
			m_acc += forwardMove * 1.25;
		if (_GD->m_KBS.S)
			m_acc -= forwardMove * 1.25;
		if (_GD->m_KBS.A)
			m_acc += leftMove * 1.25;
		if (_GD->m_KBS.D)
			m_acc -= leftMove * 1.25;
	}

	//change orientation of player
	float rotSpeed = 0.5f * _GD->m_dt;
	if (is_attacking || is_reading)
	{
		if (_GD->m_MS.x)
			m_yaw -= rotSpeed * (_GD->m_MS.x / 6);
		if (_GD->m_MS.y)
			m_pitch -= rotSpeed * (_GD->m_MS.y / 6);
	}
	else
	{
		if (_GD->m_MS.x)
			m_yaw -= rotSpeed * (_GD->m_MS.x / 1.1f);
		if (_GD->m_MS.y)
			m_pitch -= rotSpeed * (_GD->m_MS.y / 1.1f);
	}
	if (m_pitch <= -XM_PI / 4)
		m_pitch = -XM_PI / 4;
	if (m_pitch >= XM_PI / 2 - 0.05)
		m_pitch = XM_PI / 2 - 0.05;

	//jumping code
	if (_GD->m_KBS.Space && is_grounded && !is_attacking)
	{
		m_vel.y = 100;
		is_grounded = false;
		play_jump_sfx = true;
	}
	if (launching)
	{
		m_vel.y = 1000;
		launching = false;
		play_jump_sfx = true;
	}

	if (_GD->m_KBS.NumPad5 && is_grounded)
	{
		m_vel.y = 2000;
		is_grounded = false;
	}
	
	//Checkpoint reset
	if (_GD->m_KBS.C && is_grounded && !is_attacking && _GD->m_GS == GS_GAME)
		is_respawning = true;

	//limit motion of the player
	float length = m_pos.Length();
	float maxLength = 1001.0f;
	if (length > maxLength)
	{
		m_pos.Normalize();
		m_pos *= maxLength;
		m_vel *= -0.9;
	}
}

void Player::SwordTriggers(std::shared_ptr<GameData> _GD)
{
	//checks if sword bounds are active
	if (pSwordTrigger->isRendered())
		is_attacking = true;
	else
		is_attacking = false;

	//creating sword bounds
	if (_GD->m_MS.leftButton)
	{
		if (!pSwordTrigger->isRendered() && is_grounded)
		{
			play_sword_sfx = true;
			Vector3 spawn = Vector3(0.15f, -15.0f, 0.15f);
			Vector3 forwardMove = 40.0f * Vector3::Forward;
			Matrix rotMove = Matrix::CreateRotationY(m_yaw);
			forwardMove = Vector3::Transform(forwardMove, rotMove);
			pSwordTrigger->SetPos(this->GetPos() + forwardMove * spawn);
			pSwordTrigger->SetRendered(true);
			pSwordTrigger->SetYaw(this->GetYaw());
			m_vel *= 0;
		}
	}

	if (_GD->m_GS == GS_INTRO || _GD->m_GS == GS_GAME)
	{
		pSwordTrigger->SetScale(base_trigger_size);
	}
	else if (_GD->m_GS == GS_BOSS)
	{
		//changes player sword size in the boss fight to deflect projectiles better!
		pSwordTrigger->SetScale(Vector3(base_trigger_size.x * 1.5f, base_trigger_size.y * 5, base_trigger_size.z * 1.5f));
	}
}

void Player::SwordObjects()
{
	Vector3 objSpawn = Vector3(0, -2, 0);
	Vector3 forwardMove = 3 * Vector3::Forward;
	Matrix rotMove = Matrix::CreateRotationY(m_yaw);
	forwardMove = Vector3::Transform(forwardMove, rotMove);
	pSwordObject->SetRendered(true);
	pSwordObject->SetPos(this->GetPos() + (forwardMove + objSpawn));
	pSwordObject->SetYaw(this->GetYaw());
	pSwordObject->SetPitch(0);

	//sword swinging animation
	if (is_attacking)
	{
		pSwordObject->SetPitch(pSwordObject->GetPitch() - XM_PI / 4);
		if (lifetime < 0.1f)
			pSwordObject->SetPitch(pSwordObject->GetPitch() - XM_PI / 8);
		else if (lifetime < 0.2f)
			pSwordObject->SetPitch(pSwordObject->GetPitch() - XM_PI / 4);
		else if (lifetime < 0.3f)
			pSwordObject->SetPitch(pSwordObject->GetPitch() - XM_PI / 8);
		else if (lifetime < 0.4f)
			pSwordObject->SetPitch(0);
	}
	else
	{
		pSwordObject->SetPitch(0);
	}
}
