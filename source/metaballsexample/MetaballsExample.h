#pragma once

#include <memory>
#include <map>
#include <string>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
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

class RaycastingRenderer;
class ScreenSpaceFluidRenderer;

const int METABALLSCOUNT = 16;

class MetaballsExample : public gloperate::Painter
{

	std::array<glm::vec4, METABALLSCOUNT> m_metaballs;

public:
    MetaballsExample(gloperate::ResourceManager & resourceManager);
    virtual ~MetaballsExample();

    void setupProjection();

	bool getOther() const;
	void setOther(bool value);
	bool getRaycasting() const;
	void setRaycasting(bool value);
	bool getSSF() const;
	void setSSF(bool value);

private:
	
	bool m_raycasting;
	bool m_SSF;

	std::unique_ptr<ScreenSpaceFluidRenderer> m_SSFRenderer;
	std::unique_ptr<RaycastingRenderer> m_rayRenderer;


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
    globjects::ref_ptr<globjects::Buffer> m_vertices;
    globjects::ref_ptr<globjects::VertexArray> m_vao;
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Texture> m_texture;
};
