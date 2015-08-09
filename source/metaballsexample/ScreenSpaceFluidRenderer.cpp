#include "ScreenSpaceFluidRenderer.h"

#include <iostream>

#include <glm/ext.hpp>

#include <gloperate-qt\QtTextureLoader.h>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/ViewportCapability.h>

#include <globjects/Buffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>

#include "MetaballsExample.h"


ScreenSpaceFluidRenderer::ScreenSpaceFluidRenderer()
	: m_blurringIterations(100)
	, m_bilateral(false)
	, m_timeStep(0.0002f)
	, m_lightDir(-1.f, -1.f, -1.f, 1.f)
	, m_blurringScale(2.f)
{
		
}

ScreenSpaceFluidRenderer::~ScreenSpaceFluidRenderer()
{

}

void ScreenSpaceFluidRenderer::initialize(MetaballsExample * painter)
{
	setupFramebuffers(painter);
	setupMetaballs(painter);
	setupScreenAlignedQuad(painter);
	setupPrograms(painter);
	setupCubemap();
	setupGround();
	setupShadowmap(painter);

	/*for (int nHalf = 0; nHalf < 30; ++nHalf){
		m_binomOffset.push_back(static_cast<int>(m_binomCoeff.size()));
		for (int k = nHalf; k >= 0; --k){
			double num = 1.0;
			for (int i = 0; i < 2 * nHalf - k; i++)
				num = 0.5*num*(2 * nHalf - i) / (i + 1);
			for (int i = 0; i < k; i++)
				num /= 2;
			m_binomCoeff.push_back(float(num));
		}
	} */
	for (int i = 29; i >= 0; --i)
	{
		double num = 1.0;
		for (int i2 = 0; i < 2 * 29 - i; ++i2)
		{
			num = 0.5 * num * (2.0 * 29 - i2) / (i2 + 1);
		}
		num *= pow(0.5, i);
		m_binomCoeff[i] = static_cast<float>(num);
	}
}

