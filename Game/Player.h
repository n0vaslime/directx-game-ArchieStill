#ifndef _PLAYER_H_
#define _PLAYER_H_
#include "CMOGO.h"
#include "SwordTrigger.h"
#include "SwordObject.h"

//=================================================================
//Base Player Class (i.e. a GO the player controls)
//=================================================================

class Player : public CMOGO
{

public:
	Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF);
	~Player();

	virtual void Draw(DrawData* _DD) override;
	virtual void Tick(GameData* _GD) override;
	void SwordTriggers(GameData* _GD);
	void SwordObjects();

	bool is_grounded = true;
	bool is_attacking = false;
	bool is_respawning = false;

	float lifetime = 0.0f;

	vector<SwordTrigger*> m_PSwordTrigger;
	vector<CMOGO*> m_STrigger;
	SwordTrigger* sword_trigger;
	CMOGO* SwordTrigger1;
	vector<SwordObject*> m_PSwordObject;

protected:

};

#endif