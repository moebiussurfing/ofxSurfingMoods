
#pragma once

#include "ofMain.h"

/*

TODO:

++ fix link bpm ptr. allow don't break the timers/progress
	better slider

+ fix counter and stay count margins/preview
+ fix save/load settings
+ fix preview box full (merge double click with drag rect)

*/


#include "ofxSurfingImGui.h" // -> Adds all the add-on classes. You can also simplify picking what you want to use.

//-

#include "ofxSurfingHelpers.h"
#include "ofxSimpleTimer.h"
#include "ofxMarkovChain.h"
#include "ofxInteractiveRect.h" // engine to move the user clicker buttons panel. TODO: add resize by mouse too.
#include "TextBoxWidget.h"

//--

// Constants

#define MAX_ITEMS 16 // reserve max for arrays creations
#define NUM_RANGES 3 // TARGET RANGES

// with default 9 targets, 9 presets x3 (ABC), limit1 3, limit2 6
#define DEFAULT_NUM_TARGETS 9 // TARGETS
#define DEFAULT_NUM_PRESETS 9 // PRESETS
#define DEFAULT_RANGE_LIMIT_1 3 // divisor between range 0 and range 1
#define DEFAULT_RANGE_LIMIT_2 6 // divisor between range 1 and range 2

//--

// with default 16 targets, 16 presets, 8 patterns, limit1 4, limit2 11
//#define DEFAULT_NUM_TARGETS 16 // TARGETS
//#define DEFAULT_NUM_PRESETS 8 // PRESETS
//#define DEFAULT_RANGE_LIMIT_1 4 // divisor between range 0 and range 1
//#define DEFAULT_RANGE_LIMIT_2 11 // divisor between range 1 and range 2
// NOTE:
//range 0 = presets 0-3
//range 1 = presets 4-10
//range 0 = presets 11-max_preset

//-

// Snippets:

// ofApp Example:
// customize ranges/sizes (TODO: maybe not working..)
//moodsSurfer.setup(9, 9, 3, 6);
//// 9 targets, 9 presets x3 (ABC), limit1 3, limit2 6
//// 3 ranges/moods: range0 starts at 0. rage 1 starts at limit1 3, and range 2 starts at limit2 6

// ofApp Example:
//// customized position: bottom centered
//int w = 500;
//int h = 50;
//int x = ofGetWidth() / 2. - w * 0.5;
//int y = ofGetHeight() - h - 40;//TODO: there's a little offset...
//moodsSurfer.draw_PreviewWidget(x, y, w, h);

//--

#define BPM_BAR_RATIO 4 // TO SCALE TIMER/bpmSpeed

//--

class ofxSurfingMoods

{
public:

	//--------------------------------------------------------------
	ofxSurfingMoods() {
		ofAddListener(ofEvents().update, this, &ofxSurfingMoods::update);
		ofAddListener(ofEvents().draw, this, &ofxSurfingMoods::draw);
		addKeysListeners();

		rectPreview.setAutoSave(false);

		//bAutoDraw = false;
	};

	//--------------------------------------------------------------
	~ofxSurfingMoods() {
		ofRemoveListener(ofEvents().update, this, &ofxSurfingMoods::update);
		ofRemoveListener(ofEvents().draw, this, &ofxSurfingMoods::draw);
		removeKeysListeners();
		
		exit();
	};

	//-

private:
	TextBoxWidget textBoxWidget;

private:
	ofxSurfing_ImGui_Manager guiManager;
	void setup_ImGui();

public:
	void draw_ImGui();

private:
	void draw_ImGui_User();
	void draw_ImGui_Advanced();
	void draw_ImGui_ManualSlider();

	ofParameter<bool> bResetSlider{ "Reset Slider",false };
	ofParameter<bool> bResetPreviewWidget{ "Reset Preview",false };

private:

	bool bAutoDraw; // must be false when multiple ImGui instances created!

public:

