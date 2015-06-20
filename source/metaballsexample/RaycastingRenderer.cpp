<<<<<<< HEAD
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
<<<<<<< HEAD
=======

	for (unsigned int i = 0; i < m_metaballs.size(); i++)
	{
		m_metaballs[i] = glm::vec4(i * 2.f, 0.f, 0.f, 0.5f);
	}
>>>>>>> initialize ScreenSpaceFluidRenderer
}

void RaycastingRenderer::draw(
	MetaballsExample * painter, 
	const std::vector<glm::vec4> & metaballs)
{
<<<<<<< HEAD
	m_vao->bind();
=======

	vao->bind();
>>>>>>> initialize ScreenSpaceFluidRenderer
	m_program->use();
	m_program->setUniform("metaballs", metaballs);
	m_program->setUniform("eye", painter->cameraCapability()->eye());
	m_program->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_program->setUniform("view", painter->cameraCapability()->view());
	
	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
}

void RaycastingRenderer::computePhysiks(){
	m_metaballs[0] += glm::vec4(0.01f , 0.0f, 0.0f, 0.0f);
=======
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
#include <globjects/Framebuffer.h>
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


void RaycastingRenderer::initialize(MetaballsExample * painter)
{
	setupFramebuffer(painter);

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

globjects::Framebuffer * RaycastingRenderer::draw(
	MetaballsExample * painter)
{
	m_fbo->bind(),
	m_vao->bind();

	//TODO: warum funktioniert die Navigation nicht mehr wenn man den Depthbuffer cleart
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);

	m_skybox->bindActive(gl::GL_TEXTURE0);
	m_program->use();

	//Vertex Shader
	m_program->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());

	//Fragment Shader
	m_program->setUniform("metaballs", painter->getMetaballs());
	m_program->setUniform("eye", painter->cameraCapability()->eye());
	m_program->setUniform("projection", painter->projectionCapability()->projection());
	int skyboxLocation = m_program->getUniformLocation("skybox");
	m_program->setUniform(skyboxLocation, 0);

	//both
	m_program->setUniform("view", painter->cameraCapability()->view());
	
	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);

	m_program->release();

	m_skybox->unbindActive(gl::GL_TEXTURE0);
<<<<<<< HEAD
>>>>>>> added environment map
=======

	m_fbo->unbind();

	return m_fbo;
}

void RaycastingRenderer::setupFramebuffer(MetaballsExample * painter)
{
	m_fbo = new globjects::Framebuffer;

	m_colorTexture = new globjects::Texture;
	m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture, 0);

	//TODO: Herausfinden warum es nicht mit glGetfloatv(GL_VIEWPORT) funktioniert
	m_depthTexture = new globjects::Texture;
	m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_fbo->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture, 0);

	//set clearcolor and cleardepth
	m_fbo->bind();
	gl::glClearColor(0.f, 0.f, 0.f, 1.f);
	gl::glClearDepth(1.f);
	m_fbo->unbind();
>>>>>>> added depthbuffer for raycasting
}