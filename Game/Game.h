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
    std::shared_ptr<GameData> m_GD = NULL;			//Data to be shared to all Game Objects as they are ticked
    std::shared_ptr<DrawData> m_DD = NULL;			//Data to be shared to all 3D Game Objects as they are drawn
    std::shared_ptr<DrawData2D> m_DD2D = NULL;	    //Data to be passed by game to all 2D Game Objects via Draw 

    //Basic 3D renderers
    std::shared_ptr<Camera> m_cam = NULL; //principle camera
    std::shared_ptr<TPSCamera> m_TPScam = NULL;//TPS cam
    std::shared_ptr<Light> m_light = NULL; //base light

    //required for the CMO model rendering system
    DirectX::CommonStates* m_states = NULL;
    DirectX::IEffectFactory* m_fxFactory = NULL;

    //basic keyboard and mouse input system
    void ReadInput(); //Get current Mouse and Keyboard states
    std::unique_ptr<DirectX::Keyboard> m_keyboard;
    std::unique_ptr<DirectX::Mouse> m_mouse;

    std::vector<std::shared_ptr<GameObject>> m_GameObjects; //data structure to hold pointers to the 3D Game Objects
    std::vector<std::shared_ptr<GameObject2D>> m_GameObjects2D; //data structure to hold pointers to the 2D Game Objects
    std::vector<std::shared_ptr<GameObject>> m_IntroGOs;
    std::vector<std::shared_ptr<GameObject>> m_BossGOs;

    //list<CMOGO*> m_CMOGameObjects; //data structure to hold pointers to all 3D CMO Game Objects
    //list<CMOGO*> m_PhysicsObjects

    std::vector<std::shared_ptr<CMOGO>> m_ColliderObjects;
    std::vector<std::shared_ptr<CMOGO>> m_PhysicsObjects;
    std::vector<std::shared_ptr<CMOGO>> m_TriggerObjects;

    std::vector<std::shared_ptr<Player>> m_Player;
    std::vector<std::shared_ptr<Terrain>> m_Grounds;
    std::vector<std::shared_ptr<MovingPlatform>> m_Platforms;
    std::vector<std::shared_ptr<CMOGO>> m_Checkpoints;
    std::vector<std::shared_ptr<CMOGO>> m_Coins;
    std::vector<std::shared_ptr<CMOGO>> m_SwordTrigger;
    std::vector<std::shared_ptr<Enemy>> m_Enemies;
    std::vector<std::shared_ptr<CMOGO>> m_Destructibles;
    std::vector<std::shared_ptr<CMOGO>> m_EnemySensors;
    std::vector<std::shared_ptr<Sign>>  m_Signs;
    std::vector<string> m_StringLines;
    std::vector<std::shared_ptr<TextGO2D>> m_TextLines;

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

    //The Important Stuff!!!!!
    std::shared_ptr<Player> pPlayer;
    std::shared_ptr<Coin> pGoldedge;
    std::shared_ptr<Boss> pKazcranak;
    std::shared_ptr<CMOGO> pCore1;
    std::shared_ptr<CMOGO> pCore2;
    std::shared_ptr<CMOGO> pCore3;
    std::shared_ptr<CMOGO> treeCollision;
    std::shared_ptr<CMOGO> secretTrigger;

    //Assorted triggers & cave exterior
    std::shared_ptr<Terrain> pIntroExit;
    std::shared_ptr<Terrain> pDeathTrigger;
    std::shared_ptr<CMOGO> pLaunchpadTrigger;
    std::shared_ptr<Terrain> pBossTrigger;

    //Checkpoint notifications
    std::shared_ptr<TextGO2D> checkpoint_notif;
    float checkpoint_life = 0.0f;
    bool notif_active = false;
    std::shared_ptr<TextGO2D> skip_notif;

    //Text for storing .txt text
    std::shared_ptr<TextGO2D> imported_lore;
    std::shared_ptr<ImageGO2D> secret_bg;

    //Signs & sign images
    std::shared_ptr<Sign> pSign1;
    std::shared_ptr<Sign> pSign2;
    std::shared_ptr<Sign> pSign3;
    std::shared_ptr<Sign> pSign4;
    std::shared_ptr<Sign> pSign5;
    std::shared_ptr<Sign> pSign6;
    std::shared_ptr<Sign> pSign7;
    std::shared_ptr<Sign> pSign8;
    std::shared_ptr<Sign> pSign9;
    std::shared_ptr<ImageGO2D> sign1Image;
    std::shared_ptr<ImageGO2D> sign2Image;
    std::shared_ptr<ImageGO2D> sign3Image;
    std::shared_ptr<ImageGO2D> sign4Image;
    std::shared_ptr<ImageGO2D> sign5Image;
    std::shared_ptr<ImageGO2D> sign6Image;
    std::shared_ptr<ImageGO2D> sign7Image;
    std::shared_ptr<ImageGO2D> sign8Image;
    std::shared_ptr<ImageGO2D> sign9Image;
    
    //Score & lives
    int score = 0;
    std::shared_ptr<TextGO2D> scoreText;
    int lives = 9;
    std::shared_ptr<TextGO2D> livesText;

    //Assorted 2D images for states
    std::shared_ptr<ImageGO2D> title_screen;
    std::shared_ptr<ImageGO2D> credits;
    std::shared_ptr<ImageGO2D> lose_screen;
    float scroll = 2700;
    bool credits_scroll = false;
    bool reset = false;

    //Music & sound effects
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

    //Boss sounds
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

    //Moving platforms
    std::shared_ptr<MovingPlatform> pMovePlat1;
    std::shared_ptr<MovingPlatform> pMovePlat2;
    std::shared_ptr<MovingPlatform> pMovePlat3;
    std::shared_ptr<MovingPlatform> pMovePlat4;
    std::shared_ptr<MovingPlatform> pMovePlat5;
    std::shared_ptr<MovingPlatform> pMovePlat6;
    std::shared_ptr<MovingPlatform> pMovePlat7;
    std::shared_ptr<MovingPlatform> pMovePlat8;
    std::shared_ptr<MovingPlatform> pMovePlat9;
    std::shared_ptr<MovingPlatform> pMovePlatB1;
    std::shared_ptr<MovingPlatform> pMovePlatB2;
    std::shared_ptr<MovingPlatform> pMovePlatB3;
    std::shared_ptr<MovingPlatform> pMovePlatB4;

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
