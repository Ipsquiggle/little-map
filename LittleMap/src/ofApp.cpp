#include "ofApp.h"

#include "Start.h"
#include "CurveTerrain.h"
#include "LatLon.h"
#include "Landmarks.h"
#include "Paths.h"
#include "Saver.h"


char* nextMessage;
void statusMessage(char* message)
{
	if (message != nullptr)
		printf("%s\n", message);
	nextMessage = message;
}

char* subMessage;
void statusMessage2(char* message)
{
	if (message != nullptr)
		printf("%s\n", message);
	subMessage = message;
}

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetWindowTitle("Tiny Map");

	ofBackground(0, 0, 0);
	ofEnableSmoothing();

	autoAdvance = true;
	doneStep = false;

	stages = new Stage*[(int)step::done];
	stages[(int)step::start] = new Start();
	CurveTerrain *terrain = new CurveTerrain(false, false);
	stages[(int)step::islands] = terrain;
	stages[(int)step::lines] = new LatLon();
	Landmarks *landmarks = new Landmarks(*terrain);
	stages[(int)step::landmarks] = landmarks;
	stages[(int)step::paths] = new Paths(*terrain, *landmarks, 0);
	stages[(int)step::paper] = NULL;
	stages[(int)step::legend] = NULL;
	stages[(int)step::save] = new Saver();

	for (int i = 0; i < (int)step::done; i++)
	{
		if (stages[i] != NULL)
			stages[i]->Setup();
	}
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (!doneStep && currentStep < step::done)
	{
		if (stages[(int)currentStep] != NULL)
		{
			doneStep = stages[(int)currentStep]->Render();
		}
		else
		{
			statusMessage2("Done");
			doneStep = true;
		}

		if (autoAdvance && doneStep)
		{
			Advance();
		}
	}
}

void ofApp::Advance()
{
	doneStep = false;
	statusMessage2(nullptr);

	currentStep = (step)(((int)currentStep) + 1);
	switch (currentStep)
	{
	case (step::start):
		statusMessage("Start");
		break;
	case(step::islands):
		statusMessage("Islands");
		break;
	case(step::lines):
		statusMessage("Lines");
		break;
	case(step::landmarks):
		statusMessage("Landmarks");
		break;
	case(step::paths):
		statusMessage("Paths");
		break;
	case(step::paper):
		statusMessage("Paper");
		break;
	case(step::legend):
		statusMessage("Legend");
		break;
	case(step::save):
		statusMessage("Save");
		break;
	case(step::done):
		statusMessage("Done");
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
int i = 0;

void ofApp::draw()
{
	for (int i = 0; i < (int)step::done; i++)
	{
		if (stages[i] != NULL)
			stages[i]->Draw();
	}

	if (nextMessage != nullptr)
	{
		ofSetColor(ofColor::black);
		ofDrawBitmapString(nextMessage, 11, ofGetHeight() - 19);
		ofSetColor(ofColor::white);
		ofDrawBitmapString(nextMessage, 10, ofGetHeight() - 20);
	}
	if (currentStep < (int)step::done && stages[currentStep] != nullptr)
	{
		char* stageMessage = stages[currentStep]->GetMessage();
		if (stageMessage != nullptr)
		{
			ofSetColor(ofColor::black);
			ofDrawBitmapString(stageMessage, 11, ofGetHeight() - 9);
			ofSetColor(ofColor::white);
			ofDrawBitmapString(stageMessage, 10, ofGetHeight() - 10);
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	int targetStep = ((int)currentStep) + 1;
	if (key == ' ')
	{
		if (doneStep)
			Advance();
	}
	else if (key >= '0' && key <= '9')
	{
		if (currentStep < (int)done)
			stages[currentStep]->DebugNum(key);
	}
	else if (key == ')')
		targetStep = 0;
	else if (key == '!')
		targetStep = 1;
	else if (key == '@')
		targetStep = 2;
	else if (key == '#')
		targetStep = 3;
	else if (key == '$')
		targetStep = 4;
	else if (key == '%')
		targetStep = 5;
	else if (key == '^')
		targetStep = 6;
	else if (key == '&')
		targetStep = 7;
	else if (key == '*')
		targetStep = 8;
	else if (key == '(')
		targetStep = 9;

	bool reset = false;
	for (int i = currentStep; i >= targetStep; i--)
	{
		if (stages[i] != nullptr)
			stages[i]->Reset();
		reset = true;
	}
	if (reset)
	{
		autoAdvance = false;
		currentStep = (step)(targetStep-1);
		Advance();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	if (stages[currentStep] != nullptr)
	{
		stages[currentStep]->DebugClick(x, y);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::exit()
{
	if (stages[(int)save] != nullptr)
	{
		stages[(int)save]->Render();
	}
}
