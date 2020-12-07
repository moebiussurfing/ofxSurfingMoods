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
	//default structure is: 
	//3 moods/ranges (with 3 targets each) 
	//9 states/targets.
	//9 presets for each receiver A-B-C.
	//splitting the 3 moods/ranges: limit 0-1 target 3, limit 1-2 target 6.

	//-

	moodMachine.setGui_AdvancedVertical_MODE(false);
	moodMachine.setPosition(20, 20);//gui panel position

	//-

	//callbacks listeners for inside class moodMachine
	moodMachine.TARGET_Selected.addListener(this, &ofApp::Changed_Mood_TARGET);
	moodMachine.PRESET_A_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_A);
	moodMachine.PRESET_B_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_B);
	moodMachine.PRESET_C_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_C);
	moodMachine.RANGE_Selected.addListener(this, &ofApp::Changed_Mood_RANGE);
}

//--------------------------------------------------------------
void ofApp::update()
{
	moodMachine.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	moodMachine.drawPreview();

	////customized position: bottom centered
	//int w = 500;
	//int h = 50;
	//int x = ofGetWidth() / 2. - w * 0.5;
	//int y = ofGetHeight() - h - 40;//TODO: there's a little offset...
	//moodMachine.drawPreview(x, y, w, h);
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
void ofApp::exit()
{
	//callbacks listeners for inside class moodMachine
	moodMachine.TARGET_Selected.removeListener(this, &ofApp::Changed_Mood_TARGET);
	moodMachine.PRESET_A_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_A);
	moodMachine.PRESET_B_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_B);
	moodMachine.PRESET_C_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_C);
	moodMachine.RANGE_Selected.removeListener(this, &ofApp::Changed_Mood_RANGE);
}


//--------------------------------------------------------------

//callbacks to look inside the moodMachine

//we can apply current targets and presets to whatever we want into our OF_APP.
//we can modify other things depending on mood range too.

//--------------------------------------------------------------
void ofApp::Changed_Mood_TARGET(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_A(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_B(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_C(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << targetVal;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_RANGE(int &targetVal)
{
	ofLogNotice(__FUNCTION__) << targetVal;

	// example: change thr background color reflecting the Mood/Range
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
