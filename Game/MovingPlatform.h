#pragma once
#include "CMOGO.h"
#include "MoveState.h"

class MovingPlatform : public CMOGO
{
public:
	MovingPlatform(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll, Vector3 _scale);
	~MovingPlatform();

	void Tick(GameData* _GD);
	MoveState Moving;

	CMOGO* GroundCheck;

	float direction_lifetime = 0.0f;
	float back_lifetime = 5.0f;
	bool going = true;
};
