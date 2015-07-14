﻿#include "ScreenSpaceFluidRenderer.h"

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
	: m_blurFilterSize{ 0 }
	, m_blurringIterations(10)
	, m_bilateral(false)
	, m_sphereRadius(1.f)
	, m_lightDir(-1.f, -1.f, -1.f, 1.f)
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

	for (int nHalf = 0; nHalf < 30; ++nHalf){
		m_binomOffset.push_back(static_cast<int>(m_binomCoeff.size()));
		for (int k = nHalf; k >= 0; --k){
			double num = 1.0;
			for (int i = 0; i < 2 * nHalf - k; i++)
				num = 0.5*num*(2 * nHalf - i) / (i + 1);
			for (int i = 0; i < k; i++)
				num /= 2;
			m_binomCoeff.push_back(float(num));
		}
	}

	gl::glClearColor(0.f, 0.f, 0.f, 1.f);
	gl::glClearDepth(1.f);
}

globjects::Framebuffer* ScreenSpaceFluidRenderer::draw(MetaballsExample * painter)
{
	if (painter->viewportCapability()->hasChanged())
	{
		m_metaballTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_metaballDummy->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_blurringTexture[0]->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_blurringDummy[0]->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_blurringTexture[1]->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_blurringDummy[1]->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);

		m_thicknessTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_shadowThicknessTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_shadowTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_shadowDummy->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_groundDepthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
		m_groundColorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	}

	if (painter->projectionCapability()->hasChanged())
	{
		setupShadowmap(painter);
	}

	m_metaballs = painter->getMetaballs();
	m_vertices->setSubData(m_metaballs);

	drawThicknessPass(painter);
	drawMetaballs(painter);
	drawShadowmap(painter);
	drawGround(painter);
	if (!m_bilateral)
		curvatureFlowBlur(painter);
	else
		bilateralBlur(painter);
	drawThirdPass(painter);

	return m_finalFBO;
}

void ScreenSpaceFluidRenderer::setBlurFilterSize(int size)
{
	m_blurFilterSize = size;
}

int ScreenSpaceFluidRenderer::getBlurFilterSize() const 
{
	return m_blurFilterSize;
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
}

bool ScreenSpaceFluidRenderer::getBilateral() const
{
	return m_bilateral;
}

void ScreenSpaceFluidRenderer::setupFramebuffers(MetaballsExample * painter)
{
	m_metaballFBO = new globjects::Framebuffer;
	m_blurringFBO[0] = new globjects::Framebuffer;
	m_blurringFBO[1] = new globjects::Framebuffer;
	m_finalFBO = new globjects::Framebuffer;
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
	m_blurringTexture[0]->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_blurringFBO[0]->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_blurringTexture[0], 0);

	m_blurringDummy[0] = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_blurringDummy[0]->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_blurringFBO[0]->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_blurringDummy[0], 0);

	m_blurringTexture[1] = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_blurringTexture[1]->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_blurringFBO[1]->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_blurringTexture[1], 0);

	m_blurringDummy[1] = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_blurringDummy[1]->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_blurringFBO[1]->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_blurringDummy[1], 0);

	//third
	m_colorTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_finalFBO->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture, 0);

	m_depthTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_finalFBO->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture, 0);

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
	std::vector<glm::vec3> vertices = {
		glm::vec3(-5.f, -.1f, 5.f),
		glm::vec3(-5.f, -.1f, -5.f),
		glm::vec3(5.f, -.1f, 5.f),
		glm::vec3(5.f, -.1f, -5.f) };

	m_ground = new globjects::Buffer;
	m_ground->setData(vertices, gl::GL_STATIC_DRAW);

	m_vaoGround = new globjects::VertexArray;
	auto binding = m_vaoGround->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_ground, 0, 3 * sizeof(float));
	binding->setFormat(3, gl::GL_FLOAT);
	m_vaoGround->enable(0);
}

