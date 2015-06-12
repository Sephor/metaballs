#include "FluidSimulator.h"

#include <iostream>
#include <glm\geometric.hpp>

FluidSimulator::FluidSimulator() : m_gravConstant(0.f, -.3f, 0.f), m_isRunning(false)
{
	m_groundPlane.normal = glm::vec3(.0f, 1.f, .0f);
	m_groundPlane.distance = .0f;
	m_metaballs = std::vector<Metaball>();
	for (int i = 0; i < 20; i++)
	{
		Metaball m;
		m.position = glm::vec3(i * 0.75f, 3.0f + i * 0.75f, .0f);
		m.radius = 1.f;
		m.velocity = glm::vec3(.0f);
		m_metaballs.push_back(m);
	}
}

FluidSimulator::~FluidSimulator()
{
	
}

const std::array<glm::vec4, 20> FluidSimulator::getMetaballs() const
{
	std::array<glm::vec4, 20> temp;

	for (int i = 0; i < m_metaballs.size(); i++)
	{
		temp[i] = glm::vec4(m_metaballs[i].position, m_metaballs[i].radius);
	}

	return temp;
}

void FluidSimulator::startSimulation()
{
	m_isRunning = true;
	m_lastTime = std::chrono::high_resolution_clock::now();
}

void FluidSimulator::stopSimulation()
{
	m_isRunning = false;
}

bool FluidSimulator::getIsRunning() const
{
	return m_isRunning;
}

void FluidSimulator::setIsRunning(bool value)
{
	if (m_isRunning)
		stopSimulation();
	else
		startSimulation();
}

void FluidSimulator::update()
{
	if (!m_isRunning) return;
	float elapsedTime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::high_resolution_clock::now() - m_lastTime).count();
	m_lastTime = std::chrono::high_resolution_clock::now();

	for (auto & metaball : m_metaballs)
	{
		if (doesCollide(metaball, m_groundPlane, elapsedTime))
		{
			metaball.position += metaball.velocity * collisionTime(metaball, m_groundPlane);
			metaball.velocity = glm::vec3(.0f);
		}
		else
		{
			metaball.position += metaball.velocity * elapsedTime;
			metaball.velocity += m_gravConstant * elapsedTime;
		}
	}
}

bool FluidSimulator::doesCollide(const Metaball & metaball, const Plane & plane, float deltaTime)
{
	glm::vec3 latePosition = metaball.position + metaball.velocity * deltaTime;
	float lateDistanceToPlane = glm::dot(latePosition, plane.normal) - plane.distance;
	
	return lateDistanceToPlane <= .0f;
}

float FluidSimulator::collisionTime(const Metaball & metaball, const Plane & plane)
{
	float positionDotNormal = glm::dot(metaball.position, plane.normal);
	float velocityDotNormal = glm::dot(metaball.velocity, plane.normal);
	if (fabs(velocityDotNormal) <= 1.0e-10)
	{
		return .0f;
	}
	return - positionDotNormal / velocityDotNormal;
}