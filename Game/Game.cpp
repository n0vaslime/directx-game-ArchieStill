//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <time.h>

#include <iostream>
#include <fstream>

//Scarle Headers
#include "GameData.h"
#include "GameState.h"
#include "DrawData.h"
#include "DrawData2D.h"
#include "ObjectList.h"

#include "CMOGO.h"
#include <DirectXCollision.h>
#include "Collision.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_11_0)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND _window, int _width, int _height)
{
    m_window = _window;
    m_outputWidth = std::max(_width, 1);
    m_outputHeight = std::max(_height, 1);

    CreateDevice();

    CreateResources();

    //seed the random number generator
    srand((UINT)time(NULL));

    //set up keyboard and mouse system
    //documentation here: https://github.com/microsoft/DirectXTK/wiki/Mouse-and-keyboard-input
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(_window);
    m_mouse->SetMode(Mouse::MODE_RELATIVE);
    //Hide the mouse pointer
    ShowCursor(false);

    //create GameData struct and populate its pointers
    m_GD = new GameData;
    m_GD->m_GS = GS_MENU;

    //set up systems for 2D rendering
    m_DD2D = new DrawData2D();
    m_DD2D->m_Sprites.reset(new SpriteBatch(m_d3dContext.Get()));
    m_DD2D->m_Font.reset(new SpriteFont(m_d3dDevice.Get(), L"..\\Assets\\italic.spritefont"));
    m_states = new CommonStates(m_d3dDevice.Get());

    //set up DirectXTK Effects system
    m_fxFactory = new EffectFactory(m_d3dDevice.Get());
    //Tell the fxFactory to look to the correct build directory to pull stuff in from
    ((EffectFactory*)m_fxFactory)->SetDirectory(L"..\\Assets");
    //init render system for VBGOs
    VBGO::Init(m_d3dDevice.Get());

    //set audio system
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif
    m_audioEngine = std::make_unique<AudioEngine>(eflags);

    //create a base light
    m_light = new Light(Vector3(0.0f, 100.0f, 160.0f), Color(1.0f, 1.0f, 1.0f, 1.0f), Color(110.4f, 110.1f, 0.1f, 1.0f));
    // m_GameObjects.push_back(m_light);

    //find how big my window is to correctly calculate my aspect ratio
    float AR = (float)_width / (float)_height;
    
    CreateIntroGround();
    CreateGround();

    //creating basic 2D text notifications
        checkpoint_notif = std::make_shared<TextGO2D>("Checkpoint!");
    checkpoint_notif->SetPos(Vector2(30, 10));
    checkpoint_notif->SetColour(Color((float*)&Colors::Blue));
    checkpoint_notif->SetScale(1);
    checkpoint_notif->SetRendered(false);
    m_GameObjects2D.push_back(checkpoint_notif);
        skip_notif = std::make_shared<TextGO2D>("PRESS ENTER TO SKIP");
    skip_notif->SetPos(Vector2(10, 10));
    skip_notif->SetColour(Color((float*)&Colors::Red));
    skip_notif->SetScale(0.5f);
    skip_notif->SetRendered(false);
    m_GameObjects2D.push_back(skip_notif);

    //Secret Lore - creating background
    secret_bg = std::make_shared<ImageGO2D>("UIBackground", m_d3dDevice.Get());
    secret_bg->SetScale(Vector2(2, 4));
    m_GameObjects2D.push_back(secret_bg);
    secret_bg->SetRendered(false);
    //Secret Lore - importing .txt
    string line;
    int lore_offset = 10;
    m_StringLines.push_back(line);
    ifstream secret_lore("../RawAssets/TheStoryOfLordKazcranak.txt");
    if (secret_lore.is_open())
    {
        while (getline(secret_lore, line))
        {
            //Secret Lore - setting .txt lines to TextGO2D on different lines
            for (int i = 0; i < m_StringLines.size(); i++)
            {
                imported_lore = std::make_shared<TextGO2D>(line + "\n");
                imported_lore->SetPos(Vector2(20, lore_offset));
                imported_lore->SetScale(0.4f);
                imported_lore->SetColour(Color((float*)&Colors::White));
                imported_lore->Tick(m_GD);
                lore_offset += 25;
                m_GameObjects2D.push_back(imported_lore);
                m_TextLines.push_back(imported_lore);
            }
            //Secret Lore - pushing back the whole text as one object to set as rendered
            for (int j = 0; j < m_TextLines.size(); j++)
                m_TextLines[j]->SetRendered(false);
        }
        secret_lore.close();
    }
    
    //add Player - player object and adding swords to player class
    pPlayer = std::make_shared<Player>("Player", m_d3dDevice.Get(), m_fxFactory);
    m_Player.push_back(pPlayer);
    m_GameObjects.push_back(pPlayer);
    m_IntroGOs.push_back(pPlayer);
    m_BossGOs.push_back(pPlayer);
    m_GameObjects.push_back(pPlayer->pSwordTrigger);
    m_GameObjects.push_back(pPlayer->pSwordObject);
    m_IntroGOs.push_back(pPlayer->pSwordTrigger);
    m_IntroGOs.push_back(pPlayer->pSwordObject);
    m_BossGOs.push_back(pPlayer->pSwordTrigger);
    m_BossGOs.push_back(pPlayer->pSwordObject);

    //add Lord Kazcranak - boss fight enemy
    pKazcranak = std::make_shared<Boss>("LordKazcranak", m_d3dDevice.Get(), m_fxFactory);
    m_BossGOs.push_back(pKazcranak);
    m_ColliderObjects.push_back(pKazcranak);
    m_Destructibles.push_back(pKazcranak);
    m_BossGOs.push_back(pKazcranak->pBossProjectile);
    pKazcranak->pBossProjectile->SetRendered(false);
    m_TriggerObjects.push_back(pKazcranak->pBossProjectile);
    m_Destructibles.push_back(pKazcranak->pBossProjectile);

    //add a PRIMARY camera
    m_TPScam = new TPSCamera(0.5f * XM_PI, AR, 1.0f, 10000.0f, pPlayer, Vector3::UnitY, Vector3(0.0f, 0.0f, 0.1f)); // Vector3(0,0,0.1f)
    // m_GameObjects.push_back(m_TPScam);
    // m_IntroGOs.push_back(m_TPScam);
    // m_BossGOs.push_back(m_TPScam);

    CreateBossGround();

    CreateCoins();
    CreateEnemies();
    CreateSigns();

    //L-system like tree
    //     tree = std::make_shared<Tree>(4, 3, 1.0f, 8.0f * Vector3::Up, XM_PI / 6.0f, "JEMINA vase -up", m_d3dDevice.Get(), m_fxFactory);
    // m_GameObjects.push_back(tree);
    //     treeCollision = std::make_shared<CMOGO>("Player", m_d3dDevice.Get(), m_fxFactory);
    // treeCollision->SetPos(Vector3(tree->GetPos().x, tree->GetPos().y, tree->GetPos().z));
    // treeCollision->SetScale(Vector3(2.5f, 10, 2.5f));
    // m_GameObjects.push_back(treeCollision);
    // m_ColliderObjects.push_back(treeCollision);
    //     secretTrigger = std::make_shared<CMOGO>("Player", m_d3dDevice.Get(), m_fxFactory);
    // secretTrigger->SetPos(Vector3(tree->GetPos().x, tree->GetPos().y, tree->GetPos().z));
    // secretTrigger->SetScale(Vector3(5, 10, 5));
    // m_GameObjects.push_back(secretTrigger);
    // m_TriggerObjects.push_back(secretTrigger);

    //create DrawData struct and populate its pointers
    m_DD = new DrawData;
    m_DD->m_pd3dImmediateContext = nullptr;
    m_DD->m_states = m_states;
    m_DD->m_cam = m_TPScam;
    m_DD->m_light = m_light;

    //2D screens
    title_screen = std::make_shared<ImageGO2D>("TitleScreen", m_d3dDevice.Get());
    title_screen->SetPos(Vector2(400,300));
    title_screen->SetScale(0.35f);
    credits = std::make_shared<ImageGO2D>("Goldedge2Credits", m_d3dDevice.Get());
    credits->SetPos(Vector2(400, 2700));
    credits->SetScale(1.5f);
    lose_screen = std::make_shared<ImageGO2D>("GameOverScreen", m_d3dDevice.Get());
    lose_screen->SetPos(Vector2(400, 300));
    lose_screen->SetScale(0.35f);

    CreateAudio();

    DisplayMenu();
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}
// Updates the world.
void Game::Update(DX::StepTimer const& _timer)
{
    //make deltatime consistent
    if (m_GD->m_dt > 1 / 30)
        m_GD->m_dt = 1 / 30;

    float elapsedTime = float(_timer.GetElapsedSeconds());
    m_GD->m_dt = elapsedTime;

    //this will update the audio engine but give us chance to do something else if that isn't working
    if (!m_audioEngine->Update())
    {
        if (m_audioEngine->IsCriticalError())
        {
            // We lost the audio device!
        }
    }
    else
    {
        //update sounds playing
        for (std::vector<Sound*>::iterator it = m_Sounds.begin(); it != m_Sounds.end(); it++)
        {
            (*it)->Tick(m_GD);
        }
    }

    ReadInput();

    //////  update all objects
    if (m_GD->m_GS == GS_GAME)
    {
        // for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
        // {
        //     if ((*it)->isRendered() ||
        //         (*it) == pMovePlat1->GroundCheck ||
        //         (*it) == pMovePlat2->GroundCheck ||
        //         (*it) == pMovePlat3->GroundCheck ||
        //         (*it) == pMovePlat4->GroundCheck ||
        //         (*it) == pMovePlat5->GroundCheck ||
        //         (*it) == pMovePlat6->GroundCheck ||
        //         (*it) == pMovePlat7->GroundCheck ||
        //         (*it) == pMovePlat8->GroundCheck ||
        //         (*it) == pMovePlat9->GroundCheck ||
        //         (*it) == treeCollision || 
        //         (*it) == secretTrigger)
        //             (*it)->Tick(m_GD);
        // }

        for (auto GameObject : m_GameObjects)
        {
            if (GameObject->isRendered() ||
                GameObject == pMovePlat1->GroundCheck ||
                GameObject == pMovePlat2->GroundCheck ||
                GameObject == pMovePlat3->GroundCheck ||
                GameObject == pMovePlat4->GroundCheck ||
                GameObject == pMovePlat5->GroundCheck ||
                GameObject == pMovePlat6->GroundCheck ||
                GameObject == pMovePlat7->GroundCheck ||
                GameObject == pMovePlat8->GroundCheck ||
                GameObject == pMovePlat9->GroundCheck ||
                GameObject == treeCollision ||
                GameObject == secretTrigger)
            {
                GameObject->Tick(m_GD);
            }
        }

        for (int i = 0; i < m_Enemies.size(); i++)
        {
            m_Enemies[i]->player_facing = pPlayer->GetYaw();
            m_Enemies[i]->EnemySensor->SetRendered(false);
        }
    }
    else if (m_GD->m_GS == GS_INTRO)
    {
        // for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
        // {
        //     if ((*it)->isRendered())
        //         (*it)->Tick(m_GD);
        // }
        for (auto GameObject : m_IntroGOs)
        {
            if (GameObject->isRendered())
                GameObject->Tick(m_GD);
        }
    }
    else if (m_GD->m_GS == GS_BOSS)
    {
        // for (std::vector<GameObject*>::iterator it = m_BossGOs.begin(); it != m_BossGOs.end(); it++)
        // {
        //     if ((*it)->isRendered() ||
        //         (*it) == pMovePlatB1->GroundCheck ||
        //         (*it) == pMovePlatB2->GroundCheck ||
        //         (*it) == pMovePlatB3->GroundCheck ||
        //         (*it) == pMovePlatB4->GroundCheck)
        //         (*it)->Tick(m_GD);
        // }
        for (auto GameObject : m_BossGOs)
        {
            if (GameObject->isRendered() ||
                GameObject == pMovePlatB1->GroundCheck ||
                GameObject == pMovePlatB2->GroundCheck ||
                GameObject == pMovePlatB3->GroundCheck ||
                GameObject == pMovePlatB4->GroundCheck)
            {
                GameObject->Tick(m_GD);
            }
        }
        if (pKazcranak->is_talking)
            pPlayer->is_attacking = true;

        //determines where Kazcranak fires projectiles
        pKazcranak->player_adjacent = pPlayer->GetPos().x / 2;
        pKazcranak->player_opposite = pPlayer->GetPos().z / 2;
        pKazcranak->player_pitch = pPlayer->GetPitch() + 0.15f;
        //looking down
        if (pKazcranak->player_pitch > 0.9f)
            pKazcranak->player_pitch = 0.9f;
        //looking up
        if (pKazcranak->player_pitch < -0.25f)
            pKazcranak->player_pitch = -0.25f;
    }
    else if (m_GD->m_GS == GS_WIN)
    {
        //credits are just a super long image slowly moving up!
        if (credits_scroll)
        {
            if (scroll >= -2250)
            {
                scroll = scroll -= m_GD->m_dt * 20;
                credits->SetPos(Vector2(400, scroll));
            }
            else
            {
                //END AT -2250 (end of credits)
                scroll == -2250;
            }
        }
        else
            credits->SetPos(Vector2(400, 2700));
    }

    // for (std::vector<GameObject2D*>::iterator it = m_GameObjects2D.begin(); it != m_GameObjects2D.end(); it++)
    // {
    //     if ((*it)->isRendered())
    //         (*it)->Tick(m_GD);
    // }
    for (auto GameObject : m_GameObjects2D)
    {
        if (GameObject->isRendered())
            GameObject->Tick(m_GD);
    }

    //make ground and sign checks invisible
    for (int i = 0; i < m_Grounds.size(); i++)
        m_Grounds[i]->GroundCheck->SetRendered(false);
    for (int i = 0; i < m_Platforms.size(); i++)
        m_Platforms[i]->GroundCheck->SetRendered(false);
    for (int i = 0; i < m_Signs.size(); i++)
        m_Signs[i]->SignTrigger->SetRendered(false);

    //boss fight updating
    if (!pKazcranak->is_talking && m_GD->m_GS == GS_BOSS)
    {
        skip_notif->SetRendered(false);
        boss_intro->Stop();
        KZK_intro->Stop();
        if (!pKazcranak->is_dying)
            boss_music->Play();

        //plays random voice lines
        if (pKazcranak->play_combat_sfx && !pKazcranak->play_hurt_sfx)
        {
            int combat_sfx = (rand() % 6) + 1;
            switch (combat_sfx)
            {
            case(1):
                combat1->Play();
                break;
            case(2):
                combat2->Play();
                break;
            case(3):
                combat3->Play();
                break;
            case(4):
                combat4->Play();
                break;
            case(5):
                combat5->Play();
                break;
            case(6):
                combat6->Play();
                break;
            default:
                break;
            }
            pKazcranak->play_combat_sfx = false;
        }

        //Big K flying into the air after he's defeated
        if (pKazcranak->dying_words)
        {
            boss_music->Stop();
            KZK_final->Play();
            ending_music->Play();
            pKazcranak->dying_words = false;
        }
        //once his monologue ends, roll credits
        if(pKazcranak->dying_time >= 42.2f)
        {
            KZK_final->Stop();
            DisplayWin();
            pKazcranak->dying_time = 0;
        }
    }

    CheckCollision();
    CheckTriggers();
    GroundCheck();
    CheckpointCheck();
    CoinCollision();
    EnemyCollision();
    SensorCollision();
    SwordCollision();
    SignReading();
    m_TPScam->Tick(m_GD);
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();
    
    //set immediate context of the graphics device
    m_DD->m_pd3dImmediateContext = m_d3dContext.Get();

    //set which camera to be used
    // m_DD->m_cam = std::make_shared<TPSCamera> m_TPScam;

    //update the constant buffer for the rendering of VBGOs
    VBGO::UpdateConstantBuffer(m_DD);

    //Draw 3D Game Objects
    if (m_GD->m_GS == GS_GAME)
    {
        // for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
        // {
        //     if ((*it)->isRendered()
        //         && (*it) != pPlayer->pSwordTrigger
        //         && (*it) != pLaunchpadTrigger
        //         && (*it) != pBossTrigger
        //         && (*it) != treeCollision
        //         && (*it) != secretTrigger)
        //     {
        //         (*it)->Draw(m_DD);
        //     }
        // }
        for (auto GameObject : m_GameObjects)
        {
            if (GameObject->isRendered()
                && GameObject != pPlayer->pSwordTrigger
                && GameObject != pLaunchpadTrigger
                && GameObject != pBossTrigger
                && GameObject != treeCollision
                && GameObject != secretTrigger)
            {
                GameObject->Draw(m_DD);
            }
        }
        
        // for (std::vector<CMOGO*>::iterator it = m_EnemySensors.begin(); it != m_EnemySensors.end(); it++)
        //     if ((*it)->isRendered())
        //         (*it)->Draw(m_DD);
        for (auto CMOGO : m_EnemySensors)
            if (CMOGO->isRendered())
                CMOGO->Draw(m_DD);
    }
    else if (m_GD->m_GS == GS_INTRO)
    {
        // for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
        // {
        //     if ((*it)->isRendered()
        //         && (*it) != pPlayer->pSwordTrigger)
        //     {
        //         (*it)->Draw(m_DD);
        //     }
        // }
        for (auto GameObject : m_IntroGOs)
        {
            if (GameObject->isRendered()
                && GameObject != pPlayer->pSwordTrigger)
                GameObject->Draw(m_DD);
        }
    }
    else if (m_GD->m_GS == GS_BOSS)
    {
        // for (std::vector<GameObject*>::iterator it = m_BossGOs.begin(); it != m_BossGOs.end(); it++)
        // {
        //     if ((*it)->isRendered()
        //         && (*it) != pPlayer->pSwordTrigger)
        //     {
        //         (*it)->Draw(m_DD);
        //     }
        // }
        for (auto GameObject : m_BossGOs)
        {
            if (GameObject->isRendered()
                && GameObject != pPlayer->pSwordTrigger)
                GameObject->Draw(m_DD);
        }
    }

    // Draw sprite batch stuff
    m_DD2D->m_Sprites->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
    // for (std::vector<GameObject2D*>::iterator it = m_GameObjects2D.begin(); it != m_GameObjects2D.end(); it++)
    // {
    //     if ((*it)->isRendered())
    //     {
    //         (*it)->Draw(m_DD2D);
    //     }
    // }
    for (auto GameObject2D : m_GameObjects2D)
        if (GameObject2D->isRendered())
            GameObject2D->Draw(m_DD2D);
    m_DD2D->m_Sprites->End();

    //drawing text screws up the Depth Stencil State, this puts it back again!
    m_d3dContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}
// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // Game is becoming active window.
    ShowCursor(false);
}
void Game::OnDeactivated()
{
    // Game is becoming background window.
    ShowCursor(true);
}
void Game::OnSuspending()
{
    // Game is being power-suspended (or minimized).
    ShowCursor(true);
}
void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // Game is being power-resumed (or returning from minimize).
    ShowCursor(false);
}
void Game::OnWindowSizeChanged(int _width, int _height)
{
    m_outputWidth = std::max(_width, 1);
    m_outputHeight = std::max(_height, 1);

    CreateResources();
}
void Game::GetDefaultSize(int& _width, int& _height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    _width = 800;
    _height = 600;
}
// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    //creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    //something missing on the machines in 2Q28
    //this should work!
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
        ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    // TODO: Initialize device dependent objects here (independent of window size).
}
// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(static_cast<UINT>(std::size(nullViews)), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
            ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
}
void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}

void Game::ReadInput()
{
    m_GD->m_KBS = m_keyboard->GetState();
    m_GD->m_KBS_tracker.Update(m_GD->m_KBS);
    m_GD->m_MS = m_mouse->GetState();

    //lock the cursor to the centre of the window
    RECT window;
    GetWindowRect(m_window, &window);
    SetCursorPos((window.left + window.right) >> 1, (window.bottom + window.top) >> 1);

    //quit game on hitting escape
    if (m_GD->m_KBS.Escape)
    {
        ExitGame();
    }

    if (pPlayer->play_jump_sfx)
    {
        jump_sfx->Play();
        pPlayer->play_jump_sfx = false;
    }
    if (pPlayer->play_sword_sfx)
    {
        sword_sfx->Play();
        pPlayer->play_sword_sfx = false;
    }

    //state transitions from pressing Enter
    switch (m_GD->m_GS)
    {
        case(GS_MENU):
        {
            if (m_GD->m_KBS_tracker.pressed.Enter && m_GD->m_GS == GS_MENU)
            {
                m_GD->m_GS = GS_INTRO;
                DisplayIntro();
            }
        }
        case(GS_BOSS):
        {
            if (m_GD->m_KBS.Enter && pKazcranak->is_talking && m_GD->m_GS == GS_BOSS)
            {
                pKazcranak->SetPos(Vector3(pKazcranak->GetPos().x, 100, pKazcranak->GetPos().z));
                pKazcranak->is_talking = false;
            }
        }
        case(GS_LOSS):
        {
            if (m_GD->m_KBS_tracker.pressed.Enter && m_GD->m_GS == GS_LOSS)
            {
                m_GD->m_KBS_tracker.released.Enter;
                ReturnToDefault();
            }
        }
        case(GS_WIN):
        {
            if (m_GD->m_KBS_tracker.pressed.Enter && m_GD->m_GS == GS_WIN)
            {
                m_GD->m_KBS_tracker.released.Enter;
                ReturnToDefault();
            }
        }
    default:
        break;
    }
}

