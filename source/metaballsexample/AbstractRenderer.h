#pragma once

class MetaballsExample;

class AbstractRenderer
{
public: 
	AbstractRenderer(){};
	~AbstractRenderer(){};

	virtual void initialize() = 0;
	virtual void draw(MetaballsExample * painter) = 0;
};