void ScreenSpaceFluidRenderer::setupPrograms(MetaballsExample * painter)
{
	m_programThickness = new globjects::Program;
	m_programThickness->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.frag"),
		globjects::Shader::fromFile(gl::GL_GEOMETRY_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.geom")
	);

	m_programGround = new globjects::Program;
	m_programGround->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/ground.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/ground.frag"));

	m_program = new globjects::Program;
	m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/firstPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/firstPass.frag"),
		globjects::Shader::fromFile(gl::GL_GEOMETRY_SHADER, "data/metaballsexample/screen_space_fluid/firstPass.geom")
	);

	m_programSmoothing = new globjects::Program;
	m_programSmoothing->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/secPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/secPass.frag")
	);

	m_programSmoothing2 = new globjects::Program;
	m_programSmoothing2->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/secPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/secPass2.frag")
		);

	m_programSmoothing3 = new globjects::Program;
	m_programSmoothing3->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/secPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/secPass3.frag")
		);

	m_programFinal = new globjects::Program;
	m_programFinal->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/thirdPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/thirdPass.frag")
	);
}

void ScreenSpaceFluidRenderer::setupCubemap()
{
	std::vector<std::string> cubemap(6);
	cubemap[0] = "data/metaballsexample/raycasting/env_cube_px.png";
	cubemap[1] = "data/metaballsexample/raycasting/env_cube_nx.png";
	cubemap[2] = "data/metaballsexample/raycasting/env_cube_ny.png";
	cubemap[3] = "data/metaballsexample/raycasting/env_cube_py.png";
	cubemap[4] = "data/metaballsexample/raycasting/env_cube_pz.png";
	cubemap[5] = "data/metaballsexample/raycasting/env_cube_nz.png";

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
			1024, //face->getParameter(gl::GL_TEXTURE_WIDTH), 
			1024, //face->getParameter(gl::GL_TEXTURE_HEIGHT), 
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

	m_vao = new globjects::VertexArray;
	auto binding = m_vao->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_vertices, 0, 4 * sizeof(float));
	binding->setFormat(4, gl::GL_FLOAT);
	m_vao->enable(0);
}

void ScreenSpaceFluidRenderer::setupScreenAlignedQuad(MetaballsExample * painter)
{
	m_verticesPlan = new globjects::Buffer;
	m_verticesPlan->setData(std::vector<float>{
		-1.f, 1.f,
			-1.f, -1.f,
			1.f, 1.f,
			1.f, -1.f
	}, gl::GL_STATIC_DRAW);

	m_vaoPlan = new globjects::VertexArray;
	auto binding = m_vaoPlan->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_verticesPlan, 0, 2 * sizeof(float));
	binding->setFormat(2, gl::GL_FLOAT);

	m_vaoPlan->enable(0);
}

void ScreenSpaceFluidRenderer::setupShadowmap(MetaballsExample * painter)
{
	m_camera.aspectRatio = 1.f;
	m_camera.zFar = painter->projectionCapability()->zFar();
	m_camera.zNear = painter->projectionCapability()->zNear();
	m_camera.eye = glm::vec3(10.f, 6.f, -7.f);
	m_camera.center = glm::vec3(-9.f, -2.f, 8.f);
	m_camera.fovy = glm::radians(40.f);

	m_camera.projection = glm::perspective(m_camera.fovy, m_camera.aspectRatio, m_camera.zNear, m_camera.zFar);
	m_camera.projectionInverted = glm::inverse(m_camera.projection);
	m_camera.view = glm::lookAt(m_camera.eye, m_camera.center, m_camera.up);
	m_camera.viewInverted = glm::inverse(m_camera.view);
}

void ScreenSpaceFluidRenderer::drawThicknessPass(MetaballsExample * painter)
{
	m_vao->bind();

	m_thicknessFBO->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);
	gl::glDisable(gl::GL_DEPTH_TEST);
	gl::glEnable(gl::GL_BLEND);
	gl::glBlendFunc(gl::GL_ONE, gl::GL_ONE);

	m_programThickness->use();
	m_programThickness->setUniform("view", painter->cameraCapability()->view());
	m_programThickness->setUniform("projection", painter->projectionCapability()->projection());
	m_programThickness->setUniform("sphere_radius", m_sphereRadius);
	m_programThickness->setUniform("near", painter->projectionCapability()->zNear());
	m_programThickness->setUniform("far", painter->projectionCapability()->zFar());

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));

	m_vao->unbind();
	m_programThickness->release();
	m_thicknessFBO->unbind();
}

