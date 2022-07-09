#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetCircleResolution(100);

	gradient.addColor(ofColor::red);
	gradient.addColor(ofColor::yellow);
	gradient.addColor(ofColor::green);

	//-

	setupMoods();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Circle will be filled with gradient colors to visualize Preset A index
	ofPushStyle();
	ofSetColor(colorCircle);
	ofFill();
	float r = ofGetHeight() * 0.2;
	float x = ofGetWidth() * 0.5;
	float y = ofGetHeight() * 0.5;
	ofDrawCircle(x, y, r);
	ofSetLineWidth(2.0);
	ofNoFill();
	ofSetColor(64, 200);
	ofDrawCircle(x, y, r);
	ofPopStyle();

	//-

	// Gui
	moods.draw_ImGui();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == OF_KEY_F1) { bGui = !bGui; }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
	moods.windowResized(w, h);
}

//--------------------------------------------------------------
void ofApp::exit()
{
	exitMoods();
}

//----


//-------------------------
// Full Snippet to copy/paste to your project ->

// Surfing Moods
//
//setupMoods();
//exitMoods();

//--------------------------------------------------------------
void ofApp::setupMoods()
{
	// Callbacks listeners from Moods
	moods.RANGE_Selected.addListener(this, &ofApp::Changed_Mood_RANGE);
	moods.TARGET_Selected.addListener(this, &ofApp::Changed_Mood_TARGET);

	moods.PRESET_A_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_A);
	moods.PRESET_B_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_B);
	moods.PRESET_C_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_C);

	//--

	moods.setup();

	// Default structure is:
	// 3 Moods/ranges (with 3 targets each)
	// 9 States/targets.
	// 9 Presets for each preset receiver A-B-C.
	// Splitting the 3 moods/ranges:
	// Limit 0-1 target 3, limit 1-2 target 6.
}

//--------------------------------------------------------------

// Callbacks to look inside the moods
// We can apply current targets and presets to whatever we want into our OF_APP.
// We can modify other things depending on mood range too.

//--------------------------------------------------------------
void ofApp::Changed_Mood_TARGET(int &index)
{
	ofLogNotice(__FUNCTION__) << index;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_A(int &index)
{
	ofLogNotice(__FUNCTION__) << index;

	// Change the circle color reflecting the Preset A index
	float val = ofMap(index, 0, 8, 0.f, 1.f);
	colorCircle = gradient.getColorAtPercent(val);
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_B(int &index)
{
	ofLogNotice(__FUNCTION__) << index;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_C(int &index)
{
	ofLogNotice(__FUNCTION__) << index;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_RANGE(int &index)
{
	ofLogNotice(__FUNCTION__) << index;

	// Change the background color reflecting the Mood/Range

	if (index == 0)
	{
		ofBackground(ofColor::indianRed);
	}
	else if (index == 1)
	{
		ofBackground(ofColor::yellow);
	}
	else if (index == 2)
	{
		ofBackground(ofColor::lightGreen);
	}
}

//--------------------------------------------------------------
void ofApp::exitMoods()
{
	// Callbacks listeners from moods

	moods.RANGE_Selected.removeListener(this, &ofApp::Changed_Mood_RANGE);

	moods.TARGET_Selected.removeListener(this, &ofApp::Changed_Mood_TARGET);

	moods.PRESET_A_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_A);
	moods.PRESET_B_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_B);
	moods.PRESET_C_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_C);
}
