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

#include "ScreenSpaceFluidRenderer.h"
#include "RaycastingRenderer.h"

MetaballsExample::MetaballsExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
,	m_raycasting(false)
,	m_other(false)
,	m_SSF(true)
{
	setupPropertyGroup();
	//m_projectionCapability->setZFar(200.f);
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

	m_SSFRenderer = std::make_unique<ScreenSpaceFluidRenderer>();
	m_rayRenderer = std::make_unique<RaycastingRenderer>();

	m_rayRenderer->initialize(this);
	m_SSFRenderer->initialize(this);

	m_cameraCapability->setEye(glm::vec3(0.f, 2.5f, -10.f));
	m_cameraCapability->setCenter(glm::vec3(0.f, 2.5f, 0.f));
}

void MetaballsExample::onPaint()
{
	m_fluidSimulator.update();

	gl::glViewport(0, 0, m_viewportCapability->width(), m_viewportCapability->height());
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

	std::array<int, 4> rect = { { 0, 0, m_viewportCapability->width(), m_viewportCapability->height() } };

	globjects::Framebuffer * targetFBO = m_targetFramebufferCapability->framebuffer() ? m_targetFramebufferCapability->framebuffer() : globjects::Framebuffer::defaultFBO();
	globjects::Framebuffer * tmp_fbo = nullptr;

	if (m_raycasting)
	{
		tmp_fbo = m_rayRenderer->draw(this);

		targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);
		tmp_fbo->blit(gl::GL_COLOR_ATTACHMENT0, rect, targetFBO, gl::GL_BACK_LEFT, rect, gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT, gl::GL_NEAREST);
	}

	if (m_SSF)
	{	
		rect[0] += m_raycasting * static_cast<unsigned>(m_viewportCapability->width() / 2);
		
		tmp_fbo = m_SSFRenderer->draw(this);

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

bool MetaballsExample::getBlur() const
{
	return m_blur;
}

void MetaballsExample::setBlur(bool value)
{
	m_blur = value;
}

void MetaballsExample::setupPropertyGroup()
{
	addProperty<bool>("Raycasting", this,
		&MetaballsExample::getRaycasting, &MetaballsExample::setRaycasting);

	addProperty<bool>("ScreenSpaceFluid", this,
		&MetaballsExample::getSSF, &MetaballsExample::setSSF);

	addProperty<bool>("Blurring", this,
		&MetaballsExample::getBlur, &MetaballsExample::setBlur);

	addProperty<bool>("Simulate", &m_fluidSimulator,
		&FluidSimulator::getIsRunning, &FluidSimulator::setIsRunning);
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

const std::array<glm::vec4, 400> MetaballsExample::getMetaballs() const
{
	return m_fluidSimulator.getMetaballs();
}