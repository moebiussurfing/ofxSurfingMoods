#pragma once

#include "ofMain.h"

#include "ofxSurfingMoods.h"

#include "ofxColorGradient.h" // for the example only
#include "ofxWindowApp.h" // for the example only

/*
	
	NOTE:
	
	1. You can start testing by moving the Target slide
	with the player stopped.
	Each target is linked to 3 Presets that you can set.
	
	2. Then, pick a Mode and hit Play.

	3. On Mode Manual, you can drag the Control slider to set a Range. The machine must be playing.

*/


class ofApp: public ofBaseApp{

public:

    void setup();
    void update();
    void draw();
    void exit();
    void keyPressed(int key);
    void windowResized(int w, int h);

    ofxSurfingMoods moodMachine;

    // callbacks from moodMachine
    void Changed_Mood_TARGET(int & targetVal);
	void Changed_Mood_PRESET_A(int & targetVal);
    void Changed_Mood_PRESET_B(int & targetVal);
    void Changed_Mood_PRESET_C(int & targetVal);
	void Changed_Mood_RANGE(int & targetVal);

    // circle colors
    ofxColorGradient<ofColor> gradient;
    ofColor colorCircle;

	bool bGui = true;;

	ofxWindowApp windowApp;
};
