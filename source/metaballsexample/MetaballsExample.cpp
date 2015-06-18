<<<<<<< HEAD
<<<<<<< HEAD
#include "MetaballsExample.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <globjects/Texture.h>
#include <globjects/AttachedTexture.h>

#include "OtherRenderer.h"
#include "RaycastingRenderer.h"
#include "ScreenSpaceFluidRenderer.h"

MetaballsExample::MetaballsExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
,	m_raycasting(false)
,   m_SSF(false)
{
	setupPropertyGroup();
}

MetaballsExample::~MetaballsExample() = default;

void MetaballsExample::setupProjection()
{
    //static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;
}

void MetaballsExample::onInitialize()
{
    globjects::init();

	gl::glClearColor(0.0, 0.0, 0.0, 1.0);
	gl::glClearDepth(1.f);

#ifdef __APPLE__
    globjects::Shader::clearGlobalReplacements();
    globjects::Shader::globalReplace("#version 140", "#version 150");

    globjects::debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif
<<<<<<< HEAD
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);

    m_texture = new globjects::Texture;
    m_texture->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

    m_fbo = new globjects::Framebuffer;
    m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_texture, 0);

<<<<<<< HEAD
    gl::glClearColor(0.0, 0.0, 0.0, 1.0);
=======
	//m_vertices = new globjects::Buffer;
	//m_vertices->setData(std::vector<float>{
	//	-1.f, 1.f,
	//	-1.f, -1.f,
	//	1.f, 1.f,
	//	1.f, -1.f
	//}, gl::GL_STATIC_DRAW);
	for (int i = 0; i < METABALLSCOUNT; i++)
		m_metaballs[i] = glm::vec4(i * 2.f, 0.f, 0.f, 0.5f);

	m_vao = new globjects::VertexArray;

	auto binding = m_vao->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_metaballs.data, 0, 4 * sizeof(float));
	binding->setFormat(4, gl::GL_FLOAT);

	m_vao->enable(0);

    gl::glClearColor(1.0, 1.0, 1.0, 1.0);
>>>>>>> DOESN'T WORK Merge commit

    m_fbo->unbind();
=======
>>>>>>> build buffer in renderer

<<<<<<< HEAD
	m_otherRenderer = std::make_unique<OtherRenderer>();
	m_rayRenderer = std::make_unique<RaycastingRenderer>();
=======
	m_rayRenderer = std::make_unique<RaycastingRenderer>(m_viewportCapability, m_projectionCapability, m_cameraCapability);
	m_SSFRenderer = std::make_unique<ScreenSpaceFluidRenderer>(m_viewportCapability, m_projectionCapability, m_cameraCapability);
>>>>>>> initialize ScreenSpaceFluidRenderer

	m_SSFRenderer->initialize(this);
	m_rayRenderer->initialize(this);
}

