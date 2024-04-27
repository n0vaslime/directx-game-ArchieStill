#ifndef _PLAYER_H_
#define _PLAYER_H_
#include "CMOGO.h"
#include "Audio.h"

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
	void PlayerMovement(GameData* _GD);
	void SwordTriggers(GameData* _GD);
	void SwordObjects();

	bool is_grounded = false;
	bool has_sword = false;
	bool is_attacking = false;
	bool is_respawning = false;
	Vector3 respawn_pos;
	Vector3 base_game_respawn;
	bool is_reading = false;
	bool launching = false;
	Vector3 boss_pos_set;

	bool play_jump_sfx = false;
	bool play_sword_sfx = false;

	float lifetime = 0.0f;

	CMOGO* pSwordTrigger;
	CMOGO* pSwordObject;

protected:

};

#endif