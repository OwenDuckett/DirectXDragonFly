#pragma once
#include "pch.h"
#include "StepTimer.h"


class ParticleSystem
{
public:
	ParticleSystem();
	//~ParticleSystem();

	void ParticleStart(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> &context);
	void Advance();
	void Draw(DirectX::SimpleMath::Matrix &m_world, const DirectX::SimpleMath::Matrix &m_veiw, const DirectX::SimpleMath::Matrix &m_proj, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>&m_texture) const;
	void AddGravity(const DirectX::SimpleMath::Vector3 &GravScale);
	void Release();
	bool IsDead() const;

	
private:


	bool Dead = false;
	struct Particle
	{
		DirectX::SimpleMath::Vector3		m_velocity;
		DirectX::SimpleMath::Vector3		m_position;
		float size;
		std::unique_ptr<DirectX::GeometricPrimitive>	obj;

	};
	Particle particles[250];
};


