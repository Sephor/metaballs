#include "OtherRenderer.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/VertexArray.h>

OtherRenderer::OtherRenderer()
{

}

OtherRenderer::~OtherRenderer()
{

}


void OtherRenderer::initialize()
{
	m_program = new globjects::Program;
	m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/metaballsexample/other/shader.vert"),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/metaballsexample/other/shader.frag")
	);
}

void OtherRenderer::draw(globjects::ref_ptr<globjects::VertexArray> & vao)
{
	vao->bind();
	m_program->use();
	
	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
}