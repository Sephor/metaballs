#pragma once 

#include "AbstractRenderer.h"

#include <array>

#include <glm/vec4.hpp>

namespace gloperate
{
	class AbstractViewportCapability;
	class AbstractPerspectiveProjectionCapability;
	class AbstractCameraCapability;
}

class ScreenSpaceFluidRenderer: public AbstractRenderer
{
public:
	ScreenSpaceFluidRenderer() = delete;
	ScreenSpaceFluidRenderer(gloperate::AbstractViewportCapability * viewportCapability, gloperate::AbstractPerspectiveProjectionCapability * projectionCapability, gloperate::AbstractCameraCapability * cameraCapability);
	~ScreenSpaceFluidRenderer();

	void initialize();
	void draw(globjects::ref_ptr<globjects::VertexArray> & vao);

	void computePhysiks();

private:
	gloperate::AbstractViewportCapability * m_viewportCapability;
	gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
	gloperate::AbstractCameraCapability * m_cameraCapability;

	
};