//
// Game.h
//

#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "Particle.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game();
	Game(const Game &) = delete;
	//~Game() = delete;
    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
	void OnActivated() const;
	void OnDeactivated() const;
	void OnSuspending() const;
    void OnResuming();
    void OnWindowSizeChanged(const int width, const int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;


private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

	float CamX= 0.0f, CamY = 0.0f, CamZ = 0.0f;


    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>m_texture;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;

    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

	DirectX::SimpleMath::Matrix						m_world;
	DirectX::SimpleMath::Matrix						m_view;
	DirectX::SimpleMath::Matrix						m_proj;
	DirectX::SimpleMath::Matrix						m_WorldVeiw;

	DirectX::SimpleMath::Vector3					m_WorldVeiwX;
	DirectX::SimpleMath::Vector3					m_WorldVeiwY;
	DirectX::SimpleMath::Vector3					m_WorldVeiwZ;


	std::unique_ptr<DirectX::BasicEffect>			m_sphereEffect;
	std::unique_ptr<DirectX::CommonStates>			m_states;
	std::unique_ptr<DirectX::IEffectFactory>		m_fxFactory;
	std::unique_ptr<DirectX::Model>					m_model;
	std::unique_ptr<DirectX::Model>					m_model_Wings;
	std::unique_ptr<DirectX::Model>					m_model_stick;
	std::unique_ptr<DirectX::Model>					m_model_Legs;
	std::unique_ptr<DirectX::GeometricPrimitive>	m_shape;
	std::unique_ptr<DirectX::GeometricPrimitive>	m_shpere;
	std::unique_ptr<DirectX::Keyboard>				m_keyboard;
	std::unique_ptr<DirectX::Mouse>					m_mouse;

	ParticleSystem									m_Particals;


	DirectX::SimpleMath::Vector3					m_FlyPath;
	const float m_size = 10.00f;
	int State = 0;
	float Ymove = 0.0f, Xmove = 0.0f, Zmove = 0.0f, LegMoveMax = 0.007f, LegMovement = 0.00015f, RotZ = 0.0f, StoreX = 0.0f, StoreZ = 0.0f, StoreY = 0.0f, hold = LegMoveMax;
	bool TakeoffLegsStage = false, TakeoffLift = false, TakeoffAnimateWings = false, ParticleSystem = false, Land = false, Idle = true, Goback = false, LowerLegs = false, Spawn = false, Fpressed = false;


};