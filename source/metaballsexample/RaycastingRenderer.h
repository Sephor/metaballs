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

class RaycastingRenderer: public AbstractRenderer
{
public:
	RaycastingRenderer() = delete;
	RaycastingRenderer(gloperate::AbstractViewportCapability * viewportCapability, gloperate::AbstractPerspectiveProjectionCapability * projectionCapability, gloperate::AbstractCameraCapability * cameraCapability);
	~RaycastingRenderer();

	void initialize();
	void draw(globjects::ref_ptr<globjects::VertexArray> & vao);

private:
	gloperate::AbstractViewportCapability * m_viewportCapability;
	gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
	gloperate::AbstractCameraCapability * m_cameraCapability;

	std::array<glm::vec4, 16> m_metaballs;
};