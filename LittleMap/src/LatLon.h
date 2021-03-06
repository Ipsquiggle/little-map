#pragma once
#include "Stage.h"
#include "ofMain.h"

class LatLon : public Stage
{
public:
	LatLon();
	~LatLon();

	virtual void Setup();
	virtual bool Render();
	virtual void Draw();
	virtual void Reset();

private:
	float LatLonNoise(float x, float y);

	ofFbo image;
};

