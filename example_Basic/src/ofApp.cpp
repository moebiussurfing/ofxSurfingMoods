#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetFrameRate(25);

    ofSetCircleResolution(100);

    //-

    //circle gradient colors
    gradient.addColor( ofColor::red );
    gradient.addColor( ofColor::yellow );
    gradient.addColor( ofColor::green );

    //-

    //callbacks listeners from moodMachine
    moodMachine.TARGET_Selected.addListener(this, &ofApp::Changed_Mood_TARGET);
    moodMachine.PRESET_A_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_A);
    moodMachine.PRESET_B_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_B);
    moodMachine.PRESET_C_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_C);
    moodMachine.RANGE_Selected.addListener(this, &ofApp::Changed_Mood_RANGE);

    //--

    moodMachine.setup();
    //default structure is:
    //3 moods/ranges (with 3 targets each)
    //9 states/targets.
    //9 presets for each preset receiver A-B-C.
    //splitting the 3 moods/ranges:
    //limit 0-1 target 3, limit 1-2 target 6.
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

    //circle colored by Preset A
    ofPushStyle();
    ofSetColor(colorCircle);
    ofFill();
    float r = 100;
    float x = ofGetWidth()*0.5;
    float y = ofGetHeight()*0.5;
    ofDrawCircle(x, y, r);
    ofNoFill();
    ofSetColor(0, 200);
    ofDrawCircle(x, y, r);
    ofPopStyle();
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
    //callbacks listeners from moodMachine
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

    //change the circle color reflecting the Preset A index
    float val = ofMap(targetVal, 0, 8, 0.f, 1.f);
    colorCircle = gradient.getColorAtPercent(val);
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

    //change the background color reflecting the Mood/Range
    if (targetVal == 0)
    {
        ofBackground(ofColor::indianRed);
    }
    if (targetVal == 1)
    {
        ofBackground(ofColor::yellow);
    }
    if (targetVal == 2)
    {
        ofBackground(ofColor::lightGreen);
    }
}