void ScreenSpaceFluidRenderer::drawMetaballs(MetaballsExample * painter)
{
	m_vao->bind();

	m_metaballFBO->bind();
	gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LESS);

	m_program->use();
	m_program->setUniform("view", painter->cameraCapability()->view());
	m_program->setUniform("projection", painter->projectionCapability()->projection());
	m_program->setUniform("sphere_radius", m_sphereRadius);
	m_program->setUniform("light_dir", m_lightDir);
	m_program->setUniform("near", painter->projectionCapability()->zNear());
	m_program->setUniform("far", painter->projectionCapability()->zFar());

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));

	m_vao->unbind();
	m_program->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_metaballFBO->unbind();
}

void ScreenSpaceFluidRenderer::drawGround(MetaballsExample * painter)
{
	m_groundFBO->bind();
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LESS);
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

	m_vaoGround->bind();

	m_programGround->use();

	m_shadowTexture->bindActive(gl::GL_TEXTURE0);
	m_programGround->setUniform(m_programGround->getUniformLocation("shadowTexture"), 0);

	m_shadowThicknessTexture->bindActive(gl::GL_TEXTURE1);
	m_programGround->setUniform(m_programGround->getUniformLocation("thicknessTexture"), 1);

	m_programGround->setUniform("view", painter->cameraCapability()->view());
	m_programGround->setUniform("projection", painter->projectionCapability()->projection());
	m_programGround->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_programGround->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_programGround->setUniform("viewShadow", m_camera.view);
	m_programGround->setUniform("projectionShadow", m_camera.projection);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	gl::glDisable(gl::GL_DEPTH_TEST);
	m_vaoGround->unbind();
	m_programGround->release();
	m_groundFBO->unbind();
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



	m_blurringFBO[0]->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_programSmoothing->use();
	m_vaoPlan->bind();

	//m_colorTexture1->bindActive(gl::GL_TEXTURE0);
	//m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("colorTexture"), 0);

	m_metaballTexture->bindActive(gl::GL_TEXTURE0);
	m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("depthTexture"), 0);

	m_skybox->bindActive(gl::GL_TEXTURE2);
	m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("skybox"), 1);

	m_programSmoothing->setUniform("maxDepth", 1.0f);
	m_programSmoothing->setUniform("light_dir", m_lightDir);
	m_programSmoothing->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_programSmoothing->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_programSmoothing->setUniform("blur", painter->getBlur());
	m_programSmoothing->setUniform("view", painter->cameraCapability()->view());
	m_programSmoothing->setUniform("projection", painter->projectionCapability()->projection());
	m_programSmoothing->setUniform("viewport", viewport);
	m_programSmoothing->setUniform("fov", fov);
	m_programSmoothing->setUniform("focal", focal);
	m_programSmoothing->setUniform("focal2", focal2);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_blurringFBO[0]->unbind();
	m_metaballTexture->unbind();

	unsigned int current = 1;
	unsigned int notCurrent = 0;

	for (unsigned int i = 0; i < m_blurringIterations - 1; ++i)
	{
		m_blurringFBO[current]->bind();
		gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
		gl::glEnable(gl::GL_DEPTH_TEST);
		gl::glDepthFunc(gl::GL_LEQUAL);

		m_blurringTexture[notCurrent]->bindActive(gl::GL_TEXTURE0);
		m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("depthTexture"), 0);

		gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

		m_blurringTexture[notCurrent]->unbind();
		notCurrent = current;
		current = current == 0 ? 1 : 0;
	}

	//m_colorTexture1->unbind();
	m_skybox->unbind();
	m_programSmoothing->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_vaoPlan->unbind();
}

