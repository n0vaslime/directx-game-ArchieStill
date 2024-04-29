#include "pch.h"
#include "terrain.h"

Terrain::Terrain(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _pitch, float _yaw, float _roll, Vector3 _scale) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	m_pos = _pos;
	m_pitch = _pitch;
	m_roll = _roll;
	m_yaw = _yaw;
	m_scale = _scale;
	
	GroundCheck = std::make_shared<CMOGO>("GreenCube", _pd3dDevice, _EF);
	GroundCheck->SetScale(this->GetScale());
	GroundCheck->SetPos(Vector3(this->GetPos().x, this->GetPos().y + 0.25f, this->GetPos().z));

	GameObject::Tick(nullptr); //update my world_transform
}

Terrain::~Terrain()
{
	//Nothing additional here but add this just in case
}