globjects::Framebuffer* ScreenSpaceFluidRenderer::draw(MetaballsExample * painter)
{
	if (painter->viewportCapability()->hasChanged())
	{
		m_metaballTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_metaballDummy->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_blurringTexture[0]->image2D(0, gl::GL_DEPTH_COMPONENT, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_blurringDummy[0]->image2D(0, gl::GL_RGBA, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_blurringTexture[1]->image2D(0, gl::GL_DEPTH_COMPONENT, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_blurringDummy[1]->image2D(0, gl::GL_RGBA, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);

		m_thicknessTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_shadowThicknessTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_shadowTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_shadowDummy->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_groundDepthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_groundColorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	}

	m_metaballs = painter->getMetaballs();
	m_vertices->setSubData(m_metaballs);

	drawThickness(painter);
	drawMetaballs(painter);
	drawShadowmap(painter);
	drawGround(painter);
	if (!m_bilateral)
		curvatureFlowBlur(painter);
	else
		bilateralBlur(painter);
	compose(painter);

	return m_composingFBO;
}

void ScreenSpaceFluidRenderer::setBlurringIterations(unsigned int value)
{
	m_blurringIterations = value;
}

unsigned int ScreenSpaceFluidRenderer::getBlurringIterations() const
{
	return m_blurringIterations;
}

void ScreenSpaceFluidRenderer::setBilateral(bool value)
{
	m_bilateral = value;
	setBlurringScale(static_cast<float>(!value) + 1.f);
}

bool ScreenSpaceFluidRenderer::getBilateral() const
{
	return m_bilateral;
}

void ScreenSpaceFluidRenderer::setCurvatureFlow(bool value)
{
	m_bilateral = !value;
}

bool ScreenSpaceFluidRenderer::getCurvatureFlow() const
{
	return !m_bilateral;
}

void ScreenSpaceFluidRenderer::setBlurringScale(float value)
{
	m_blurringScale = value;
}

float ScreenSpaceFluidRenderer::getBlurringScale() const
{
	return m_blurringScale;
}

void ScreenSpaceFluidRenderer::setTimeStep(float value)
{
	m_timeStep = value;
}

float ScreenSpaceFluidRenderer::getTimeStep() const
{
	return m_timeStep;
}

void ScreenSpaceFluidRenderer::setupFramebuffers(MetaballsExample * painter)
{
	m_metaballFBO = new globjects::Framebuffer;
	m_blurringFBO[0] = new globjects::Framebuffer;
	m_blurringFBO[1] = new globjects::Framebuffer;
	m_composingFBO = new globjects::Framebuffer;
	m_thicknessFBO = new globjects::Framebuffer;
	m_shadowFBO = new globjects::Framebuffer;
	m_shadowThicknessFBO = new globjects::Framebuffer;
	m_groundFBO = new globjects::Framebuffer;

	m_metaballTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_metaballTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_metaballFBO->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_metaballTexture, 0);

	m_metaballDummy = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_metaballDummy->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_metaballFBO->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_metaballDummy, 0);

	//blurring
	m_blurringTexture[0] = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_blurringTexture[0]->image2D(0, gl::GL_DEPTH_COMPONENT, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_blurringFBO[0]->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_blurringTexture[0], 0);

	m_blurringDummy[0] = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_blurringDummy[0]->image2D(0, gl::GL_RGBA, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_blurringFBO[0]->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_blurringDummy[0], 0);

	m_blurringTexture[1] = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_blurringTexture[1]->image2D(0, gl::GL_DEPTH_COMPONENT, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_blurringFBO[1]->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_blurringTexture[1], 0);

	m_blurringDummy[1] = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_blurringDummy[1]->image2D(0, gl::GL_RGBA, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_blurringFBO[1]->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_blurringDummy[1], 0);

	//third
	m_colorTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_composingFBO->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture, 0);

	m_depthTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_composingFBO->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture, 0);

	//thickness
	m_thicknessTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_thicknessTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_thicknessFBO->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_thicknessTexture, 0);

	//shadowmap
	m_shadowTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_shadowTexture->image2D(0, gl::GL_DEPTH_COMPONENT, 1024, 1024, 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_shadowFBO->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_shadowTexture, 0);

	m_shadowDummy = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_shadowDummy->image2D(0, gl::GL_RGBA, 1024, 1024, 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_shadowFBO->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_shadowDummy, 0);

	m_shadowThicknessTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_shadowThicknessTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_shadowThicknessFBO->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_shadowThicknessTexture, 0);

	//ground
	m_groundDepthTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_groundDepthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_groundFBO->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_groundDepthTexture, 0);

	m_groundColorTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_groundColorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_groundFBO->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_groundColorTexture, 0);
}

void ScreenSpaceFluidRenderer::setupGround()
{
	m_groundVAO = new globjects::VertexArray;
	m_groundVAO->bind();
	std::vector<glm::vec3> vertices = {
		glm::vec3(-5.f, -.0f, 5.f),
		glm::vec3(-5.f, -.0f, -5.f),
		glm::vec3(5.f, -.0f, 5.f),
		glm::vec3(5.f, -.0f, -5.f) };

	m_ground = new globjects::Buffer;
	m_ground->setData(vertices, gl::GL_STATIC_DRAW);

	globjects::ref_ptr<globjects::Buffer> coords = new globjects::Buffer;
	std::vector<glm::vec2> texCoords = {
		glm::vec2(0.f, 1.f),
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 1.f),
		glm::vec2(1.f, 0.f)
	};

	coords->setData(texCoords, gl::GL_STATIC_DRAW);

	auto binding = m_groundVAO->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_ground, 0, 3 * sizeof(float));
	binding->setFormat(3, gl::GL_FLOAT);
	m_groundVAO->enable(0);

	binding = m_groundVAO->binding(1);
	binding->setAttribute(1);
	binding->setBuffer(coords, 0, 2 * sizeof(float));
	binding->setFormat(2, gl::GL_FLOAT);
	m_groundVAO->enable(1);

	m_groundVAO->unbind();

	gloperate_qt::QtTextureLoader loader;
	m_groundTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_groundTexture = loader.load("data/metaballsexample/ground.png", nullptr);
}

