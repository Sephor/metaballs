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

ScreenSpaceFluidRenderer::ScreenSpaceFluidRenderer() : blurFilterSize{ 0 }
{

}

ScreenSpaceFluidRenderer::~ScreenSpaceFluidRenderer()
{

}

void ScreenSpaceFluidRenderer::initialize(MetaballsExample * painter)
{
	m_fbo = new globjects::Framebuffer;
	
	//Textures
	m_colorTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
	m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorTexture, 0);

	m_depthTexture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
	m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	m_fbo->attachTexture(gl::GL_DEPTH_ATTACHMENT, m_depthTexture, 0);

	m_fbo->bind();
	gl::glClearDepth(1.0);
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);
	m_fbo->unbind();
	//--//

	//first rendering pass
	m_metaballs = painter->getMetaballs();

	m_vertices = new globjects::Buffer;
	m_vertices->setData(m_metaballs, gl::GL_DYNAMIC_DRAW);

	m_vao = new globjects::VertexArray;
	auto binding = m_vao->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_vertices, 0, 4 * sizeof(float));
	binding->setFormat(4, gl::GL_FLOAT);
	m_vao->enable(0);


	//setup plan for second render pass
	m_verticesPlan = new globjects::Buffer;
	m_verticesPlan->setData(std::vector<float>{
		-1.f, 1.f,
		-1.f, -1.f,
		1.f, 1.f,
		1.f, -1.f
	}, gl::GL_STATIC_DRAW);

	m_vaoPlan = new globjects::VertexArray;

	auto binding2 = m_vaoPlan->binding(0);
	binding2->setAttribute(0);
	binding2->setBuffer(m_verticesPlan, 0, 2 * sizeof(float));
	binding2->setFormat(2, gl::GL_FLOAT);

	m_vaoPlan->enable(0);
	//--//
	//setup programs
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
	//--//

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

globjects::Framebuffer* ScreenSpaceFluidRenderer::draw(MetaballsExample * painter)
{
	if (painter->viewportCapability()->hasChanged())
	{
		m_colorTexture->image2D(0, gl::GL_RGBA, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);
		m_depthTexture->image2D(0, gl::GL_DEPTH_COMPONENT, painter->viewportCapability()->width(), painter->viewportCapability()->height(), 0, gl::GL_DEPTH_COMPONENT, gl::GL_FLOAT, nullptr);
	}

	m_metaballs = painter->getMetaballs();
	//parameters
	float sphere_radius = 1.0f;
	glm::vec4 light_dir = glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);

	//setup Metaballs data
	m_vertices->setSubData(m_metaballs);
	m_vao->bind();
	
	//bind buffer
	m_fbo->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

	gl::glEnable(gl::GL_DEPTH_TEST);
	gl::glDepthFunc(gl::GL_LESS);

	//use shader programs
	//1. rendering Pass
	m_program->use();
	m_program->setUniform("view", painter->cameraCapability()->view());
	m_program->setUniform("projection", painter->projectionCapability()->projection());
	m_program->setUniform("sphere_radius", sphere_radius);
	m_program->setUniform("light_dir", light_dir);
	m_program->setUniform("near", painter->projectionCapability()->zNear());
	m_program->setUniform("far", painter->projectionCapability()->zFar());

	gl::glDrawArrays(gl::GL_POINTS, 0, static_cast<gl::GLsizei>(m_metaballs.size()));
	m_vao->unbind();
	m_program->release();

	gl::glDisable(gl::GL_DEPTH_TEST);

	m_programSmoothing->use();
	m_vaoPlan->bind();

	//bind textures

	//2. rendering Pass
	m_colorTexture->bindActive(gl::GL_TEXTURE0);
	m_programSmoothing->setUniform( m_programSmoothing->getUniformLocation("colorTexture") , 0);
	
	m_depthTexture->bindActive(gl::GL_TEXTURE1);
	m_programSmoothing->setUniform( m_programSmoothing->getUniformLocation("depthTexture"), 1);

	m_skybox->bindActive(gl::GL_TEXTURE2);
	m_programSmoothing->setUniform(m_programSmoothing->getUniformLocation("skybox"), 2);
	//--//
	m_programSmoothing->setUniform("maxDepth", 1.0f);
	m_programSmoothing->setUniform("light_dir", light_dir);
	m_programSmoothing->setUniform("projectionInverted", painter->projectionCapability()->projectionInverted());
	m_programSmoothing->setUniform("viewInverted", painter->cameraCapability()->viewInverted());
	m_programSmoothing->setUniform("blur", painter->getBlur());
	m_programSmoothing->setUniform("view", painter->cameraCapability()->view());
	m_programSmoothing->setUniform("projection", painter->projectionCapability()->projection());
	
	//drawing
	gl::glFinish();
	gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
	
	//release
	m_depthTexture->unbind();
	m_colorTexture->unbind();
	m_programSmoothing->release();
	m_vaoPlan->unbind();
	m_fbo->unbind();

	return m_fbo;
}

void ScreenSpaceFluidRenderer::setBlurFilterSize(int size){
	this->blurFilterSize = size;
}

int ScreenSpaceFluidRenderer::getBlurFilterSize() const {
	return this->blurFilterSize;
}