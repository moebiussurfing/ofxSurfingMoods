
#pragma once

#include "ofMain.h"

/*

	TODO:

	+ add divider to make globally slower
	+ add LOCK mode to disable momentary editing ranges preset
	+ reverse manual slider bc matrix presets is inverted 1-9
	+ allow draw slider and preview widget a part, independently
	+ 


	BUG:

	+ mode b toggle not showing..
	
*/


//-

#include "ofxSimpleTimer.h"
#include "ofxMarkovChain.h"
#include "ofxSurfingImGui.h" 
#include "ofxSurfingHelpers.h"
#include "ofxInteractiveRect.h" // engine to move and resize the preview widget
#include "TextBoxWidget.h" // help box

//--

// Constants
#define NUM_RANGES 3 // TARGET RANGES
#define MAX_ITEMS 16 // reserve max for arrays creations

#define PREVIEW_START_AT_ZERO

//----

// Default hard coded template
// With default 9 targets, 
// 9 presets x3 (ABC), limit1 3, limit2 6
#define DEFAULT_NUM_TARGETS 9	// TARGETS
#define DEFAULT_NUM_PRESETS 9	// PRESETS
#define DEFAULT_RANGE_LIMIT_1 3 // divisor between range 0 and range 1
#define DEFAULT_RANGE_LIMIT_2 6 // divisor between range 1 and range 2

//----

// Another possible template
// With default 16 targets, 
// 16 presets, 8 patterns, limit1 4, limit2 11
//#define DEFAULT_NUM_TARGETS 16	// TARGETS
//#define DEFAULT_NUM_PRESETS 8		// PRESETS
//#define DEFAULT_RANGE_LIMIT_1 4	// divisor between range 0 and range 1
//#define DEFAULT_RANGE_LIMIT_2 11	// divisor between range 1 and range 2
// NOTE:
//range 0 = presets 0-3
//range 1 = presets 4-10
//range 0 = presets 11-max_preset

//----

/*

// Example Snippet:
// ofApp Example:
// customize ranges/sizes (TODO: maybe not working..)
moodsSurfer.setup(9, 9, 3, 6);
// 9 targets, 9 presets x3 (ABC), limit1 3, limit2 6
// 3 ranges/moods: range0 starts at 0.
// range 1 starts at limit1 3, and range 2 starts at limit2 6

*/

//--

#define BPM_BAR_RATIO 4 // To scale ms Timer / BPM Speed

//--

class ofxSurfingMoods
{
public:

	//--------------------------------------------------------------
	ofxSurfingMoods() 
	{
		ofAddListener(ofEvents().update, this, &ofxSurfingMoods::update);

		addKeysListeners();

		rectPreview.setAutoSave(false);
	};

	//--------------------------------------------------------------
	~ofxSurfingMoods()
	{
		ofRemoveListener(ofEvents().update, this, &ofxSurfingMoods::update);

		removeKeysListeners();

		exit();
	};

public:

	void setup();

private:

	void update(ofEventArgs & args);

public:

	void startup();
	void exit();
	void windowResized(int w, int h);

private:

	// Keys

	void keyPressed(ofKeyEventArgs &eventArgs);
	void keyReleased(ofKeyEventArgs &eventArgs);
	void addKeysListeners();
	void removeKeysListeners();

	//-

private:

	ofxSurfing_ImGui_Manager guiManager;
	void setupGui();

public:

	void drawGui();

private:

	void draw_ImGui_Main();
	void draw_ImGui_Advanced();
	void draw_ImGui_Matrices();
	void draw_ImGui_ManualSlider();

private:

	ofParameter<bool> bResetSlider{ "Reset Slider",false };
	ofParameter<bool> bResetPreviewWidget{ "Reset Preview",false };

private:

	TextBoxWidget textBoxWidget;

	//-

private:

	ofxInteractiveRect rectPreview = { "_PreviewRect" };
	std::string path_rect;

private:

	bool bMarkovFileFound = false;

private:

	// Preview widget

	void draw_PreviewWidget();
	void update_PreviewColors();
	void draw_PreviewWidget(int x, int  y, int  w, int  h);

	ofColor cBg;
	ofColor cBord;
	ofColor c1, c2, c3;

	//--

public:

	// Should make an outside listener to receive changes!
	// (on parent scope like ofApp. Look the example for snippets)

	ofParameter<int> RANGE_Selected;

	// Each target/state is linked to
	// (or trigs) 3 other receiver selectors together: Preset A-B-C
	
	// Current index target. always starts from 0
	ofParameter<int> TARGET_Selected;

	ofParameter<int> PRESET_A_Selected;
	ofParameter<int> PRESET_B_Selected;
	ofParameter<int> PRESET_C_Selected;

	ofParameter<bool> PRESET_A_Enable;
	ofParameter<bool> PRESET_B_Enable;
	ofParameter<bool> PRESET_C_Enable;

	//--

private:

	ofParameterGroup params_Listeners;
	ofParameterGroup params_AppSettings;
	ofParameterGroup parameters_ranges;

	//-

	// Markov

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

	ofParameter<bool> MODE_StartLocked{ "START LOCKED", false };
	// Every time we arrive to a range, will start from 1st from target of the mood range.
	
	ofParameter<bool> MODE_AvoidRepeat{ "NO REPEAT", true };
	// Next random will go to a different than previous state, if enabled.