void ScreenSpaceFluidRenderer::setupPrograms(MetaballsExample * painter)
{
	m_thicknessProgram = new globjects::Program;
	m_thicknessProgram->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.frag"),
		globjects::Shader::fromFile(gl::GL_GEOMETRY_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.geom")
	);

	m_groundProgram = new globjects::Program;
	m_groundProgram->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/ground.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/ground.frag"));

	m_program = new globjects::Program;
	m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/firstPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/firstPass.frag"),
		globjects::Shader::fromFile(gl::GL_GEOMETRY_SHADER, "data/metaballsexample/screen_space_fluid/firstPass.geom")
	);

	m_curvatureFlowProgram = new globjects::Program;
	m_curvatureFlowProgram->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/secPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/secPass.frag")
	);

	m_verticalBilateralProgram = new globjects::Program;
	m_verticalBilateralProgram->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/secPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/secPass2.frag")
		);

	m_horizontalBilateralProgram = new globjects::Program;
	m_horizontalBilateralProgram->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/secPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/secPass3.frag")
		);

	m_composingProgram = new globjects::Program;
	m_composingProgram->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/thirdPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/thirdPass.frag")
	);

	m_backgroundProgram = new globjects::Program;
	m_backgroundProgram->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/background.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/background.frag")
	);
}

void ScreenSpaceFluidRenderer::setupCubemap()
{
	std::vector<std::string> cubemap(6);
	cubemap[0] = "data/metaballsexample/cubemaps/cube_gen_11_px.jpg";
	cubemap[1] = "data/metaballsexample/cubemaps/cube_gen_11_nx.jpg";
	cubemap[2] = "data/metaballsexample/cubemaps/cube_gen_11_ny.jpg";
	cubemap[3] = "data/metaballsexample/cubemaps/cube_gen_11_py.jpg";
	cubemap[4] = "data/metaballsexample/cubemaps/cube_gen_11_pz.jpg";
	cubemap[5] = "data/metaballsexample/cubemaps/cube_gen_11_nz.jpg";

	m_skybox = globjects::Texture::createDefault(gl::GL_TEXTURE_CUBE_MAP);

	gloperate_qt::QtTextureLoader loader;
	globjects::ref_ptr<globjects::Texture> face = new globjects::Texture;
	for (unsigned int i = 0; i < cubemap.size(); ++i)
	{
		face = loader.load(cubemap[i], nullptr);
		m_skybox->image2D(
			gl::GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			gl::GL_RGBA,
			1024,
			1024, 
			0,
			gl::GL_RGBA,
			gl::GL_UNSIGNED_BYTE,
			face->getImage(0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE).data());
	}
}

void ScreenSpaceFluidRenderer::setupMetaballs(MetaballsExample * painter)
{
	m_metaballs = painter->getMetaballs();

	m_vertices = new globjects::Buffer;
	m_vertices->setData(m_metaballs, gl::GL_DYNAMIC_DRAW);

	m_VAO = new globjects::VertexArray;
	auto binding = m_VAO->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_vertices, 0, 4 * sizeof(float));
	binding->setFormat(4, gl::GL_FLOAT);
	m_VAO->enable(0);
}

void ScreenSpaceFluidRenderer::setupScreenAlignedQuad(MetaballsExample * painter)
{
	m_screenAlignedQuad = new globjects::Buffer;
	m_screenAlignedQuad->setData(std::vector<float>{
		-1.f,  1.f,
		-1.f, -1.f,
		 1.f,  1.f,
		 1.f, -1.f
	}, gl::GL_STATIC_DRAW);

	m_screenAlignedQuadVAO = new globjects::VertexArray;
	auto binding = m_screenAlignedQuadVAO->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_screenAlignedQuad, 0, 2 * sizeof(float));
	binding->setFormat(2, gl::GL_FLOAT);

	m_screenAlignedQuadVAO->enable(0);
}

void ScreenSpaceFluidRenderer::setupShadowmap(MetaballsExample * painter)
{
	m_camera.aspectRatio = 1.f;
	m_camera.zFar = painter->projectionCapability()->zFar();
	m_camera.zNear = painter->projectionCapability()->zNear();
	m_camera.eye = glm::vec3(10.f, 10.f, -7.f);
	m_camera.center = glm::vec3(-9.f, -2.f, 8.f);
	m_camera.fovy = glm::radians(40.f);

	m_camera.projection = glm::perspective(m_camera.fovy, m_camera.aspectRatio, m_camera.zNear, m_camera.zFar);
	m_camera.projectionInverted = glm::inverse(m_camera.projection);
	m_camera.view = glm::lookAt(m_camera.eye, m_camera.center, m_camera.up);
	m_camera.viewInverted = glm::inverse(m_camera.view);
}

