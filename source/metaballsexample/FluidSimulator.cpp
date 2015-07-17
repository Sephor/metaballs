#include "FluidSimulator.h"

#include <iostream>
#include <math.h>
#include <glm\geometric.hpp>
#include <glm\gtx\rotate_vector.hpp>

FluidSimulator::FluidSimulator()
	: m_gravConstant(0.f, -1.f, 0.f)
	, m_isRunning(false)
	, m_metaballSelector(0)
	, m_gen(m_rd())
	, m_dis(0, 1)
	, m_twoPi(6.283185f)
	, m_repulsionLimitFactor(.7f)
	, m_repulsionFactor(5.f)
	, m_attractionFactor(1.f)
	, m_grid(Grid(100, 0.4f, glm::vec3(-20.f, 0.f, -20.f)))
{
	m_groundPlane.normal = glm::vec3(.0f, 1.f, .0f);
	m_groundPlane.distance = .0f;
	m_groundPlane.friction = .5f;

	m_metaballEmitter.period = .01f;
	m_metaballEmitter.position = glm::vec3(-1.f, 5.f, 0.f);
	m_metaballEmitter.startVelocity = glm::vec3(1.f, 0.f, 0.f);
	m_metaballEmitter.nextEmission = 0;
	m_metaballEmitter.metaballRadius = .1f;
	m_metaballEmitter.spread = .5f;
	m_metaballEmitter.spray = .1f;

	m_metaballs = std::vector<Metaball>();
	m_metaballs.resize(2000);
	for (int i = 0; i < m_metaballs.size(); i++)
	{
		Metaball m;
		m.position = glm::vec3(i * 0.9f, 0.f, 500.f);
		m.radius = 0.f;
		m.velocity = glm::vec3(.0f);
		m.acceleration = glm::vec3(.0f);
		m_metaballs[i] = m;
	}
	m_lastTime = std::chrono::high_resolution_clock::now();
}

FluidSimulator::~FluidSimulator()
{
	
}

