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
,	m_raycasting(true)
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

	for (unsigned int i = 0; i < m_metaballs.size(); i++)
	{
		m_metaballs[i] = glm::vec4(i * 2.f, 0.f, 0.f, 1.f);
	}

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

	m_SSFRenderer = std::make_unique<ScreenSpaceFluidRenderer>();
	m_rayRenderer = std::make_unique<RaycastingRenderer>();

	m_SSFRenderer->initialize();
	m_rayRenderer->initialize();
}

void MetaballsExample::onPaint()
{
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
<<<<<<< HEAD
	{	
		
		m_rayRenderer->computePhysiks();
		m_rayRenderer->draw(m_vao);
=======
	{
		m_rayRenderer->draw(this);
>>>>>>> master

		targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);
		m_fbo->blit(gl::GL_COLOR_ATTACHMENT0, rect, targetFBO, gl::GL_BACK_LEFT, rect, gl::GL_COLOR_BUFFER_BIT, gl::GL_LINEAR);
	}
	if (m_SSF)
	{
		rect[0] += m_raycasting * static_cast<unsigned>(m_viewportCapability->width() / 2);
			
<<<<<<< HEAD
		m_SSFRenderer->draw(m_vao);
=======
		m_otherRenderer->draw(this);
>>>>>>> master

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
	addProperty<bool>("ScreenSpaceFluid", this,
		&MetaballsExample::getSSF, &MetaballsExample::setSSF);
=======
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
}

const std::array<glm::vec4, 20> & MetaballsExample::metaballs() const
{
	return m_metaballs;
>>>>>>> master
}