void ScreenSpaceFluidRenderer::drawThickness(MetaballsExample * painter)
{
	m_VAO->bind();

	m_thicknessFBO->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);
	gl::glDisable(gl::GL_DEPTH_TEST);
	gl::glEnable(gl::GL_BLEND);
	gl::glBlendFunc(gl::GL_ONE, gl::GL_ONE);

	m_thicknessProgram->use();
	m_thicknessProgram->setUniform("view", painter->cameraCapability()->view());
	m_thicknessProgram->setUniform("projection", painter->projectionCapability()->projection());
	m_thicknessProgram->setUniform("near", painter->projectionCapability()->zNear());
	m_thicknessProgram->setUniform("far", painter->projectionCapability()->zFar());

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));

	m_VAO->unbind();
	m_thicknessProgram->release();
	m_thicknessFBO->unbind();
}

void ScreenSpaceFluidRenderer::drawMetaballs(MetaballsExample * painter)
{
	m_VAO->bind();

	m_metaballFBO->bind();
	gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LESS);

	m_program->use();
	m_program->setUniform("view", painter->cameraCapability()->view());
	m_program->setUniform("projection", painter->projectionCapability()->projection());
	m_program->setUniform("light_dir", m_lightDir);
	m_program->setUniform("near", painter->projectionCapability()->zNear());
	m_program->setUniform("far", painter->projectionCapability()->zFar());

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));

	m_VAO->unbind();
	m_program->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_metaballFBO->unbind();
}

void ScreenSpaceFluidRenderer::drawGround(MetaballsExample * painter)
{
	m_groundFBO->bind();
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

	m_screenAlignedQuadVAO->bind();
	m_backgroundProgram->use();

	m_skybox->bindActive(gl::GL_TEXTURE0);
	m_backgroundProgram->setUniform(m_backgroundProgram->getUniformLocation("skybox"), 0);

	m_backgroundProgram->setUniform("view", painter->cameraCapability()->view());
	m_backgroundProgram->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_screenAlignedQuadVAO->unbind();
	m_backgroundProgram->release();

	gl::glDepthFunc(gl::GL_ALWAYS);

	m_groundVAO->bind();

	m_groundProgram->use();

	m_shadowTexture->bindActive(gl::GL_TEXTURE0);
	m_groundProgram->setUniform(m_groundProgram->getUniformLocation("shadowTexture"), 0);

	m_shadowThicknessTexture->bindActive(gl::GL_TEXTURE1);
	m_groundProgram->setUniform(m_groundProgram->getUniformLocation("thicknessTexture"), 1);

	m_groundTexture->bindActive(gl::GL_TEXTURE2);
	m_groundProgram->setUniform(m_groundProgram->getUniformLocation("groundTexture"), 2);

	m_groundProgram->setUniform("view", painter->cameraCapability()->view());
	m_groundProgram->setUniform("projection", painter->projectionCapability()->projection());
	m_groundProgram->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_groundProgram->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_groundProgram->setUniform("viewShadow", m_camera.view);
	m_groundProgram->setUniform("projectionShadow", m_camera.projection);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_groundProgram->release();
	m_groundVAO->unbind();
	m_groundFBO->unbind();

	gl::glDisable(gl::GL_DEPTH_TEST);
}

