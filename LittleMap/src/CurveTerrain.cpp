#include "CurveTerrain.h"
#include "Noise.h"

const int cellSize = 10;
const float noiseScale = 0.015f;
const int noiseOctaves = 5;


CurveTerrain::CurveTerrain(bool debug, bool drawNoise)
{
	this->debug = debug;
	this->drawNoise = drawNoise;

	landColor[0] = ofColor( 90, 140, 195, 255);
	landColor[1] = ofColor(110, 160, 215, 255);
	landColor[2] = ofColor(130, 180, 235, 255);
	landColor[3] = ofColor(150, 200, 255, 255);
	landColor[4] = ofColor( 90, 195, 140, 255);
	landColor[5] = ofColor(110, 215, 160, 255);
	landColor[6] = ofColor(130, 235, 180, 255);
	landColor[7] = ofColor(150, 255, 200, 255);
	lineColor = ofColor::black;
}

CurveTerrain::~CurveTerrain()
{
}

void CurveTerrain::Setup()
{
	SetupTiles();

	cellWidth = ofGetWidth() / cellSize;
	cellHeight = ofGetHeight() / cellSize;
	cells = new Cell[cellWidth * cellHeight];

	noiseMap = new float[ofGetWidth() * ofGetHeight()];

	Reset();
}

void CurveTerrain::Reset()
{
	for (int y = 0; y < ofGetHeight(); y++)
	{
		for (int x = 0; x < ofGetWidth(); x++)
		{
			float landValue = ComputeLandValue(x, y);
			noiseMap[x + y * ofGetWidth()] = landValue;
		}
	}

	render_x = 0;
	render_y = 0;

	if (image.isAllocated())
		image.clear();
	image.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	image.begin();
	ofClear(0, 0, 0, 0);
	image.end();
}

void CurveTerrain::RenderNoiseMap()
{
	ofPixels pixels = ofPixels();
	image.readToPixels(pixels);

	for (int y = 0; y < ofGetHeight(); y++)
	{
		for (int x = 0; x < ofGetWidth(); x++)
		{
			float landValue = GetLandValue(x, y);
			ofColor color = ofColor((landValue + 0.45f) * 255);
			pixels.setColor(x, y, color);
		}
	}
	image.getTexture().loadData(pixels);
}

float CurveTerrain::ComputeLandValue(float x, float y)
{
	return Noise(x*noiseScale, y*noiseScale, noiseOctaves, 0.5f, 0.6f) - 0.45f;
}

float Clamp(float v, float min, float max)
{
	return std::min(max, std::max(min, v));
}

float CurveTerrain::GetLandValue(float x, float y)
{
	int ix = (int)std::floor(Clamp(x, 0, ofGetWidth()-1));
	int iy = (int)std::floor(Clamp(y, 0, ofGetHeight()-1));
	return noiseMap[ix + iy * ofGetWidth()];
}

// Takes floats but expects cell coordinates.
float CurveTerrain::OnLand(float x, float y)
{
	// note this test is for cellWidth, not cellWidth-1, because we test both the left
	// and right edge of a cell, i.e. the final cell we test's right edge is actually
	// at cellWidth, even though the cell's index is cellWidth-1
	if (x == 0 || y == 0 || x == cellWidth || y == cellHeight)
		return -0.001f; // just a little bit ocean at the edges

	return GetLandValue(x * cellSize, y * cellSize);
}

void CurveTerrain::Biases(float biases[4], int x, int y)
{
	biases[0] = OnLand(x, y);
	biases[1] = OnLand(x + 1, y);
	biases[2] = OnLand(x, y + 1);
	biases[3] = OnLand(x + 1, y + 1);
}

void CurveTerrain::BHits(float biases[4], int hits[4])
{

	hits[0] = (int)(biases[0] > 0);
	hits[1] = (int)(biases[1] > 0);
	hits[2] = (int)(biases[2] > 0);
	hits[3] = (int)(biases[3] > 0);
}

void CurveTerrain::Hits(int hits[4], int x, int y)
{
	float biases[4];
	Biases(biases, x, y);
	BHits(biases, hits);
}

CurveTerrain::Tile CurveTerrain::TileForPos(int x, int y, float biases[4])
{
	Biases(biases, x, y);
	int hits[4];
	BHits(biases, hits);

	if (debug)
	{
		ofNoFill();
		ofSetColor(150, 150, 150, 50);
		ofDrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize);
		ofFill();
		for (int i = 0; i < 4; i++)
		{
			int ix = x * cellSize + (cellSize / 8) + (i % 2) * cellSize * 6 / 8;
			int iy = y * cellSize + (cellSize / 8) + (i / 2) * cellSize * 6 / 8;
			if (hits[i])
				ofSetColor(0, 150, 0, 255);
			else
				ofSetColor(200, 0, 0, 255);
			ofDrawCircle(ix, iy, 2);
		}
	}

	int index = hits[3] << 3 | hits[2] << 2 | hits[1] << 1 | hits[0];
	return tiles[index];
}

