#include "FluidSimulator.h"

#include <iostream>
#include <math.h>
#include <glm\geometric.hpp>
#include <glm\gtx\rotate_vector.hpp>

FluidSimulator::FluidSimulator() 
	: m_gravConstant(0.f, -0.3f, 0.f)
	, m_isRunning(false)
	, m_metaballSelector(0)
	, m_gen(m_rd())
	, m_dis(0, 1)
	, m_twoPi(6.283185f)
	, m_repulsionLimitFactor(.7f)
	, m_repulsionFactor(5.f)
	, m_attractionFactor(1.f)
{
	m_groundPlane.normal = glm::vec3(.0f, 1.f, .0f);
	m_groundPlane.distance = .0f;
	m_groundPlane.friction = .5f;

	m_metaballEmitter.period = .03f;
	m_metaballEmitter.position = glm::vec3(-1.f, 5.f, 0.f);
	m_metaballEmitter.startVelocity = glm::vec3(0.f, 0.f, 0.f);
	m_metaballEmitter.nextEmission = 0;
	m_metaballEmitter.metaballRadius = .1f;
	m_metaballEmitter.spread = .5f;
	m_metaballEmitter.spray = .1f;

	m_metaballs = std::vector<Metaball>();
	for (int i = 0; i < 20; i++)
		for (int j = 0; j < 20; j++)
		{
			Metaball m;
			m.position = glm::vec3(i * 0.9f, 0.f, j * 0.9f);
			m.radius = 0.f;
			m.velocity = glm::vec3(.0f);
			m.acceleration = glm::vec3(.0f);
			m_metaballs.push_back(m);
		}
	m_lastTime = std::chrono::high_resolution_clock::now();
}

FluidSimulator::~FluidSimulator()
{
	
}

const std::array<glm::vec4, 400> FluidSimulator::getMetaballs() const
{
	std::array<glm::vec4, 400> temp;

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

void FluidSimulator::applyRepulsion()
{
	for (int i = 0; i < (m_metaballs.size() - 1); i++)
		for (int j = i + 1; j < m_metaballs.size(); j++)
		{
			float radiusSum = m_metaballs[j].radius + m_metaballs[i].radius;
			float repulsionLimit = radiusSum * m_repulsionLimitFactor;
			glm::vec3 difference = m_metaballs[j].position - m_metaballs[i].position;
			float distance = glm::dot(difference, difference);

			if (distance > radiusSum*radiusSum) continue;
			
			distance = sqrtf(distance);
			float sqrtRadiusSum = sqrtf(radiusSum);
			float forceMagnitude;

			if (distance > repulsionLimit)
			{
				float halfAttInterval = (radiusSum - radiusSum * m_repulsionLimitFactor) * .5f;
				float term = (distance - radiusSum + halfAttInterval) / halfAttInterval * sqrtRadiusSum;
				forceMagnitude = m_attractionFactor * (term * term - radiusSum);
			}
			else
			{
				float term = distance / (radiusSum * m_repulsionLimitFactor) * sqrtRadiusSum;
				forceMagnitude = m_repulsionFactor * (radiusSum - term * term);
				forceMagnitude *= collisionFactor(m_metaballs[j], m_metaballs[i]);
			}
			glm::vec3 force = forceMagnitude / distance * difference;
			m_metaballs[i].acceleration -= force;
			m_metaballs[j].acceleration += force;
		}
}

float FluidSimulator::collisionFactor(Metaball& ball1 , Metaball& ball2){

	glm::vec3 veloDifference = ball1.velocity - ball2.velocity;
	glm::vec3 posDifference = ball2.position - ball1.position;

	float factor = glm::dot( veloDifference , glm::normalize(posDifference)); 

	if (factor > 0 ){
		return 1.f + 10* factor;
	} else 
	return 1.f;
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
	m_metaballs[m_metaballSelector].position = m_metaballEmitter.position + posOffset;
	m_metaballs[m_metaballSelector].velocity = m_metaballEmitter.startVelocity + velOffset;
	m_metaballs[m_metaballSelector].radius = m_metaballEmitter.metaballRadius;
	m_metaballSelector = (m_metaballSelector + 1) % m_metaballs.size();
}

void FluidSimulator::update()
{
	if (!m_isRunning) return;

	float elapsedTime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::high_resolution_clock::now() - m_lastTime).count();
	m_lastTime = std::chrono::high_resolution_clock::now();

	m_metaballEmitter.nextEmission -= elapsedTime;
	while (m_metaballEmitter.nextEmission <= 0.f)
	{
		emitMetaball();
	}

	applyRepulsion();

	for (auto & metaball : m_metaballs)
	{
		metaball.acceleration += m_gravConstant;
		metaball.velocity += metaball.acceleration * elapsedTime;
		if (doesCollide(metaball, m_groundPlane, elapsedTime))
		{
			float t0 = collisionTime(metaball, m_groundPlane);
			metaball.position += metaball.velocity * t0;
			metaball.velocity = metaball.velocity - glm::dot(metaball.velocity, m_groundPlane.normal) * m_groundPlane.normal;
			metaball.velocity -= metaball.velocity * m_groundPlane.friction * (elapsedTime - t0);
			metaball.position += metaball.velocity * (elapsedTime - t0);
		}
		else
		{
			metaball.position += metaball.velocity * elapsedTime;
		}
		metaball.acceleration = glm::vec3(.0f);
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