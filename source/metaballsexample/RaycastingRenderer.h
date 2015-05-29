#pragma once 

#include "AbstractRenderer.h"


class RaycastingRenderer: public AbstractRenderer {

public:
	RaycastingRenderer();
	~RaycastingRenderer();

	void initialize();
	void draw( globjects::ref_ptr<globjects::VertexArray>& vao);


};