ofPoint CurveTerrain::PointForDir(dir d, float bias[4])
{
	float div = 0;
	float b = 0;
	switch (d)
	{
	case top:
		div = (bias[0] - bias[1]);
		b = div == 0 ? 0.5f : bias[0] / div;
		return ofPoint(b, 0);
	case left:
		div = (bias[0] - bias[2]);
		b = div == 0 ? 0.5f : bias[0] / div;
		return ofPoint(0, b);
	case right:
		div = (bias[1] - bias[3]);
		b = div == 0 ? 0.5f : bias[1] / div;
		return ofPoint(1.0f, b);
	case bottom:
		div = (bias[2] - bias[3]);
		b = div == 0 ? 0.5f : bias[2] / div;
		return ofPoint(b, 1.0f);
	}
}

CurveTerrain::dir CurveTerrain::PairDir(CurveTerrain::dir d)
{
	switch (d)
	{
	case top: return bottom;
	case bottom: return top;
	case left: return right;
	case right: return left;
	}
}

ofPoint CurveTerrain::DirOffset(CurveTerrain::dir d)
{
	switch (d)
	{
	case top: return ofPoint(0, -1);
	case bottom: return ofPoint(0, 1);
	case left: return ofPoint(-1, 0);
	case right: return ofPoint(1, 0);
	}
}

void CurveTerrain::DrawLink(int x, int y, dir start, dir end, float bias[4])
{
	ofPoint corner(x*cellSize, y*cellSize);
	ofPoint ps = corner + PointForDir(start, bias) * cellSize;
	ofPoint pe = corner + PointForDir(end, bias) * cellSize;
	ofDrawLine(ps, pe);
}

ofPoint CurveTerrain::LinkPos(int x, int y, dir end, float bias[4])
{
	ofPoint corner(x*cellSize, y*cellSize);
	return corner + PointForDir(end, bias) * cellSize;
}

void CurveTerrain::NextCell(int x, int y, CurveTerrain::dir currentDir, int &outx, int &outy, CurveTerrain::dir &nextDir)
{
	Cell &current = cells[y*cellWidth + x];
	ofPoint offset = DirOffset(current.tile.links[currentDir]);
	outx = x + offset.x;
	outy = y + offset.y;
	nextDir = PairDir(current.tile.links[currentDir]);
}

bool CurveTerrain::DrawIsland(int x, int y)
{
	// the first cell in the link is kinda weird.
	Cell *first = &cells[y*cellWidth + x];
	Cell *next = first;
	// pick the first link we can find in the cell.
	dir d = none;
	for (int i = 0; i < 4; i++)
	{
		if (first->tile.links[i] != none)
		{
			d = (dir)i;
		}
	}
	
	if (d == none)
		return false;

	ofPolyline path = ofPolyline();
	//path.setStrokeWidth(3);
	//path.setStrokeColor(lineColor);

	ofPoint linkPos = LinkPos(x, y, d, next->bias);
	// ofPath doesn't draw the first or last points, they are just control points for the curve, 
	// so we will draw the entrance and exit points in the start cell.
	// This makes the first Drawn point the exit of the first cell.
	path.curveTo(linkPos);
	do {
		next->visited = true;
		linkPos = LinkPos(x, y, next->tile.links[d], next->bias);
		path.curveTo(linkPos);

		NextCell(x, y, d, x, y, d);
		next = &cells[y*cellWidth + x];
	} while (next != first);
	// ... and since the last Drawn point also needs, to be the exit of the first cell,
	// we draw through the first cell AGAIN, and the second cell AGAIN! Nice....
	linkPos = LinkPos(x, y, next->tile.links[d], next->bias);
	path.curveTo(linkPos);

	NextCell(x, y, d, x, y, d);
	next = &cells[y*cellWidth + x];

	linkPos = LinkPos(x, y, next->tile.links[d], next->bias);
	path.curveTo(linkPos);

	ofSetColor(next->tile.drawLand ? landColor[4] : landColor[3]);
	ofFill();
	ofEnableSmoothing();

	if (!debug)
	{
		ofTessellator tesselator = ofTessellator();
		ofMesh mesh;
		tesselator.tessellateToMesh(path, ofPolyWindingMode::OF_POLY_WINDING_ODD, mesh);
		mesh.draw();
	}

	if (!debug)
	{
		ofSetColor(ofColor::black);
	}
	ofFill();
	ofEnableSmoothing();
	float len = 0;
	while (path.getIndexAtLength(len) < path.size()-1) // apparently size - 1....
	{
		ofPoint pt = path.getPointAtLength(len);
		
		ofDrawCircle(pt, ofRandom(1.5f, 3.0f));

		len += 1;
	}

	return true;
}