void Game::CheckCollision()
{
    for (int i = 0; i < m_Player.size(); i++) for (int j = 0; j < m_ColliderObjects.size(); j++)
    {
        if (m_ColliderObjects[j]->isRendered() && m_Player[i]->Intersects(*m_ColliderObjects[j]))
        {
            XMFLOAT3 eject_vect = Collision::ejectionCMOGO(*m_Player[i], *m_ColliderObjects[j]);
            auto pos = m_Player[i]->GetPos();
            m_Player[i]->SetPos(pos - eject_vect);
        }
    }
}
void Game::CheckTriggers()
{
    for (int i = 0; i < m_Player.size(); i++) for (int j = 0; j < m_TriggerObjects.size(); j++)
    {
        if (m_Player[i]->Intersects(*m_TriggerObjects[j]) && m_TriggerObjects[j]->isRendered())
        {
            if (m_TriggerObjects[j] == pGoldedge)
            {
                pGoldedge->SetRendered(false);
                pPlayer->has_sword = true;
            }
            if (m_TriggerObjects[j] == pIntroExit)
            {
                CreateUI();
                DisplayGame();
            }
            if (m_TriggerObjects[j] == pDeathTrigger)
            {
                LoseLife();
            }
            if (m_TriggerObjects[j] == pLaunchpadTrigger)
            {
                if (m_GD->m_KBS.Space)
                    pPlayer->launching = true;
            }
            if (m_TriggerObjects[j] == pBossTrigger)
            {
                DisplayBoss();
            }
            if (m_TriggerObjects[j] == pKazcranak->pBossProjectile)
            {
                m_TriggerObjects[j]->SetRendered(false);
                LoseLife();
            }
            if (m_TriggerObjects[j] == secretTrigger && score == 30)
            {
                secret_bg->SetRendered(true);
                for (int secret = 0; secret < m_TextLines.size(); secret++)
                    m_TextLines[secret]->SetRendered(true);
            }
        }
        else
        {
            secret_bg->SetRendered(false);
            for (int secret = 0; secret < m_TextLines.size(); secret++)
                m_TextLines[secret]->SetRendered(false);
        }
    }
}
void Game::GroundCheck()
{
    for (int i = 0; i < m_Player.size(); i++) 
        for (int gnd = 0; gnd < m_Grounds.size(); gnd++)
        for (int plt = 0; plt < m_Platforms.size(); plt++)
        {
            if (m_Player[i]->Intersects(*m_Grounds[gnd]->GroundCheck) ||
                m_Player[i]->Intersects(*m_Platforms[plt]->GroundCheck))
            {
                pPlayer->is_grounded = true;
            }
        }
}
void Game::CheckpointCheck()
{
    for (int i = 0; i < m_Player.size(); i++) for (int j = 0; j < m_Checkpoints.size(); j++)
    {
        if (m_Player[i]->Intersects(*m_Checkpoints[j]) && m_Checkpoints[j]->isRendered())
        {
            //changes player's respawn when colliding
            pPlayer->respawn_pos = m_Checkpoints[j]->GetPos();
            checkpoint_notif->SetRendered(true);
            notif_active = true;
        }
        if (notif_active)
        {
            //checkpoint notification is active for 2 seconds after leaving checkpoint bounds
            checkpoint_life += m_GD->m_dt;
            if (checkpoint_life >= 2)
            {
                checkpoint_life = 0;
                checkpoint_notif->SetRendered(false);
                notif_active = false;
            }
        }
    }
}

void Game::CoinCollision()
{
    for (int i = 0; i < m_Coins.size(); i++) for (int j = 0; j < m_Player.size(); j++)
    {
        if (m_Coins[i]->isRendered() && m_Coins[i]->Intersects(*m_Player[j]))
        {
            m_Coins[i]->SetRendered(false);
            CollectCoin();
        }
    }
}
void Game::EnemyCollision()
{
    for (int i = 0; i < m_Enemies.size(); i++) for (int j = 0; j < m_Player.size(); j++)
    {
        if (m_Enemies[i]->isRendered() && m_Enemies[i]->Intersects(*m_Player[j]))
        {
            LoseLife();
        }
    }
}
void Game::SensorCollision()
{
    for (int i = 0; i < m_Player.size(); i++) for (int j = 0; j < m_EnemySensors.size(); j++)
    {
        if (m_Player[i]->Intersects(*m_EnemySensors[j]))
        {
            m_Enemies[j]->player_spotted = true;
        }
        else
        {
            m_Enemies[j]->player_spotted = false;
        }   
    }
}
void Game::SwordCollision()
{
    if (pPlayer->is_attacking)
        m_SwordTrigger.push_back(pPlayer->pSwordTrigger);

    if (pPlayer->lifetime == 0.0f)
        m_SwordTrigger.clear();

    for (int i = 0; i < m_Enemies.size(); i++) for (int j = 0; j < m_Destructibles.size(); j++)
        for (int sword = 0; sword < m_SwordTrigger.size(); sword++)
    {
        if (m_Enemies[i]->isRendered() && m_Enemies[i]->Intersects(*m_SwordTrigger[sword]))
        {
            hit_sfx->Play();
            m_Enemies[i]->SetRendered(false);
            m_Enemies[i]->EnemySensor->SetRendered(false);
        }
        if (m_Destructibles[j]->isRendered() && m_Destructibles[j]->Intersects(*m_SwordTrigger[sword]))
        {
            if (m_Destructibles[j] == pKazcranak)
            {
                pKazcranak->is_dying = true;
                pKazcranak->dying_words = true;
            }
            else
            {
                m_Destructibles[j]->SetRendered(false);
                hit_sfx->Play();
                if (m_Destructibles[j] == pCore1 ||
                    m_Destructibles[j] == pCore2 ||
                    m_Destructibles[j] == pCore3)
                {
                    if (!pKazcranak->play_combat_sfx)
                        pKazcranak->play_hurt_sfx = true;

                    pKazcranak->boss_health--;
                    if (pKazcranak->boss_health == 2)
                        hurt1->Play();
                    if (pKazcranak->boss_health == 1)
                        hurt2->Play();
                    if (pKazcranak->boss_health == 0)
                        hurt3->Play();
                }
            }
        }
    }
}
void Game::SignReading()
{
    for (int i = 0; i < m_Player.size(); i++) for (int j = 0; j < m_Signs.size(); j++)
    {
        if (m_Signs[j]->isRendered() && m_Player[i]->Intersects(*m_Signs[j]->SignTrigger))
        {
            m_Signs[j]->ReadText->SetRendered(true);
            if (m_GD->m_KBS.E && !pPlayer->is_reading)
            {
                m_Signs[j]->is_reading = true;
            }
        }
        else
        {
            m_Signs[j]->ReadText->SetRendered(false);
            m_Signs[j]->is_reading = false;
            sign1Image->SetRendered(false);
            sign2Image->SetRendered(false);
            sign3Image->SetRendered(false);
            sign4Image->SetRendered(false);
            sign5Image->SetRendered(false);
            sign6Image->SetRendered(false);
            sign7Image->SetRendered(false);
            sign8Image->SetRendered(false);
            sign9Image->SetRendered(false);
        }

        //choosing image to render depending on the sign
        if (pSign1->is_reading)
            sign1Image->SetRendered(true);
        if (pSign2->is_reading)
            sign2Image->SetRendered(true);
        if (pSign3->is_reading)
            sign3Image->SetRendered(true);
        if (pSign4->is_reading)
            sign4Image->SetRendered(true);
        if (pSign5->is_reading)
            sign5Image->SetRendered(true);
        if (pSign6->is_reading)
            sign6Image->SetRendered(true);
        if (pSign7->is_reading)
            sign7Image->SetRendered(true);
        if (pSign8->is_reading)
            sign8Image->SetRendered(true);
        if (pSign9->is_reading)
            sign9Image->SetRendered(true);

        //removing read prompt while player is reading
        if (m_Signs[j]->is_reading)
            m_Signs[j]->ReadText->SetRendered(false);
    }
}

void Game::CollectCoin()
{
    scoreText->SetRendered(false);
    coin_sfx->Play();
    score++;
    scoreText = std::make_shared<TextGO2D>(std::to_string(score));
    if (score < 10)
        scoreText->SetPos(Vector2(705, 15));
    else
        scoreText->SetPos(Vector2(690, 15));
    scoreText->SetColour(Color((float*)&Colors::Black));
    scoreText->SetScale(Vector2(1.1f, 1));
    m_GameObjects2D.push_back(scoreText);
}
void Game::LoseLife()
{
    livesText->SetRendered(false);
    death_sfx->Play();
    lives--;
    livesText = std::make_shared<TextGO2D>(std::to_string(lives));
    livesText->SetPos(Vector2(595, 15));
    livesText->SetColour(Color((float*)&Colors::Black));
    livesText->SetScale(Vector2(1.1f, 1));
    m_GameObjects2D.push_back(livesText);
    pPlayer->is_respawning = true;
    if (lives == 0)
        DisplayLoss();
}
void Game::ReturnToDefault()
{
    reset = true;
    if (reset)
    {
        reset = false;
        score = 0;
        lives = 9;
        scroll = 2700;
        skip_notif->SetRendered(false);
        m_GameObjects2D.clear();
        m_GD->m_GS = GS_MENU;
        DisplayMenu();
        m_GameObjects2D.push_back(checkpoint_notif);
        m_GameObjects2D.push_back(skip_notif);
        for (int i = 0; i < m_Signs.size(); i++)
            m_GameObjects2D.push_back(m_Signs[i]->ReadText);
        m_GameObjects2D.push_back(sign1Image);
        m_GameObjects2D.push_back(sign2Image);
        m_GameObjects2D.push_back(sign3Image);
        m_GameObjects2D.push_back(sign4Image);
        m_GameObjects2D.push_back(sign5Image);
        m_GameObjects2D.push_back(sign6Image);
        m_GameObjects2D.push_back(sign7Image);
        m_GameObjects2D.push_back(sign8Image);
        m_GameObjects2D.push_back(sign9Image);
        m_GameObjects2D.push_back(secret_bg);
        for(int i = 0; i < m_TextLines.size(); i++)
            m_GameObjects2D.push_back(m_TextLines[i]);
        credits_scroll = false;
        credits->SetPos(Vector2(400, 2700));
        game_music->Stop();
        boss_music->Stop();
        ending_music->Stop();
        pPlayer->respawn_pos = Vector3(0, -7.5f, -10);
        pPlayer->is_respawning = true;
        pPlayer->has_sword = false;
    }
}

