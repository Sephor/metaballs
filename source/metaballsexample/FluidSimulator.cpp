#include "FluidSimulator.h"

#include <iostream>

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
	m_lastTime = std::chrono::high_resolution_clock::now();
}


FluidSimulator::~FluidSimulator()
{

}

std::vector<glm::vec4> FluidSimulator::metaballs()
{
	std::vector<glm::vec4> temp(m_metaballs.size());

	for (int i = 0; i < m_metaballs.size(); i++)
	{
		temp[i] = glm::vec4(m_metaballs[i].position, m_metaballs[i].radius);
	}

	return temp;
}

void FluidSimulator::update()
{
	float elapsedTime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::high_resolution_clock::now() - m_lastTime).count();
	m_lastTime = std::chrono::high_resolution_clock::now();

	for (auto & metaball : m_metaballs)
	{
		metaball.velocity += m_gravConstant * elapsedTime;
		metaball.position += metaball.velocity * elapsedTime;
	}
}