void CurveTerrain::RenderBegin()
{
	image.begin();
	if (debug)
	{
		ofClear(ofColor::white);
	}
	else
	{
		//ofClear(ofColor::white);
		ofClear(landColor[3]);
	}

	ofSetColor(0, 0, 255, 255);
	ofSetLineWidth(1);
	for (int y = 0; y < cellHeight; y++)
	{
		for (int x = 0; x < cellWidth; x++)
		{
			float bias[4];
			float defaultBias[4]{ 0,0,0,0 };
			Tile t = TileForPos(x, y, bias);
			cells[y*cellWidth + x] = { t, false, bias[0], bias[1], bias[2], bias[3] };
			if (debug)
			{
				for (int l = 0; l < 4; l++)
				{
					if (t.links[l] != none)
					{
						ofSetColor(ofColor::red);
						DrawLink(x, y, (CurveTerrain::dir)l, t.links[l], defaultBias);
						ofSetColor(ofColor::black);
						DrawLink(x, y, (CurveTerrain::dir)l, t.links[l], bias);
					}
				}
			}
		}
	}
}

void CurveTerrain::RenderStep()
{
	Cell c = cells[render_y*cellWidth + render_x];
	if (c.visited == false)
	{
		bool newIsland = DrawIsland(render_x, render_y);
		if (debug)
		{
			if (newIsland)
				ofSetColor(0, 255, 0, 30);
			else
				ofSetColor(255, 0, 0, 30);
		}
	}
	else if (debug)
	{
		ofSetColor(0, 0, 255, 30);
	}

	if (debug)
	{
		ofFill();
		ofDrawRectangle(render_x * cellSize, render_y * cellSize, cellSize, cellSize);
	}
}

bool CurveTerrain::DoRender()
{
	if (render_y == cellHeight)
		return true;

	if (render_x == 0 && render_y == 0)
	{
		RenderBegin();
	}
	else
	{
		image.begin();
	}

	RenderStep();

	image.end();

	render_x++;
	if (render_x == cellWidth)
	{
		render_y++;
		render_x = 0;
	}

	return false;
}

bool CurveTerrain::Render()
{
	if (debug)
	{
		if (drawNoise)
		{
			RenderNoiseMap();
		}
		DoRender();
		return false;
		//return DoRender();
	}
	else
	{
		while (!DoRender()) {}
		return true;
	}
}

void CurveTerrain::Draw()
{
	if(image.isAllocated())
		image.draw(0, 0);
}

void CurveTerrain::SetupTiles()
{
	// RULE IS: 1 on the left, 0 on the right

	// 0 0
	// 0 0
	tiles[0] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::none} };
	// 1 0
	// 0 0
	tiles[1] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::top, CurveTerrain::dir::none,
				CurveTerrain::dir::none},
			false };
	// 0 1
	// 0 0
	tiles[2] = { {CurveTerrain::dir::right,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::none},
			true };
	// 1 1
	// 0 0
	tiles[3] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::right, CurveTerrain::dir::none,
				CurveTerrain::dir::none},
			false };
	// 0 0
	// 1 0
	tiles[4] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::left} };
	// 1 0
	// 1 0
	tiles[5] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::top},
			false };
	// 0 1
	// 1 0
	tiles[6] = { {CurveTerrain::dir::right,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::left},
			true };
	// 1 1
	// 1 0
	tiles[7] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::right},
			false };
	// 0 0
	// 0 1
	tiles[8] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::bottom,
				CurveTerrain::dir::none},
			true };
	// 1 0
	// 0 1
	tiles[9] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::top, CurveTerrain::dir::bottom,
				CurveTerrain::dir::none},
			true };
	// 0 1
	// 0 1
	tiles[10] = { {CurveTerrain::dir::bottom,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::none},
			true };
	// 1 1
	// 0 1
	tiles[11] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::bottom, CurveTerrain::dir::none,
				CurveTerrain::dir::none} };
	// 0 0
	// 1 1
	tiles[12] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::left,
				CurveTerrain::dir::none} };
	// 1 0
	// 1 1
	tiles[13] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::top,
				CurveTerrain::dir::none},
			false };
	// 0 1
	// 1 1
	tiles[14] = { {CurveTerrain::dir::left,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::none},
			true };
	// 1 1
	// 1 1
	tiles[15] = { {CurveTerrain::dir::none,
		CurveTerrain::dir::none, CurveTerrain::dir::none,
				CurveTerrain::dir::none} };
}