void Game::DisplayMenu()
{
    //set menu active
    m_GD->m_GS = GS_MENU;
    title_screen->SetRendered(true);
    m_GameObjects2D.push_back(title_screen);
    pPlayer->SetYaw(0);
    ambience->Play();

    //set others inactive
    // for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
    //     (*it)->SetRendered(false);
    for (auto GameObject : m_GameObjects)
        GameObject->SetRendered(false);
}
void Game::DisplayIntro()
{
    //set intro active
    m_GD->m_GS = GS_INTRO;
    ambience->Stop();
    intro_music->Play();

    //for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
    //    (*it)->SetRendered(true);
    for (auto GameObject : m_IntroGOs)
        GameObject->SetRendered(true);

    pPlayer->pSwordTrigger->SetRendered(false);
    pPlayer->pSwordObject->SetRendered(false);
    title_screen->SetRendered(false);
}
void Game::DisplayGame()
{
    //set game active
    m_GD->m_GS = GS_GAME;
    intro_music->Stop();
    game_music->Play();

    pPlayer->respawn_pos = pPlayer->base_game_respawn;
    pPlayer->is_respawning = true;
    pPlayer->SetPos(Vector3(pPlayer->GetPos().x, pPlayer->GetPos().y + 25, pPlayer->GetPos().z));
    // for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
    //     (*it)->SetRendered(true);
    for (auto GameObject : m_GameObjects)
        GameObject->SetRendered(true);
    pPlayer->pSwordTrigger->SetRendered(false);

    //set others inactive
    // for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
    // {
    //     if ((*it) == pPlayer)
    //         (*it)->SetRendered(true);
    //     else
    //         (*it)->SetRendered(false);
    // }
    for (auto GameObject : m_IntroGOs)
    {
        if (GameObject == pPlayer)
            GameObject->SetRendered(true);
        else
            GameObject->SetRendered(false);
    }
}
void Game::DisplayBoss()
{
    //set boss active
    m_GD->m_GS = GS_BOSS;
    pKazcranak->SetScale(1.25f);
    pKazcranak->SetPos(Vector3(0, 375, 0));
    pKazcranak->SetPitch(0);
    pKazcranak->is_talking = true;
    pKazcranak->is_dying = false;
    pKazcranak->boss_health = 3;

    skip_notif->SetRendered(true);
    game_music->Stop();
    boss_intro->Play();
    KZK_intro->Play();

    pPlayer->respawn_pos = Vector3(0, 5, 50);
    pPlayer->is_respawning = true;
    pPlayer->is_attacking = true;
    
    // for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
    // {
    //     (*it)->SetRendered(false);
    // }
    // for (std::vector<GameObject*>::iterator it = m_BossGOs.begin(); it != m_BossGOs.end(); it++)
    // {
    //     (*it)->SetRendered(true);
    // }

    for (auto GameObject : m_BossGOs)
        GameObject->SetRendered(true);
    for (auto GameObject : m_GameObjects)
        GameObject->SetRendered(false);

    pPlayer->pSwordTrigger->SetRendered(false);
}
void Game::DisplayWin()
{
    //set win active
    m_GD->m_GS = GS_WIN;
    m_GameObjects2D.clear();
    credits_scroll = true;
    credits->SetPos(Vector2(400, 2700));
    credits->SetRendered(true);
    m_GameObjects2D.push_back(credits);
    skip_notif->SetRendered(true);
    m_GameObjects2D.push_back(skip_notif);

    //set others inactive
    // for (std::vector<GameObject*>::iterator it = m_BossGOs.begin(); it != m_BossGOs.end(); it++)
    //     (*it)->SetRendered(false);
    for (auto GameObject : m_BossGOs)
        GameObject->SetRendered(false);
}
void Game::DisplayLoss()
{
    //set loss active
    m_GD->m_GS = GS_LOSS;
    m_GameObjects2D.clear();
    lose_screen->SetRendered(true);
    m_GameObjects2D.push_back(lose_screen);

    //set others inactive
    // for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
    //     (*it)->SetRendered(false);
    // for (std::vector<GameObject*>::iterator it = m_BossGOs.begin(); it != m_BossGOs.end(); it++)
    //     (*it)->SetRendered(false);
    for (auto GameObject : m_GameObjects)
        GameObject->SetRendered(false);
    for (auto GameObject : m_BossGOs)
        GameObject->SetRendered(false);
}

