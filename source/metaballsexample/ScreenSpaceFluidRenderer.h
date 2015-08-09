#pragma once 

#include <array>
#include <vector>

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

class MetaballsExample;

class ScreenSpaceFluidRenderer
{
public:
	ScreenSpaceFluidRenderer();
	~ScreenSpaceFluidRenderer();

protected:
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

public:
	void setBlurringIterations(unsigned int value);
	unsigned int getBlurringIterations() const;

	void setBilateral(bool value);
	bool getBilateral() const;

	void setBlurringScale(float value);
	float getBlurringScale() const;

	void setTimeStep(float value);
	float getTimeStep() const;

	void initialize(MetaballsExample * painter);
	globjects::Framebuffer* draw(MetaballsExample * painter);

protected:
	const glm::vec4 m_lightDir;
	
	Camera m_camera;
	float m_blurringScale;
	float m_timeStep;
	bool m_bilateral;
	unsigned int m_blurringIterations;

	std::array<float, 30> m_binomCoeff;

	std::vector<glm::vec4> m_metaballs;

	//VBOs
	globjects::ref_ptr<globjects::Buffer> m_vertices;
	globjects::ref_ptr<globjects::Buffer> m_screenAlignedQuad;
	globjects::ref_ptr<globjects::Buffer> m_ground;

	//VAOs
	globjects::ref_ptr<globjects::VertexArray> m_VAO;
	globjects::ref_ptr<globjects::VertexArray> m_screenAlignedQuadVAO;
	globjects::ref_ptr<globjects::VertexArray> m_groundVAO;

	//Shader programs
	globjects::ref_ptr<globjects::Program> m_program;
	globjects::ref_ptr<globjects::Program> m_groundProgram;
	globjects::ref_ptr<globjects::Program> m_curvatureFlowProgram;
	globjects::ref_ptr<globjects::Program> m_verticalBilateralProgram;
	globjects::ref_ptr<globjects::Program> m_horizontalBilateralProgram;
	globjects::ref_ptr<globjects::Program> m_composingProgram;
	globjects::ref_ptr<globjects::Program> m_thicknessProgram;
	globjects::ref_ptr<globjects::Program> m_backgroundProgram;

	//FBOs
	globjects::ref_ptr<globjects::Framebuffer> m_metaballFBO;
	std::array<globjects::ref_ptr<globjects::Framebuffer>, 2> m_blurringFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_composingFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_thicknessFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_shadowFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_shadowThicknessFBO;
	globjects::ref_ptr<globjects::Framebuffer> m_groundFBO;

	//textures
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

	//setup
	void setupFramebuffers(MetaballsExample * painter);
	void setupGround();
	void setupPrograms(MetaballsExample * painter);
	void setupCubemap();
	void setupMetaballs(MetaballsExample * painter);
	void setupScreenAlignedQuad(MetaballsExample * painter);
	void setupShadowmap(MetaballsExample * painter);


	//drawing
	void drawThickness(MetaballsExample * painter);
	void drawMetaballs(MetaballsExample * painter);
	void drawShadowmap(MetaballsExample * painter);
	void drawGround(MetaballsExample * painter);
	void curvatureFlowBlur(MetaballsExample * painter);
	void bilateralBlur(MetaballsExample * painter);
	void compose(MetaballsExample * painter);
};