void MetaballsExample::onPaint()
{
	//m_fluidSimulator.update();

	gl::glViewport(0, 0, m_viewportCapability->width(), m_viewportCapability->height());
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

	std::array<int, 4> rect = { { 0, 0, m_viewportCapability->width(), m_viewportCapability->height() } };

	globjects::Framebuffer * targetFBO = m_targetFramebufferCapability->framebuffer() ? m_targetFramebufferCapability->framebuffer() : globjects::Framebuffer::defaultFBO();
	globjects::Framebuffer * tmp_fbo = nullptr;

	if (m_raycasting)
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	{
		m_rayRenderer->draw(this);
=======
	{	
		
		m_rayRenderer->computePhysiks();
		m_rayRenderer->draw(m_vao);
>>>>>>> initialize ScreenSpaceFluidRenderer
=======

	{
<<<<<<< HEAD
		gl::glClear(gl::GL_COLOR_BUFFER_BIT);
		m_rayRenderer->draw(this);
>>>>>>> fixed merge issues & initialize SSFR
=======
		m_rayRenderer->draw(this, m_fluidSimulator.metaballs());
>>>>>>> Physik im raycast Renderer eingebunden
=======
	{
		tmp_fbo = m_rayRenderer->draw(this);
>>>>>>> added depthbuffer for raycasting

		targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);
		tmp_fbo->blit(gl::GL_COLOR_ATTACHMENT0, rect, targetFBO, gl::GL_BACK_LEFT, rect, gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT, gl::GL_NEAREST);
	}

	if (m_SSF)
	{	
		rect[0] += m_raycasting * static_cast<unsigned>(m_viewportCapability->width() / 2);
<<<<<<< HEAD
			
<<<<<<< HEAD
<<<<<<< HEAD
		m_otherRenderer->draw(this);
=======
		m_SSFRenderer->draw(m_vao);
>>>>>>> initialize ScreenSpaceFluidRenderer
=======
		
<<<<<<< HEAD
<<<<<<< HEAD
		m_SSFRenderer->draw(this);
>>>>>>> fixed merge issues & initialize SSFR
=======
		m_otherRenderer->draw(this, m_fluidSimulator.metaballs());
>>>>>>> Physik im raycast Renderer eingebunden
=======
		tmp_fbo = globjects::ref_ptr<globjects::Framebuffer>(m_SSFRenderer->draw(this));
>>>>>>> build buffer in renderer
=======
		tmp_fbo = m_SSFRenderer->draw(this);
>>>>>>> added depthbuffer for raycasting

		targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);
		tmp_fbo->blit(gl::GL_COLOR_ATTACHMENT0, rect, targetFBO, gl::GL_BACK_LEFT, rect, gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT, gl::GL_NEAREST);
	}

	globjects::Framebuffer::unbind();
}

bool MetaballsExample::getRaycasting() const
{
	return m_raycasting;
}

void MetaballsExample::setRaycasting(bool value)
{
	m_raycasting = value;
	//m_other = !m_raycasting;
}

bool MetaballsExample::getSSF() const
{
	return m_SSF;
}

void MetaballsExample::setSSF(bool value)
{
	m_SSF = value;
	//m_other = !m_raycasting;
}

void MetaballsExample::setupPropertyGroup()
{
	addProperty<bool>("Raycasting", this,
		&MetaballsExample::getRaycasting, &MetaballsExample::setRaycasting);
<<<<<<< HEAD
<<<<<<< HEAD
	addProperty<bool>("Other", this,
		&MetaballsExample::getOther, &MetaballsExample::setOther);
=======

	addProperty<bool>("ScreenSpaceFluid", this,
		&MetaballsExample::getSSF, &MetaballsExample::setSSF);

>>>>>>> fixed merge issues & initialize SSFR
}

const gloperate::AbstractTargetFramebufferCapability * MetaballsExample::targetFramebufferCapability() const
{
	return m_targetFramebufferCapability;
}

const gloperate::AbstractViewportCapability * MetaballsExample::viewportCapability() const
{
	return m_viewportCapability;
}

const gloperate::AbstractPerspectiveProjectionCapability * MetaballsExample::projectionCapability() const
{
	return m_projectionCapability;
}

const gloperate::AbstractCameraCapability * MetaballsExample::cameraCapability() const
{
	return m_cameraCapability;
<<<<<<< HEAD
}

const std::array<glm::vec4, 400> MetaballsExample::getMetaballs() const
{
<<<<<<< HEAD
	return m_metaballs;
<<<<<<< HEAD
=======
	addProperty<bool>("ScreenSpaceFluid", this,
		&MetaballsExample::getSSF, &MetaballsExample::setSSF);
>>>>>>> initialize ScreenSpaceFluidRenderer
=======

>>>>>>> fixed merge issues & initialize SSFR
=======
>>>>>>> Physik im raycast Renderer eingebunden
=======
=======
>>>>>>> fixed merge conflict
#include "MetaballsExample.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <globjects/Texture.h>
#include <globjects/AttachedTexture.h>

#include "OtherRenderer.h"
#include "RaycastingRenderer.h"

MetaballsExample::MetaballsExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
,	m_raycasting(true)
,	m_other(false)
{
	setupPropertyGroup();
}

MetaballsExample::~MetaballsExample() = default;

void MetaballsExample::setupProjection()
{
    //static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;
}

void MetaballsExample::onInitialize()
{
    globjects::init();

#ifdef __APPLE__
    globjects::Shader::clearGlobalReplacements();
    globjects::Shader::globalReplace("#version 140", "#version 150");

    globjects::debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif
	gl::glClearColor(0.0, 0.0, 0.0, 1.0);

    m_texture = new globjects::Texture;
    m_texture->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

    m_fbo = new globjects::Framebuffer;
    m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_texture, 0);

    gl::glClearColor(0.0, 0.0, 0.0, 1.0);

    m_fbo->unbind();

	m_otherRenderer = std::make_unique<OtherRenderer>();
	m_rayRenderer = std::make_unique<RaycastingRenderer>();

	m_otherRenderer->initialize();
	m_rayRenderer->initialize();
}

void MetaballsExample::onPaint()
{
	m_fluidSimulator.update();

	if (m_viewportCapability->hasChanged())
	{
		m_texture->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

		m_viewportCapability->setChanged(false);
	}

	gl::glViewport(0, 0, m_viewportCapability->width(), m_viewportCapability->height());
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);

	m_fbo->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);

	std::array<int, 4> rect = { { 0, 0, m_viewportCapability->width(), m_viewportCapability->height() } };

	globjects::Framebuffer * targetFBO = m_targetFramebufferCapability->framebuffer() ? m_targetFramebufferCapability->framebuffer() : globjects::Framebuffer::defaultFBO();

	if (m_raycasting)
	{
		m_rayRenderer->draw(this, m_fluidSimulator.metaballs());

		targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);
		m_fbo->blit(gl::GL_COLOR_ATTACHMENT0, rect, targetFBO, gl::GL_BACK_LEFT, rect, gl::GL_COLOR_BUFFER_BIT, gl::GL_LINEAR);
	}
	if (m_other)
	{
		rect[0] += m_raycasting * static_cast<unsigned>(m_viewportCapability->width() / 2);
			
		m_otherRenderer->draw(this, m_fluidSimulator.metaballs());

		targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);
		m_fbo->blit(gl::GL_COLOR_ATTACHMENT0, rect, targetFBO, gl::GL_BACK_LEFT, rect, gl::GL_COLOR_BUFFER_BIT, gl::GL_LINEAR);
	}

	globjects::Framebuffer::unbind();
}

