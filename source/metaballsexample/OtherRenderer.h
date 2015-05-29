#pragma once 

#include "AbstractRenderer.h"

class OtherRenderer: public AbstractRenderer
{
public:
	OtherRenderer();
	~OtherRenderer();

	void initialize();
	void draw(globjects::ref_ptr<globjects::VertexArray> & vao);
};