void ScreenSpaceFluidRenderer::bilateralBlur(MetaballsExample * painter)
{
	m_blurringFBO[0]->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_programSmoothing2->use();
	m_vaoPlan->bind();

	m_metaballTexture->bindActive(gl::GL_TEXTURE0);
	m_programSmoothing2->setUniform(m_programSmoothing2->getUniformLocation("depthTexture"), 0);
	glm::vec2 viewport(static_cast<float>(painter->viewportCapability()->width()), static_cast<float>(painter->viewportCapability()->height()));
	m_programSmoothing2->setUniform("viewport", viewport);
	m_programSmoothing2->setUniform("binomCoeff", m_binomCoeff);
	m_programSmoothing2->setUniform("binomOffset", m_binomOffset);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	gl::glDisable(gl::GL_DEPTH_TEST);

	m_programSmoothing2->release();
	m_blurringFBO[0]->unbind();
	m_metaballTexture->unbind();

	m_blurringFBO[1]->bind();

	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_programSmoothing2->use();

	m_blurringTexture[0]->bindActive(gl::GL_TEXTURE0);
	m_programSmoothing3->setUniform(m_programSmoothing3->getUniformLocation("depthTexture"), 0);
	m_programSmoothing3->setUniform("viewport", viewport);
	m_programSmoothing3->setUniform("binomCoeff", m_binomCoeff);
	m_programSmoothing3->setUniform("binomOffset", m_binomOffset);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	gl::glDisable(gl::GL_DEPTH_TEST);

	m_programSmoothing3->release();
	m_blurringFBO[0]->unbind();
	m_blurringTexture[0]->unbind();
	m_vaoPlan->unbind();
}

void ScreenSpaceFluidRenderer::drawThirdPass(MetaballsExample * painter)
{
	m_finalFBO->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_programFinal->use();
	m_vaoPlan->bind();

	//m_colorTexture2->bindActive(gl::GL_TEXTURE0);
	//m_programFinal->setUniform(m_programFinal->getUniformLocation("colorTexture"), 0);

	if (!m_bilateral)
		m_blurringTexture[!(m_blurringIterations % 2)]->bindActive(gl::GL_TEXTURE0);
	else
		m_blurringTexture[1]->bindActive(gl::GL_TEXTURE0);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("depthTexture"), 0);

	m_thicknessTexture->bindActive(gl::GL_TEXTURE1);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("thicknessTexture"), 1);

	m_skybox->bindActive(gl::GL_TEXTURE2);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("skybox"), 2);

	m_shadowTexture->bindActive(gl::GL_TEXTURE3);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("shadowTexture"), 3);

	m_groundColorTexture->bindActive(gl::GL_TEXTURE4);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("groundTexture"), 4);

	m_groundDepthTexture->bindActive(gl::GL_TEXTURE5);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("groundDepthTexture"), 5);

	m_shadowThicknessTexture->bindActive(gl::GL_TEXTURE6);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("shadowThicknessTexture"), 6);

	m_programFinal->setUniform("view", painter->cameraCapability()->view());
	m_programFinal->setUniform("projection", painter->projectionCapability()->projection());
	m_programFinal->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_programFinal->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_programFinal->setUniform("near", painter->projectionCapability()->zNear());
	m_programFinal->setUniform("far", painter->projectionCapability()->zFar());
	m_programFinal->setUniform("lightPos", m_camera.eye);

	m_programFinal->setUniform("viewShadow", m_camera.view);
	m_programFinal->setUniform("projectionShadow", m_camera.projection);

	glm::vec2 viewport(static_cast<float>(painter->viewportCapability()->width()), static_cast<float>(painter->viewportCapability()->height()));
	m_programFinal->setUniform("viewport", viewport);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	if (!m_bilateral)
		m_blurringTexture[!(m_blurringIterations % 2)]->unbind();
	else
		m_blurringTexture[1]->unbind();
	//m_colorTexture2->unbind();
	m_programFinal->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_vaoPlan->unbind();
	m_finalFBO->unbind();
}

void ScreenSpaceFluidRenderer::drawShadowmap(MetaballsExample * painter)
{
	m_vao->bind();

	m_shadowFBO->bind();
	gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LESS);

	m_program->use();
	m_program->setUniform("view", m_camera.view);
	m_program->setUniform("projection", m_camera.projection);
	m_program->setUniform("sphere_radius", m_sphereRadius);
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

	m_programThickness->use();
	m_programThickness->setUniform("view", m_camera.view);
	m_programThickness->setUniform("projection", m_camera.projection);
	m_programThickness->setUniform("sphere_radius", m_sphereRadius);
	m_programThickness->setUniform("near", painter->projectionCapability()->zNear());
	m_programThickness->setUniform("far", painter->projectionCapability()->zFar());

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));

	m_vao->unbind();
	m_programThickness->release();
	m_shadowThicknessFBO->unbind();
}