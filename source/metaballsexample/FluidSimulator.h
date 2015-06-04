#pragma once

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

	std::vector<glm::vec4> metaballs();
	void update(float elapsedTime);

private:
	const glm::vec3 m_gravConstant;
	std::vector<Metaball> m_metaballs;
};