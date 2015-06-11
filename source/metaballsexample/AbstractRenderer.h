#pragma once

#include <vector>

#include <glm/vec4.hpp>

class MetaballsExample;

class AbstractRenderer
{
public: 
	AbstractRenderer(){};
	~AbstractRenderer(){};

	virtual void initialize() = 0;
	virtual void draw(MetaballsExample * painte) = 0;
};