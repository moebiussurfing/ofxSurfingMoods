///BUG:
///+++++ count duration changes, destroys bar previews. must rescale

///TODO:
///++ repair browsing targets by keys collapses with range selectr engine
///++ add blocking to stay into same range running timmer
///++ define different cycling modes: star allways at first target, from last to first..

#pragma once

#include "ofMain.h"

#include "ofxSimpleTimer.h"
#include "ofxGuiExtended2.h"

//--

#pragma mark DEFINES

#define MAX_ITEMS 16//reserve max for arrays creations
#define DEFAULT_NUM_TARGETS 16//TARGETS
#define DEFAULT_NUM_PRESETS 8//PRESETS

#define NUM_RANGES 3//TARGET RANGES
#define DEFAULT_RANGE_LIMIT_1 4//divisor between range 1 and range 2
#define DEFAULT_RANGE_LIMIT_2 11//divisor between range 2 and range 3

#define USE_PRESETS_STARTS_AT_1_NOT_0//we use presets from 1 to 8

//NOTE:
//range 0 = presets 0-3
//range 1 = presets 4-10
//range 0 = presets 11-max_preset

#define BPM_BAR_RATIO 4 //TO SCALE TIMER/BPM

//--

class ofxSurfingMoods
{

public:

	//-

#pragma mark PUBLIC

	//preview
	void drawPreview();
	void drawPreview(int x, int  y, int  w, int  h);

	//-

#pragma mark OF

	void setup();
	void update();
	void draw();
	void exit();
	void windowResized(int w, int h);

	//--

#pragma mark API

	//should make an outside listener to receive changes!
	//each target is linked or trigs two other selectors together: preset and pattern
	ofParameter<int> TARGET_Selected;//current target. allways starts from 0
	
	ofParameter<int> PRESET_A_Selected;
	ofParameter<int> PRESET_B_Selected;
	ofParameter<int> PRESET_C_Selected;
	
	ofParameter<int> RANGE_Selected;

	ofParameter<bool> PRESET_A_Enable;
	ofParameter<bool> PRESET_B_Enable;
	ofParameter<bool> PRESET_C_Enable;

	//--

private:

	ofParameterGroup params_Listeners;
	ofParameterGroup params_STORE;
	ofParameterGroup params_USER;
	ofParameterGroup parameters_ranges;

	//-------------------------------------------

	//API

public:

	void setup(int numTargets, int numPresets, int limit1, int limit2);
	//all 3 preset have the same size, usualy 8 

	void play();
	void stop();
	void playSwitch();

	void setPathSettingsFolder(string s)
	{
		pathFolder = s;
	}

	bool isPlaying()
	{
		return PLAY.get();
	}

	//bool isPLAY()
	//{
	//	return PLAY.get();
	//}

	//private:
	//
	//	bool isPlaying()
	//	{
	//		return bIsPlaying;
	//	}

public:

	void setBPM(float bpm);
	void setLEN_bars(int bars);

	float getBPM()
	{
		return BPM.get();
	}

	void setRange(int r)
	{
		RANGE_Selected = r;
	}

	void setTarget(int t)
	{
		TARGET_Selected = t;
	}

	void setPosition(int x, int y);

	float getGuiUserWidth()
	{
		return group_USER->getWidth();
	}

	void setGuiAdvancedPositon(int _x, int _y)
	{
		group_Advanced->setPosition(_x, _y);
	}

	void setGuiUserPositon(int _x, int _y)
	{
		group_USER->setPosition(_x, _y);
	}

	//preview boxes bar
	void setPreviewPosition(int x, int y, int w, int h)
	{
		positionPreviewBoxes = glm::vec2(x, y);
		positionPreviewBoxes_Width = w;
		positionPreviewBoxes_Height = h;

		bUseCustomPreviewPosition = true;
	}
	void setPreviewVisible(bool b)
	{
		SHOW_Preview = b;
	}
	void togglePreviewVisible()
	{
		SHOW_Preview = !SHOW_Preview;
	}

private:

	glm::vec2 positionPreviewBoxes;
	float positionPreviewBoxes_Width;
	float positionPreviewBoxes_Height;
	bool bUseCustomPreviewPosition = false;

public:

	void setGui_visible(bool enable);
	void setGui_AdvancedVertical_MODE(bool enable);

	void setNextTarget()
	{
		int t = TARGET_Selected;
		t++;
		if (t >= NUM_TARGETS)
			t = NUM_TARGETS - 1;
		TARGET_Selected = t;
	}

	void setPreviousTarget()
	{
		int t = TARGET_Selected;
		t--;
		if (t <= 0)
			t = 0;
		TARGET_Selected = t;
	}

	void setShowGuiUser(bool b)
	{
		SHOW_GuiUser = b;
	}

	void setShowGuiAdvanced(bool b)
	{
		SHOW_GuiAdvanced = b;
	}

	//-------------------------------------------

private:

#pragma mark SETTNGS

	//path for xml settings
	string pathFolder;
	string fileSettings;
	string fileBank;

	void saveSettings(string path);
	void loadSettings(string path);
	void saveBanks(string path);
	void loadBanks(string path);
	bool autoSaveLoad_settings = true;

