<<<<<<< HEAD
#pragma once

#include <memory>
#include <map>
#include <string>
#include <array>
#include <glm/vec4.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>

#include <gloperate/painter/Painter.h>

<<<<<<< HEAD
=======
#include "FluidSimulator.h"

>>>>>>> Physik im raycast Renderer eingebunden
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

	const gloperate::AbstractTargetFramebufferCapability * targetFramebufferCapability() const;
	const gloperate::AbstractViewportCapability * viewportCapability() const;
	const gloperate::AbstractPerspectiveProjectionCapability * projectionCapability() const;
	const gloperate::AbstractCameraCapability * cameraCapability() const;

<<<<<<< HEAD
	const std::array<glm::vec4, METABALLSCOUNT> & metaballs() const;

=======
>>>>>>> Physik im raycast Renderer eingebunden
private:
	
	bool m_raycasting;
<<<<<<< HEAD
	bool m_SSF;

	std::unique_ptr<ScreenSpaceFluidRenderer> m_SSFRenderer;
	std::unique_ptr<RaycastingRenderer> m_rayRenderer;
=======
	std::unique_ptr<AbstractRenderer> m_rayRenderer;
	std::unique_ptr<AbstractRenderer> m_otherRenderer;
	FluidSimulator m_fluidSimulator;
>>>>>>> Physik im raycast Renderer eingebunden


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
<<<<<<< HEAD

	std::array<glm::vec4, METABALLSCOUNT> m_metaballs;
=======
>>>>>>> Physik im raycast Renderer eingebunden
};
=======
#pragma once

#include <memory>
#include <map>
#include <string>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>

#include <gloperate/painter/Painter.h>

#include "FluidSimulator.h"

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

private:
	bool m_other;
	bool m_raycasting;
	std::unique_ptr<AbstractRenderer> m_rayRenderer;
	std::unique_ptr<AbstractRenderer> m_otherRenderer;
	FluidSimulator m_fluidSimulator;


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
};
>>>>>>> added environment map
