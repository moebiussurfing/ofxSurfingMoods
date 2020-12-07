#pragma once

#include "ofMain.h"

#include "ofxSurfingMoods.h"

class ofApp: public ofBaseApp{

public:

    void setup();
    void update();
    void draw();
    void exit();
    void keyPressed(int key);
    void windowResized(int w, int h);
	
    ofxSurfingMoods moodMachine;

    //callbacks looking inside moodMachine
    void Changed_Mood_TARGET(int & targetVal);
    void Changed_Mood_PRESET_A(int & targetVal);
	void Changed_Mood_PRESET_B(int & targetVal);
	void Changed_Mood_PRESET_C(int & targetVal);
	void Changed_Mood_RANGE(int & targetVal);
};