	void stopGen();

	//--

public:
	ofParameter<bool> PLAY;
	ofParameter<float> BPM;//bpm
	ofParameter<int> LEN_BARS;//in bars


//----

private:
	ofParameter<bool> MOOD_Color_Preview{ "RANGE MOOD", false };
	ofColor colorLabel;
	ofColor color_MOOD1, color_MOOD2, color_MOOD3;
	void refresh_MOOD_Color();

	ofParameter<string> labelRange{ "RANGE", "" };
	ofParameter<string> labelTarget{ "TARGET", "" };

	//blink
	bool bBlink = false;
	int blinkCounterFrames = 0;
	float blinkDuration;

	//labels
	ofTrueTypeFont myFont;
	string myTTF;
	int sizeTTF;
	string fname;

	void setup_Params();

	//this function can be used to trig ranges jumps externally without using the internal timer.
	void EngineSwitch();

	bool BLOCK_CALLBACK_Feedback = false;

	//-

#pragma mark ENGINE

	//-

	//default settings
	int NUM_TARGETS = (int)DEFAULT_NUM_TARGETS; //TARGETS
	int NUM_PRESETS_A = (int)DEFAULT_NUM_PRESETS; //PRESETS
	int NUM_PRESETS_B = (int)DEFAULT_NUM_PRESETS; //PRESETS
	int NUM_PRESETS_C = (int)DEFAULT_NUM_PRESETS; //PRESETS

	//-

	//ranges delimiters

	//range 0: starts at target 0

	//range 1: starts at target rLimit1
	int rLimit1 = (int)DEFAULT_RANGE_LIMIT_1;

	//range 2: starts at target rLimit2
	int rLimit2 = (int)DEFAULT_RANGE_LIMIT_2;

	//-

	bool bIsPlaying = false;
	int TARGET_Selected_PRE;

	//-

	//ofxGuiExtended

	void setup_GUI_Main();
	void setup_GUI_User();
	void setup_GUI_Target();
	void setup_GUI_Ranges();

	ofxGui gui;
	ofxGuiGroup2 *group_Advanced;
	ofxGuiGroup2 *group_USER;
	ofxGuiGroup2 *group_RANGES;
	ofxGuiGroup2 *group_TARGETS;
	ofxGuiGroup2 *group_CLOCK;

	//-

	//theme
	void setup_GUI_Customize();
	ofJson j_Gui, j_container, j_itemMini, confItem_Big, j_itemFat, j_itemMedium;

	//-

	ofParameter<int> timer;//timer
	ofParameter<int> timer_Progress;//% to finish timer
	ofParameter<int> Range_Min;//range
	ofParameter<int> Range_Max;//range
	void Changed_Params_Listeners(ofAbstractParameter &e);

	//ofParameter<string> MONITOR1;
	ofParameter<string> MONITOR2;

	//-

	//each target handles two 'sub targets' aka 'preset + pattern';

	int presets_A[MAX_ITEMS];
	int presets_B[MAX_ITEMS];
	int presets_C[MAX_ITEMS];

	ofParameter<bool> clone_TARGETS;
	void clone();

	ofJson js_targets;
	ofJson js_tar;
	string targets_subPath;

	//-

	ofParameter<bool> bReset_Settings;
	ofParameter<bool> bResetSort_Bank;
	ofParameter<bool> bReset_Bank;
	ofParameter<bool> bRandomize_Bank;
	void resetClock();
	void resetBank(bool RANDOMIZED = false, bool SORT_RELATIVE = true);

	//-

	bool stopBack = true; //WORKFLOW: goes to range 0 when stops
	bool ENABLED_SwitcherGen;

	//can be enabled only when default positioner mode
	bool MODE_vertical = false;

	bool SHOW_GUI_SwitcherGen;
	ofParameter<bool> SHOW_GuiUser;
	ofParameter<bool> SHOW_GuiAdvanced;
	ofParameter<bool> SHOW_Preview;

	//void updateLabels();

	//-

#pragma mark TIMER

	//TIMER
	ofxSimpleTimer timer_Range;
	void timer_Range_Complete(int &args);
	void timer_Range_Started(int &args);
	//bool SHOW_timer;

	//-

#pragma mark GUI LAYOUT

	//panel
	int gui_w;
	//widgets
	int gui_slider_mini_h;
	int gui_slider_big_h;
	int gui_button_big_h;

	string myTTF_Gui;
	int sizeTTF_Gui;

	//-

#pragma mark RANGES

	//RANGES
	void Changed_Ranges(ofAbstractParameter &e);
	int RANGE_Selected_PRE;

	struct range
	{
		ofParameter<string> name;
		ofParameter<int> min;
		ofParameter<int> max;
	};
	range myRange;

	vector<range> ranges;

	ofParameter<int> COUNT_Duration;
	ofParameter<int> COUNTER_step;
	ofParameter<int> COUNTER_step_FromOne;
	bool directionUp = true;

	void load_range(int r);
	void save_range(int r);
	ofParameter<bool> range_autoSave = true;
	ofParameter<bool> range_autoLoad = true;

	ofParameter<bool> target_autoSave = true;
	ofParameter<bool> target_autoLoad = true;
};

