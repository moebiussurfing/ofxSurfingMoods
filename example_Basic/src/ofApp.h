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
	
    ofxSurfingMoods GEN;

    //listener for inside class GEN
    void Changed_GEN_TARGET(int & targetVal);
    void Changed_GEN_PRESET_A(int & targetVal);
	void Changed_GEN_PRESET_B(int & targetVal);
	void Changed_GEN_PRESET_C(int & targetVal);
	void Changed_GEN_MOOD_RANGE(int & targetVal);
};
