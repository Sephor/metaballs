#include "ScreenSpaceFluidRenderer.h"

#include <iostream>

#include <glm/ext.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/ViewportCapability.h>

#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/VertexArray.h>

ScreenSpaceFluidRenderer::ScreenSpaceFluidRenderer(
	gloperate::AbstractViewportCapability * viewportCapability, 
	gloperate::AbstractPerspectiveProjectionCapability * projectionCapability,
	gloperate::AbstractCameraCapability * cameraCapability)

:	m_cameraCapability(cameraCapability)
,	m_viewportCapability(viewportCapability)
,	m_projectionCapability(projectionCapability)
{

}

ScreenSpaceFluidRenderer::~ScreenSpaceFluidRenderer()
{

}


void ScreenSpaceFluidRenderer::initialize()
{
	m_program = new globjects::Program;
	m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/screen_space_fluid/shader.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/screen_space_fluid/shader.frag")
	);

	for (unsigned int i = 0; i < m_metaballs.size(); i++)
	{
		m_metaballs[i] = glm::vec4(i * 2.f, 0.f, 0.f, 0.5f);
	}
}

void ScreenSpaceFluidRenderer::draw(globjects::ref_ptr<globjects::VertexArray> & vao)
{

	vao->bind();
	m_program->use();
	m_program->setUniform("metaballs", m_metaballs);
	m_program->setUniform("eye", m_cameraCapability->eye());
	m_program->setUniform("projectionInverted", m_projectionCapability->projectionInverted());
	m_program->setUniform("view", m_cameraCapability->view());
	
	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
}