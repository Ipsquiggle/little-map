#pragma once
#include "Stage.h"
#include "ofMain.h"

class Saver : public Stage
{
public:
	Saver();
	~Saver();

	virtual void Setup();
	virtual void Render();
	virtual void Draw();
};