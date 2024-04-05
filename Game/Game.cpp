//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <time.h>

#include <iostream>

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

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

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

    //create a set of dummy things to show off the engine
    score = 0;

    //create a base light
    m_light = new Light(Vector3(0.0f, 100.0f, 160.0f), Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.4f, 0.1f, 0.1f, 1.0f));
    m_GameObjects.push_back(m_light);

    //find how big my window is to correctly calculate my aspect ratio
    float AR = (float)_width / (float)_height;

    CreateIntroGround();
    CreateGround();

    //create a base camera
    // m_cam = new Camera(0.25f * XM_PI, AR, 1.0f, 10000.0f, Vector3::UnitY, Vector3::Zero);
    // m_cam->SetPos(Vector3(0.0f, 200.0f, 200.0f));
    // m_GameObjects.push_back(m_cam);

    //add Player - player object and adding swords to player class
    pPlayer = new Player("Player", m_d3dDevice.Get(), m_fxFactory);
    m_GameObjects.push_back(pPlayer);
    m_IntroGOs.push_back(pPlayer);
    m_PhysicsObjects.push_back(pPlayer);
    m_GameObjects.push_back(pPlayer->pSwordTrigger);
    m_GameObjects.push_back(pPlayer->pSwordObject);
    m_IntroGOs.push_back(pPlayer->pSwordTrigger);
    m_IntroGOs.push_back(pPlayer->pSwordObject);

    //add a PRIMARY camera
    m_TPScam = new TPSCamera(0.5f * XM_PI, AR, 1.0f, 10000.0f, pPlayer, Vector3::UnitY, Vector3(0.0f, 0.0f, 0.1f)); // Vector3(0,0,0.1f)
    m_GameObjects.push_back(m_TPScam);
    m_IntroGOs.push_back(m_TPScam);

    //add Coins
        pCoin1 = new Coin("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(20.0f, 0.0f, 20.0f));
    m_GameObjects.push_back(pCoin1);
    m_Coins.push_back(pCoin1);
        pCoin2 = new Coin("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-20.0f, 0.0f, -20.0f));
    m_GameObjects.push_back(pCoin2);
    m_Coins.push_back(pCoin2);
        pCoin3 = new Coin("Coin", m_d3dDevice.Get(), m_fxFactory, Vector3(-30.0f, 0.0f, 10.0f));
    m_GameObjects.push_back(pCoin3);
    m_Coins.push_back(pCoin3);

    //add Enemies
        pEnemy1 = new Enemy("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(50.0f, 1.0f, 30.0f),0,0,0);
        pEnemy1->SetRendered(true);
    m_GameObjects.push_back(pEnemy1);
    m_Enemies.push_back(pEnemy1);
    m_GameObjects.push_back(pEnemy1->EnemySensor);
    m_EnemySensors.push_back(pEnemy1->EnemySensor);
        pEnemy2 = new Enemy("Enemy", m_d3dDevice.Get(), m_fxFactory, Vector3(-30.0f, 1.0f, -50.0f), 0, 0, 0);
        pEnemy2->SetRendered(true);
    m_GameObjects.push_back(pEnemy2);
    m_Enemies.push_back(pEnemy2);
    m_GameObjects.push_back(pEnemy2->EnemySensor);
    m_EnemySensors.push_back(pEnemy2->EnemySensor);


    //add Sign - sign objects & text
        pSign1 = new Sign("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -5, -35));
    m_IntroGOs.push_back(pSign1);
    m_Signs.push_back(pSign1);
    m_ColliderObjects.push_back(pSign1);
    m_IntroGOs.push_back(pSign1->pSignTrigger);
        pSign2 = new Sign("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -5, -85));
    m_IntroGOs.push_back(pSign2);
    m_Signs.push_back(pSign2);
    m_ColliderObjects.push_back(pSign2);
    m_IntroGOs.push_back(pSign2->pSignTrigger);
        pSign3 = new Sign("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -2, -30));
    m_GameObjects.push_back(pSign3);
    m_Signs.push_back(pSign3);
    m_ColliderObjects.push_back(pSign3);
    m_GameObjects.push_back(pSign3->pSignTrigger);
        pSign4 = new Sign("Sign", m_d3dDevice.Get(), m_fxFactory, Vector3(25, -2, -60));
    m_GameObjects.push_back(pSign4);
    m_Signs.push_back(pSign4);
    m_ColliderObjects.push_back(pSign4);
    m_GameObjects.push_back(pSign4->pSignTrigger);

        sign1Image = new ImageGO2D("IntroLoreSign", m_d3dDevice.Get());
    sign1Image->SetPos(Vector2(400, 300));
    sign1Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign1Image);
        sign2Image = new ImageGO2D("IntroHTPSign", m_d3dDevice.Get());
    sign2Image->SetPos(Vector2(400, 300));
    sign2Image->SetScale(Vector2(0.75f, 0.75f));
    m_GameObjects2D.push_back(sign2Image);

    m_GameObjects2D.push_back(pSign1->pReadText);
    m_GameObjects2D.push_back(pSign2->pReadText);
    m_GameObjects2D.push_back(pSign3->pReadText);
    m_GameObjects2D.push_back(pSign4->pReadText);



    //L-system like tree
    Tree* tree = new Tree(1, 4, .6f, 10.0f * Vector3::Up, XM_PI / 6.0f, "JEMINA vase -up", m_d3dDevice.Get(), m_fxFactory);
    m_GameObjects.push_back(tree);

    //create DrawData struct and populate its pointers
    m_DD = new DrawData;
    m_DD->m_pd3dImmediateContext = nullptr;
    m_DD->m_states = m_states;
    m_DD->m_cam = m_TPScam;
    m_DD->m_light = m_light;

    //example basic 2D stuff
    ImageGO2D* bug_test = new ImageGO2D("pain", m_d3dDevice.Get());
    bug_test->SetPos(150.0f * Vector2::One);
    bug_test->SetScale(0.1f);
    m_GameObjects2D.push_back(bug_test);

        scoreText = new TextGO2D("Coins: " + std::to_string(score));
    scoreText->SetPos(Vector2(100, 10));
    scoreText->SetColour(Color((float*)&Colors::Yellow));
    scoreText->SetScale(1);
    m_GameObjects2D.push_back(scoreText);
        livesText = new TextGO2D("Lives: " + std::to_string(lives));
    livesText->SetPos(Vector2(100, 30));
    livesText->SetColour(Color((float*)&Colors::Red));
    livesText->SetScale(1);
    m_GameObjects2D.push_back(livesText);

    title_screen = new ImageGO2D("TitleScreen", m_d3dDevice.Get());
    title_screen->SetPos(Vector2(400,300));
    title_screen->SetScale(1.25f);
    m_GameObjects2D.push_back(title_screen);

    //Test Sounds
    Loop* loop = new Loop(m_audioEngine.get(), "NightAmbienceSimple_02");
    loop->SetVolume(0.1f);
    loop->Play();
    m_Sounds.push_back(loop);

    // Loop* introMusic = new Loop(m_audioEngine.get(), "BondsOfSeaAndFlame");
    // introMusic->SetVolume(0.25f);
    // introMusic->Play();
    // m_Sounds.push_back(introMusic);

    // TestSound* TS = new TestSound(m_audioEngine.get(), "Explo1");
    // m_Sounds.push_back(TS);

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

    //update all objects
    if (m_GD->m_GS == GS_GAME)
    {
        for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
        {
            if ((*it)->isRendered())
                (*it)->Tick(m_GD);
        }
        for (int i = 0; i < m_Enemies.size(); i++)
        {
            m_Enemies[i]->player_facing = pPlayer->GetYaw();
            m_Enemies[i]->EnemySensor->SetRendered(false);
        }
    }

    if (m_GD->m_GS == GS_INTRO)
    {
        for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
        {
            if ((*it)->isRendered())
                (*it)->Tick(m_GD);
        }
    }

    for (std::vector<GameObject2D*>::iterator it = m_GameObjects2D.begin(); it != m_GameObjects2D.end(); it++)
    {
        if ((*it)->isRendered())
            (*it)->Tick(m_GD);
    }

    CheckCollision();
    CheckTriggers();
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
    m_DD->m_cam = m_TPScam;
    if (m_GD->m_GS == GS_GAME)
    {
        m_DD->m_cam = m_TPScam;
    }

    //update the constant buffer for the rendering of VBGOs
    VBGO::UpdateConstantBuffer(m_DD);

    //Draw 3D Game Objects
    if (m_GD->m_GS == GS_GAME)
    {
        for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
        {
            if ((*it)->isRendered()
                && (*it) != pPlayer->pSwordTrigger
                && (*it) != pSign3->pSignTrigger
                && (*it) != pSign4->pSignTrigger
                && (*it) != pF1GroundCheck
                && (*it) != pF2GroundCheck)
            {
                (*it)->Draw(m_DD);
            }
        }
        for (std::vector<CMOGO*>::iterator it = m_EnemySensors.begin(); it != m_EnemySensors.end(); it++)
        {
            if ((*it)->isRendered())
            {
                (*it)->Draw(m_DD);
            }
        }
    }
    else if (m_GD->m_GS == GS_INTRO)
    {
        for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
        {
            if ((*it)->isRendered()
                && (*it) != pPlayer->pSwordTrigger
                && (*it) != pSign1->pSignTrigger
                && (*it) != pSign2->pSignTrigger
                && (*it) != pIntroGroundCheck)
            {
                (*it)->Draw(m_DD);
            }
        }
    }

    // Draw sprite batch stuff 
    m_DD2D->m_Sprites->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
    for (std::vector<GameObject2D*>::iterator it = m_GameObjects2D.begin(); it != m_GameObjects2D.end(); it++)
    {
        if ((*it)->isRendered())
        {
            (*it)->Draw(m_DD2D);
        }
    }
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

    switch (m_GD->m_GS)
    {
        case(GS_MENU):
        {
            if (m_GD->m_KBS.Enter)
            {
                m_GD->m_GS = GS_INTRO;
                //introMusic->Play();
                DisplayIntro();
            }
        }
    default:
        break;
    }
}

void Game::CheckCollision()
{
    for (int i = 0; i < m_PhysicsObjects.size(); i++) for (int j = 0; j < m_ColliderObjects.size(); j++)
    {
        if (m_ColliderObjects[j]->isRendered() && m_PhysicsObjects[i]->Intersects(*m_ColliderObjects[j]))
        {
            XMFLOAT3 eject_vect = Collision::ejectionCMOGO(*m_PhysicsObjects[i], *m_ColliderObjects[j]);
            auto pos = m_PhysicsObjects[i]->GetPos();
            m_PhysicsObjects[i]->SetPos(pos - eject_vect);
        }
    }
}
void Game::CheckTriggers()
{
    for (int i = 0; i < m_PhysicsObjects.size(); i++) for (int j = 0; j < m_TriggerObjects.size(); j++)
    {
        if (m_PhysicsObjects[i]->Intersects(*m_TriggerObjects[j])) //std::cout << "Trigger Detected!" << std::endl;
        {
            if (m_TriggerObjects[j]->isRendered())
            {
                if (m_PhysicsObjects[i] == pPlayer)
                {
                    if (m_TriggerObjects[j] == pIntroGroundCheck || 
                        pF1GroundCheck || pF2GroundCheck)
                    {
                        pPlayer->is_grounded = true;
                    }
                    if (m_TriggerObjects[j] == pFloatingSword)
                    {
                        pFloatingSword->SetRendered(false);
                        pPlayer->has_sword = true;
                    }
                    if (m_TriggerObjects[j] == pIntroExit)
                    {
                        m_GD->m_GS = GS_GAME;
                        DisplayGame();
                    }
                    if (m_TriggerObjects[j] == pDeathTrigger)
                    {
                        LoseLife();
                    }
                }
            }
        }
    }
}

void Game::CoinCollision()
{
    for (int i = 0; i < m_Coins.size(); i++) for (int j = 0; j < m_PhysicsObjects.size(); j++)
    {
        if (m_Coins[i]->isRendered() && m_Coins[i]->Intersects(*m_PhysicsObjects[j]))
        {
            if (m_PhysicsObjects[j] == pPlayer)
            {
                scoreText->SetRendered(false);
                m_Coins[i]->SetRendered(false);
                score++;
                scoreText = new TextGO2D("Coins: " + std::to_string(score));
                scoreText->SetPos(Vector2(100, 10));
                scoreText->SetColour(Color((float*)&Colors::Yellow));
                scoreText->SetScale(1);
                m_GameObjects2D.push_back(scoreText);
            }
        }
    }
}
void Game::EnemyCollision()
{
    for (int i = 0; i < m_Enemies.size(); i++) for (int j = 0; j < m_PhysicsObjects.size(); j++)
    {
        if (m_Enemies[i]->isRendered() && m_Enemies[i]->Intersects(*m_PhysicsObjects[j]))
        {
            if (m_PhysicsObjects[j] == pPlayer)
            {
                LoseLife();
            }
        }
    }
}
void Game::SensorCollision()
{
    for (int i = 0; i < m_PhysicsObjects.size(); i++) for (int j = 0; j < m_EnemySensors.size(); j++)
    {
        if (m_PhysicsObjects[i]->Intersects(*m_EnemySensors[j]))
        {
            if (m_PhysicsObjects[i] == pPlayer)
            {
                m_Enemies[j]->player_spotted = true;
            }
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
            m_Enemies[i]->SetRendered(false);
            m_Enemies[i]->EnemySensor->SetRendered(false);
        }
        if (m_Destructibles[j]->isRendered() && m_Destructibles[j]->Intersects(*m_SwordTrigger[sword]))
        {
            m_Destructibles[j]->SetRendered(false);
        }
    }
}
void Game::SignReading()
{
    for (int i = 0; i < m_PhysicsObjects.size(); i++) for (int j = 0; j < m_Signs.size(); j++)
    {
        if (m_Signs[j]->pSignTrigger->isRendered() && m_PhysicsObjects[i]->Intersects(*m_Signs[j]->pSignTrigger))
        {
            if (m_PhysicsObjects[i] == pPlayer)
            {
                m_Signs[j]->pReadText->SetRendered(true);
                if (m_GD->m_KBS.E && !pPlayer->is_reading)
                {
                    m_Signs[j]->is_reading = true;
                }
            }
        }
        else
        {
            m_Signs[j]->pReadText->SetRendered(false);
            m_Signs[j]->is_reading = false;
            sign1Image->SetRendered(false);
            sign2Image->SetRendered(false);
        }

        //choosing image to render depending on the sign
        if (pSign1->is_reading)
            sign1Image->SetRendered(true);
        if (pSign2->is_reading)
            sign2Image->SetRendered(true);
        if (pSign3->is_reading)
            sign2Image->SetRendered(true);
        if (pSign4->is_reading)
            sign1Image->SetRendered(true);
        //removing read prompt while player is reading
        if (m_Signs[j]->is_reading)
            m_Signs[j]->pReadText->SetRendered(false);
    }
}

void Game::LoseLife()
{
    livesText->SetRendered(false);
    lives--;
    livesText = new TextGO2D("Lives: " + std::to_string(lives));
    livesText->SetPos(Vector2(100, 30));
    livesText->SetColour(Color((float*)&Colors::Red));
    livesText->SetScale(1);
    m_GameObjects2D.push_back(livesText);
    pPlayer->is_respawning = true;
}

void Game::DisplayMenu()
{
    //set menu active
    m_GD->m_GS = GS_MENU;
    title_screen->SetRendered(true);

    //set others inactive
    scoreText->SetRendered(false);
    for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
        (*it)->SetRendered(false);
}
void Game::DisplayIntro()
{
    //set intro active
    m_GD->m_GS = GS_INTRO;
    scoreText->SetRendered(true);
    for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
    {
        (*it)->SetRendered(true);
    }
    pPlayer->pSwordTrigger->SetRendered(false);

    //set others inactive
    title_screen->SetRendered(false);
    for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
    {
        if ((*it) == pPlayer)
            (*it)->SetRendered(true);
        else
            (*it)->SetRendered(false);
    }
}
void Game::DisplayGame()
{
    //set game active
    m_GD->m_GS = GS_GAME;
    pPlayer->is_respawning = true;
    scoreText->SetRendered(true);
    for (std::vector<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
    {
        (*it)->SetRendered(true);
    }
    pPlayer->pSwordTrigger->SetRendered(false);

    //set others inactive
    title_screen->SetRendered(false);
    for (std::vector<GameObject*>::iterator it = m_IntroGOs.begin(); it != m_IntroGOs.end(); it++)
    {
        if ((*it) == pPlayer)
            (*it)->SetRendered(true);
        else
            (*it)->SetRendered(false);
    }
}
void Game::DisplayWin()
{
    //set win active
    m_GD->m_GS = GS_WIN;
}
void Game::DisplayLoss()
{
    //set loss active
    m_GD->m_GS = GS_LOSS;
}

void Game::CreateGround()
{
    //Death trigger
    pDeathTrigger = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -50, 0), 0.0f, 0.0f, 0.0f, Vector3(1000, 1, 1000));
    m_GameObjects.push_back(pDeathTrigger);
    m_TriggerObjects.push_back(pDeathTrigger);

    //Cave outside
    Terrain* pCave = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, 200), 0.0f, 0.0f, 0.0f, Vector3(15, 15, 50));
    m_GameObjects.push_back(pCave);
    m_ColliderObjects.push_back(pCave);

    //Floor 1 ground
    Terrain* pF1Floor = new Terrain("GreenCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0,-10,0), 0.0f, 0.0f, 0.0f, Vector3(15, 1, 25));
    m_GameObjects.push_back(pF1Floor);
    m_ColliderObjects.push_back(pF1Floor);
    pF1GroundCheck = new Terrain("GreenCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -8, 0), 0.0f, 0.0f, 0.0f, Vector3(15, 1, 25));
    m_GameObjects.push_back(pF1GroundCheck);
    m_TriggerObjects.push_back(pF1GroundCheck);

    //Floor2 ground
    Terrain* pF2Floor = new Terrain("GreenCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, -250), 0.0f, 0.0f, 0.0f, Vector3(25, 1, 25));
    m_GameObjects.push_back(pF2Floor);
    m_ColliderObjects.push_back(pF2Floor);
    pF2GroundCheck = new Terrain("GreenCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 2, -250), 0.0f, 0.0f, 0.0f, Vector3(25, 1, 25));
    m_GameObjects.push_back(pF2GroundCheck);
    m_TriggerObjects.push_back(pF2GroundCheck);
}
void Game::CreateIntroGround()
{
    Terrain* pIntroFloor = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -10, -50), 0.0f, 0.0f, 0.0f, Vector3(15, 1, 35));
    m_IntroGOs.push_back(pIntroFloor);
    m_ColliderObjects.push_back(pIntroFloor);
    pIntroGroundCheck = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -8, -50), 0.0f, 0.0f, 0.0f, Vector3(15, 1, 35));
    m_IntroGOs.push_back(pIntroGroundCheck);
    m_TriggerObjects.push_back(pIntroGroundCheck);

    Terrain* pIntroLWall = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(-35, 0, -50), 0.0f, 0.0f, 0.0f, Vector3(1, 10, 35));
    m_IntroGOs.push_back(pIntroLWall);
    m_ColliderObjects.push_back(pIntroLWall);
    Terrain* pIntroRWall = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(35, 0, -50), 0.0f, 0.0f, 0.0f, Vector3(1, 10, 35));
    m_IntroGOs.push_back(pIntroRWall);
    m_ColliderObjects.push_back(pIntroRWall);
    Terrain* pIntroCeiling = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 25, -50), 0.0f, 0.0f, 0.0f, Vector3(15, 1, 35));
    m_IntroGOs.push_back(pIntroCeiling);
    m_ColliderObjects.push_back(pIntroCeiling);
    Terrain* pIntroBackWall = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, 5), 0.0f, 0.0f, 0.0f, Vector3(15, 10, 1));
    m_IntroGOs.push_back(pIntroBackWall);
    m_ColliderObjects.push_back(pIntroBackWall);
    Terrain* pIntroFrontWall = new Terrain("CaveCube", m_d3dDevice.Get(), m_fxFactory, Vector3(0, 0, -135), 0.0f, 0.0f, 0.0f, Vector3(15, 10, 1));
    m_IntroGOs.push_back(pIntroFrontWall);
    m_ColliderObjects.push_back(pIntroFrontWall);

    Terrain* pIntroBreakable = new Terrain("CrackedWall", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -2.5f, -134), 0.0f, 0.0f, 0.0f, Vector3(4, 5, 3));
    m_IntroGOs.push_back(pIntroBreakable);
    m_ColliderObjects.push_back(pIntroBreakable);
    m_Destructibles.push_back(pIntroBreakable);
    pIntroExit = new Terrain("IntroExit", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -2.5f, -135.25f), 0.0f, 0.0f, 0.0f, Vector3(3, 5, 3));
    m_IntroGOs.push_back(pIntroExit);
    m_TriggerObjects.push_back(pIntroExit);

    pFloatingSword = new Coin("Sword", m_d3dDevice.Get(), m_fxFactory, Vector3(0, -5, -75));
    pFloatingSword->SetYaw(90);
    pFloatingSword->SetScale(Vector3(0.25f, 0.3f, 0.25f));
    m_IntroGOs.push_back(pFloatingSword);
    m_TriggerObjects.push_back(pFloatingSword);
}