	// Force autodraw
	//--------------------------------------------------------------
	void setImGuiAutodraw(bool b) { bAutoDraw = b; } // must be called befor setup!

	//-

private:

	ofxInteractiveRect rectPreview = { "_PreviewRect" };
	std::string path_rect;

public:
	void setup();

private:
	void update(ofEventArgs & args);
	void draw(ofEventArgs & args);

public:
	void startup();
	void exit();
	void windowResized(int w, int h);

	// keys
private:
	void keyPressed(ofKeyEventArgs &eventArgs);
	void keyReleased(ofKeyEventArgs &eventArgs);
	void addKeysListeners();
	void removeKeysListeners();

private:
	bool bMarkovFileFound = false;

	// preview widget
private:
	void draw_PreviewWidget();
	void update_PreviewColors();
	void draw_PreviewWidget(int x, int  y, int  w, int  h);

	ofColor cBg;
	ofColor cBord;
	ofColor c1, c2, c3;

	//ofParameter<glm::vec2> positionGui_Engine;

	//--

public:

	// should make an outside listener to receive changes!

	ofParameter<int> RANGE_Selected;

	// each target/state is linked or trigs 3 other receiver selectors together: Preset A-B-C
	ofParameter<int> TARGET_Selected;//current target. allways starts from 0

	ofParameter<int> PRESET_A_Selected;
	ofParameter<int> PRESET_B_Selected;
	ofParameter<int> PRESET_C_Selected;

	ofParameter<bool> PRESET_A_Enable;
	ofParameter<bool> PRESET_B_Enable;
	ofParameter<bool> PRESET_C_Enable;

	//--

private:

	ofParameterGroup params_Listeners;
	ofParameterGroup params_STORE;
	ofParameterGroup params_USER;
	ofParameterGroup parameters_ranges;

	//-

	// markov

private:

	ofxMC::MarkovChain markov;

	ofParameter<bool> MODE_Ranged{ "A MODE RANGED", false };
	ofParameter<bool> MODE_MarkovChain{ "B MODE MARKOV", false };
	ofParameter<bool> MODE_Manual{ "C MODE MANUAL", false };

	//--------------------------------------------------------------
	void refresModeshWorkflow() {
		if (!MODE_Manual && !MODE_MarkovChain && !MODE_Ranged)
		{
			MODE_Manual = true;
		}
	};

	ofParameter<bool> MODE_StartLocked{ "START LOCKED", false};//each arrive to a range will start from 1st from range.
	ofParameter<bool> MODE_AvoidRepeat{ "AVOID REPEAT", true};

	ofParameter<float> controlManual{ "MANUAL CTRL", 0, 0, 1.f };

	//ofParameterGroup params_Ranged{"RANGED"};
	//ofParameterGroup params_Manual{"MANUAL"};


	ofColor cRange;
	ofColor cRangeRaw;

	std::string path_markovMatrix;

	//-------------------------------------------

	// API

public:

	void setup(int numTargets, int numPresets, int limit1, int limit2);
	// all 3 preset have the same size, usualy 8 

	void play();
	void stop();
	void setTogglePlay();

	//--------------------------------------------------------------
	void setPathSettingsFolder(string s)
	{
		path_Folder = s;
	}

	//--------------------------------------------------------------
	bool isPlaying()
	{
		return bPLAY.get();
	}
	
	//-

public:

	//TODO: link bpm's
	float *bpmPtr = NULL;
	void setBpmPtr(float &_bpmPtr) {
		bpmPtr = &_bpmPtr;
	};

	void setBpm(float bpm);
	void setBarsScale(int bars);

	//-

	// To run external clock/timers
	// we will receive each incomming ticks.
	// that will inform to step-next. 
	// This modes disables the internal timers.

private:

	ofParameter<bool> bModeClockExternal{ "External Clock", false };

	ofParameter<bool> bModeAutomatic{ "Automatic", false };//some workflow features

public:

