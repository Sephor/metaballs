#pragma once

#include <array>
#include <chrono>
#include <vector>
#include <random>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Grid.h"

class FluidSimulator
{
public:
	FluidSimulator();
	~FluidSimulator();

protected:
	struct Plane
	{
		glm::vec3 normal;
		float friction;
		float distance;
		float restitution;
	};

	struct Emitter
	{
		glm::vec3 position;
		glm::vec3 startVelocity;
		float metaballRadius;
		float period;
		float nextEmission;
		float spread;
		float spray;
	};

public:
	bool getIsRunning() const;
	void setIsRunning(bool value);
	float getAttractionFactor() const;
	void setAttractionFactor(float value);
	float getRepulsionFactor() const;
	void setRepulsionFactor(float value);
	float getMetaballRadius() const;
	void setMetaballRadius(float value);
	float getSpread() const;
	void setSpread(float value);
	float getSpray() const;
	void setSpray(float value);
	float getEmitterPeriod() const;
	void setEmitterPeriod(float value);

	void startSimulation();
	void stopSimulation();
	void update();

	const std::vector<glm::vec4>& getMetaballs();

protected:
	//Member
	const glm::vec3 m_gravConstant;
	const float m_twoPi;
	const size_t m_metaballCount;

	float m_repulsionLimitFactor;
	float m_repulsionFactor;
	float m_attractionFactor;
	std::vector<Metaball> m_metaballs;
	std::vector<glm::vec4> m_metaballsAsVec;
	bool m_metaballsChanged;
	Emitter m_metaballEmitter;
	bool m_isRunning;
	int m_metaballSelector;
	Grid m_grid;
	std::vector<Plane> m_planes;

	//random generator
	std::random_device m_rd;
	std::mt19937 m_gen;
	std::uniform_real_distribution<float> m_dis;

	std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration> m_lastTime;

	//Functions
	void initializePlanes();

	void emitMetaball();
	void updateRepulsion();
	void updatePositions(float elapsedTime);
	void updatePlaneCollision();
	glm::vec3 computeInteractions(Metaball& metaball, std::vector<Metaball*>& neighbours);

	float distanceToPlane(Metaball& metaball, Plane& plane);
};