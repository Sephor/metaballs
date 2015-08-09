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

MetaballsExample::MetaballsExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
{
	setupPropertyGroup();
}

MetaballsExample::~MetaballsExample() = default;

void MetaballsExample::onInitialize()
{
    globjects::init();

#ifdef __APPLE__
    globjects::Shader::clearGlobalReplacements();
    globjects::Shader::globalReplace("#version 140", "#version 150");

    globjects::debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

	m_SSFRenderer.initialize(this);

	m_cameraCapability->setEye(glm::vec3(0.f, 2.5f, -10.f));
	m_cameraCapability->setCenter(glm::vec3(0.f, 2.5f, 0.f));

	gl::glClearColor(0.f, 0.f, 0.f, 1.f);
	gl::glClearDepth(1.f);
}

void MetaballsExample::onPaint()
{
	m_fluidSimulator.update();

	gl::glViewport(0, 0, m_viewportCapability->width(), m_viewportCapability->height());
	gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

	std::array<int, 4> rect = { { 0, 0, m_viewportCapability->width(), m_viewportCapability->height() } };

	globjects::Framebuffer * targetFBO = m_targetFramebufferCapability->framebuffer() ? m_targetFramebufferCapability->framebuffer() : globjects::Framebuffer::defaultFBO();
	globjects::Framebuffer * tempFBO = m_SSFRenderer.draw(this);

	targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);
	tempFBO->blit(gl::GL_COLOR_ATTACHMENT0, rect, targetFBO, gl::GL_BACK_LEFT, rect, gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT, gl::GL_NEAREST);

	globjects::Framebuffer::unbind();
}

void MetaballsExample::setupPropertyGroup()
{
	addProperty<bool>("Bilateral", &m_SSFRenderer,
		&ScreenSpaceFluidRenderer::getBilateral, &ScreenSpaceFluidRenderer::setBilateral);

	addProperty<bool>("Simulate", &m_fluidSimulator,
		&FluidSimulator::getIsRunning, &FluidSimulator::setIsRunning);

	addProperty<unsigned int>("Iterations", &m_SSFRenderer,
		&ScreenSpaceFluidRenderer::getBlurringIterations, &ScreenSpaceFluidRenderer::setBlurringIterations)->setOptions({
			{ "minimum", 1 },
			{ "maximum", 5000 },
			{ "step", 100 },
			{ "precision", 2u } });

	addProperty<float>("MetaballSize", &m_fluidSimulator,
		&FluidSimulator::getMetaballRadius, &FluidSimulator::setMetaballRadius)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.01f },
			{ "precision", 2u } });

	addProperty<float>("Attraction", &m_fluidSimulator,
		&FluidSimulator::getAttractionFactor, &FluidSimulator::setAttractionFactor)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 10.0f },
			{ "step", 0.1f },
			{ "precision", 1u } });

	addProperty<float>("Repulsion", &m_fluidSimulator,
		&FluidSimulator::getRepulsionFactor, &FluidSimulator::setRepulsionFactor)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 10.0f },
			{ "step", 0.1f },
			{ "precision", 1u } });

	addProperty<float>("SpawnTime", &m_fluidSimulator,
		&FluidSimulator::getEmitterPeriod, &FluidSimulator::setEmitterPeriod)->setOptions({
			{ "minimum", 0.01f },
			{ "maximum", 1.0f },
			{ "step", 0.01f },
			{ "precision", 2u } });

	addProperty<float>("Spread", &m_fluidSimulator,
		&FluidSimulator::getSpread, &FluidSimulator::setSpread)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.01f },
			{ "precision", 2u } });

	addProperty<float>("Spray", &m_fluidSimulator,
		&FluidSimulator::getSpray, &FluidSimulator::setSpray)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 2.0f },
			{ "step", 0.01f },
			{ "precision", 2u } });

	addProperty<float>("BlurringScale", &m_SSFRenderer,
		&ScreenSpaceFluidRenderer::getBlurringScale, &ScreenSpaceFluidRenderer::setBlurringScale)->setOptions({
			{ "minimum", 1.f },
			{ "maximum", 3.f },
			{ "step", 0.5f },
			{ "precision", 2u } });

	addProperty<float>("TimeStep", &m_SSFRenderer,
		&ScreenSpaceFluidRenderer::getTimeStep, &ScreenSpaceFluidRenderer::setTimeStep)->setOptions({
			{ "minimum", 0.00001f },
			{ "maximum", 0.001f },
			{ "precision", 5u } });
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

const std::vector<glm::vec4> MetaballsExample::getMetaballs()
{
	return m_fluidSimulator.getMetaballs();
}