void ScreenSpaceFluidRenderer::curvatureFlowBlur(MetaballsExample * painter)
{
	glm::vec2 fov(0.f);
	fov.y = painter->projectionCapability()->fovy();
	fov.x = fov.y * painter->projectionCapability()->aspectRatio();
	glm::vec2 viewport(static_cast<float>(painter->viewportCapability()->width()), static_cast<float>(painter->viewportCapability()->height()));
	glm::vec2 focal(0.f);
	focal.x = sinf(fov.x / 2.f) / tanf(fov.x / 2.f);
	focal.y = sinf(fov.y / 2.f) / tanf(fov.y / 2.f);
	glm::vec2 focal2(-painter->projectionCapability()->projection()[0][0], -painter->projectionCapability()->projection()[1][1]);

	gl::glViewport(0, 0, static_cast<gl::GLsizei>(painter->viewportCapability()->width() / m_blurringScale), static_cast<gl::GLsizei>(painter->viewportCapability()->height() / m_blurringScale));

	m_blurringFBO[0]->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_curvatureFlowProgram->use();
	m_screenAlignedQuadVAO->bind();

	//m_colorTexture1->bindActive(gl::GL_TEXTURE0);
	//m_curvatureFlowProgram->setUniform(m_curvatureFlowProgram->getUniformLocation("colorTexture"), 0);

	m_metaballTexture->bindActive(gl::GL_TEXTURE0);
	m_curvatureFlowProgram->setUniform(m_curvatureFlowProgram->getUniformLocation("depthTexture"), 0);

	m_skybox->bindActive(gl::GL_TEXTURE2);
	m_curvatureFlowProgram->setUniform(m_curvatureFlowProgram->getUniformLocation("skybox"), 1);

	m_curvatureFlowProgram->setUniform("maxDepth", 1.0f);
	m_curvatureFlowProgram->setUniform("light_dir", m_lightDir);
	m_curvatureFlowProgram->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_curvatureFlowProgram->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_curvatureFlowProgram->setUniform("view", painter->cameraCapability()->view());
	m_curvatureFlowProgram->setUniform("projection", painter->projectionCapability()->projection());
	m_curvatureFlowProgram->setUniform("viewport", viewport);
	m_curvatureFlowProgram->setUniform("fov", fov);
	m_curvatureFlowProgram->setUniform("focal", focal);
	m_curvatureFlowProgram->setUniform("focal2", focal2);
	m_curvatureFlowProgram->setUniform("timeStep", m_timeStep);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_blurringFBO[0]->unbind();

	unsigned int current = 1;
	unsigned int notCurrent = 0;

	for (unsigned int i = 0; i < m_blurringIterations - 1; ++i)
	{
		m_blurringFBO[current]->bind();
		gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
		gl::glEnable(gl::GL_DEPTH_TEST);
		gl::glDepthFunc(gl::GL_LEQUAL);

		m_blurringTexture[notCurrent]->bindActive(gl::GL_TEXTURE0);
		m_curvatureFlowProgram->setUniform(m_curvatureFlowProgram->getUniformLocation("depthTexture"), 0);

		gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

		notCurrent = current;
		current = current == 0 ? 1 : 0;
	}

	m_curvatureFlowProgram->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_screenAlignedQuadVAO->unbind();

	gl::glViewport(0, 0, painter->viewportCapability()->width(), painter->viewportCapability()->height());
}

void ScreenSpaceFluidRenderer::bilateralBlur(MetaballsExample * painter)
{
	m_blurringFBO[0]->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_verticalBilateralProgram->use();
	m_screenAlignedQuadVAO->bind();

	m_metaballTexture->bindActive(gl::GL_TEXTURE0);
	m_verticalBilateralProgram->setUniform(m_verticalBilateralProgram->getUniformLocation("depthTexture"), 0);
	glm::vec2 viewport(static_cast<float>(painter->viewportCapability()->width()), static_cast<float>(painter->viewportCapability()->height()));
	m_verticalBilateralProgram->setUniform("viewport", viewport);
	m_verticalBilateralProgram->setUniform("binomCoeff", m_binomCoeff);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	gl::glDisable(gl::GL_DEPTH_TEST);

	m_verticalBilateralProgram->release();
	m_blurringFBO[0]->unbind();

	m_blurringFBO[1]->bind();

	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_horizontalBilateralProgram->use();

	m_blurringTexture[0]->bindActive(gl::GL_TEXTURE0);
	m_horizontalBilateralProgram->setUniform(m_horizontalBilateralProgram->getUniformLocation("depthTexture"), 0);
	m_horizontalBilateralProgram->setUniform("viewport", viewport);
	m_horizontalBilateralProgram->setUniform("binomCoeff", m_binomCoeff);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	gl::glDisable(gl::GL_DEPTH_TEST);

	m_horizontalBilateralProgram->release();
	m_blurringFBO[0]->unbind();
	m_screenAlignedQuadVAO->unbind();
}

