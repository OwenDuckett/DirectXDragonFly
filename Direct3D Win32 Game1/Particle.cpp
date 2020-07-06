#include "pch.h"
#include "Particle.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;

ParticleSystem::ParticleSystem()
{

}


void ParticleSystem::ParticleStart(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> &context)
{
		for (int i = 0; i < 250; i++)
		{
			const float Velmin = 0.00001f;
			const float Velmax = 0.003f;

			const float Velx = Velmin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (Velmax - Velmin)));
			const float Vely = Velmin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (Velmax - Velmin)));
			const float Velz = Velmin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (Velmax - Velmin)));

			const float Posmin = 0.006f;
			const float Posmax = 0.008f;

			const float Posx = Posmin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (Posmax - Posmin)));
			const float Posy = 0.1f;
			const float Posz = Posmin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (Posmax - Posmin)));


			const float min = 0.0001f;
			const float max = 0.01f;
			const float Size = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));

			particles[i].m_position = Vector3::Vector3(Posx, Posy, Posz);
			particles[i].m_velocity = Vector3::Vector3(Velx, Vely, Velz);
			particles[i].obj = GeometricPrimitive::CreateTetrahedron(context.Get(), Size);
		}

		this->Dead = false;
}

//ParticleSystem::~ParticleSystem()
//{
//
//}

void ParticleSystem::Advance()
{
	for (int i = 0; i < 250; i++)
	{
		particles[i].m_position += particles[i].m_velocity;

		if (particles[i].m_position.y < 0.0f)
			this->Dead = true;
	}
}
void ParticleSystem::AddGravity(const DirectX::SimpleMath::Vector3 &GravScale)
{
	for (int i = 0; i < 250; i++)
	{
		particles[i].m_velocity += GravScale;
	}
}
void ParticleSystem::Draw(DirectX::SimpleMath::Matrix &m_world, const DirectX::SimpleMath::Matrix &m_veiw, const DirectX::SimpleMath::Matrix &m_proj, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>&m_texture) const
{
	for (int i = 0; i < 250; i++)
	{
		m_world *= Matrix::CreateTranslation(particles[i].m_position);
		particles[i].obj->Draw(m_world, m_veiw, m_proj, Colors::Gray, m_texture.Get());
		m_world *= Matrix::CreateTranslation(-particles[i].m_position);
	}
}
bool  ParticleSystem::IsDead() const
{
	return Dead;
}
void ParticleSystem::Release()
{
	for (int i = 0; i < 250; i++)
	{
		particles[i].obj.reset();
	}
}