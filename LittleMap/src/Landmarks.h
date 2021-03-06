#pragma once
#include "ofMain.h"
#include "Stage.h"

#include "CurveTerrain.h"

#include <vector>

class Landmarks : public Stage
{
public:
	Landmarks(CurveTerrain &terrain);
	~Landmarks();

	virtual void Setup();
	virtual bool Render();
	virtual void Draw();
	virtual void Reset();

	struct Landmark {
		ofPoint pos;
		int iconIdx;
		float onLand;
	};

	const vector<Landmark> GetLandmarks();
	ofRectangle DrawIcon(int idx, ofPoint pt);
	Landmark GetRandomLandmark();
	Landmark GetNthClosestLandmark(Landmark landmark, int n);

private:
	CurveTerrain &terrain;

	vector<ofFile> files;
	vector<ofImage> icons;
	vector<Landmark> landmarks;

	ofFbo image;
};
