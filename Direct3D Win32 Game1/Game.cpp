//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Particle.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(const HWND window, const int width, const int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);
    
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    
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
void Game::Update(DX::StepTimer const& timer)
{
	const float elapsedTime = static_cast<float>(timer.GetElapsedSeconds());
	
	const auto kb = m_keyboard->GetState();
	if (kb.Escape)
		PostQuitMessage(0);

	if (kb.W && !kb.LeftControl)
		CamZ += 0.005f;
	if (kb.S && !kb.LeftControl)
		CamZ -= 0.005f;

	if (kb.A && !kb.LeftControl)
		RotZ -= 0.005f;
	if (kb.D && !kb.LeftControl)
		RotZ += 0.005f;


	if (kb.W && kb.LeftControl)
		CamY -= 0.005f;
	if (kb.S && kb.LeftControl)
		CamY += 0.005f;

	if (kb.D && kb.LeftControl)
		CamX -= 0.005f;
	if (kb.A && kb.LeftControl)
		CamX += 0.005f;

	if (kb.Space && Spawn == false || kb.F12 && Spawn == false)
	{
		Idle = false;
		TakeoffLegsStage = true;
		Spawn = true;
	}
	if (kb.F1)
		m_view = Matrix::CreateLookAt(Vector3(4.0f, 2.0f, 4.0f),Vector3::Zero, Vector3::UnitY);
	if (kb.F2)
		m_view = Matrix::CreateLookAt(Vector3(0.8f, 0.8f, 0.8f), Vector3(0.6f,0.5f,0.3f), Vector3::UnitY);
	if (kb.F3)
		m_view = Matrix::CreateLookAt(Vector3(0.8f, StoreY, 0.8f), Vector3(0.6f, 0.5f, 0.3f), Vector3::UnitY);
	if (kb.F4)
		m_view = Matrix::CreateLookAt(Vector3(0.5f, 0.5f, 0.5f), Vector3(0.6f, 0.5f, 0.3f), Vector3::UnitY);
	if (kb.F5)
	{
		if (Fpressed == false)
		{ 
			Fpressed = true;
			State += 1;
			if (State >= 3)
			{ 
				State = 0;
			}
		}
	}
	if (!kb.F5)
		Fpressed = false;

	if (State == 1)
	{
		m_model_stick->UpdateEffects([&](IEffect* effect)
		{
			auto Texture = dynamic_cast<BasicEffect*>(effect);
			if (Texture)
				Texture->SetTextureEnabled(false);
		});
		m_model->UpdateEffects([&](IEffect* effect)
		{
			auto Texture = dynamic_cast<BasicEffect*>(effect);
			if (Texture)
				Texture->SetTextureEnabled(false);
		});
		m_model_Legs->UpdateEffects([&](IEffect* effect)
		{
			auto Texture = dynamic_cast<BasicEffect*>(effect);
			if (Texture)
				Texture->SetTextureEnabled(false);
		});
	}
	else
	{
		m_model_stick->UpdateEffects([&](IEffect* effect)
		{
			auto Texture = dynamic_cast<BasicEffect*>(effect);
			if (Texture)
				Texture->SetTextureEnabled(true);
		});
		m_model->UpdateEffects([&](IEffect* effect)
		{
			auto Texture = dynamic_cast<BasicEffect*>(effect);
			if (Texture)
				Texture->SetTextureEnabled(true);
		});
		m_model_Legs->UpdateEffects([&](IEffect* effect)
		{
			auto Texture = dynamic_cast<BasicEffect*>(effect);
			if (Texture)
				Texture->SetTextureEnabled(true);
		});
	}

	const XMMATRIX move = XMMatrixTranslation(CamX, CamY, CamZ);
	const XMMATRIX Rot = XMMatrixRotationY(RotZ);

	m_view *= Rot;
	m_view *= move;
	CamX = 0.0f; CamY = 0.0f; CamZ = 0.0f; RotZ = 0.0f;
	m_sphereEffect->SetView(m_view);


	if (TakeoffLift)
	{
		StoreX += 0.001f;
		StoreY += 0.001f;
		StoreZ += 0.001f;
	}
	if (Goback)
	{
		StoreX -= 0.001f;
		StoreY -= 0.001f;
		StoreZ -= 0.001f;
	}
	if (LowerLegs)
	{
		hold -= 0.0001f;
	}
	if (ParticleSystem)
	{
		m_Particals.Advance();
		m_Particals.AddGravity(Vector3::Vector3(0.0f, -0.00001f, 0.0f));
	}
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


	XMMATRIX trans = XMMatrixTranslation(0.0f, -4.95f, 0.0f);
	m_world *= trans;
	if (State == 0)
	{
		m_shape->Draw(m_world, m_view, m_proj, Colors::Gray, m_texture.Get());
	}
	if (State == 1)
	{
		m_shape->Draw(m_world, m_view, m_proj, Colors::Gray);
	}
	if (State == 2)
	{
		m_shape->Draw(m_world, m_view, m_proj, Colors::Gray, m_texture.Get(),true);
	}
	trans = XMMatrixTranslation(0.0f, 4.95f, 0.0f);
	m_world *= trans;

	if (State == 0)
		m_model_stick->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
	if (State == 1)
	{
		m_model_stick->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
	}
	if (State == 2)
		m_model_stick->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj,true);

	if (Idle)
	{
		LowerLegs = false;
		if (State == 0)
		{
			m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
		}
		if (State == 1)
		{
			m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
		}
		if (State == 2)
		{
			m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);
			m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);
			m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj , true);
		}


	}
	if (TakeoffLegsStage)
	{
		Idle = false;
		ParticleSystem = true;
		if (State == 0)
		{
			m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
		}
		if (State == 1)
		{
			m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
		}
		if (State == 2)
		{
			m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj , true);
			m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj , true);
		}

		if (LegMovement <= LegMoveMax)
		{
			Matrix trans = Matrix::CreateTranslation(0.0f, LegMovement, 0.0f);
			m_world *= trans;

			if (State == 0)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			
			if (State == 1)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);


			trans = Matrix::CreateTranslation(0.0f, -LegMovement, 0.0f);
			m_world *= trans;
			LegMovement += 0.0001f;
		}
		if (LegMovement > LegMoveMax)
		{
			TakeoffLift = true;
			LegMovement = 0.00001f;
		}
	}
	if (TakeoffLift)
	{
		TakeoffLegsStage = false;
		const float time = static_cast<float>(m_timer.GetTotalSeconds());
		const Vector3 Pos = Vector3(StoreX, StoreY, StoreZ);
		if (StoreX >= Xmove && StoreY >= Ymove && StoreZ >= Zmove)
		{
			Goback = true;
		}
		else
		{
			m_world *= Matrix::CreateTranslation(Pos);

			if (State == 0)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);





			Matrix trans = Matrix::CreateTranslation(0.0f, LegMoveMax, 0.0f);
			m_world *= trans;

			if (State == 0)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);



			trans = Matrix::CreateTranslation(0.0f, -LegMoveMax, 0.0f);
			m_world *= trans;



			const Vector3 Dest = Vector3(0.0f, 0.01f, 0.0f);
			const Vector3 poss = Vector3::Lerp(Vector3::Zero, Dest, cos(time));
			trans = Matrix::CreateTranslation(poss);

			m_world *= trans;

			if (State == 0)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);

			trans = Matrix::CreateTranslation(-poss);

			m_world *= trans;
			m_world *= Matrix::CreateTranslation(-Pos);
		}
	}
	if (Goback)
	{
		TakeoffLift = false;
		if (StoreX <= 0.0f && StoreY <= 0.0f && StoreZ <= 0.0f)
		{
			LowerLegs = true;
		}
		else
		{
			const Vector3 Pos = Vector3(StoreX, StoreY, StoreZ);
			const float time = static_cast<float>(m_timer.GetTotalSeconds());
			m_world *= Matrix::CreateTranslation(Pos);

			if (State == 0)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);



			Matrix trans = Matrix::CreateTranslation(0.0f, LegMoveMax, 0.0f);
			m_world *= trans;

			if (State == 0)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj,true);


			trans = Matrix::CreateTranslation(0.0f, -LegMoveMax, 0.0f);
			m_world *= trans;



			const Vector3 Dest = Vector3(0.0f, 0.01f, 0.0f);
			const Vector3 poss = Vector3::Lerp(Vector3::Zero, Dest, cos(time));
			trans = Matrix::CreateTranslation(poss);

			m_world *= trans;

			if (State == 0)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);

			trans = Matrix::CreateTranslation(-poss);

			m_world *= trans;
			m_world *= Matrix::CreateTranslation(-Pos);
		}
	}
	if (LowerLegs)
	{
		Goback = false;
		if (hold >= 0.0f)
		{
			if (State == 0)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			if (State == 1)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);
			if (State == 2)
				m_model->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);


			Matrix trans = Matrix::CreateTranslation(0.0f, hold, 0.0f);
			m_world *= trans;

			if (State == 0)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model_Legs->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);


			trans = Matrix::CreateTranslation(0.0f, -hold, 0.0f);
			m_world *= trans;

			const Vector3 origin = Vector3(0.0f, 0.0f, 0.0f);
			const Vector3 Dest = Vector3(0.0f, 0.01f, 0.0f);

			const float time = static_cast<float>(m_timer.GetTotalSeconds());
			const Vector3 poss = Vector3::Lerp(origin, Dest, cos(time));
			trans = Matrix::CreateTranslation(poss);

			m_world *= trans;

			if (State == 0)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 1)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

			if (State == 2)
				m_model_Wings->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj, true);

			trans = Matrix::CreateTranslation(-poss);

			m_world *= trans;
		}
		else
		{
			Spawn = false;
			Idle = true;
		}
	}
	if (ParticleSystem)
	{
		m_Particals.Draw(m_world, m_view, m_proj, m_texture);

	}
		
		

		m_sphereEffect->SetProjection(m_proj);
		m_sphereEffect->SetView(m_view);
		m_sphereEffect->SetWorld(m_world);
		m_sphereEffect->Apply(m_d3dContext.Get());

		if (State == 0)
			m_shpere->Draw(m_sphereEffect.get(), m_inputLayout.Get(), true);
		if (State == 1)
			m_shpere->Draw(m_sphereEffect.get(), m_inputLayout.Get(), true);
		if (State == 2)
			m_shpere->Draw(m_sphereEffect.get(), m_inputLayout.Get(), true,true);
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
	const CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
	const HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == 0x887A0005L || hr == 0x887A0007L) // 0x887A0005L    DXGI_ERROR_DEVICE_REMOVED      0x887A0007L    DXGI_ERROR_DEVICE_RESET
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated() const
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated() const
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending() const
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(const int width, const int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
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
        _countof(featureLevels),
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
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));
		
	DX::ThrowIfFailed(
		CreateWICTextureFromFile(m_d3dDevice.Get(), m_d3dContext.Get(),L"sand-texture.jpg", nullptr,
			m_texture.ReleaseAndGetAddressOf()));

	const float min = 0.0001f;
	const float max = 0.3f;
	const float maxY = 0.8f;

	Xmove = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	Ymove = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maxY - min)));
	Zmove = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));

	if (Xmove > 0.3)
		Xmove = 0.3f;

	if(Ymove > 0.8)
		Ymove = 0.8f;

	if (Zmove > 0.3)
		Zmove = 0.3f;


	StoreX = Xmove / 100;
	StoreY = Ymove / 100;
	StoreZ = Zmove / 100;

	

	m_FlyPath = Vector3(Xmove, Ymove, Zmove);

	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());

	m_fxFactory = std::make_unique<EffectFactory>(m_d3dDevice.Get());

	m_model = Model::CreateFromSDKMESH(m_d3dDevice.Get(), L"Fly.sdkmesh", *m_fxFactory);

	m_model_stick = Model::CreateFromSDKMESH(m_d3dDevice.Get(), L"Log.sdkmesh", *m_fxFactory);

	m_model_Wings = Model::CreateFromSDKMESH(m_d3dDevice.Get(), L"Wings.sdkmesh", *m_fxFactory);

	m_model_Legs = Model::CreateFromSDKMESH(m_d3dDevice.Get(), L"Legs.sdkmesh", *m_fxFactory);

	m_world = Matrix::Identity;

	m_shape = GeometricPrimitive::CreateCube(m_d3dContext.Get(), m_size);


	m_sphereEffect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
	m_sphereEffect->SetTextureEnabled(false);
	m_sphereEffect->SetPerPixelLighting(true);
	m_sphereEffect->SetLightingEnabled(true);
	m_sphereEffect->SetLightEnabled(0, true);
	m_sphereEffect->SetLightDiffuseColor(0, Colors::GhostWhite);
	m_sphereEffect->SetLightDirection(0, -Vector3::UnitY);
	m_sphereEffect->SetAlpha(0.4f);

	m_shpere = GeometricPrimitive::CreateSphere(m_d3dContext.Get(), 4.5f);



	m_shpere->CreateInputLayout(m_sphereEffect.get(), m_inputLayout.ReleaseAndGetAddressOf());

	m_Particals.ParticleStart(m_d3dContext);

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

	const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
	const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
	const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	const UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
		const HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == 0x887A0005L || hr == 0x887A0007L)  // 0x887A0005L                 DXGI_ERROR_DEVICE_RESET             0x887A0007L
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
	const CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

	const CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
	CamX = 2.0f; CamY = 2.0f; CamZ = 2.0f;
	m_view = Matrix::CreateLookAt(Vector3(CamX, CamY, CamZ),Vector3::Zero, Vector3::UnitY);
	m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,static_cast<float>(backBufferWidth) / static_cast<float>(backBufferHeight), 0.1f, 10.f);
	CamX = 0.0f; CamY = 0.0f; CamZ = 0.0f;
}

void Game::OnDeviceLost()
{
	m_states.reset();
	m_fxFactory.reset();
	m_model.reset();
	m_model_stick.reset();
	m_model_Legs.reset();
	m_model_Wings.reset();
	m_shape.reset();
	m_shpere.reset();
    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();
	m_texture.Reset();
    CreateDevice();
    CreateResources();
}