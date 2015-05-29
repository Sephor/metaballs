#pragma once

#include <globjects/base/ref_ptr.h>

namespace globjects
{
	class VertexArray;
	class Program;
}

class AbstractRenderer
{
public: 
	AbstractRenderer(){};
	~AbstractRenderer(){};

	virtual void initialize() = 0;
	virtual void draw(globjects::ref_ptr<globjects::VertexArray> & vao) = 0;

protected:
	globjects::ref_ptr<globjects::Program> m_program;
};