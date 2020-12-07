#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);

	//ofSetLogLevel(OF_LOG_SILENT);
	//ofSetLogLevel("ofApp", OF_LOG_SILENT);
	//ofSetLogLevel("ofxSurfingMoods", OF_LOG_SILENT);

	//--

	//ofxSurfingMoods

	moodMachine.setup();

	//customize sizes (TODO: maybe not working..)
	//moodMachine.setup(9, 9, 3, 6);
	////9 targets, 9 presets x3 (ABC), limit1 3, limit2 6
	////3 ranges/moods: range0 starts at 0. rage 1 starts at limit1 3, and range 2 starts at limit2 6

	//-

	moodMachine.setGui_AdvancedVertical_MODE(false);
	moodMachine.setPosition(20, 20);//gui panel position

	//--

	//callbacks listeners for inside class moodMachine
	moodMachine.TARGET_Selected.addListener(this, &ofApp::Changed_GEN_TARGET);
	moodMachine.PRESET_A_Selected.addListener(this, &ofApp::Changed_GEN_PRESET_A);
	moodMachine.PRESET_B_Selected.addListener(this, &ofApp::Changed_GEN_PRESET_B);
	moodMachine.PRESET_C_Selected.addListener(this, &ofApp::Changed_GEN_PRESET_C);
	moodMachine.RANGE_Selected.addListener(this, &ofApp::Changed_GEN_MOOD_RANGE);
}

//--------------------------------------------------------------
void ofApp::update()
{
	moodMachine.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	////next to the gui
	moodMachine.drawPreview();

	////customized position: bottom centered
	//int w = 500;
	//int h = 50;
	//int x = ofGetWidth() / 2. - w * 0.5;
	//int y = ofGetHeight() - h - 40;//TODO: there's a little offset...
	//moodMachine.drawPreview(x, y, w, h);
}

//--------------------------------------------------------------
void ofApp::exit()
{
	//calbacks listeners for inside class moodMachine
	moodMachine.TARGET_Selected.removeListener(this, &ofApp::Changed_GEN_TARGET);
	moodMachine.PRESET_A_Selected.removeListener(this, &ofApp::Changed_GEN_PRESET_A);
	moodMachine.PRESET_B_Selected.removeListener(this, &ofApp::Changed_GEN_PRESET_B);
	moodMachine.PRESET_C_Selected.removeListener(this, &ofApp::Changed_GEN_PRESET_C);
	moodMachine.RANGE_Selected.removeListener(this, &ofApp::Changed_GEN_MOOD_RANGE);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	moodMachine.keyPressed(key);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
	moodMachine.windowResized(w, h);
}


//--------------------------------------------------------------

//listeners for inside class moodMachine

//we can apply current targets and presets to whatever we want into our OF_APP.
//we can modify other things depending on mood range too.

//--------------------------------------------------------------
void ofApp::Changed_GEN_TARGET(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << "GEN_TARGET: " << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_GEN_PRESET_A(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << "GEN_PRESET_A: " << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_GEN_PRESET_B(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << "GEN_PRESET_B: " << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_GEN_PRESET_C(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << "GEN_PRESET_C: " << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_GEN_MOOD_RANGE(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << "GEN_MOOD_RANGE: " << targetVal;

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
