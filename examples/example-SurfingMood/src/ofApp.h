#pragma once

#include "ofMain.h"

#include "ofxSurfingMoods.h"

#include "ofxColorGradient.h" // for the example only
#include "ofxWindowApp.h" // for the example only

/*
	
	NOTE:
	
	1. You could start testing by moving the Target slide
	with the player stopped.
	Each Target is linked to 3 Presets that you can configure or to disable.
	
	2. Then, pick a Mode and hit Play.

	3. Then, on the Mode Manual, you can drag the Control slider to set a Range. 
	The machine must be playing, and wil be walking between the 'mood' states. 
	All the time locked to the clock Bpm.

*/


class ofApp: public ofBaseApp{
public:

    void setup();
    void draw();
    void exit();
    void keyPressed(int key);
    void windowResized(int w, int h);
	ofxWindowApp windowApp;

    // Scene circle colors
    ofxColorGradient<ofColor> gradient;
    ofColor colorCircle;
	bool bGui = true;;


	//-------------------------

	// Snippet to copy/paste to you project ->

	// Surfing Moods
    ofxSurfingMoods moods;
    void setupMoods();
    void exitMoods();

    // Callbacks from moods
    void Changed_Mood_TARGET(int & targetIndex);
	void Changed_Mood_PRESET_A(int & targetIndex);
    void Changed_Mood_PRESET_B(int & targetIndex);
    void Changed_Mood_PRESET_C(int & targetIndex);
	void Changed_Mood_RANGE(int & targetIndex);
};
