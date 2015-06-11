#pragma once

#include <array>
#include <chrono>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class FluidSimulator
{
public:
	FluidSimulator();
	~FluidSimulator();
	
	struct Metaball
	{
		float radius;
		glm::vec3 position;
		glm::vec3 velocity;
	};

	const std::array<glm::vec4, 20> getMetaballs() const;
	void update();

private:
	const glm::vec3 m_gravConstant;
	std::vector<Metaball> m_metaballs;

	std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration> m_lastTime;
};