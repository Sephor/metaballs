#include "RaycastingRenderer.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/VertexArray.h>

RaycastingRenderer::RaycastingRenderer()
{

}

RaycastingRenderer::~RaycastingRenderer()
{

}


void RaycastingRenderer::initialize()
{
	m_program = new globjects::Program;
	m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/raycasting/shader.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/raycasting/shader.frag")
	);
}

void RaycastingRenderer::draw(globjects::ref_ptr<globjects::VertexArray> & vao)
{
	vao->bind();
	m_program->use();
	
	gl::glDrawArrays(gl::GL_TRIANGLES, 0, 3);
}