bool MetaballsExample::getRaycasting() const
{
	return m_raycasting;
}

void MetaballsExample::setRaycasting(bool value)
{
	m_raycasting = value;
	//m_other = !m_raycasting;
}

bool MetaballsExample::getOther() const
{
	return m_other;
}

void MetaballsExample::setOther(bool value)
{
	m_other = value;
	//m_raycasting = !m_other;
}

void MetaballsExample::setupPropertyGroup()
{
	addProperty<bool>("Raycasting", this,
		&MetaballsExample::getRaycasting, &MetaballsExample::setRaycasting);
	addProperty<bool>("Other", this,
		&MetaballsExample::getOther, &MetaballsExample::setOther);
}

const gloperate::AbstractTargetFramebufferCapability * MetaballsExample::targetFramebufferCapability() const
{
	return m_targetFramebufferCapability;
}

const gloperate::AbstractViewportCapability * MetaballsExample::viewportCapability() const
{
	return m_viewportCapability;
}

const gloperate::AbstractPerspectiveProjectionCapability * MetaballsExample::projectionCapability() const
{
	return m_projectionCapability;
}

const gloperate::AbstractCameraCapability * MetaballsExample::cameraCapability() const
{
	return m_cameraCapability;
<<<<<<< HEAD
>>>>>>> added environment map
=======
>>>>>>> fixed merge conflict
=======
	return m_fluidSimulator.getMetaballs();
>>>>>>> fixed metaballsexample and raycastingrenderer
}