	ofParameter<float> controlManual{ "CONTROL", 0, 0, 1.f };

	ofColor cRange;
	ofColor cRangeRaw;

	std::string path_markovMatrix;

	//-------------------------------------------

	// API

public:

	void setup(int numTargets, int numPresets, int limit1, int limit2);
	// All 3 preset have the same size, usually 8 

	void play();
	void stop();
	void setTogglePlay();

	//--------------------------------------------------------------
	void setPathSettingsFolder(string s)
	{
		path_Folder = s;
		ofxSurfingHelpers::CheckFolder(path_Folder);
	}

	//--------------------------------------------------------------
	bool isPlaying() const
	{
		return bPLAY.get();
	}

	//-

public:

	////TODO: link BPMS's
	// We will use makeReferenceTo between ofParmas!
	// 
	//float *bpmPtr = NULL;
	////--------------------------------------------------------------
	//void setBpmPtr(float &_bpmPtr) {
	//	bpmPtr = &_bpmPtr;
	//};

	void setBpm(float bpm);
	void setBarsScale(int bars);

	//--------------------------------------------------------------
	float getBPM() const
	{
		return bpmSpeed.get();
	}

	//-

	// To run external clock/timers
	// we will receive each incoming ticks.
	// that will inform to step-next. 
	// This modes disables the internal timers.

	//--------------------------------------------------------------
	void setEnableExternalClock(bool b) {
		bModeClockExternal = b;
	}

	////--------------------------------------------------------------
	//void setEnableInternalClock(bool b) {
	//	bInternalClock = b;
	//}

private:

	//bool bInternalClock = true;

	ofParameter<bool> bModeClockExternal{ "External Clock", false };
	// disables internal timers to receive ticks. We can force bets internally.

	ofParameter<bool> bModeAutomatic{ "Automatic", false }; // some workflow features

public:

	void setTickMode(bool b) { bModeClockExternal = b; };
	void doBeatTick();
	void doRunStep(bool bforced = false);

public:

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

	//--

public:

	// Reset preview layout
	void doResetPreviewWidget();
	void doResetManualSlider();
	bool bResetLayout = false;

	// Preview boxes bar
	//--------------------------------------------------------------
	void setPreviewPosition(int x, int y, int w, int h)
	{
		rectPreview.setRect(x, y, w, h);

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

	//TODO:
	//void setGui_AdvancedVertical_MODE(bool enable);
	// can be enabled only when default positioner mode
	//bool MODE_vertical = false;

public:

	void setGui_Visible(bool enable);
	void setGui_ToggleVisible();

	//--------------------------------------------------------------
	void setShowGuiAdvanced(bool b)
	{
		bGui_Advanced = b;
	}

	//-

public:

	// Force browse targets
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

	// Settings

	std::string path_Folder;
	std::string filename_AppSettings;
	std::string filename_Bank;

	void saveSettings(std::string path);
	void loadSettings(std::string path);

	void saveBanks(std::string path);
	bool loadBanks(std::string path);

	ofParameter<bool> autoSaveLoad_settings{ "MODE EDIT", true };

	//--

public:

	void stopMachine();

	ofParameter<bool> bPLAY; // Main play toggle
	ofParameter<float> bpmSpeed; // Main BPM
	ofParameter<int> bpmLenghtBars; // Timer duration in bars

	ofParameter<bool> bExternalLocked{ "EXTERNAL LOCKED", true };

	//----

private:

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

	bool bDISABLE_CALLBACKS = false;

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

	// Callbacks
	void Changed_Params_Listeners(ofAbstractParameter &e);

	//-

	// Each target handles two 'sub targets' aka 'preset + pattern';

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

	// BPM to 120
	void resetClock();

	// Creates bank assignments to each target index
	void resetBank(bool RANDOMIZED = false, bool SORT_RELATIVE = true);

	//-

	bool stopBack = true; // WORKFLOW: goes to range 0 when stops
	bool bEnable;

public:

	ofParameter<bool> bGui_Main;

	void setKeySpace(bool b) { bKeySpace = b; }//to avoid collapse with other space key as players start key.

private:

	ofParameter<bool> bKeys;
	ofParameter<bool> bKeySpace;
	ofParameter<bool> bGui_Advanced;
	ofParameter<bool> bGui_Matrices;
	ofParameter<bool> bGui_ManualSlider;
	ofParameter<bool> bGui_ManualSliderHeader;
	ofParameter<bool> bEdit_PreviewWidget;
	ofParameter<bool> bGui_PreviewWidget;
	ofParameter<bool> bUseCustomPreviewPosition;

	//-

	// Timer

	ofxSimpleTimer timer_Range;
	void timer_Range_Complete(int &args);
	void timer_Range_Started(int &args);

	//-

	std::string myTTF_Gui;
	int sizeTTF_Gui;

	//-

	// Ranges

private:

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

	ofParameter<int> countToDuration; // the one to be setted by the user
	ofParameter<int> counterStep; // monitor the counting but starting in 0
	ofParameter<int> counterStepFromOne; // monitor the counting but starting in 1

	bool directionUp = true;

	ofParameter<bool> range_autoSave = true;
	ofParameter<bool> range_autoLoad = true;

	ofParameter<bool> target_autoSave = true;
	ofParameter<bool> target_autoLoad = true;

	void load_range(int r);
	void save_range(int r);
};