	void setTickMode(bool b) { bModeClockExternal = b; };
	void doBeatTick();
	void doRunStep(bool bforced = false);

public:

	//--------------------------------------------------------------
	float getBPM()
	{
		return bpmSpeed.get();
	}

	//--------------------------------------------------------------
	void setRange(int r)
	{
		RANGE_Selected = r;
	}

	//--------------------------------------------------------------
	void setTarget(int t)
	{
		TARGET_Selected = t;
	}

	//-

	// reset preview layout
	void doResetPreviewWidget();
	void doResetManualSlider();

	// preview boxes bar
	//--------------------------------------------------------------
	void setPreviewPosition(int x, int y, int w, int h)
	{
		positionPreviewBoxes = glm::vec2(x, y);
		positionPreviewBoxes_Width = w;
		positionPreviewBoxes_Height = h;

		rectPreview.setRect(positionPreviewBoxes.x, positionPreviewBoxes.y,
			positionPreviewBoxes_Width, positionPreviewBoxes_Height);

		bUseCustomPreviewPosition = true;
	}
	//--------------------------------------------------------------
	void setPreviewVisible(bool b)
	{
		bGui_PreviewWidget = b;
	}
	//--------------------------------------------------------------
	void setPreviewToggleVisible()
	{
		bGui_PreviewWidget = !bGui_PreviewWidget;
	}

private:

	glm::vec2 positionPreviewBoxes;
	float positionPreviewBoxes_Width;
	float positionPreviewBoxes_Height;

public:

	//TODO: global gui enabler. not implemented..
	void setGui_Visible(bool enable);
	void setGui_ToggleVisible();
	//void setGui_AdvancedVertical_MODE(bool enable);

	//--------------------------------------------------------------
	void setShowGuiAdvanced(bool b)
	{
		bGui_Advanced = b;
	}

	////--------------------------------------------------------------
	//void setShowGuiUser(bool b)
	//{
	//	SHOW_GuiUser = b;
	//}

	//-

public:
	// force browse targets
	//--------------------------------------------------------------
	void setNextTarget()
	{
		int t = TARGET_Selected;
		t++;
		if (t >= NUM_TARGETS)
			t = NUM_TARGETS - 1;
		TARGET_Selected = t;
	}

	//--------------------------------------------------------------
	void setPreviousTarget()
	{
		int t = TARGET_Selected;
		t--;
		if (t <= 0)
			t = 0;
		TARGET_Selected = t;
	}

	//-------------------------------------------

private:

	// Settngs

	// path for xml settings
	std::string path_Folder;

	std::string filename_Settings;
	std::string filename_Bank;

	void saveSettings(std::string path);
	void loadSettings(std::string path);

	void saveBanks(std::string path);
	void loadBanks(std::string path);

	ofParameter<bool> autoSaveLoad_settings{ "MODE EDIT", true };

	void stopMachine();

	//--

public:

	ofParameter<bool> bPLAY;//main play toggle
	ofParameter<float> bpmSpeed;//main bpm
	ofParameter<int> bpmLenghtBars;//timer duration in bars

	ofParameter<bool> bExternalLocked{ "EXTERNAL LOCKED", true };

	//----

private:

	ofParameter<bool> MOOD_Color_Preview{ "RANGE MOOD", false };
	ofColor colorLabel;
	ofColor color_MOOD1, color_MOOD2, color_MOOD3;
	void refresh_MOOD_Color();

	// Blink
	bool bBlink = false;
	int blinkCounterFrames = 0;
	float blinkDuration;

	// Labels
	ofTrueTypeFont myFont;
	std::string myTTF;
	int sizeTTF;
	std::string fname;

	void setup_Params();

	// This function can be used to trig ranges jumps externally without using the internal timer.
	void doRunEngineStep();

	bool BLOCK_CALLBACK_Feedback = false;

	//-

	// Engine

	//-

private:

