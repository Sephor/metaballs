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

	void setBlurFilterSize(int);
	int getBlurFilterSize() const;

private:
	int m_blurFilterSize;
	const float m_sphereRadius;
	const glm::vec4 m_lightDir;

	std::array<glm::vec4, 400> m_metaballs;

	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::Buffer> m_verticesPlan;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::VertexArray> m_vaoPlan;
	globjects::ref_ptr<globjects::Program> m_program;
	globjects::ref_ptr<globjects::Program> m_programSmoothing;
	globjects::ref_ptr<globjects::Program> m_programFinal;
	globjects::ref_ptr<globjects::Program> m_programThickness;
	globjects::ref_ptr<globjects::Framebuffer> m_fbo1;
	globjects::ref_ptr<globjects::Framebuffer> m_fbo2;
	globjects::ref_ptr<globjects::Framebuffer> m_fbo3;
	globjects::ref_ptr<globjects::Framebuffer> m_fbo4;

	globjects::ref_ptr<globjects::Texture> m_colorTexture1;
	globjects::ref_ptr<globjects::Texture> m_depthTexture1;

	globjects::ref_ptr<globjects::Texture> m_colorTexture2;
	globjects::ref_ptr<globjects::Texture> m_depthTexture2;

	globjects::ref_ptr<globjects::Texture> m_colorTexture3;
	globjects::ref_ptr<globjects::Texture> m_depthTexture3;

	globjects::ref_ptr<globjects::Texture> m_colorTexture4;

	globjects::ref_ptr<globjects::Texture> m_skybox;

	void setupFramebuffers(MetaballsExample * painter);
	void setupPrograms(MetaballsExample * painter);
	void setupCubemap();
	void setupMetaballs(MetaballsExample * painter);
	void setupScreenAlignedQuad(MetaballsExample * painter);

	void drawFirstPass(MetaballsExample * painter);
	void drawSecondPass(MetaballsExample * painter);
	void drawThirdPass(MetaballsExample * painter);
	void drawThicknessPass(MetaballsExample * painter);
};