#include "RaycastingRenderer.h"

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
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>

#include "MetaballsExample.h"

RaycastingRenderer::RaycastingRenderer()
{

}

RaycastingRenderer::~RaycastingRenderer()
{

}


void RaycastingRenderer::initialize()
{
	m_vertices = new globjects::Buffer;
	m_vertices->setData(std::vector<float>{
		-1.f, 1.f,
			-1.f, -1.f,
			1.f, 1.f,
			1.f, -1.f
	}, gl::GL_STATIC_DRAW);

	m_vao = new globjects::VertexArray;

	auto binding = m_vao->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_vertices, 0, 2 * sizeof(float));
	binding->setFormat(2, gl::GL_FLOAT);

	m_vao->enable(0);

	m_program = new globjects::Program;
	m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/raycasting/shader.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/raycasting/shader.frag")
	);
}

void RaycastingRenderer::draw(MetaballsExample * painter)
{
	m_vao->bind();
	m_program->use();
	m_program->setUniform("metaballs", painter->metaballs());
	m_program->setUniform("eye", painter->cameraCapability()->eye());
	m_program->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_program->setUniform("view", painter->cameraCapability()->view());
	
	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
}