void Game::CreateGround()
{
    //Death trigger
    pDeathTrigger = std::make_shared<Terrain>("GreenCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -50, 0), 0.0f, 0.0f, 0.0f, Vector3(1000, 0.01f, 1000));
    m_GameObjects.push_back(pDeathTrigger);
    m_BossGOs.push_back(pDeathTrigger);
    m_TriggerObjects.push_back(pDeathTrigger);
    //Boss fight trigger
    pBossTrigger = std::make_shared<Terrain>("GreenCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 900, 0), 0.0f, 0.0f, 0.0f, Vector3(1000, 1, 1000));
    m_GameObjects.push_back(pBossTrigger);
    m_TriggerObjects.push_back(pBossTrigger);

    //Cave exterior
        std::shared_ptr<Terrain> pCave = std::make_shared<Terrain>("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, 200), 0.0f, 0.0f, 0.0f, Vector3(7.5f, 7.5f, 25));
    m_GameObjects.push_back(pCave);
    m_ColliderObjects.push_back(pCave);

    // LAYER 1 - BASICS
        std::shared_ptr<Terrain> pGround1 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0,-10,0), 0.0f, 0.0f, 0.0f, Vector3(10, 1, 25));
    m_GameObjects.push_back(pGround1);
    m_ColliderObjects.push_back(pGround1);
    m_Grounds.push_back(pGround1);
    m_GameObjects.push_back(pGround1->GroundCheck);
        std::shared_ptr<Terrain> pGround2 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, -200), 0.0f, 0.0f, 0.0f, Vector3(20, 1, 20));
    m_GameObjects.push_back(pGround2);
    m_ColliderObjects.push_back(pGround2);
    m_Grounds.push_back(pGround2);
    m_GameObjects.push_back(pGround2->GroundCheck);
        std::shared_ptr<Terrain> pGround3 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 8, -415), 0.0f, 0.0f, 0.0f, Vector3(20, 2, 20));
    m_GameObjects.push_back(pGround3);
    m_ColliderObjects.push_back(pGround3);
    m_Grounds.push_back(pGround3);
    m_GameObjects.push_back(pGround3->GroundCheck);
        pMovePlat1 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-190, 12, -415), 0.0f, 45.0f, 0.0f, Vector3(10, 1, 10));
        pMovePlat1->Moving = ROTATEANTICLOCKWISE;
    m_GameObjects.push_back(pMovePlat1);
    m_ColliderObjects.push_back(pMovePlat1);
    m_Platforms.push_back(pMovePlat1);
    m_GameObjects.push_back(pMovePlat1->GroundCheck);
        pMovePlat2 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-340, 12, -415), 0.0f, 45.0f, 0.0f, Vector3(10, 1, 10));
        pMovePlat2->Moving = ROTATECLOCKWISE;
    m_GameObjects.push_back(pMovePlat2);
    m_ColliderObjects.push_back(pMovePlat2);
    m_Platforms.push_back(pMovePlat2);
    m_GameObjects.push_back(pMovePlat2->GroundCheck);
        std::shared_ptr<Terrain> pGround4 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-400, 15, -225), 0.0f, 0.0f, 0.0f, Vector3(20, 2, 20));
    m_GameObjects.push_back(pGround4);
    m_ColliderObjects.push_back(pGround4);
    m_Grounds.push_back(pGround4);
    m_GameObjects.push_back(pGround4->GroundCheck);
        std::shared_ptr<CMOGO> pCheckpoint1 = std::make_shared<CMOGO>("Checkpoint", m_d3dDevice.Get(), m_fxFactory);
    pCheckpoint1->SetPos(Vector3(-400, 25, -175));
    pCheckpoint1->SetScale(Vector3(1, 1, 1));
    m_GameObjects.push_back(pCheckpoint1);
    m_Checkpoints.push_back(pCheckpoint1);

    // LAYER 2 - COMBAT
        pMovePlat3 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-400, 0, -60), 0.0f, 0.0f, 0.0f, Vector3(7.5f, 1.5f, 7.5f));
        pMovePlat3->Moving = MOVEUP;
    m_GameObjects.push_back(pMovePlat3);
    m_ColliderObjects.push_back(pMovePlat3);
    m_Platforms.push_back(pMovePlat3);
    m_GameObjects.push_back(pMovePlat3->GroundCheck);
        std::shared_ptr<Terrain> pGround5 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-300, 90, 25), 0.0f, 0.0f, 0.0f, Vector3(30, 2, 4));
    m_GameObjects.push_back(pGround5);
    m_ColliderObjects.push_back(pGround5);
    m_Grounds.push_back(pGround5);
    m_GameObjects.push_back(pGround5->GroundCheck);
        pMovePlat4 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 90, 25), 0.0f, 0.0f, 0.0f, Vector3(25, 2, 3));
        pMovePlat4->Moving = ROTATECLOCKWISE;
    m_GameObjects.push_back(pMovePlat4);
    m_ColliderObjects.push_back(pMovePlat4);
    m_Platforms.push_back(pMovePlat4);
    m_GameObjects.push_back(pMovePlat4->GroundCheck);
        std::shared_ptr<Terrain> pGround6 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(300, 90, 25), 0.0f, 0.0f, 0.0f, Vector3(30, 2, 4));
    m_GameObjects.push_back(pGround6);
    m_ColliderObjects.push_back(pGround6);
    m_Grounds.push_back(pGround6);
    m_GameObjects.push_back(pGround6->GroundCheck);
        std::shared_ptr<CMOGO> pCheckpoint2 = std::make_shared<CMOGO>("Checkpoint", m_d3dDevice.Get(), m_fxFactory);
    pCheckpoint2->SetPos(Vector3(425, 100, 25));
    pCheckpoint2->SetScale(Vector3(1, 1, 1));
    m_GameObjects.push_back(pCheckpoint2);
    m_Checkpoints.push_back(pCheckpoint2);

    // LAYER 3 - PLATFORMING
        std::shared_ptr<Terrain> pGround7 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(425, 105, -15), 0.0f, 0.0f, 0.0f, Vector3(2, 1, 2));
    m_GameObjects.push_back(pGround7);
    m_ColliderObjects.push_back(pGround7);
    m_Grounds.push_back(pGround7);
    m_GameObjects.push_back(pGround7->GroundCheck);
        std::shared_ptr<Terrain> pGround8 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(400, 115, -45), 0.0f, 0.0f, 0.0f, Vector3(1.75f, 1, 1.75f));
    m_GameObjects.push_back(pGround8);
    m_ColliderObjects.push_back(pGround8);
    m_Grounds.push_back(pGround8);
    m_GameObjects.push_back(pGround8->GroundCheck);
        std::shared_ptr<Terrain> pGround9 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(375, 125, -75), 0.0f, 0.0f, 0.0f, Vector3(1.5f, 1, 1.5f));
    m_GameObjects.push_back(pGround9);
    m_ColliderObjects.push_back(pGround9);
    m_Grounds.push_back(pGround9);
    m_GameObjects.push_back(pGround9->GroundCheck);
        std::shared_ptr<Terrain> pGround10 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(350, 135, -105), 0.0f, 0.0f, 0.0f, Vector3(1.25f, 1, 1.25f));
    m_GameObjects.push_back(pGround10);
    m_ColliderObjects.push_back(pGround10);
    m_Grounds.push_back(pGround10);
    m_GameObjects.push_back(pGround10->GroundCheck);
        std::shared_ptr<Terrain> pGround11 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(325, 145, -135), 0.0f, 0.0f, 0.0f, Vector3(1, 1, 1));
    m_GameObjects.push_back(pGround11);
    m_ColliderObjects.push_back(pGround11);
    m_Grounds.push_back(pGround11);
    m_GameObjects.push_back(pGround11->GroundCheck);
        std::shared_ptr<Terrain> pGround12 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(300, 145, -105), 0.0f, 0.0f, 0.0f, Vector3(1, 1, 1));
    m_GameObjects.push_back(pGround12);
    m_ColliderObjects.push_back(pGround12);
    m_Grounds.push_back(pGround12);
    m_GameObjects.push_back(pGround12->GroundCheck);
        std::shared_ptr<Terrain> pGround13 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(275, 145, -135), 0.0f, 0.0f, 0.0f, Vector3(1, 1, 1));
    m_GameObjects.push_back(pGround13);
    m_ColliderObjects.push_back(pGround13);
    m_Grounds.push_back(pGround13);
    m_GameObjects.push_back(pGround13->GroundCheck);
        std::shared_ptr<Terrain> pGround14 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(250, 145, -105), 0.0f, 0.0f, 0.0f, Vector3(1, 1, 1));
    m_GameObjects.push_back(pGround14);
    m_ColliderObjects.push_back(pGround14);
    m_Grounds.push_back(pGround14);
    m_GameObjects.push_back(pGround14->GroundCheck);
        std::shared_ptr<Terrain> pGround15 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(225, 145, -135), 0.0f, 0.0f, 0.0f, Vector3(1, 1, 1));
    m_GameObjects.push_back(pGround15);
    m_ColliderObjects.push_back(pGround15);
    m_Grounds.push_back(pGround15);
    m_GameObjects.push_back(pGround15->GroundCheck);
        pMovePlat5 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(200, 150, -165), 0.0f, 0.0f, 0.0f, Vector3(4, 1, 4));
        pMovePlat5->Moving = MOVELEFTX;
    m_GameObjects.push_back(pMovePlat5);
    m_ColliderObjects.push_back(pMovePlat5);
    m_Platforms.push_back(pMovePlat5);
    m_GameObjects.push_back(pMovePlat5->GroundCheck);
        std::shared_ptr<Terrain> pGround16 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 150, -165), 0.0f, 0.0f, 0.0f, Vector3(5, 1.5f, 5));
    m_GameObjects.push_back(pGround16);
    m_ColliderObjects.push_back(pGround16);
    m_Grounds.push_back(pGround16);
    m_GameObjects.push_back(pGround16->GroundCheck);
        pMovePlat6 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 150, -225), 0.0f, 0.0f, 0.0f, Vector3(4, 1, 4));
        pMovePlat6->Moving = MOVEFORWARDZ;
    m_GameObjects.push_back(pMovePlat6);
    m_ColliderObjects.push_back(pMovePlat6);
    m_Platforms.push_back(pMovePlat6);
    m_GameObjects.push_back(pMovePlat6->GroundCheck);
        std::shared_ptr<Terrain> pGround17 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 150, -385), 0.0f, 0.0f, 0.0f, Vector3(2.5f, 1.5f, 4));
    m_GameObjects.push_back(pGround17);
    m_ColliderObjects.push_back(pGround17);
    m_Grounds.push_back(pGround17);
    m_GameObjects.push_back(pGround17->GroundCheck);
        std::shared_ptr<CMOGO> pCheckpoint3 = std::make_shared<CMOGO>("Checkpoint", m_d3dDevice.Get(), m_fxFactory);
    pCheckpoint3->SetPos(Vector3(50, 157.5f, -377.5f));
    pCheckpoint3->SetScale(Vector3(1, 1, 1));
    m_GameObjects.push_back(pCheckpoint3);
    m_Checkpoints.push_back(pCheckpoint3);

    // LAYER 4 - END
        std::shared_ptr<Terrain> pGround18 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 150, -400), 0.0f, 0.0f, 0.0f, Vector3(5, 1, 1));
    m_GameObjects.push_back(pGround18);
    m_ColliderObjects.push_back(pGround18);
    m_Grounds.push_back(pGround18);
    m_GameObjects.push_back(pGround18->GroundCheck);
        std::shared_ptr<Terrain> pGround19 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-90, 150, -400), 0.0f, 0.0f, 0.0f, Vector3(10, 2, 10));
    m_GameObjects.push_back(pGround19);
    m_ColliderObjects.push_back(pGround19);
    m_Grounds.push_back(pGround19);
    m_GameObjects.push_back(pGround19->GroundCheck);
        pMovePlat7 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-175, 150, -400), 0.0f, 0.0f, 0.0f, Vector3(4, 1, 4));
        pMovePlat7->Moving = MOVEUP;
    m_GameObjects.push_back(pMovePlat7);
    m_ColliderObjects.push_back(pMovePlat7);
    m_Platforms.push_back(pMovePlat7);
    m_GameObjects.push_back(pMovePlat7->GroundCheck);
        pMovePlat8 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-225, 325, -400), 0.0f, 0.0f, 0.0f, Vector3(4, 1, 4));
        pMovePlat8->Moving = MOVEDOWN;
    m_GameObjects.push_back(pMovePlat8);
    m_ColliderObjects.push_back(pMovePlat8);
    m_Platforms.push_back(pMovePlat8);
    m_GameObjects.push_back(pMovePlat8->GroundCheck);
        pMovePlat9 = std::make_shared<MovingPlatform>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 300, -400), 0.0f, 0.0f, 0.0f, Vector3(4, 1, 4));
        pMovePlat9->Moving = MOVEUP;
    m_GameObjects.push_back(pMovePlat9);
    m_ColliderObjects.push_back(pMovePlat9);
    m_Platforms.push_back(pMovePlat9);
    m_GameObjects.push_back(pMovePlat9->GroundCheck);
        std::shared_ptr<Terrain> pGround20 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 400, -325), 0.0f, 0.0f, 0.0f, Vector3(10, 2, 10));
    m_GameObjects.push_back(pGround20);
    m_ColliderObjects.push_back(pGround20);
    m_Grounds.push_back(pGround20);
    m_GameObjects.push_back(pGround20->GroundCheck);
        std::shared_ptr<Terrain> pGround21 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 400, -255), 0.0f, 0.0f, 0.0f, Vector3(2, 1, 2));
    m_GameObjects.push_back(pGround21);
    m_ColliderObjects.push_back(pGround21);
    m_Grounds.push_back(pGround21);
    m_GameObjects.push_back(pGround21->GroundCheck);
        std::shared_ptr<Terrain> pGround22 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 400, -220), 0.0f, 0.0f, 0.0f, Vector3(2, 1, 2));
    m_GameObjects.push_back(pGround22);
    m_ColliderObjects.push_back(pGround22);
    m_Grounds.push_back(pGround22);
    m_GameObjects.push_back(pGround22->GroundCheck);
        std::shared_ptr<Terrain> pGround23 = std::make_shared<Terrain>("GrassCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 400, -175), 0.0f, 0.0f, 0.0f, Vector3(5, 2, 5));
    m_GameObjects.push_back(pGround23);
    m_ColliderObjects.push_back(pGround23);
    m_Grounds.push_back(pGround23);
    m_GameObjects.push_back(pGround23->GroundCheck);
        std::shared_ptr<CMOGO> pCheckpoint4 = std::make_shared<CMOGO>("Checkpoint", m_d3dDevice.Get(), m_fxFactory);
    pCheckpoint4->SetPos(Vector3(-275, 410, -175));
    pCheckpoint4->SetScale(Vector3(1, 1, 1));
    m_GameObjects.push_back(pCheckpoint4);
    m_Checkpoints.push_back(pCheckpoint4);

    // BOSS APPROACH
        std::shared_ptr<Terrain> pLaunchpad = std::make_shared<Terrain>("Launchpad", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 400, -125), 0.0f, 0.0f, 0.0f, Vector3(4, 1, 4));
    m_GameObjects.push_back(pLaunchpad);
    m_ColliderObjects.push_back(pLaunchpad);
    m_Grounds.push_back(pLaunchpad);
    m_GameObjects.push_back(pLaunchpad->GroundCheck);
        pLaunchpadTrigger = std::make_shared<CMOGO>("Launchpad", m_d3dDevice.Get(), m_fxFactory);
    pLaunchpadTrigger->SetPos(Vector3(-275, 410, -125));
    pLaunchpadTrigger->SetScale(Vector3(4, 1, 4));
    m_GameObjects.push_back(pLaunchpadTrigger);
    m_TriggerObjects.push_back(pLaunchpadTrigger);
        std::shared_ptr<Terrain> pBossStageBelow = std::make_shared<Terrain>("BossStage", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 900, -125), 0.0f, 0.0f, 0.0f, Vector3(5, 1, 5));
    m_GameObjects.push_back(pBossStageBelow);
}
void Game::CreateIntroGround()
{
        std::shared_ptr<Terrain> pGroundIntro = std::make_shared<Terrain>("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -12.5f, -50), 0.0f, 0.0f, 0.0f, Vector3(15, 1, 17.5f));
    m_IntroGOs.push_back(pGroundIntro);
    m_ColliderObjects.push_back(pGroundIntro);
    m_Grounds.push_back(pGroundIntro);
    m_IntroGOs.push_back(pGroundIntro->GroundCheck);
        std::shared_ptr<Terrain> pIntroLWall = std::make_shared<Terrain>("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-35, 0, -50), 0.0f, 0.0f, 0.0f, Vector3(1, 5, 17.5f));
    m_IntroGOs.push_back(pIntroLWall);
    m_ColliderObjects.push_back(pIntroLWall);
        std::shared_ptr<Terrain> pIntroRWall = std::make_shared<Terrain>("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(35, 0, -50), 0.0f, 0.0f, 0.0f, Vector3(1, 5, 17.5f));
    m_IntroGOs.push_back(pIntroRWall);
    m_ColliderObjects.push_back(pIntroRWall);
        std::shared_ptr<Terrain> pIntroCeiling = std::make_shared<Terrain>("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 30, -50), 0.0f, 0.0f, 0.0f, Vector3(7.5f, 1, 17.5f));
    m_IntroGOs.push_back(pIntroCeiling);
    m_ColliderObjects.push_back(pIntroCeiling);
        std::shared_ptr<Terrain> pIntroBackWall = std::make_shared<Terrain>("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, 5), 0.0f, 0.0f, 0.0f, Vector3(7.5f, 5, 3));
    m_IntroGOs.push_back(pIntroBackWall);
    m_ColliderObjects.push_back(pIntroBackWall);
        std::shared_ptr<Terrain> pIntroFrontWall = std::make_shared<Terrain>("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, -137.5f), 0.0f, 0.0f, 0.0f, Vector3(7.5f, 5, 1));
    m_IntroGOs.push_back(pIntroFrontWall);
    m_ColliderObjects.push_back(pIntroFrontWall);

        std::shared_ptr<Terrain> pIntroBreakable = std::make_shared<Terrain>("CrackedWall", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -2.5, -134), 0.0f, 0.0f, 0.0f, Vector3(4, 5.5f, 3));
    m_IntroGOs.push_back(pIntroBreakable);
    m_ColliderObjects.push_back(pIntroBreakable);
    m_Destructibles.push_back(pIntroBreakable);
        pIntroExit = std::make_shared<Terrain>("WhiteCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -2.5f, -135.25f), 0.0f, 0.0f, 0.0f, Vector3(3, 5, 3));
    m_IntroGOs.push_back(pIntroExit);
    m_TriggerObjects.push_back(pIntroExit);

        pGoldedge = std::make_shared<Coin>("Sword", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -5, -75));
    pGoldedge->SetYaw(90);
    pGoldedge->SetScale(Vector3(0.25f, 0.3f, 0.25f));
    m_IntroGOs.push_back(pGoldedge);
    m_TriggerObjects.push_back(pGoldedge);
}
void Game::CreateBossGround()
{
        std::shared_ptr<Terrain> pBGMain = std::make_shared<Terrain>("BossStage", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, 0), 0.0f, 0.0f, 0.0f, Vector3(25, 1, 25));
    m_BossGOs.push_back(pBGMain);
    m_ColliderObjects.push_back(pBGMain);
    m_Grounds.push_back(pBGMain);
    m_BossGOs.push_back(pBGMain->GroundCheck);

    //Core 1
        std::shared_ptr<Terrain> pBG1 = std::make_shared<Terrain>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(150, 0, 0), 0.0f, 0.0f, 0.0f, Vector3(3, 1, 3));
    m_BossGOs.push_back(pBG1);
    m_ColliderObjects.push_back(pBG1);
    m_Grounds.push_back(pBG1);
    m_BossGOs.push_back(pBG1->GroundCheck);
        std::shared_ptr<Terrain> pBG2 = std::make_shared<Terrain>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(190, 10, 0), 0.0f, 0.0f, 0.0f, Vector3(2.5f, 1, 2.5f));
    m_BossGOs.push_back(pBG2);
    m_ColliderObjects.push_back(pBG2);
    m_Grounds.push_back(pBG2);
    m_BossGOs.push_back(pBG2->GroundCheck);
        std::shared_ptr<Terrain> pBG3 = std::make_shared<Terrain>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(225, 20, 0), 0.0f, 0.0f, 0.0f, Vector3(2, 1, 2));
    m_BossGOs.push_back(pBG3);
    m_ColliderObjects.push_back(pBG3);
    m_Grounds.push_back(pBG3);
    m_BossGOs.push_back(pBG3->GroundCheck);
        std::shared_ptr<Terrain> pBG4 = std::make_shared<Terrain>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(260, 30, 0), 0.0f, 0.0f, 0.0f, Vector3(1.5f, 1, 1.5f));
    m_BossGOs.push_back(pBG4);
    m_ColliderObjects.push_back(pBG4);
    m_Grounds.push_back(pBG4);
    m_BossGOs.push_back(pBG4->GroundCheck);
        std::shared_ptr<Terrain> pBG5 = std::make_shared<Terrain>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(300, 40, 0), 0.0f, 0.0f, 0.0f, Vector3(3.5f, 1, 3.5f));
    m_BossGOs.push_back(pBG5);
    m_ColliderObjects.push_back(pBG5);
    m_Grounds.push_back(pBG5);
    m_BossGOs.push_back(pBG5->GroundCheck);
        pCore1 = std::make_shared<CMOGO>("BossCore", m_d3dDevice.Get(), m_fxFactory);
    pCore1->SetPos(Vector3(300, 45, 0));
    pCore1->SetScale(Vector3::One * 0.75f);
    m_BossGOs.push_back(pCore1);
    m_ColliderObjects.push_back(pCore1);
    m_Destructibles.push_back(pCore1);

    //Core 2
        pMovePlatB1 = std::make_shared<MovingPlatform>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(-150, -10, -150), 0.0f, 40.0f, 0.0f, Vector3(3, 1, 3));
        pMovePlatB1->Moving = MOVEUP;
        pMovePlatB1->updown_speed = 35;
    m_BossGOs.push_back(pMovePlatB1);
    m_ColliderObjects.push_back(pMovePlatB1);
    m_Platforms.push_back(pMovePlatB1);
    m_BossGOs.push_back(pMovePlatB1->GroundCheck);
        pMovePlatB2 = std::make_shared<MovingPlatform>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(-175, 180, -175), 0.0f, 40.0f, 0.0f, Vector3(3, 1, 3));
        pMovePlatB2->Moving = MOVEDOWN;
        pMovePlatB2->updown_speed = 35;
    m_BossGOs.push_back(pMovePlatB2);
    m_ColliderObjects.push_back(pMovePlatB2);
    m_Platforms.push_back(pMovePlatB2);
    m_BossGOs.push_back(pMovePlatB2->GroundCheck);
        std::shared_ptr<Terrain> pBG6 = std::make_shared<Terrain>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(-210, 170, -210), 0.0f, 40.0f, 0.0f, Vector3(3.5f, 1, 3.5f));
    m_BossGOs.push_back(pBG6);
    m_ColliderObjects.push_back(pBG6);
    m_Grounds.push_back(pBG6);
    m_BossGOs.push_back(pBG6->GroundCheck);
        pCore2 = std::make_shared<CMOGO>("BossCore", m_d3dDevice.Get(), m_fxFactory);
    pCore2->SetPos(Vector3(-210, 175, -210));
    pCore2->SetScale(Vector3::One * 0.75f);
    m_BossGOs.push_back(pCore2);
    m_ColliderObjects.push_back(pCore2);
    m_Destructibles.push_back(pCore2);

    //Core 3
        pMovePlatB3 = std::make_shared<MovingPlatform>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(-185, 0, 185), 0.0f, 40.0f, 0.0f, Vector3(15, 1, 3));
        pMovePlatB3->Moving = ROTATEANTICLOCKWISE;
        pMovePlatB3->rotate_speed = 2.25f;
    m_BossGOs.push_back(pMovePlatB3);
    m_ColliderObjects.push_back(pMovePlatB3);
    m_Platforms.push_back(pMovePlatB3);
    m_BossGOs.push_back(pMovePlatB3->GroundCheck);
        pMovePlatB4 = std::make_shared<MovingPlatform>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(-300, 0, 300), 0.0f, 40.0f, 0.0f, Vector3(15, 1, 3));
        pMovePlatB4->Moving = ROTATECLOCKWISE;
        pMovePlatB4->rotate_speed = 2.25f;
    m_BossGOs.push_back(pMovePlatB4);
    m_ColliderObjects.push_back(pMovePlatB4);
    m_Platforms.push_back(pMovePlatB4);
    m_BossGOs.push_back(pMovePlatB4->GroundCheck);
        std::shared_ptr<Terrain> pBG7 = std::make_shared<Terrain>("BossStageBase", m_d3dDevice.Get(), m_fxFactory, Vector3(-375, 0, 375), 0.0f, 40.0f, 0.0f, Vector3(3.5f, 1, 3.5f));
    m_BossGOs.push_back(pBG7);
    m_ColliderObjects.push_back(pBG7);
    m_Grounds.push_back(pBG7);
    m_BossGOs.push_back(pBG7->GroundCheck);
        pCore3 = std::make_shared<CMOGO>("BossCore", m_d3dDevice.Get(), m_fxFactory);
    pCore3->SetPos(Vector3(-375, 5, 375));
    pCore3->SetScale(Vector3::One * 0.75f);
    m_BossGOs.push_back(pCore3);
    m_ColliderObjects.push_back(pCore3);
    m_Destructibles.push_back(pCore3);
}

