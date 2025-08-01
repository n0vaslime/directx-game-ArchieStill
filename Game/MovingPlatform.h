#pragma once
#include "CMOGO.h"
#include "MoveState.h"

class MovingPlatform : public CMOGO
{
public:
	MovingPlatform(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll, Vector3 _scale);
	~MovingPlatform();

	void Tick(std::shared_ptr<GameData> _GD);
	MoveState Moving;

	std::shared_ptr<CMOGO> GroundCheck;

	float direction_lifetime = 0.0f;
	float back_lifetime = 5.0f;
	bool going = true;

	float rotate_speed = 4;
	float updown_speed = 20;
	float leftright_speed = 20;
};
