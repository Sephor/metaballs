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
	class Framebuffer;
	class Texture;
}

class ScreenSpaceFluidRenderer
{
public:
	ScreenSpaceFluidRenderer();
	~ScreenSpaceFluidRenderer();

	void initialize(MetaballsExample * painter);
	globjects::Framebuffer* draw(MetaballsExample * painter);

private:
	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::Buffer> m_verticesPlan;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::VertexArray> m_vaoPlan;
	globjects::ref_ptr<globjects::Program> m_program;
	globjects::ref_ptr<globjects::Program> m_programSmoothing;
	globjects::ref_ptr<globjects::Framebuffer> m_fbo;	

	globjects::ref_ptr<globjects::Texture> m_colorTexture;
	globjects::ref_ptr<globjects::Texture> m_normalTexture;
	globjects::ref_ptr<globjects::Texture> m_depthTexture;
};