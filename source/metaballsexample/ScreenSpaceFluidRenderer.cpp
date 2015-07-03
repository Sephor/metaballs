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
	: m_blurFilterSize{ 0 }
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
}

globjects::Framebuffer* ScreenSpaceFluidRenderer::draw(MetaballsExample * painter)
{
	if (painter->viewportCapability()->hasChanged())
	{
		m_colorTexture1->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture1->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);

		m_colorTexture2->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture2->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);

		m_colorTexture3->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture3->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);

		m_colorTexture4->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	}

	drawThicknessPass(painter);
	drawFirstPass(painter);
	drawSecondPass(painter);
	drawThirdPass(painter);

	return m_fbo3;
}

void ScreenSpaceFluidRenderer::setBlurFilterSize(int size)
{
	m_blurFilterSize = size;
}

int ScreenSpaceFluidRenderer::getBlurFilterSize() const 
{
	return m_blurFilterSize;
}

void ScreenSpaceFluidRenderer::setupFramebuffers(MetaballsExample * painter)
{
	m_fbo1 = new globjects::Framebuffer;
	m_fbo2 = new globjects::Framebuffer;
	m_fbo3 = new globjects::Framebuffer;
	m_fbo4 = new globjects::Framebuffer;

	//first
	m_colorTexture1 = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_colorTexture1->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo1->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture1, 0);

	m_depthTexture1 = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_depthTexture1->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_fbo1->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture1, 0);

	m_fbo1->bind();
	gl::glClearDepth(1.0);
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);
	m_fbo1->unbind();

	//second
	m_colorTexture2 = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_colorTexture2->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo2->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture2, 0);

	m_depthTexture2 = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_depthTexture2->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_fbo2->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture2, 0);

	m_fbo2->bind();
	gl::glClearDepth(1.0);
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);
	m_fbo2->unbind();

	//third
	m_colorTexture3 = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_colorTexture3->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo3->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture3, 0);

	m_depthTexture3 = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_depthTexture3->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_fbo3->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture3, 0);

	m_fbo3->bind();
	gl::glClearDepth(1.0);
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);
	m_fbo3->unbind();

	//thickness
	m_colorTexture4 = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_colorTexture4->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo4->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture4, 0);

	m_fbo4->bind();
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);
	m_fbo4->unbind();
}

void ScreenSpaceFluidRenderer::setupPrograms(MetaballsExample * painter)
{
	m_programThickness = new globjects::Program;
	m_programThickness->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.frag"),
		globjects::Shader::fromFile(gl::GL_GEOMETRY_SHADER, "data/metaballsexample/screen_space_fluid/thicknessPass.geom")
	);

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

void ScreenSpaceFluidRenderer::drawThicknessPass(MetaballsExample * painter)
{
	m_metaballs = painter->getMetaballs();
	m_vertices->setSubData(m_metaballs);
	m_vao->bind();

	m_fbo4->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
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
	m_fbo4->unbind();
}

void ScreenSpaceFluidRenderer::drawFirstPass(MetaballsExample * painter)
{
	m_metaballs = painter->getMetaballs();
	m_vertices->setSubData(m_metaballs);
	m_vao->bind();

	m_fbo1->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
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
	m_fbo1->unbind();
}

void ScreenSpaceFluidRenderer::drawSecondPass(MetaballsExample * painter)
{
	m_fbo2->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_programSmoothing->use();
	m_vaoPlan->bind();

	m_colorTexture1->bindActive(gl::GL_TEXTURE0);
	m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("colorTexture"), 0);

	m_depthTexture1->bindActive(gl::GL_TEXTURE1);
	m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("depthTexture"), 1);

	m_skybox->bindActive(gl::GL_TEXTURE2);
	m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("skybox"), 2);

	m_programSmoothing->setUniform("maxDepth", 1.0f);
	m_programSmoothing->setUniform("light_dir", m_lightDir);
	m_programSmoothing->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_programSmoothing->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_programSmoothing->setUniform("blur", painter->getBlur());
	m_programSmoothing->setUniform("view", painter->cameraCapability()->view());
	m_programSmoothing->setUniform("projection", painter->projectionCapability()->projection());
	glm::vec2 viewport(static_cast<float>(painter->viewportCapability()->width()), static_cast<float>(painter->viewportCapability()->height()));
	m_programSmoothing->setUniform("viewport", viewport);
	glm::vec2 fov(0.f);
	fov.y = painter->projectionCapability()->fovy();
	fov.x = fov.y * painter->projectionCapability()->aspectRatio();
	m_programSmoothing->setUniform("fov", fov);
	glm::vec2 focal(0.f);
	focal.x = sinf(fov.x / 2.f) / tanf(fov.x / 2.f);
	focal.y = sinf(fov.y / 2.f) / tanf(fov.y / 2.f);
	glm::vec2 focal2(-painter->projectionCapability()->projection()[0][0], -painter->projectionCapability()->projection()[1][1]);
	m_programSmoothing->setUniform("focal", focal);
	m_programSmoothing->setUniform("focal2", focal2);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_depthTexture1->unbind();
	m_colorTexture1->unbind();
	m_programSmoothing->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_vaoPlan->unbind();
	m_fbo2->unbind();
}

void ScreenSpaceFluidRenderer::drawThirdPass(MetaballsExample * painter)
{
	m_fbo3->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LEQUAL);

	m_programFinal->use();
	m_vaoPlan->bind();

	m_colorTexture2->bindActive(gl::GL_TEXTURE0);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("colorTexture"), 0);

	m_depthTexture2->bindActive(gl::GL_TEXTURE1);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("depthTexture"), 1);

	m_skybox->bindActive(gl::GL_TEXTURE2);
	m_programFinal->setUniform(m_programFinal->getUniformLocation("skybox"), 2);

	m_programFinal->setUniform("view", painter->cameraCapability()->view());
	m_programFinal->setUniform("projection", painter->projectionCapability()->projection());
	m_programFinal->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_programFinal->setUniform("viewInverted", painter->cameraCapability()->viewInverted());

	glm::vec2 viewport(static_cast<float>(painter->viewportCapability()->width()), static_cast<float>(painter->viewportCapability()->height()));
	m_programFinal->setUniform("viewport", viewport);

	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_depthTexture2->unbind();
	m_colorTexture2->unbind();
	m_programFinal->release();
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_vaoPlan->unbind();
	m_fbo3->unbind();
}