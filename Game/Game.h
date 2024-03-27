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
#include "SwordTrigger.h"
#include "Coin.h"
#include "Enemy.h"
#include "SwordObject.h"
#include "Sign.h"
#include "SignTrigger.h"

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

    list<GameObject*> m_GameObjects; //data structure to hold pointers to the 3D Game Objects
    list<GameObject2D*> m_GameObjects2D; //data structure to hold pointers to the 2D Game Objects
    list<GameObject*> m_IntroGOs;

    //list<CMOGO*> m_CMOGameObjects; //data structure to hold pointers to all 3D CMO Game Objects
    //list<CMOGO*> m_PhysicsObjects

    std::vector<CMOGO*> m_ColliderObjects;
    std::vector<CMOGO*> m_PhysicsObjects;
    std::vector<CMOGO*> m_TriggerObjects;

    std::vector<CMOGO*> m_Coins;
    std::vector<CMOGO*> m_Enemies;
    std::vector<CMOGO*> m_EnemySensors;
    std::vector<CMOGO*> m_SignTrigger;

    std::vector<CMOGO*> m_SwordTrigger;
    std::vector<SwordTrigger*> m_SwordTriggerVector;
    std::vector<CMOGO*> m_SwordObject;

    void CheckCollision();
    void CheckTriggers();
    void CoinCollision();
    void SwordCollision();
    void EnemyCollision();
    void SensorCollision();
    void SignCollision();

    //sound stuff
	//This uses a simple system, but a better pipeline can be used using Wave Banks
	//See here: https://github.com/Microsoft/DirectXTK/wiki/Creating-and-playing-sounds Using wave banks Section
    std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
    list<Sound*>m_Sounds;

    Player* pPlayer;
    SwordTrigger* pSwordTrigger;
    SwordObject* pSword;

    Terrain* pF1GroundCheck;
    Terrain* pF2GroundCheck;

    Coin* pCoin1;
    Coin* pCoin2;
    Coin* pCoin3;

    Terrain* EnemySensor;
    bool player_spotted = false;
    Enemy* pEnemy1;
    Enemy* pEnemy2;

    TextGO2D* readText;
    SignTrigger* pSignReadTrigger;
    bool is_reading = false;
    Sign* pSigns;
    Sign* pSign1;
    ImageGO2D* sign1Image;

    int score;
    TextGO2D* scoreText;

    ImageGO2D* title_screen;

    bool terrain = false;

    void DisplayMenu();
    void DisplayIntro();
    void DisplayGame();
    void DisplayWin();
    void DisplayLoss();

    void InitMenuAssets();
    void InitGameAssets();
    void InitWinAssets();
    void InitLossAssets();

    void CreateGround();
    void CreateIntroGround();
    void EnemyAI();
};
