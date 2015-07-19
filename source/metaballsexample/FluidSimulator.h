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

	struct Plane
	{
		glm::vec3 normal;
		float friction;
		float distance;
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

	const std::vector<glm::vec4>& getMetaballs();
	void update();
	void startSimulation();
	void stopSimulation();
	bool doesCollide(const Metaball & metaball, const Plane & plane, float deltaTime);
	float collisionTime(const Metaball & metaball, const Plane & plane);
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

	void updateRepulsion();
	glm::vec3 computeInteractions(Metaball& metaball, std::vector<Metaball*>& neighbours);
	void updatePositions(float elapsedTime);
	void emitMetaball();

private:
	const glm::vec3 m_gravConstant;
	const float m_twoPi;
	const size_t m_metaballCount;
	float m_repulsionLimitFactor;
	float m_repulsionFactor;
	float m_attractionFactor;
	std::vector<Metaball> m_metaballs;
	std::vector<glm::vec4> m_metaballsAsVec;
	bool m_metaballsChanged;
	Plane m_groundPlane;
	Emitter m_metaballEmitter;
	bool m_isRunning;
	int m_metaballSelector;
	Grid m_grid;

	std::random_device m_rd;
	std::mt19937 m_gen;
	std::uniform_real_distribution<float> m_dis;

	std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration> m_lastTime;
};