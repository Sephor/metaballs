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

#include <globjects/VertexAttributeBinding.h>
#include <globjects/Texture.h>
#include <globjects/AttachedTexture.h>

#include "RaycastingRenderer.h"

MetaballsExample::MetaballsExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
,	m_raycasting(true)
,	m_rayRenderer(std::make_unique<RaycastingRenderer>())
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

    m_texture = new globjects::Texture;
    m_texture->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

    m_fbo = new globjects::Framebuffer;
    m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_texture, 0);

	m_vertices = new globjects::Buffer;
	m_vertices->setData(std::vector<float>{
		-0.8f, -0.8f,
			0.8f, -0.8f,
			0.8f, 0.8f
	}, gl::GL_STATIC_DRAW);

	m_vao = new globjects::VertexArray;

	auto binding = m_vao->binding(0);
	binding->setAttribute(0);
	binding->setBuffer(m_vertices, 0, 2 * sizeof(float));
	binding->setFormat(2, gl::GL_FLOAT);

	m_vao->enable(0);

    gl::glClearColor(1.0, 1.0, 1.0, 1.0);

    m_fbo->unbind();

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

	m_fbo->bind();
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);

	if (m_raycasting)
		m_rayRenderer->draw(m_vao);

	std::array<int, 4> sourceRect = {{ 0, 0, m_viewportCapability->width(), m_viewportCapability->height() }};
    std::array<int, 4> destRect = {{ 0, 0, m_viewportCapability->width(), m_viewportCapability->height() }};

    globjects::Framebuffer * targetFBO = m_targetFramebufferCapability->framebuffer() ? m_targetFramebufferCapability->framebuffer() : globjects::Framebuffer::defaultFBO();

	targetFBO->bind(gl::GL_DRAW_FRAMEBUFFER);

    m_fbo->blit(gl::GL_COLOR_ATTACHMENT0, sourceRect, targetFBO, gl::GL_BACK_LEFT, destRect, gl::GL_COLOR_BUFFER_BIT, gl::GL_NEAREST);

	globjects::Framebuffer::unbind();
}

bool MetaballsExample::getRaycasting() const
{
	return m_raycasting;
}

void MetaballsExample::setRaycasting(bool value)
{
	m_raycasting = value;
}

void MetaballsExample::setupPropertyGroup()
{
	addProperty<bool>("Raycasting", this,
		&MetaballsExample::getRaycasting, &MetaballsExample::setRaycasting);
}