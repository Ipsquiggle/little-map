#pragma once
#include "Stage.h"
#include "ofMain.h"

class Saver : public Stage
{
public:
	Saver();
	~Saver();

	virtual void Setup();
	virtual bool Render();
	virtual void Draw();

	void Save(bool force);

private:
	bool saved;
};
