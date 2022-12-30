#pragma once

#include "ofMain.h"

#include "ofxSurfingMoods.h"

#include "ofxColorGradient.h" // Not required. For the example only
#include "ofxWindowApp.h" // Not required. For the example only

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

    ofxSurfingGui ui;

	//--------------------------------------------------------------

	// SURFING MOODS
         
	// Snippet to copy / paste to you project ->
    
	ofxSurfingMoods moods;
	
	void setupMoods();
    void exitMoods();

	//--
	 
    // Callbacks to be notified from MOODS evolution

    void Changed_Mood_TARGET(int &index); // The Mater State!

	void Changed_Mood_RANGE(int &index); // The Mood for the State.

	void Changed_Mood_PRESET_A(int &index);
    void Changed_Mood_PRESET_B(int &index);
    void Changed_Mood_PRESET_C(int &index);
};
