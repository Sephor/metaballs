#include "RaycastingRenderer.h"

#include <iostream>

#include <gloperate-qt\QtTextureLoader.h>
//#include <gloperate-qt\gloperate-qt_api.h>

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
#include <globjects/Texture.h>
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

void RaycastingRenderer::draw(
	MetaballsExample * painter)
{
	m_vao->bind();

	m_skybox->bindActive(gl::GL_TEXTURE0);

	m_program->use();
	m_program->setUniform("metaballs", painter->getMetaballs());
	m_program->setUniform("eye", painter->cameraCapability()->eye());
	m_program->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_program->setUniform("view", painter->cameraCapability()->view());
	int test = m_program->getUniformLocation("skybox");
	m_program->setUniform(test, 0);
	
	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_program->release();

	m_skybox->unbindActive(gl::GL_TEXTURE0);
}