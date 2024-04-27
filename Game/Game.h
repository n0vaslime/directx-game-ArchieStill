//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include <list>
#include "CommonStates.h"
#include "../DirectXTK/Inc/Effects.h" //this clashes with a SDK file so must explitily state it
#include "Keyboard.h"
#include "Mouse.h"
#include "Audio.h"
#include "CMOGO.h"
#include "Player.h"
#include "Terrain.h"
#include "TextGO2D.h"
#include "ImageGO2D.h"
#include "Coin.h"
#include "Enemy.h"
#include "Sign.h"
#include "MovingPlatform.h"
#include "Boss.h"
#include "Loop.h"

using std::list;

// Forward declarations
struct GameData;
struct DrawData;
struct DrawData2D;
class GameObject;
class GameObject2D;
class Camera;
class TPSCamera;
class Light;
class Sound;

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game() noexcept;
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND _window, int _width, int _height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int _width, int _height);

    // Properties
    void GetDefaultSize( int& _width, int& _height ) const noexcept;

private:

    void Update(DX::StepTimer const& _timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

    //Scarle Added stuff
    GameData* m_GD = NULL;			//Data to be shared to all Game Objects as they are ticked
    DrawData* m_DD = NULL;			//Data to be shared to all 3D Game Objects as they are drawn
    DrawData2D* m_DD2D = NULL;	    //Data to be passed by game to all 2D Game Objects via Draw 

    //Basic 3D renderers
    Camera* m_cam = NULL; //principle camera
    TPSCamera* m_TPScam = NULL;//TPS cam
    Light* m_light = NULL; //base light

    //required for the CMO model rendering system
    DirectX::CommonStates* m_states = NULL;
    DirectX::IEffectFactory* m_fxFactory = NULL;

    //basic keyboard and mouse input system
    void ReadInput(); //Get current Mouse and Keyboard states
    std::unique_ptr<DirectX::Keyboard> m_keyboard;
    std::unique_ptr<DirectX::Mouse> m_mouse;

    std::vector<GameObject*> m_GameObjects; //data structure to hold pointers to the 3D Game Objects
    std::vector<GameObject2D*> m_GameObjects2D; //data structure to hold pointers to the 2D Game Objects
    std::vector<GameObject*> m_IntroGOs;
    std::vector<GameObject*> m_BossGOs;

    //list<CMOGO*> m_CMOGameObjects; //data structure to hold pointers to all 3D CMO Game Objects
    //list<CMOGO*> m_PhysicsObjects

    std::vector<CMOGO*> m_ColliderObjects;
    std::vector<CMOGO*> m_PhysicsObjects;
    std::vector<CMOGO*> m_TriggerObjects;

    std::vector<Player*> m_Player;
    std::vector<Terrain*> m_Grounds;
    std::vector<MovingPlatform*> m_Platforms;
    std::vector<CMOGO*> m_Checkpoints;
    std::vector<CMOGO*> m_Coins;
    std::vector<CMOGO*> m_SwordTrigger;
    std::vector<Enemy*> m_Enemies;
    std::vector<CMOGO*> m_Destructibles;
    std::vector<CMOGO*> m_EnemySensors;
    std::vector<Sign*>  m_Signs;

    void CheckCollision();
    void CheckTriggers();
    void GroundCheck();
    void CheckpointCheck();
    void CoinCollision();
    void SwordCollision();
    void EnemyCollision();
    void SensorCollision();
    void SignReading();

    //sound stuff
	//This uses a simple system, but a better pipeline can be used using Wave Banks
	//See here: https://github.com/Microsoft/DirectXTK/wiki/Creating-and-playing-sounds Using wave banks Section
    std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
    std::vector<Sound*>m_Sounds;
    std::vector<Loop*>m_Music;

    Player* pPlayer;
    Coin* pFloatingSword;
    Boss* pKazcranak;
    CMOGO* pCore1;
    CMOGO* pCore2;
    CMOGO* pCore3;
    CMOGO* treeCollision;

    Terrain* pIntroExit;
    Terrain* pDeathTrigger;
    CMOGO* pLaunchpadTrigger;
    Terrain* pBossTrigger;

    TextGO2D* checkpoint_notif;
    float checkpoint_life = 0.0f;
    bool notif_active = false;
    TextGO2D* skip_notif;

    Sign* pSign1;
    Sign* pSign2;
    Sign* pSign3;
    Sign* pSign4;
    Sign* pSign5;
    Sign* pSign6;
    Sign* pSign7;
    Sign* pSign8;
    ImageGO2D* sign1Image;
    ImageGO2D* sign2Image;
    ImageGO2D* sign3Image;
    ImageGO2D* sign4Image;
    ImageGO2D* sign5Image;
    ImageGO2D* sign6Image;
    ImageGO2D* sign7Image;
    ImageGO2D* sign8Image;
    
    int score = 0;
    TextGO2D* scoreText;
    int lives = 9;
    TextGO2D* livesText;
    ImageGO2D* spottedImage;

    ImageGO2D* title_screen;
    ImageGO2D* credits;
    ImageGO2D* lose_screen;
    float scroll = 2700;
    bool credits_scroll = false;
    bool reset = false;

    Loop* ambience;
    Loop* intro_music;
    Loop* game_music;
    Loop* boss_intro;
    Loop* boss_music;
    Loop* ending_music;
    Sound* hit_sfx;
    Sound* coin_sfx;
    Sound* death_sfx;
    Sound* jump_sfx;
    Sound* sword_sfx;

    Loop* KZK_intro;
    Sound* combat1;
    Sound* combat2;
    Sound* combat3;
    Sound* combat4;
    Sound* combat5;
    Sound* combat6;
    Sound* hurt1;
    Sound* hurt2;
    Sound* hurt3;
    Loop* KZK_final;

    MovingPlatform* pMovePlat1;
    MovingPlatform* pMovePlat2;
    MovingPlatform* pMovePlat3;
    MovingPlatform* pMovePlat4;
    MovingPlatform* pMovePlat5;
    MovingPlatform* pMovePlat6;
    MovingPlatform* pMovePlat7;
    MovingPlatform* pMovePlat8;
    MovingPlatform* pMovePlat9;

    MovingPlatform* pMovePlatB1;
    MovingPlatform* pMovePlatB2;
    MovingPlatform* pMovePlatB3;
    MovingPlatform* pMovePlatB4;

    void CollectCoin();
    void LoseLife();
    void ReturnToDefault();

    void DisplayMenu();
    void DisplayIntro();
    void DisplayGame();
    void DisplayBoss();
    void DisplayWin();
    void DisplayLoss();

    void CreateGround();
    void CreateIntroGround();
    void CreateBossGround();

    void CreateAudio();
    void CreateUI();
    void CreateCoins();
    void CreateEnemies();
    void CreateSigns();
};
