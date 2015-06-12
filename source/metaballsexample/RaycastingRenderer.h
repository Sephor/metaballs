<<<<<<< HEAD
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

class RaycastingRenderer: public AbstractRenderer
{
public:
	RaycastingRenderer();
	~RaycastingRenderer();

	void initialize();
	void draw(
		MetaballsExample * painter,
		const std::vector<glm::vec4> & metaballs);

	void computePhysiks();

private:
	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::Program> m_program;
=======
#pragma once 

#include "AbstractRenderer.h"

#include <array>

#include <glm/vec4.hpp>

#include <globjects\base\ref_ptr.h>

namespace globjects
{
	class Buffer;
	class Framebuffer;
	class Program;
	class VertexArray;
	class Texture;
}

class RaycastingRenderer
{
public:
	RaycastingRenderer();
	~RaycastingRenderer();

	void initialize(MetaballsExample * painter);
	globjects::Framebuffer * draw(MetaballsExample * painter);

private:
	void setupFramebuffer(MetaballsExample * painter);

	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::Program> m_program;
	globjects::ref_ptr<globjects::Texture> m_skybox;
<<<<<<< HEAD
>>>>>>> added environment map
=======

	globjects::ref_ptr<globjects::Framebuffer> m_fbo;
	globjects::ref_ptr<globjects::Texture> m_colorTexture;
	globjects::ref_ptr<globjects::Texture> m_depthTexture;
>>>>>>> added depthbuffer for raycasting
};