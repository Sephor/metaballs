#pragma once 

#include "AbstractRenderer.h"

#include <globjects\base\ref_ptr.h>

namespace globjects
{
	class Program;
	class VertexArray;
	class Buffer;
}

class MetaballsExample;

class OtherRenderer: public AbstractRenderer
{
public:
	OtherRenderer();
	~OtherRenderer();

	void initialize();
	void draw(MetaballsExample * painter);

private:
	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::Program> m_program;
};