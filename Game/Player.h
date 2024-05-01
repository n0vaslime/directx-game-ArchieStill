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

	virtual void Draw(std::shared_ptr<DrawData>) override;
	virtual void Tick(std::shared_ptr<GameData>) override;
	void PlayerMovement(std::shared_ptr<GameData>);
	void SwordTriggers(std::shared_ptr<GameData>);
	void SwordObjects();

	bool is_grounded = false;
	bool has_sword = false;
	bool is_attacking = false;
	bool is_respawning = false;
	Vector3 respawn_pos;
	Vector3 base_game_respawn;
	bool is_reading = false;
	bool launching = false;

	bool play_jump_sfx = false;
	bool play_sword_sfx = false;

	float lifetime = 0.0f;

	std::shared_ptr<CMOGO> pSwordTrigger;
	std::shared_ptr<CMOGO> pSwordObject;
	Vector3 base_trigger_size;

protected:

};

#endif