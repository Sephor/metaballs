#pragma once

#include <memory>
#include <map>
#include <string>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>

#include <gloperate/painter/Painter.h>


namespace globjects
{
    class Program;
}

namespace gloperate
{
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}

class AbstractRenderer;

class MetaballsExample : public gloperate::Painter
{
public:
    MetaballsExample(gloperate::ResourceManager & resourceManager);
    virtual ~MetaballsExample();

    void setupProjection();

	bool getOther() const;
	void setOther(bool value);
	bool getRaycasting() const;
	void setRaycasting(bool value);

	const gloperate::AbstractTargetFramebufferCapability * targetFramebufferCapability() const;
	const gloperate::AbstractViewportCapability * viewportCapability() const;
	const gloperate::AbstractPerspectiveProjectionCapability * projectionCapability() const;
	const gloperate::AbstractCameraCapability * cameraCapability() const;

	const std::array<glm::vec4, 20> & metaballs() const;

private:
	bool m_other;
	bool m_raycasting;
	std::unique_ptr<AbstractRenderer> m_rayRenderer;
	std::unique_ptr<AbstractRenderer> m_otherRenderer;

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;
	void setupPropertyGroup();

protected:
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Texture> m_texture;

	std::array<glm::vec4, 20> m_metaballs;
};
