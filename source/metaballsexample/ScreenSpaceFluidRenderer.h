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

	void setBlurringIterations(unsigned int value);
	unsigned int getBlurringIterations() const;

	void setBilateral(bool value);
	bool getBilateral() const;

private:
	bool m_bilateral;
	int m_blurFilterSize;
	unsigned int m_blurringIterations;
	const float m_sphereRadius;
	const glm::vec4 m_lightDir;

	std::vector<float> m_binomCoeff;
	std::vector<int> m_binomOffset;

	std::vector<glm::vec4> m_metaballs;

	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::Buffer> m_verticesPlan;
	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::VertexArray> m_vaoPlan;
	globjects::ref_ptr<globjects::Program> m_program;
	globjects::ref_ptr<globjects::Program> m_programSmoothing;
	globjects::ref_ptr<globjects::Program> m_programSmoothing2;
	globjects::ref_ptr<globjects::Program> m_programSmoothing3;
	globjects::ref_ptr<globjects::Program> m_programFinal;
	globjects::ref_ptr<globjects::Program> m_programThickness;
	globjects::ref_ptr<globjects::Framebuffer> m_metaballFBO;
	std::array<globjects::ref_ptr<globjects::Framebuffer>, 2> m_blurringFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_finalFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_thicknessFBO;

	//globjects::ref_ptr<globjects::Texture> m_colorTexture1;
	globjects::ref_ptr<globjects::Texture> m_metaballTexture;

	//globjects::ref_ptr<globjects::Texture> m_colorTexture2;
	std::array<globjects::ref_ptr<globjects::Texture>, 2> m_blurringTexture;

	globjects::ref_ptr<globjects::Texture> m_colorTexture;
	globjects::ref_ptr<globjects::Texture> m_depthTexture;

	globjects::ref_ptr<globjects::Texture> m_thicknessTexture;

	globjects::ref_ptr<globjects::Texture> m_skybox;

	void setupFramebuffers(MetaballsExample * painter);
	void setupPrograms(MetaballsExample * painter);
	void setupCubemap();
	void setupMetaballs(MetaballsExample * painter);
	void setupScreenAlignedQuad(MetaballsExample * painter);

	void drawMetaballs(MetaballsExample * painter);
	void curvatureFlowBlur(MetaballsExample * painter);
	void bilateralBlur(MetaballsExample * painter);
	void drawThirdPass(MetaballsExample * painter);
	void drawThicknessPass(MetaballsExample * painter);
};