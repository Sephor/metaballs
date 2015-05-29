#pragma once 

#include <globjects/Program.h>
#include <globjects\VertexArray.h>

class AbstractRenderer {

public: 
	AbstractRenderer(){};
	~AbstractRenderer(){};

	virtual void initialize() = 0;
	virtual void draw(globjects::ref_ptr<globjects::VertexArray>& vao) = 0;

protected:
	globjects::ref_ptr<globjects::Program> m_program;
};