#pragma once

#include "ofMain.h"

#include "ofxSurfingMoods.h"

#include "ofxColorGradient.h" // Not required. For the example only
#include "ofxWindowApp.h" // Not required. For the example only

/*
	
	NOTE:
	
	1. You could start testing by moving the PRESET A slider.
	That Preset Index will change the circle color.
	Set the player stopped first to clarify.

	2. Then you could go testing by moving the STATE TARGET slider.
	First you should ENABLE some preset controller A, B or C.
	Each Target is linked to 3 Presets that you can configure / assign or to disable.
	
	2. Pick a Mode (A, B or C) and hit Play.
	On A and B Mode, the machine must be playing, and will be walking between the 3 RANGE MOODS. 

	3. On the Mode Manual, you can drag the Control slider to set a Range Mood.
	When you set the slider to a mood, the machine will be walking/randomizing on the STATE TARGETS but locked inside the Mood.
	
	All the timers are locked to the clock BPM.

*/


class ofApp: public ofBaseApp
{
public:

    void setup();
    void draw();
    void exit();
    void keyPressed(int key);
    void windowResized(int w, int h);

    // Scene circle colors
    ofxColorGradient<ofColor> gradient;
    ofColor colorCircle;
	bool bGui = true;;

	ofxWindowApp windowApp;


	//--------------------------------------------------------------
	// Snippet to copy / paste to you project ->

	// Surfing Moods
    
	ofxSurfingMoods moods;
	
	void setupMoods();
    void exitMoods();

	//--
	 
    // Callbacks to be notified from MOODS evolution

    void Changed_Mood_TARGET(int &index);

	void Changed_Mood_PRESET_A(int &index);
    void Changed_Mood_PRESET_B(int &index);
    void Changed_Mood_PRESET_C(int &index);

	void Changed_Mood_RANGE(int &index);
};
