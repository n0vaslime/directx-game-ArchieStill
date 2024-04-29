#pragma once
#include "CMOGO.h"
#include "TextGO2D.h"
#include "ImageGO2D.h"

class Sign : public CMOGO
{
public:
	Sign(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF, Vector3 _pos, float _yaw);
	~Sign();

	virtual void Draw(DrawData* _DD) override;
	virtual void Tick(GameData* _GD) override;
	void SetTriggerPos(GameData* _GD);
	void CreateText();

	std::shared_ptr<CMOGO> SignTrigger;

	bool can_read = false;
	bool is_reading = false;
	std::shared_ptr<TextGO2D> ReadText;
};
