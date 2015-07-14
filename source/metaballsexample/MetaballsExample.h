#pragma once

#include <memory>
#include <map>
#include <string>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/painter/Painter.h>

#include "FluidSimulator.h"
#include "RaycastingRenderer.h"
#include "ScreenSpaceFluidRenderer.h"

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
	bool getSSF() const;
	void setSSF(bool value);
	bool getBlur() const;
	void setBlur(bool value);

	const std::vector<glm::vec4> getMetaballs();

	const gloperate::AbstractTargetFramebufferCapability * targetFramebufferCapability() const;
	const gloperate::AbstractViewportCapability * viewportCapability() const;
	const gloperate::AbstractPerspectiveProjectionCapability * projectionCapability() const;
	const gloperate::AbstractCameraCapability * cameraCapability() const;

private:
	bool m_other;
	bool m_raycasting;
	bool m_blur;
	RaycastingRenderer m_rayRenderer;
	ScreenSpaceFluidRenderer m_SSFRenderer;
	FluidSimulator m_fluidSimulator;
	bool m_SSF;

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
};
