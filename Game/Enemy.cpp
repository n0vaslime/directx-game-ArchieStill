#include "pch.h"
#include "Enemy.h"
#include <dinput.h>
#include "GameData.h"

Enemy::Enemy(string _filename, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos) : CMOGO(_filename, _pd3dDevice, _EF)
{
	m_pos = _pos;
	SetScale(Vector3(0.075f, -0.075f, 0.05f));
}

Enemy::~Enemy()
{
}

void Enemy::Tick(GameData* _GD)
{

}
