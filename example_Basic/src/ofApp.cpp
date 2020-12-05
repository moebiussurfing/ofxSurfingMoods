#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(30);

	ofSetLogLevel("ofApp", OF_LOG_NOTICE);
	ofSetLogLevel("ofxSurfingMoods", OF_LOG_VERBOSE);
	//ofSetLogLevel("ofxSurfingMoods", OF_LOG_SILENT);

	//-


	GEN.setup(9, 8, 3, 6);
	//9 targets, 8 presets, 8 patterns, 
	//3 ranges/moods: range0 starts at 0. rage 1 starts at limit1 3, and range 2 starts at limit2 6

	//GEN.setup();
	//with default 16 targets, 16 presets, 8 patterns, limit1 4, limit2 11

	GEN.setGui_AdvancedVertical_MODE(false);
	GEN.setPosition(20, 20);//gui panel position

	//-

	//calbacks listeners for inside class GEN
	GEN.TARGET_Selected.addListener(this, &ofApp::Changed_GEN_TARGET);
	GEN.PRESET_A_Selected.addListener(this, &ofApp::Changed_GEN_PRESET_A);
	GEN.PRESET_B_Selected.addListener(this, &ofApp::Changed_GEN_PRESET_B);
	GEN.PRESET_C_Selected.addListener(this, &ofApp::Changed_GEN_PRESET_C);
	GEN.RANGE_Selected.addListener(this, &ofApp::Changed_GEN_MOOD_RANGE);
}

//--------------------------------------------------------------
void ofApp::update()
{
	GEN.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	GEN.drawPreview();

	////customized
	//int x = 400;
	//int y = 100;
	//GEN.drawPreview(x, y, ofGetWidth() - x -100, ofGetHeight()*0.2f - y);
}

//--------------------------------------------------------------
void ofApp::exit()
{
	GEN.exit();

	//-

	//calbacks listeners for inside class GEN
	GEN.TARGET_Selected.removeListener(this, &ofApp::Changed_GEN_TARGET);
	GEN.PRESET_A_Selected.removeListener(this, &ofApp::Changed_GEN_PRESET_A);
	GEN.PRESET_B_Selected.removeListener(this, &ofApp::Changed_GEN_PRESET_B);
	GEN.PRESET_C_Selected.removeListener(this, &ofApp::Changed_GEN_PRESET_C);
	GEN.RANGE_Selected.removeListener(this, &ofApp::Changed_GEN_MOOD_RANGE);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == ' ')
	{
		GEN.playSwitch();
	}

	else if (key == 'p')
	{
		GEN.play();
	}

	else if (key == 's')
	{
		GEN.stop();
	}

	else if (key == '1')
	{
		GEN.setRange(0);
	}

	else if (key == '2')
	{
		GEN.setRange(1);
	}

	else if (key == '3')
	{
		GEN.setRange(2);
	}
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
	GEN.windowResized(w, h);
}

//--------------------------------------------------------------
//listeners for inside class GEN

void ofApp::Changed_GEN_TARGET(int &targetVal)
{
	ofLogNotice("ofApp") << "GEN_TARGET: " << targetVal;
}

void ofApp::Changed_GEN_PRESET_A(int &targetVal)
{
	ofLogNotice("ofApp") << "GEN_PRESET_A: " << targetVal;
}

void ofApp::Changed_GEN_PRESET_B(int &targetVal)
{
	ofLogNotice("ofApp") << "GEN_PRESET_B: " << targetVal;
}

void ofApp::Changed_GEN_PRESET_C(int &targetVal)
{
	ofLogNotice("ofApp") << "GEN_PRESET_C: " << targetVal;
}

void ofApp::Changed_GEN_MOOD_RANGE(int &targetVal)
{
	ofLogNotice("ofApp") << "GEN_MOOD_RANGE: " << targetVal;

	//if (targetVal == 0)
	//{
	//    ofBackground(color_MOOD1);
	//}
	//if (targetVal == 1)
	//{
	//    ofBackground(color_MOOD2);
	//}
	//if (targetVal == 2)
	//{
	//    ofBackground(color_MOOD3);
	//}
}
