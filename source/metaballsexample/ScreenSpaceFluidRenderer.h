#pragma once 

#include "AbstractRenderer.h"

#include <array>

#include <glm/vec4.hpp>

#include <globjects\base\ref_ptr.h>

namespace globjects
{
	class Program;
	class VertexArray;
	class Buffer;
}

class ScreenSpaceFluidRenderer: public AbstractRenderer
{
public:
	ScreenSpaceFluidRenderer();
	~ScreenSpaceFluidRenderer();

	void initialize();
	void draw(MetaballsExample * painter);

	void computePhysiks();

private:
	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::Program> m_program;

	
};