void Game::CreateAudio()
{
        ambience = new Loop(m_audioEngine.get(), "NightAmbienceSimple_02");
        intro_music = new Loop(m_audioEngine.get(), "BondsOfSeaAndFire");
        game_music = new Loop(m_audioEngine.get(), "GaurPlains");
        boss_intro = new Loop(m_audioEngine.get(), "Courtesy");
        boss_music = new Loop(m_audioEngine.get(), "Awakening");
        ending_music = new Loop(m_audioEngine.get(), "SmallTwoOfPieces");

        hit_sfx = new Sound(m_audioEngine.get(), "Explo1");
    hit_sfx->SetVolume(0.3f);
    hit_sfx->SetPitch(0.9f);
    m_Sounds.push_back(hit_sfx);
        coin_sfx = new Sound(m_audioEngine.get(), "Coin");
    coin_sfx->SetVolume(1);
    coin_sfx->SetPitch(0.9f);
    m_Sounds.push_back(coin_sfx);
        death_sfx = new Sound(m_audioEngine.get(), "LoseLife");
    death_sfx->SetVolume(1);
    m_Sounds.push_back(death_sfx);
        jump_sfx = new Sound(m_audioEngine.get(), "Jump");
    jump_sfx->SetVolume(1.5f);
    m_Sounds.push_back(jump_sfx);
        sword_sfx = new Sound(m_audioEngine.get(), "Attack");
    sword_sfx->SetVolume(0.75f);
    m_Sounds.push_back(sword_sfx);

        KZK_intro = new Loop(m_audioEngine.get(), "KazcranakIntro");
        combat1 = new Sound(m_audioEngine.get(), "Combat1");
    combat1->SetVolume(0.4f);
    m_Sounds.push_back(combat1);
        combat2 = new Sound(m_audioEngine.get(), "Combat2");
    combat2->SetVolume(0.4f);
    m_Sounds.push_back(combat2);
        combat3 = new Sound(m_audioEngine.get(), "Combat3");
    combat3->SetVolume(0.4f);
    m_Sounds.push_back(combat3);
        combat4 = new Sound(m_audioEngine.get(), "Combat4");
    combat4->SetVolume(0.4f);
    m_Sounds.push_back(combat4);
        combat5 = new Sound(m_audioEngine.get(), "Combat5");
    combat5->SetVolume(0.4f);
    m_Sounds.push_back(combat5);
        combat6 = new Sound(m_audioEngine.get(), "Combat6");
    combat6->SetVolume(0.4f);
    m_Sounds.push_back(combat6);
        hurt1 = new Sound(m_audioEngine.get(), "Hurt1");
    hurt1->SetVolume(0.4f);
    m_Sounds.push_back(hurt1);
        hurt2 = new Sound(m_audioEngine.get(), "Hurt2");
    hurt2->SetVolume(0.4);
    m_Sounds.push_back(hurt2);
        hurt3 = new Sound(m_audioEngine.get(), "Hurt3");
    hurt3->SetVolume(0.4f);
    m_Sounds.push_back(hurt3);
        KZK_final = new Loop(m_audioEngine.get(), "KazcranakFinal");
}
void Game::CreateUI()
{
        std::shared_ptr<ImageGO2D> UIBG = std::make_shared<ImageGO2D>("UIBackground", m_d3dDevice.Get());
    UIBG->SetPos(Vector2(700, 50));
    UIBG->SetScale(Vector2(0.15f, 0.2f));
    UIBG->SetRendered(true);
    m_GameObjects2D.push_back(UIBG);

        std::shared_ptr<ImageGO2D> pLivesCount = std::make_shared<ImageGO2D>("Heart", m_d3dDevice.Get());
    pLivesCount->SetPos(Vector2(615, 50));
    pLivesCount->SetScale(0.075f);
    pLivesCount->SetRendered(true);
        m_GameObjects2D.push_back(pLivesCount);
    livesText = std::make_shared<TextGO2D>(std::to_string(lives));
    livesText->SetPos(Vector2(595, 15));
    livesText->SetColour(Color((float*)&Colors::Black));
    livesText->SetScale(Vector2(1.1f, 1));
    m_GameObjects2D.push_back(livesText);

        std::shared_ptr<ImageGO2D> pScoreCount = std::make_shared<ImageGO2D>("Coin", m_d3dDevice.Get());
    pScoreCount->SetPos(Vector2(725, 50));
    pScoreCount->SetScale(Vector2(0.09f, 0.075f));
    pScoreCount->SetRendered(true);
        m_GameObjects2D.push_back(pScoreCount);
    scoreText = std::make_shared<TextGO2D>(std::to_string(score));
    scoreText->SetPos(Vector2(705, 15));
    scoreText->SetColour(Color((float*)&Colors::Black));
    scoreText->SetScale(Vector2(1.1f, 1));
    m_GameObjects2D.push_back(scoreText);
}

