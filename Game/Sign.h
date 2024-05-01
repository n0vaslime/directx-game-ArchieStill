#pragma once
#include "CMOGO.h"
#include "TextGO2D.h"
#include "ImageGO2D.h"

class Sign : public CMOGO
{
public:
	Sign(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _yaw);
	~Sign();

	virtual void Draw(std::shared_ptr<DrawData>) override;
	virtual void Tick(std::shared_ptr<GameData>) override;
	void SetTriggerPos(std::shared_ptr<GameData>);
	void CreateText();

	std::shared_ptr<CMOGO> SignTrigger;

	bool can_read = false;
	bool is_reading = false;
	std::shared_ptr<TextGO2D> ReadText;
};