const std::vector<glm::vec4>& FluidSimulator::getMetaballs()
{
	if (m_metaballsChanged)
	{
		if (m_metaballs.size() != m_metaballsAsVec.size())
			m_metaballsAsVec.resize(m_metaballs.size());
		for (int i = 0; i < m_metaballs.size(); i++)
		{
			m_metaballsAsVec[i] = glm::vec4(m_metaballs[i].position, m_metaballs[i].radius);
		}
	}
	return m_metaballsAsVec;
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

float FluidSimulator::getAttractionFactor() const
{
	return m_attractionFactor;
}

void FluidSimulator::setAttractionFactor(float value)
{
	m_attractionFactor = value;
}

float FluidSimulator::getRepulsionFactor() const
{
	return m_repulsionFactor;
}

void FluidSimulator::setRepulsionFactor(float value)
{
	m_repulsionFactor = value;
}

float FluidSimulator::getSpread() const
{
	return m_metaballEmitter.spread;
}

void FluidSimulator::setSpread(float value)
{
	m_metaballEmitter.spread = value;
}

float FluidSimulator::getSpray() const
{
	return m_metaballEmitter.spray;
}

void FluidSimulator::setSpray(float value)
{
	m_metaballEmitter.spray = value;
}

float FluidSimulator::getMetaballRadius() const
{
	return m_metaballEmitter.metaballRadius;
}

void FluidSimulator::setMetaballRadius(float value)
{
	m_metaballEmitter.metaballRadius = value;
}

float FluidSimulator::getEmitterPeriod() const
{
	return m_metaballEmitter.period;
}

void FluidSimulator::setEmitterPeriod(float value)
{
	m_metaballEmitter.period = value;
}

glm::vec3 FluidSimulator::computeInteractions(Metaball& metaball, std::vector<Metaball*>& neighbours)
{

	glm::vec3 forceSum{0.f};
	for (auto& other_metaball : neighbours)
	{
		float radiusSum = other_metaball->radius + metaball.radius;
		float repulsionLimit = radiusSum * m_repulsionLimitFactor;
		glm::vec3 difference = other_metaball->position - metaball.position;
		
		float distance = glm::dot(difference, difference);
		if (distance > radiusSum * radiusSum) continue;

		distance = sqrt(distance);
		float sqrtRadiusSum = sqrt(radiusSum);
		float forceMagnitude;

		if (distance > repulsionLimit)
		{
			float halfAttInterval = (radiusSum - radiusSum * m_repulsionLimitFactor) * .5f;
			float term = (distance - radiusSum + halfAttInterval) / halfAttInterval * sqrtRadiusSum;
			forceMagnitude = m_attractionFactor * (term * term - radiusSum);
		}
		else	//attraction 
		{
			float term = distance / (radiusSum * m_repulsionLimitFactor) * sqrtRadiusSum;
			forceMagnitude = m_repulsionFactor * (radiusSum - term * term);
		}
		forceSum -= forceMagnitude / distance * difference;
	}

	return forceSum;
}

void FluidSimulator::updateRepulsion()
{
	for (auto& metaball : m_metaballs)
	{
		auto& neighbours = m_grid.getNeighbors(metaball);
		metaball.acceleration += computeInteractions(metaball, neighbours);
	}
}

void FluidSimulator::updatePositions(float elapsedTime)
{
	for (auto & metaball : m_metaballs)
	{
		glm::vec3 newPosition;
		metaball.acceleration += m_gravConstant;
		metaball.velocity += metaball.acceleration * elapsedTime;
		if (doesCollide(metaball, m_groundPlane, elapsedTime))
		{
			float t0 = collisionTime(metaball, m_groundPlane);
			newPosition = metaball.position + metaball.velocity * t0;
			metaball.velocity = metaball.velocity - glm::dot(metaball.velocity, m_groundPlane.normal) * m_groundPlane.normal;
			metaball.velocity -= metaball.velocity * m_groundPlane.friction * (elapsedTime - t0);
			newPosition += metaball.velocity * (elapsedTime - t0);
		}
		else
		{
			newPosition = metaball.position + metaball.velocity * elapsedTime;
		}
		m_grid.updateMetaball(metaball, newPosition);

		metaball.acceleration = glm::vec3(.0f);
	}
}

void FluidSimulator::emitMetaball()
{
	glm::vec3 posOffset(m_dis(m_gen) * m_metaballEmitter.spread, .0f, .0f);
	glm::vec3 velOffset(m_dis(m_gen) * m_metaballEmitter.spray, .0f, .0f);
	glm::vec3 normal(0.f, 0.f, 1.f);

	auto randomAngle = m_dis(m_gen) * 2.f * m_twoPi;
	posOffset = glm::rotateY(posOffset, randomAngle);
	normal = glm::rotateY(normal, randomAngle);
	posOffset = glm::rotate(posOffset, m_dis(m_gen) * m_twoPi, normal);

	normal = glm::vec3(0.f, 0.f, 1.f);

	randomAngle = m_dis(m_gen) * 2.f * m_twoPi;
	velOffset = glm::rotateY(velOffset, randomAngle);
	normal = glm::rotateY(normal, randomAngle);
	velOffset = glm::rotate(velOffset, m_dis(m_gen) * m_twoPi, normal);

	m_metaballEmitter.nextEmission += m_metaballEmitter.period;
	m_metaballs[m_metaballSelector].velocity = m_metaballEmitter.startVelocity + velOffset;
	m_metaballs[m_metaballSelector].radius = m_metaballEmitter.metaballRadius;
	//GRID
	m_grid.updateMetaball(m_metaballs[m_metaballSelector], m_metaballEmitter.position + posOffset);
	//!GRID
	m_metaballSelector = (m_metaballSelector + 1) % m_metaballs.size();
}

void FluidSimulator::update()
{
	if (!m_isRunning) return;
	m_metaballsChanged = true;

	float elapsedTime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::high_resolution_clock::now() - m_lastTime).count();
	m_lastTime = std::chrono::high_resolution_clock::now();
//DEBUG
	static int frame;
	if (frame % 30 == 0) {
		std::cout << "Framerate: " << 1/elapsedTime << std::endl;
	};
//!DEBUG
	elapsedTime = (elapsedTime > 0.05f) ? 0.05f : elapsedTime;
	m_metaballEmitter.nextEmission -= elapsedTime;
	while (m_metaballEmitter.nextEmission <= 0.f)
	{
		emitMetaball();
	}

	updateRepulsion();
	updatePositions(elapsedTime);
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