void Game::CreateCoins()
{
    // LAYER 1
        std::shared_ptr<Coin> pCoin1 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, 50));
    m_GameObjects.push_back(pCoin1);
    m_Coins.push_back(pCoin1);
        std::shared_ptr<Coin> pCoin2 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, 35));
    m_GameObjects.push_back(pCoin2);
    m_Coins.push_back(pCoin2);
        std::shared_ptr<Coin> pCoin3 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-30, 0, 10));
    m_GameObjects.push_back(pCoin3);
    m_Coins.push_back(pCoin3);
        std::shared_ptr<Coin> pCoin4 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(30, 0, 10));
    m_GameObjects.push_back(pCoin4);
    m_Coins.push_back(pCoin4);
        std::shared_ptr<Coin> pCoin5 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 10, -125));
    m_GameObjects.push_back(pCoin5);
    m_Coins.push_back(pCoin5);
        std::shared_ptr<Coin> pCoin6 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-50, 10, -175));
    m_GameObjects.push_back(pCoin6);
    m_Coins.push_back(pCoin6);
        std::shared_ptr<Coin> pCoin7 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 10, -225));
    m_GameObjects.push_back(pCoin7);
    m_Coins.push_back(pCoin7);
        std::shared_ptr<Coin> pCoin8 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 25, -425));
    m_GameObjects.push_back(pCoin8);
    m_Coins.push_back(pCoin8);
        std::shared_ptr<Coin> pCoin9 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(25, 25, -450));
    m_GameObjects.push_back(pCoin9);
    m_Coins.push_back(pCoin9);
        std::shared_ptr<Coin> pCoin10 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-25, 25, -450));
    m_GameObjects.push_back(pCoin10);
    m_Coins.push_back(pCoin10);
        std::shared_ptr<Coin> pCoin11 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-200, 25, -415));
    m_GameObjects.push_back(pCoin11);
    m_Coins.push_back(pCoin11);
        std::shared_ptr<Coin> pCoin12 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-350, 25, -415));
    m_GameObjects.push_back(pCoin12);
    m_Coins.push_back(pCoin12);

    // LAYER 2
        std::shared_ptr<Coin> pCoin13 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-400, 125, -60));
    m_GameObjects.push_back(pCoin13);
    m_Coins.push_back(pCoin13);
        std::shared_ptr<Coin> pCoin14 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-375, 105, 25));
    m_GameObjects.push_back(pCoin14);
    m_Coins.push_back(pCoin14);
        std::shared_ptr<Coin> pCoin15 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-325, 105, 25));
    m_GameObjects.push_back(pCoin15);
    m_Coins.push_back(pCoin15);
        std::shared_ptr<Coin> pCoin16 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 105, 25));
    m_GameObjects.push_back(pCoin16);
    m_Coins.push_back(pCoin16);
        std::shared_ptr<Coin> pCoin17 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 105, -90));
    m_GameObjects.push_back(pCoin17);
    m_Coins.push_back(pCoin17);
        std::shared_ptr<Coin> pCoin18 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 105, 140));
    m_GameObjects.push_back(pCoin18);
    m_Coins.push_back(pCoin18);
        std::shared_ptr<Coin> pCoin19 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(225, 105, 25));
    m_GameObjects.push_back(pCoin19);
    m_Coins.push_back(pCoin19);
        std::shared_ptr<Coin> pCoin20 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(175, 105, 25));
    m_GameObjects.push_back(pCoin20);
    m_Coins.push_back(pCoin20);

    // LAYER 3
        std::shared_ptr<Coin> pCoin21 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(300, 155, -105));
    m_GameObjects.push_back(pCoin21);
    m_Coins.push_back(pCoin21);
        std::shared_ptr<Coin> pCoin22 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(250, 155, -105));
    m_GameObjects.push_back(pCoin22);
    m_Coins.push_back(pCoin22);
        std::shared_ptr<Coin> pCoin23 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(150, 162.5f, -165));
    m_GameObjects.push_back(pCoin23);
    m_Coins.push_back(pCoin23);
        std::shared_ptr<Coin> pCoin24 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 162.5f, -275));
    m_GameObjects.push_back(pCoin24);
    m_Coins.push_back(pCoin24);
        std::shared_ptr<Coin> pCoin25 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 162.5f, -165));
    m_GameObjects.push_back(pCoin25);
    m_Coins.push_back(pCoin25);
    
    // LAYER 4
        std::shared_ptr<Coin> pCoin26 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-90, 165, -400));
    m_GameObjects.push_back(pCoin26);
    m_Coins.push_back(pCoin26);
        std::shared_ptr<Coin> pCoin27 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-175, 200, -400));
    m_GameObjects.push_back(pCoin27);
    m_Coins.push_back(pCoin27);
        std::shared_ptr<Coin> pCoin28 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-225, 275, -400));
    m_GameObjects.push_back(pCoin28);
    m_Coins.push_back(pCoin28);
        std::shared_ptr<Coin> pCoin29 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 350, -400));
    m_GameObjects.push_back(pCoin29);
    m_Coins.push_back(pCoin29);
        std::shared_ptr<Coin> pCoin30 = std::make_shared<Coin>("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 415, -325));
    m_GameObjects.push_back(pCoin30);
    m_Coins.push_back(pCoin30);
}
void Game::CreateEnemies()
{
    // LAYER 1
    std::shared_ptr<Enemy> pEnemy1 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, -50));
    m_GameObjects.push_back(pEnemy1);
    m_Enemies.push_back(pEnemy1);
    m_GameObjects.push_back(pEnemy1->EnemySensor);
    m_EnemySensors.push_back(pEnemy1->EnemySensor);
    std::shared_ptr<Enemy> pEnemy2 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(30, 10, -180));
    m_GameObjects.push_back(pEnemy2);
    m_Enemies.push_back(pEnemy2);
    m_GameObjects.push_back(pEnemy2->EnemySensor);
    m_EnemySensors.push_back(pEnemy2->EnemySensor);
    std::shared_ptr<Enemy> pEnemy3 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-30, 10.0f, -180));
    m_GameObjects.push_back(pEnemy3);
    m_Enemies.push_back(pEnemy3);
    m_GameObjects.push_back(pEnemy3->EnemySensor);
    m_EnemySensors.push_back(pEnemy3->EnemySensor);
    std::shared_ptr<Enemy> pEnemy4 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 24, -375));
    m_GameObjects.push_back(pEnemy4);
    m_Enemies.push_back(pEnemy4);
    m_GameObjects.push_back(pEnemy4->EnemySensor);
    m_EnemySensors.push_back(pEnemy4->EnemySensor);
    std::shared_ptr<Enemy> pEnemy5 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(15, 24, -425));
    m_GameObjects.push_back(pEnemy5);
    m_Enemies.push_back(pEnemy5);
    m_GameObjects.push_back(pEnemy5->EnemySensor);
    m_EnemySensors.push_back(pEnemy5->EnemySensor);
    std::shared_ptr<Enemy> pEnemy6 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-15, 24, -425));
    m_GameObjects.push_back(pEnemy6);
    m_Enemies.push_back(pEnemy6);
    m_GameObjects.push_back(pEnemy6->EnemySensor);
    m_EnemySensors.push_back(pEnemy6->EnemySensor);
    std::shared_ptr<Enemy> pEnemy7 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-375, 30, -250));
    m_GameObjects.push_back(pEnemy7);
    m_Enemies.push_back(pEnemy7);
    m_GameObjects.push_back(pEnemy7->EnemySensor);
    m_EnemySensors.push_back(pEnemy7->EnemySensor);
    std::shared_ptr<Enemy> pEnemy8 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-425, 30, -250));
    m_GameObjects.push_back(pEnemy8);
    m_Enemies.push_back(pEnemy8);
    m_GameObjects.push_back(pEnemy8->EnemySensor);
    m_EnemySensors.push_back(pEnemy8->EnemySensor);
    std::shared_ptr<Enemy> pStrongE1 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-400, 30, -210));
        pStrongE1->speed = pStrongE1->speed * 2;
    m_GameObjects.push_back(pStrongE1);
    m_Enemies.push_back(pStrongE1);
    m_GameObjects.push_back(pStrongE1->EnemySensor);
    m_EnemySensors.push_back(pStrongE1->EnemySensor);
    std::shared_ptr<Enemy> pEnemy9 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-425, 30, -250));
    m_GameObjects.push_back(pEnemy9);
    m_Enemies.push_back(pEnemy9);
    m_GameObjects.push_back(pEnemy9->EnemySensor);
    m_EnemySensors.push_back(pEnemy9->EnemySensor);

    // LAYER 2
    std::shared_ptr<Enemy> pEnemy10 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-225, 105, 25));
    m_GameObjects.push_back(pEnemy10);
    m_Enemies.push_back(pEnemy10);
    m_GameObjects.push_back(pEnemy10->EnemySensor);
    m_EnemySensors.push_back(pEnemy10->EnemySensor);
    std::shared_ptr<Enemy> pEnemy11 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-175, 105, 25));
    m_GameObjects.push_back(pEnemy11);
    m_Enemies.push_back(pEnemy11);
    m_GameObjects.push_back(pEnemy11->EnemySensor);
    m_EnemySensors.push_back(pEnemy11->EnemySensor);
    std::shared_ptr<Enemy> pStrongE2 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 105, 25));
        pStrongE2->speed = pStrongE2->speed * 2;
    m_GameObjects.push_back(pStrongE2);
    m_Enemies.push_back(pStrongE2);
    m_GameObjects.push_back(pStrongE2->EnemySensor);
    m_EnemySensors.push_back(pStrongE2->EnemySensor);
    std::shared_ptr<Enemy> pEnemy12 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(275, 105, 25));
    m_GameObjects.push_back(pEnemy12);
    m_Enemies.push_back(pEnemy12);
    m_GameObjects.push_back(pEnemy12->EnemySensor);
    m_EnemySensors.push_back(pEnemy12->EnemySensor);
    std::shared_ptr<Enemy> pEnemy13 = std::make_shared<Enemy>("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(325, 105, 25));
    m_GameObjects.push_back(pEnemy13);
    m_Enemies.push_back(pEnemy13);
    m_GameObjects.push_back(pEnemy13->EnemySensor);
    m_EnemySensors.push_back(pEnemy13->EnemySensor);
    std::shared_ptr<Enemy> pStrongE3 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(375, 105, 25));
        pStrongE3->speed = pStrongE3->speed * 2;
    m_GameObjects.push_back(pStrongE3);
    m_Enemies.push_back(pStrongE3);
    m_GameObjects.push_back(pStrongE3->EnemySensor);
    m_EnemySensors.push_back(pStrongE3->EnemySensor);

    // LAYER 4
    std::shared_ptr<Enemy> pStrongE4 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-125, 165, -365));
        pStrongE4->speed = pStrongE4->speed * 2;
    m_GameObjects.push_back(pStrongE4);
    m_Enemies.push_back(pStrongE4);
    m_GameObjects.push_back(pStrongE4->EnemySensor);
    m_EnemySensors.push_back(pStrongE4->EnemySensor);
    std::shared_ptr<Enemy> pStrongE5 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-125, 165, -435));
        pStrongE5->speed = pStrongE5->speed * 2;
    m_GameObjects.push_back(pStrongE5);
    m_Enemies.push_back(pStrongE5);
    m_GameObjects.push_back(pStrongE5->EnemySensor);
    m_EnemySensors.push_back(pStrongE5->EnemySensor);
    std::shared_ptr<Enemy> pStrongE6 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 415, -300));
        pStrongE6->speed = pStrongE6->speed * 2;
    m_GameObjects.push_back(pStrongE6);
    m_Enemies.push_back(pStrongE6);
    m_GameObjects.push_back(pStrongE6->EnemySensor);
    m_EnemySensors.push_back(pStrongE6->EnemySensor);
    std::shared_ptr<Enemy> pStrongE7 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-250, 415, -285));
        pStrongE7->speed = pStrongE7->speed * 2;
    m_GameObjects.push_back(pStrongE7);
    m_Enemies.push_back(pStrongE7);
    m_GameObjects.push_back(pStrongE7->EnemySensor);
    m_EnemySensors.push_back(pStrongE7->EnemySensor);
    std::shared_ptr<Enemy> pStrongE8 = std::make_shared<Enemy>("StrongEnemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-300, 415, -285));
        pStrongE8->speed = pStrongE8->speed * 2;
    m_GameObjects.push_back(pStrongE8);
    m_Enemies.push_back(pStrongE8);
    m_GameObjects.push_back(pStrongE8->EnemySensor);
    m_EnemySensors.push_back(pStrongE8->EnemySensor);
}
void Game::CreateSigns()
{
        pSign1 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -5, -35), 0);
    m_IntroGOs.push_back(pSign1);
    m_Signs.push_back(pSign1);
    m_ColliderObjects.push_back(pSign1);
    m_IntroGOs.push_back(pSign1->SignTrigger);
        pSign2 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -5, -85), 0);
    m_IntroGOs.push_back(pSign2);
    m_Signs.push_back(pSign2);
    m_ColliderObjects.push_back(pSign2);
    m_IntroGOs.push_back(pSign2->SignTrigger);
        pSign3 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -2, 15), 0);
    m_GameObjects.push_back(pSign3);
    m_Signs.push_back(pSign3);
    m_ColliderObjects.push_back(pSign3);
    m_GameObjects.push_back(pSign3->SignTrigger);
        pSign4 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(15, 8, -120), 0);
    m_GameObjects.push_back(pSign4);
    m_Signs.push_back(pSign4);
    m_ColliderObjects.push_back(pSign4);
    m_GameObjects.push_back(pSign4->SignTrigger);
        pSign5 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(-375, 28, -175), 179);
    m_GameObjects.push_back(pSign5);
    m_Signs.push_back(pSign5);
    m_ColliderObjects.push_back(pSign5);
    m_GameObjects.push_back(pSign5->SignTrigger);
        pSign6 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(-160, 102.5f, 25), 80);
    m_GameObjects.push_back(pSign6);
    m_Signs.push_back(pSign6);
    m_ColliderObjects.push_back(pSign6);
    m_GameObjects.push_back(pSign6->SignTrigger);
        pSign7 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(50, 160, -400), 0);
    m_GameObjects.push_back(pSign7);
    m_Signs.push_back(pSign7);
    m_ColliderObjects.push_back(pSign7);
    m_GameObjects.push_back(pSign7->SignTrigger);
        pSign8 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(-275, 412.5f, -160), 179);
    m_GameObjects.push_back(pSign8);
    m_Signs.push_back(pSign8);
    m_ColliderObjects.push_back(pSign8);
    m_GameObjects.push_back(pSign8->SignTrigger);
        pSign9 = std::make_shared<Sign>("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(-290, 412.5f, -155), 179);
    m_GameObjects.push_back(pSign9);
    m_Signs.push_back(pSign9);
    m_ColliderObjects.push_back(pSign9);
    m_GameObjects.push_back(pSign9->SignTrigger);

        sign1Image = std::make_shared<ImageGO2D>("IntroLoreSign", m_d3dDevice.Get());
    sign1Image->SetPos(Vector2(400, 300));
    sign1Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign1Image);
        sign2Image = std::make_shared<ImageGO2D>("IntroHTPSign", m_d3dDevice.Get());
    sign2Image->SetPos(Vector2(400, 300));
    sign2Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign2Image);
        sign3Image = std::make_shared<ImageGO2D>("TreeInfoSign", m_d3dDevice.Get());
    sign3Image->SetPos(Vector2(400, 300));
    sign3Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign3Image);
        sign4Image = std::make_shared<ImageGO2D>("CubeMinionSign", m_d3dDevice.Get());
    sign4Image->SetPos(Vector2(400, 300));
    sign4Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign4Image);
        sign5Image = std::make_shared<ImageGO2D>("CheckpointSign", m_d3dDevice.Get());
    sign5Image->SetPos(Vector2(400, 300));
    sign5Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign5Image);
        sign6Image = std::make_shared<ImageGO2D>("MovePlatHint", m_d3dDevice.Get());
    sign6Image->SetPos(Vector2(400, 300));
    sign6Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign6Image);
        sign7Image = std::make_shared<ImageGO2D>("AlmostThereSign", m_d3dDevice.Get());
    sign7Image->SetPos(Vector2(400, 300));
    sign7Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign7Image);
        sign8Image = std::make_shared<ImageGO2D>("FinalSign", m_d3dDevice.Get());
    sign8Image->SetPos(Vector2(400, 300));
    sign8Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign8Image);
        sign9Image = std::make_shared<ImageGO2D>("BossHintSign", m_d3dDevice.Get());
    sign9Image->SetPos(Vector2(400, 300));
    sign9Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign9Image);

    for (int i = 0; i < m_Signs.size(); i++)
        m_GameObjects2D.push_back(m_Signs[i]->ReadText);
}
