#pragma once
#include "CMOGO.h"
#include "MoveState.h"

class MovingPlatform : public CMOGO
{
public:
	MovingPlatform(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll, Vector3 _scale);
	~MovingPlatform();

	void Tick(GameData* _GD);
	int state;
	MoveState Moving;

	CMOGO* GroundCheck;
};
