#pragma once 

#include "AbstractRenderer.h"

#include <array>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
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

	void setBlurringScale(float value);
	float getBlurringScale() const;

	void setTimeStep(float value);
	float getTimeStep() const;

	void setReload(bool value);
	bool getReload() const;

private:

	struct Camera
	{
		float zNear = 0.1f;
		float zFar = 100.f;
		float aspectRatio;
		float fovy;
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
		glm::vec3 eye;
		glm::vec3 center = glm::vec3(0.f);

		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 projectionInverted;
		glm::mat4 viewInverted;
	};

	float m_blurringScale;
	float m_timeStep;
	bool m_bilateral;
	bool m_reload;
	int m_blurFilterSize;
	unsigned int m_blurringIterations;
	const glm::vec4 m_lightDir;
	Camera m_camera;

	std::vector<float> m_binomCoeff;
	std::vector<int> m_binomOffset;

	std::vector<glm::vec4> m_metaballs;

	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::Buffer> m_verticesPlan;
	globjects::ref_ptr<globjects::Buffer> m_ground;

	globjects::ref_ptr<globjects::VertexArray> m_vao;
	globjects::ref_ptr<globjects::VertexArray> m_vaoPlan;
	globjects::ref_ptr<globjects::VertexArray> m_vaoGround;

	globjects::ref_ptr<globjects::Program> m_program;
	globjects::ref_ptr<globjects::Program> m_programGround;
	globjects::ref_ptr<globjects::Program> m_programSmoothing;
	globjects::ref_ptr<globjects::Program> m_programSmoothing2;
	globjects::ref_ptr<globjects::Program> m_programSmoothing3;
	globjects::ref_ptr<globjects::Program> m_programFinal;
	globjects::ref_ptr<globjects::Program> m_programThickness;
	globjects::ref_ptr<globjects::Program> m_programBackground;

	globjects::ref_ptr<globjects::Framebuffer> m_metaballFBO;
	std::array<globjects::ref_ptr<globjects::Framebuffer>, 2> m_blurringFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_finalFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_thicknessFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_shadowFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_shadowThicknessFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_groundFBO;

	globjects::ref_ptr<globjects::Texture> m_groundTexture;

	globjects::ref_ptr<globjects::Texture> m_metaballDummy;
	globjects::ref_ptr<globjects::Texture> m_metaballTexture;

	std::array<globjects::ref_ptr<globjects::Texture>, 2> m_blurringDummy;
	std::array<globjects::ref_ptr<globjects::Texture>, 2> m_blurringTexture;

	globjects::ref_ptr<globjects::Texture> m_colorTexture;
	globjects::ref_ptr<globjects::Texture> m_depthTexture;

	globjects::ref_ptr<globjects::Texture> m_thicknessTexture;

	globjects::ref_ptr<globjects::Texture> m_shadowThicknessTexture;

	globjects::ref_ptr<globjects::Texture> m_shadowTexture;
	globjects::ref_ptr<globjects::Texture> m_shadowDummy;

	globjects::ref_ptr<globjects::Texture> m_groundColorTexture;
	globjects::ref_ptr<globjects::Texture> m_groundDepthTexture;

	globjects::ref_ptr<globjects::Texture> m_skybox;

	void setupFramebuffers(MetaballsExample * painter);
	void setupGround();
	void setupPrograms(MetaballsExample * painter);
	void setupCubemap();
	void setupMetaballs(MetaballsExample * painter);
	void setupScreenAlignedQuad(MetaballsExample * painter);
	void setupShadowmap(MetaballsExample * painter);

	void drawMetaballs(MetaballsExample * painter);
	void drawGround(MetaballsExample * painter);
	void curvatureFlowBlur(MetaballsExample * painter);
	void bilateralBlur(MetaballsExample * painter);
	void drawThirdPass(MetaballsExample * painter);
	void drawThicknessPass(MetaballsExample * painter);
	void drawShadowmap(MetaballsExample * painter);
};