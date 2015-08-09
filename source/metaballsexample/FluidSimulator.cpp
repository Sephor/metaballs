#include "FluidSimulator.h"

#include <iostream>
#include <math.h>
#include <glm\geometric.hpp>
#include <glm\gtx\rotate_vector.hpp>

FluidSimulator::FluidSimulator()
	: m_gravConstant(0.f, -2.f, 0.f)
	, m_isRunning(false)
	, m_metaballSelector(0)
	, m_gen(m_rd())
	, m_dis(0, 1)
	, m_twoPi(6.283185f)
	, m_repulsionLimitFactor(.9f)
	, m_repulsionFactor(5.f)
	, m_attractionFactor(1.f)
	, m_grid(Grid(40, 0.15f, glm::vec3(-3.f, 0.f, -3.f)))
	, m_metaballCount(10000)
{
	//create the collision planes
	m_planes = std::vector<Plane>{};
	initializePlanes();

	//setup the emitter
	m_metaballEmitter.period = .01f;
	m_metaballEmitter.position = glm::vec3(0.f, 5.f, 0.f);
	m_metaballEmitter.startVelocity = glm::vec3(0.f, 0.f, 0.f);
	m_metaballEmitter.nextEmission = 0;
	m_metaballEmitter.metaballRadius = .1f;
	m_metaballEmitter.spread = .5f;
	m_metaballEmitter.spray = .1f;

	//create the metaballs
	m_metaballs = std::vector<Metaball>();
	m_metaballs.reserve(m_metaballCount);
	for (int i = 0; i < m_metaballCount; i++)
	{
		Metaball m;
		m.jump = glm::vec3(0.f);
		m.collisionVelocity = glm::vec3(0.f);
		m.position = glm::vec3(i * 0.1f, 0.f, 5000.f);
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

void FluidSimulator::initializePlanes()
{
	//ground plane
	Plane plane;
	plane.friction = 0.02f;
	plane.restitution = .25f;
	plane.normal = glm::vec3(.0f, 1.f, .0f);
	plane.distance = -m_grid.bottom();
	m_planes.push_back(plane);
	
	//side planes
	plane.friction = 0.f;
	plane.restitution = 1.f;
	plane.normal = glm::vec3(1.f, .0f, .0f);
	plane.distance = m_grid.left();
	m_planes.push_back(plane);

	plane.normal = glm::vec3(-1.f, .0f, .0f);
	plane.distance = -m_grid.right();
	m_planes.push_back(plane);
	
	plane.normal = glm::vec3(.0f, .0f, -1.f);
	plane.distance = m_grid.front();
	m_planes.push_back(plane);

	plane.normal = glm::vec3(.0f, .0f, 1.f);
	plane.distance = -m_grid.back();
	m_planes.push_back(plane);
}

const std::vector<glm::vec4>& FluidSimulator::getMetaballs()
{
	//The metaballs are expected as glm::vec4 with position (xyz) and radius (w)
	//If the metaballs didn't change (simulation off) we don't need to do anything but return the old vector.
	//Otherwise we need to convert the metaball structs to glm::vec4
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

void FluidSimulator::update()
{
	//don't do anything if the simulation is stopped
	if (!m_isRunning) return;

	//mark the metaballs as changed
	m_metaballsChanged = true;

	//get the elapsed time since the last update, but clamp it to .05 seconds 
	//(at least 20 physic updates per simulated second)
	float elapsedTime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::high_resolution_clock::now() - m_lastTime).count();
	m_lastTime = std::chrono::high_resolution_clock::now();
	elapsedTime = (elapsedTime > 0.05f) ? 0.05f : elapsedTime;

	//emit metaballs
	m_metaballEmitter.nextEmission -= elapsedTime;
	while (m_metaballEmitter.nextEmission <= 0.f)
	{
		emitMetaball();
	}

	//calculate effects on metaballs
	updateRepulsion();
	updatePositions(elapsedTime);
	updatePlaneCollision();
}

void FluidSimulator::updatePositions(float elapsedTime)
{
	for (auto & metaball : m_metaballs)
	{
		//not emitted yet
		if (metaball.radius == 0.f)
		{
			continue;
		}

		//gravity
		metaball.acceleration += m_gravConstant;

		//average velocity after all collisions
		metaball.collisionVelocity /= std::max(metaball.collisionCount, 1);
		if (metaball.collisionCount > 0)
		{
			metaball.velocity = metaball.collisionVelocity;
		}
		
		metaball.velocity += metaball.acceleration * elapsedTime;
		glm::vec3 newPosition = metaball.position + metaball.velocity * elapsedTime + metaball.jump;
		
		//update the position through the grid
		m_grid.updateMetaball(metaball, newPosition);

		//reset per tick values
		metaball.acceleration = glm::vec3(.0f);
		metaball.jump = glm::vec3(0.f);
		metaball.collisionVelocity = glm::vec3(0.f);
	}
}

void FluidSimulator::updateRepulsion()
{
	for (auto& metaball : m_metaballs)
	{
		if (metaball.radius == 0.f)
		{
			continue;
		}

		//compute interactions between a metaball and all metaball in neighboring grid cells
		metaball.collisionCount = 0;
		glm::ivec3 coords(m_grid.cellCoords(metaball));
		for (int xx = -1; xx <= 1; xx++)
		{
			for (int yy = -1; yy <= 1; yy++)
			{
				for (int zz = -1; zz <= 1; zz++)
				{
					glm::ivec3 neighborCoords = coords + glm::ivec3(xx, yy, zz);
					if (!m_grid.isInGrid(neighborCoords)) continue;

					metaball.acceleration += computeInteractions(metaball, m_grid.getMetaballs(neighborCoords));
				}
			}
		}
	}
}

void FluidSimulator::updatePlaneCollision()
{

	for (auto& metaball : m_metaballs)
	{
		if (metaball.radius == 0.f)
		{
			continue;
		}

		for (auto& plane : m_planes)
		{
			float distance = distanceToPlane(metaball, plane);
			
			//if the distance is less than 0 the metaball is behind the plane
			if (distance < 0)
			{
				//place the metaball back on the plane
				distance = -distance;
				metaball.position += distance * plane.normal;
				
				//reflect velocity and apply friction and restitution
				metaball.velocity = glm::reflect(metaball.velocity, plane.normal);
				float projection = glm::dot(metaball.velocity, plane.normal);
				glm::vec3 orthogonal = plane.normal * projection;
				glm::vec3 parallel = metaball.velocity - orthogonal;
				metaball.velocity -= orthogonal * (1 - plane.restitution);
				metaball.velocity -= parallel * plane.friction;
			}
		}
	}
}

glm::vec3 FluidSimulator::computeInteractions(Metaball& metaball, std::vector<Metaball*>& neighbours)
{
	glm::vec3 forceSum{0.f};

	for (auto& other_metaball : neighbours)
	{
		if (other_metaball->radius == 0.f || other_metaball == &metaball)
		{
			continue;
		}

		float radiusSum = other_metaball->radius + metaball.radius;
		float repulsionLimit = radiusSum * m_repulsionLimitFactor;
		glm::vec3 difference = (other_metaball->position + other_metaball->jump) - (metaball.position + metaball.jump);
		
		float distance = glm::dot(difference, difference);

		//skip the interactions if metaballs are to far away from eachother
		if (distance > radiusSum * radiusSum) continue;

		distance = sqrt(distance);
		float sqrtRadiusSum = sqrt(radiusSum);
		float forceMagnitude;

		//attraction
		if (distance > repulsionLimit)
		{
			float halfAttInterval = (radiusSum - radiusSum * m_repulsionLimitFactor) * .5f;
			float term = (distance - radiusSum + halfAttInterval) / halfAttInterval * sqrtRadiusSum;
			forceMagnitude = m_attractionFactor * (term * term - radiusSum);
		}
		//repulsion
		else
		{
			float collisionLimit = repulsionLimit * 0.8f;
			//collision
			if (distance < collisionLimit)
			{
				//the distance both metaballs need to move for them to not collide
				float offset = (collisionLimit - distance) / 2.0f;

				if (distance != 0.f)
				{
					//calculate the collision
					metaball.jump += -glm::normalize(difference) * offset;
					metaball.collisionCount++;
					glm::vec3 v1 = metaball.velocity;
					glm::vec3 v2 = other_metaball->velocity;
					glm::vec3 resultingVelocity = (v1 + v2 -(v1 - v2) * 1.f) * 0.5f;
					metaball.collisionVelocity += resultingVelocity;
				}
				//the distance has now been changed so set it to the correct value
				distance = collisionLimit;
			}

			//these parameters were chosen through experimentation
			float temp = distance / (radiusSum * repulsionLimit);
			temp = powf(temp, 4.f) * 7.f;
			forceMagnitude = m_repulsionFactor * 1 / temp;
		}
		//add the force
		forceSum -= forceMagnitude / distance * difference;
	}
	
	return forceSum;
}

void FluidSimulator::emitMetaball()
{
	//generate two vectors with random lenth
	glm::vec3 posOffset(m_dis(m_gen) * m_metaballEmitter.spread, .0f, .0f);
	glm::vec3 velOffset(m_dis(m_gen) * m_metaballEmitter.spray, .0f, .0f);
	glm::vec3 normal(0.f, 0.f, 1.f);

	//randomly rotate the position offset
	auto randomAngle = m_dis(m_gen) * 2.f * m_twoPi;
	posOffset = glm::rotateY(posOffset, randomAngle);
	normal = glm::rotateY(normal, randomAngle);
	posOffset = glm::rotate(posOffset, m_dis(m_gen) * m_twoPi, normal);
	normal = glm::vec3(0.f, 0.f, 1.f);

	//randomly rotate the velocity offset
	randomAngle = m_dis(m_gen) * 2.f * m_twoPi;
	velOffset = glm::rotateY(velOffset, randomAngle);
	normal = glm::rotateY(normal, randomAngle);
	velOffset = glm::rotate(velOffset, m_dis(m_gen) * m_twoPi, normal);

	//emit a metaball
	m_metaballEmitter.nextEmission += m_metaballEmitter.period;
	m_metaballs[m_metaballSelector].velocity = m_metaballEmitter.startVelocity + velOffset;
	m_metaballs[m_metaballSelector].radius = m_metaballEmitter.metaballRadius;
	
	//add the metaball to the grid
	m_grid.updateMetaball(m_metaballs[m_metaballSelector], m_metaballEmitter.position + posOffset);

	//increase the counter to the next metaball
	m_metaballSelector = (m_metaballSelector + 1) % m_metaballs.size();
}

float FluidSimulator::distanceToPlane(Metaball& metaball, Plane& plane)
{
	return glm::dot(metaball.position, plane.normal) - plane.distance;
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