	// Default settings
	int NUM_TARGETS = (int)DEFAULT_NUM_TARGETS; // TARGETS
	int NUM_PRESETS_A = (int)DEFAULT_NUM_PRESETS; // PRESETS
	int NUM_PRESETS_B = (int)DEFAULT_NUM_PRESETS; // PRESETS
	int NUM_PRESETS_C = (int)DEFAULT_NUM_PRESETS; // PRESETS

	//-

	// Ranges delimiters

	// Range 0: starts at target 0

	// Range 1: starts at target rLimit1
	int rLimit1 = (int)DEFAULT_RANGE_LIMIT_1;

	// Range 2: starts at target rLimit2
	int rLimit2 = (int)DEFAULT_RANGE_LIMIT_2;

	//-

	bool bIsPlaying = false;
	int TARGET_Selected_PRE;

	//-

private:

	ofParameter<int> timer; // timer
	ofParameter<int> timer_Progress; // % to finish timer
	ofParameter<int> timer_ProgressComplete; // % to finish timer
	ofParameter<int> Range_Min; // range
	ofParameter<int> Range_Max; // range
	void Changed_Params_Listeners(ofAbstractParameter &e);

	//-

	// each target handles two 'sub targets' aka 'preset + pattern';

	int presets_A[MAX_ITEMS];
	int presets_B[MAX_ITEMS];
	int presets_C[MAX_ITEMS];

	ofParameter<bool> bClone_TARGETS;
	void clone();

	ofJson js_targets;
	ofJson js_tar;
	std::string targets_subPath;

	//-

private:

	ofParameter<bool> bResetClockSettings;
	ofParameter<bool> bResetSort_Bank;
	ofParameter<bool> bReset_Bank;
	ofParameter<bool> bRandomize_Bank;
	void resetClock();
	void resetBank(bool RANDOMIZED = false, bool SORT_RELATIVE = true);

	//-

	bool stopBack = true; // WORKFLOW: goes to range 0 when stops
	bool ENABLED_MoodMachine;

	// can be enabled only when default positioner mode
	//bool MODE_vertical = false;

public:

	ofParameter<bool> bGui;

private:

	ofParameter<bool> bKeys;
	ofParameter<bool> bGui_Advanced;
	ofParameter<bool> bGui_ManualSlider;
	ofParameter<bool> bGui_ManualSliderHeader;
	ofParameter<bool> bEdit_PreviewWidget;
	ofParameter<bool> bGui_PreviewWidget;
	ofParameter<bool> bUseCustomPreviewPosition;
	//ofParameter<bool> SHOW_GuiUser;
	//ofParameter<bool> SHOW_Clocks;
	//bool bUseCustomPreviewPosition = false;

	//void updateLabels();

	//-

	// Timer

	// timer
	ofxSimpleTimer timer_Range;
	void timer_Range_Complete(int &args);
	void timer_Range_Started(int &args);
	//bool SHOW_timer;

	//-

	// Gui layout

	// panel
	int gui_w;
	// widgets
	int gui_slider_mini_h;
	int gui_slider_big_h;
	int gui_button_big_h;

	std::string myTTF_Gui;
	int sizeTTF_Gui;

	//-

	// Ranges

private:

	// ranges
	void Changed_Ranges(ofAbstractParameter &e);
	int RANGE_Selected_PRE;

	struct range
	{
		ofParameter<std::string> name;
		ofParameter<int> min;
		ofParameter<int> max;
	};
	range myRange;

	vector<range> ranges;

	ofParameter<int> countToDuration; // the one setted by the user

	ofParameter<int> counterStep;
	ofParameter<int> counterStepFromOne;

	bool directionUp = true;

	void load_range(int r);
	void save_range(int r);
	ofParameter<bool> range_autoSave = true;
	ofParameter<bool> range_autoLoad = true;

	ofParameter<bool> target_autoSave = true;
	ofParameter<bool> target_autoLoad = true;
};

