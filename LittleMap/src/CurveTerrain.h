#pragma once
#include "Stage.h"
#include "ofMain.h"

class CurveTerrain : public Stage
{
public:
	CurveTerrain(bool debug);
	~CurveTerrain();

	virtual void Setup();
	virtual bool Render();
	virtual void Draw();


	enum dir {
		top,
		left,
		right,
		bottom,
		none,
	};

	struct Tile {
		dir links[4];
	};
	struct Cell {
		Tile tile;
		bool visited;
		float bias[4];
	};

private:
	// config
	bool debug = false;
	ofColor waterColor;
	ofColor landColor;
	ofColor lineColor;

	void RenderBegin();
	void RenderStep();
	bool DoRender();

	int render_x;
	int render_y;

	ofFbo image;

	void Biases(float biases[4], int x, int y);
	void BHits(float biases[4], int hits[4]);
	void Hits(int hits[4], int x, int y);
	float OnLand(int x, int y);
	Tile TileForPos(int x, int y, float bias[4]);

	ofPoint PointForDir(dir d, float bias[4]);
	dir PairDir(dir d);
	ofPoint DirOffset(dir d);
	void DrawLink(int x, int y, dir start, dir end, float bias[4]);
	ofPoint LinkPos(int x, int y, dir end, float bias[4]);
	void NextCell(int x, int y, dir currentDir, int &outx, int &outy, dir &nextDir);
	bool DrawIsland(int cellx, int celly);

	void SetupTiles();
	Tile tiles[16];

	int cellWidth;
	int cellHeight;
	Cell* cells;
};

