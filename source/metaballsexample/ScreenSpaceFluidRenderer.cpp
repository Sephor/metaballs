#include "ScreenSpaceFluidRenderer.h"

#include <iostream>

#include <glm/ext.hpp>

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
{

}

ScreenSpaceFluidRenderer::~ScreenSpaceFluidRenderer()
{

}

void ScreenSpaceFluidRenderer::initialize(MetaballsExample * painter)
{
	m_fbo = new globjects::Framebuffer;

	//Textures
	m_colorTexture = new globjects::Texture;
	m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture, 0);

	m_normalTexture = new globjects::Texture;
	m_normalTexture->image2D(0, gl::GL_RGB, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT1, m_normalTexture, 0);

	m_depthTexture = new globjects::Texture;
	m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_fbo->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture, 0);

	m_program = new globjects::Program;
	m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/shader.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/shader.frag"),
		globjects::Shader::fromFile(gl::GL_GEOMETRY_SHADER, "data/metaballsexample/screen_space_fluid/quad_emmiting.geom")
	);

	m_programSmoothing = new globjects::Program;
	m_programSmoothing->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/shaderSmoothing.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/shaderSmoothing.frag")
	);
}

globjects::Framebuffer* ScreenSpaceFluidRenderer::draw(MetaballsExample * painter)
{
	if (painter->viewportCapability()->hasChanged())
	{
		m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_normalTexture->image2D(0, gl::GL_RGB, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	}

	std::array<glm::vec4, 20> m_metaballs = painter->getMetaballs();
	//parameters
	float sphere_radius = 1.0f;
	glm::vec4 light_dir = glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);

	//setup Metaballs data
	m_vertices = new globjects::Buffer;
	m_vertices->setData( m_metaballs , gl::GL_STATIC_DRAW);

	m_vao = new globjects::VertexArray;
	auto binding = m_vao->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_vertices, 0, 4 * sizeof(float));
	binding->setFormat(4, gl::GL_FLOAT);
	m_vao->enable(0);
	m_vao->bind();
	
	//bind buffer
	m_fbo->bind();
	gl::glDisable(gl::GL_CULL_FACE);

	gl::glClear(gl::GL_COLOR_BUFFER_BIT);
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);

	gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
	gl::glClearDepth(1.0);

	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LESS);


	//use shader programs
	glm::vec4 eye = glm::vec4( painter->cameraCapability()->eye() , 1.0f);
	eye = painter->projectionCapability()->projection() * painter->cameraCapability()->view() * eye;
	m_program->use();
	m_program->setUniform("view", painter->cameraCapability()->view());
	m_program->setUniform("projection", painter->projectionCapability()->projection());
	m_program->setUniform("eye_pos", painter->cameraCapability()->eye());
	m_program->setUniform("sphere_radius", sphere_radius);
	m_program->setUniform("light_dir", light_dir);

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));
	m_vao->unbind();

	m_programSmoothing->use();
	
	gl::glDisable(gl::GL_DEPTH_TEST);
	m_fbo->unbind();

	return m_fbo;
}