void ScreenSpaceFluidRenderer::compose(MetaballsExample * painter)
{
	m_composingFBO->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_composingProgram->use();
	m_screenAlignedQuadVAO->bind();

	//m_colorTexture2->bindActive(gl::GL_TEXTURE0);
	//m_composingProgram->setUniform(m_composingProgram->getUniformLocation("colorTexture"), 0);

	if (!m_bilateral)
		m_blurringTexture[!(m_blurringIterations % 2)]->bindActive(gl::GL_TEXTURE0);
	else
		m_blurringTexture[1]->bindActive(gl::GL_TEXTURE0);
	m_composingProgram->setUniform(m_composingProgram->getUniformLocation("depthTexture"), 0);

	m_thicknessTexture->bindActive(gl::GL_TEXTURE1);
	m_composingProgram->setUniform(m_composingProgram->getUniformLocation("thicknessTexture"), 1);

	m_skybox->bindActive(gl::GL_TEXTURE2);
	m_composingProgram->setUniform(m_composingProgram->getUniformLocation("skybox"), 2);

	m_shadowTexture->bindActive(gl::GL_TEXTURE3);
	m_composingProgram->setUniform(m_composingProgram->getUniformLocation("shadowTexture"), 3);

	m_groundColorTexture->bindActive(gl::GL_TEXTURE4);
	m_composingProgram->setUniform(m_composingProgram->getUniformLocation("groundTexture"), 4);

	m_groundDepthTexture->bindActive(gl::GL_TEXTURE5);
	m_composingProgram->setUniform(m_composingProgram->getUniformLocation("groundDepthTexture"), 5);

	m_shadowThicknessTexture->bindActive(gl::GL_TEXTURE6);
	m_composingProgram->setUniform(m_composingProgram->getUniformLocation("shadowThicknessTexture"), 6);

	m_composingProgram->setUniform("view", painter->cameraCapability()->view());
	m_composingProgram->setUniform("projection", painter->projectionCapability()->projection());
	m_composingProgram->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_composingProgram->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_composingProgram->setUniform("near", painter->projectionCapability()->zNear());
	m_composingProgram->setUniform("far", painter->projectionCapability()->zFar());
	m_composingProgram->setUniform("lightPos", m_camera.eye);
	m_composingProgram->setUniform("eye", painter->cameraCapability()->eye());

	m_composingProgram->setUniform("viewShadow", m_camera.view);
	m_composingProgram->setUniform("projectionShadow", m_camera.projection);

	glm::vec2 viewport(static_cast<float>(painter->viewportCapability()->width()), static_cast<float>(painter->viewportCapability()->height()));
	m_composingProgram->setUniform("viewport", viewport);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_composingProgram->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_screenAlignedQuadVAO->unbind();
	m_composingFBO->unbind();
}

void ScreenSpaceFluidRenderer::drawShadowmap(MetaballsExample * painter)
{
	m_VAO->bind();

	m_shadowFBO->bind();
	gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LESS);

	m_program->use();
	m_program->setUniform("view", m_camera.view);
	m_program->setUniform("projection", m_camera.projection);
	m_program->setUniform("light_dir", m_lightDir);
	m_program->setUniform("near", m_camera.zNear);
	m_program->setUniform("far", m_camera.zFar);

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));

	m_program->release();

	gl::glDisable(gl::GL_DEPTH_TEST);
	m_shadowFBO->unbind();

	m_shadowThicknessFBO->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);
	gl::glEnable(gl::GL_BLEND);
	gl::glBlendFunc(gl::GL_ONE, gl::GL_ONE);

	m_thicknessProgram->use();
	m_thicknessProgram->setUniform("view", m_camera.view);
	m_thicknessProgram->setUniform("projection", m_camera.projection);
	m_thicknessProgram->setUniform("near", painter->projectionCapability()->zNear());
	m_thicknessProgram->setUniform("far", painter->projectionCapability()->zFar());

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));

	m_VAO->unbind();
	m_thicknessProgram->release();
	m_shadowThicknessFBO->unbind();

	gl::glDisable(gl::GL_BLEND);
}