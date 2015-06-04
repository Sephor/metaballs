#include "FluidSimulator.h"

FluidSimulator::FluidSimulator() : m_gravConstant(0.f, -.1f, 0.f)
{
	m_metaballs = std::vector<Metaball>();
	for (int i = 0; i < 20; i++)
	{
		Metaball m;
		m.position = glm::vec3(i * 0.75f, .0f, .0f);
		m.radius = 1.f;
		m.velocity = glm::vec3(.0f);
		m_metaballs.push_back(m);
	}
}


FluidSimulator::~FluidSimulator()
{

}

const std::vector<FluidSimulator::Metaball> & FluidSimulator::getMetaballs() const
{
	return m_metaballs;
}

void FluidSimulator::update(float elapsedTime)
{
	elapsedTime = elapsedTime > .05f ? .05f : elapsedTime;

	for (auto & metaball : m_metaballs)
	{
		metaball.velocity += m_gravConstant * elapsedTime;
		metaball.position += metaball.velocity * elapsedTime;
	}
}