
#include "ofxSurfingMoods.h"

//--------------------------------------------------------------
void ofxSurfingMoods::setup(int numTargets, int numPresets, int limit1, int limit2)
{
	NUM_TARGETS = numTargets; // TARGET

	NUM_PRESETS_A = numPresets; // PRESETS
	NUM_PRESETS_B = numPresets; // PRESETS
	NUM_PRESETS_C = numPresets; // PRESETS

	rLimit1 = limit1; // borders limits
	rLimit2 = limit2;

	setup();
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup()
{
	// Erase arrays
	for (int i = 0; i < MAX_ITEMS; i++)
	{
		presets_A[i] = 0;
		presets_B[i] = 0;
		presets_C[i] = 0;
	}

	//--

	//TODO:
	// matrix colors
	colors.clear();
	for (size_t i = 0; i < 9; i++)
	{
		ofColor c;
		if (i < 3) c = ofColor::green;
		else if (i < 6) c = ofColor::yellow;
		else if (i < 10) c = ofColor::red;

		colors.push_back(c);
	}
	keyCommandsChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	setColorized(true);

	//--

	// Settings paths
	path_Folder = "ofxSurfingMoods/"; // default folder
	filename_AppSettings = "Moods_AppSettings"; // settings
	filename_Bank = "Moods_Bank.json"; // ranges bank: any target to relatives preset/pattern

	//--

	// Markov
	path_markovMatrix = path_Folder + "markov/" + "transitionMatrix.txt";

	// Avoid crash
	ofFile file;
	file.open(path_markovMatrix);
	bMarkovFileFound = file.exists();
	if (bMarkovFileFound) {
		ofxMC::Matrix mat(path_markovMatrix);
		markov.setup(mat, 0);
	}
	else {
		ofLogError("ofxSurfingMoods") << (__FUNCTION__) << "Markov file " << path_markovMatrix << " not found!";
	}

	//--

	// Font to draw preview boxes
	//fname = "overpass-mono-bold.otf";
	fname = "JetBrainsMono-Bold.ttf";
	myTTF = "assets/fonts/" + fname;
	sizeTTF = 9; // font size
	bool isLoaded = myFont.load(myTTF, sizeTTF, true, true);
	if (!isLoaded)
	{
		ofLogError("ofxSurfingMoods") << (__FUNCTION__) << "ofTrueTypeFont FONT FILE '" << myTTF << "' NOT FOUND!";
		ofLogError("ofxSurfingMoods") << (__FUNCTION__) << "Load default font.";

		myFont.load(OF_TTF_MONO, sizeTTF, true, true);
	}

	//--

	// Params
	setup_Params();

	//--	

	// Help
	std::string helpInfo = "";
	helpInfo += "HELP\n";
	helpInfo += "KEY COMMANDS\n";
	helpInfo += "\n";
	helpInfo += "G       GUI\n";
	helpInfo += "SPACE   PLAY\n";
	helpInfo += "+|-     SET COUNTER\n";
	helpInfo += "1-2-3   SET RANGE\n";
	helpInfo += "B       FORCE BEAT\n";
	//helpInfo = ofToUpper(helpInfo); // make uppercase

	textBoxWidget.setPath(path_Folder);
	textBoxWidget.setText(helpInfo);
	textBoxWidget.setup();

	//--

	// Callbacks

	// 1.
	ofAddListener(params_Listeners.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);

	// 2.
	ofAddListener(parameters_ranges.parameterChangedE(), this, &ofxSurfingMoods::Changed_Ranges);

	//--

	// Timer
	// 60,000 / bpmSpeed = MS
	timer = bpmLenghtBars * (BPM_BAR_RATIO * (60000 / bpmSpeed));
	timer_Range.setup(timer);
	ofAddListener(timer_Range.TIMER_COMPLETE, this, &ofxSurfingMoods::timer_Range_Complete);
	ofAddListener(timer_Range.TIMER_STARTED, this, &ofxSurfingMoods::timer_Range_Started);

	//--

	// Gui
	setupGui();

	//--

	toggles_Target.setup(TARGET_Selected);
	toggles_PresetA.setup(PRESET_A_Selected);
	toggles_PresetB.setup(PRESET_B_Selected);
	toggles_PresetC.setup(PRESET_C_Selected);

	//--------

	// Startup
	startup();
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup_Params()
{
	// Colors to show moods (ranges)
	int a = 128;
	color_MOOD1.set(ofColor(ofColor::red, a));
	color_MOOD2.set(ofColor(ofColor::yellow, a));
	color_MOOD3.set(ofColor(ofColor::green, a));

	//-

	// 1. Params

	bGui.set("MOODS", true);
	bGui_Matrices.set("MATRICES", false);
	bGui_Advanced.set("ADVANCED", false);
	bGui_PreviewWidget.set("WIDGET", false);
	bGui_ManualSlider.set("SLIDER", false);
	bGui_ManualSliderHeader.set("Slider Header", false);

	bPLAY.set("PLAY", false);

	bpmSpeed.set("BPM", 120.f, 30.f, 400.f);//bmp
	bpmLenghtBars.set("BARS LEN", 1, 1, 16);//bars
	timer.set("TIMER", 1000, 1, 60000);//ms
	timer.setSerializable(false);

	TARGET_Selected.set("STATE", 0, 0, NUM_TARGETS - 1);//target
	//TARGET_Selected.set("TARGET", 0, 0, NUM_TARGETS - 1);//target

	PRESET_A_Enable.set("ENABLE A", true);
	PRESET_B_Enable.set("ENABLE B", true);
	PRESET_C_Enable.set("ENABLE C", true);

	// Presets margins

	PRESET_A_Selected.set("PRESET A", 0, 0, NUM_PRESETS_A - 1);
	PRESET_B_Selected.set("PRESET B", 0, 0, NUM_PRESETS_B - 1);
	PRESET_C_Selected.set("PRESET C", 0, 0, NUM_PRESETS_C - 1);

	Range_Min.set("MIN TARGET", 0, 0, NUM_TARGETS - 1);
	Range_Max.set("MAX TARGET", NUM_TARGETS - 1, 0, NUM_TARGETS - 1);

	bResetClockSettings.set("RESET", false);
	bResetClockSettings.setSerializable(false);
	bClone_TARGETS.set("BANK CLONE >", false);
	bClone_TARGETS.setSerializable(false);

	bKeys.set("Keys", true);
	bKeySpace.set("Key Space", true);

	bUseCustomPreviewPosition.set("Custom", false);

	//bEdit_PreviewWidget.set("Edit Preview Widget", false);
	bEdit_PreviewWidget.makeReferenceTo(rectPreview.bGui);
	rectPreview.setPath(path_Folder + "Widget/");
	rectPreview.setup();
	rectPreview.setBorderColor(ofColor(64, 24));
	rectPreview.setRectConstraintMin(glm::vec2(300, 50));

	//----

	// 3. Setup_GUI_Ranges
	// Init ranges vector
	ranges.resize(NUM_RANGES);

	// Ranges

	// Hard coded for NUM_RANGES = 3
	ranges[0].name = "0";
	ranges[0].min = 0;
	ranges[0].max = rLimit1 - 1;
	ranges[1].name = "1";
	ranges[1].min = rLimit1;
	ranges[1].max = rLimit2 - 1;
	ranges[2].name = "2";
	ranges[2].min = rLimit2;
	ranges[2].max = NUM_TARGETS - 1;

	//--

	// Ranges

	// Define params
	timer_Progress.set("COMPLETE", 0, 0, 100);//%
	timer_ProgressComplete.set("COMPLETE ", 0, 0, 100);//%
	RANGE_Selected.set("MOOD", 0, 0, NUM_RANGES - 1);
	myRange.name.set("MOOD NAME", ranges[RANGE_Selected].name);

	// Main counters
	countToDuration.set("TO", 4, 1, 8);
	counterStepFromOne.set("COUNTER", 1, 0, countToDuration);

	bRandomize_Bank.set("BANK RANDOMIZE", false);
	bRandomize_Bank.setSerializable(false);
	bReset_Bank.set("BANK RESET CLEAR", false);
	bReset_Bank.setSerializable(false);
	bResetSort_Bank.set("BANK RESET SORTED", false);
	bResetSort_Bank.setSerializable(false);

	//---

	myRange.min.set("TARGET MIN", 0, 0, NUM_TARGETS - 1);
	myRange.max.set("TARGET MAX", 0, 0, NUM_TARGETS - 1);

	//---

	// link to gui manager
	bKeys.makeReferenceTo(ui.bKeys);
	//bHelp.makeReferenceTo(ui.bHelp);

	//---

	// Group params outside gui but to use listeners and xml settings
	parameters_ranges.setName("RANGES");
	parameters_ranges.add(RANGE_Selected);
	parameters_ranges.add(myRange.min);
	parameters_ranges.add(myRange.max);

	//---

	// Store params (grouped only to save/load, not to allow on gui or callbacks)
	params_AppSettings.setName("ofxSurfingMoods_AppSettings");
	params_AppSettings.add(bGui);
	params_AppSettings.add(bGui_Matrices);
	params_AppSettings.add(bGui_Advanced);
	params_AppSettings.add(bGui_ManualSlider);
	params_AppSettings.add(bGui_ManualSliderHeader);
	params_AppSettings.add(bGui_PreviewWidget);
	params_AppSettings.add(bPLAY);
	params_AppSettings.add(bpmSpeed);
	params_AppSettings.add(MODE_MarkovChain);
	params_AppSettings.add(MODE_Ranged);
	params_AppSettings.add(MODE_StartLocked);
	params_AppSettings.add(MODE_AvoidRepeat);
	params_AppSettings.add(MODE_Manual);
	params_AppSettings.add(controlManual);
	params_AppSettings.add(countToDuration);
	params_AppSettings.add(bpmLenghtBars);
	params_AppSettings.add(PRESET_A_Enable);
	params_AppSettings.add(PRESET_B_Enable);
	params_AppSettings.add(PRESET_C_Enable);
	params_AppSettings.add(TARGET_Selected);
	params_AppSettings.add(bUseCustomPreviewPosition);
	params_AppSettings.add(bModeExternalClock);
	params_AppSettings.add(ui.params_Advanced);
	//params_AppSettings.add(bMode_Edit);
	params_AppSettings.add(bExpand);

	//-

	// Group params for callback listener only
	params_Listeners.setName("MoodMachine_params");

	params_Listeners.add(bPLAY);
	params_Listeners.add(bModeExternalClock);
	params_Listeners.add(bModeAutomatic);
	params_Listeners.add(bpmSpeed);
	params_Listeners.add(bpmLenghtBars);
	params_Listeners.add(bResetClockSettings);
	params_Listeners.add(bRandomize_Bank);
	params_Listeners.add(bResetSort_Bank);
	params_Listeners.add(bReset_Bank);
	params_Listeners.add(bEdit_PreviewWidget);
	params_Listeners.add(TARGET_Selected);
	params_Listeners.add(bClone_TARGETS);
	params_Listeners.add(PRESET_A_Selected);
	params_Listeners.add(PRESET_B_Selected);
	params_Listeners.add(PRESET_C_Selected);
	params_Listeners.add(timer);
	params_Listeners.add(MODE_MarkovChain);
	params_Listeners.add(MODE_Ranged);
	params_Listeners.add(MODE_StartLocked);
	params_Listeners.add(MODE_AvoidRepeat);
	params_Listeners.add(MODE_Manual);
	params_Listeners.add(controlManual);
	params_Listeners.add(countToDuration);
	params_Listeners.add(ui.bMinimize);
	params_Listeners.add(bMode_Edit);

	// Exclude from file settings
	//bPLAY.setSerializable(false);
	counterStepFromOne.setSerializable(false);
	bGui_ManualSliderHeader.setSerializable(false);
	bMode_Edit.setSerializable(false);//always disabled. mode locked
}

//--------------------------------------------------------------
void ofxSurfingMoods::startup()
{
	//--

	// Reset
	//// Erase bank targets
	//resetBank(false);
	//// Reset clock
	//resetClock();

	//--

	setGui_Visible(true);

	//--

	bEnable = true;
	bIsPlaying = false;
	timer = 0;
	bPLAY = false;

	TARGET_Selected_PRE = -1;
	RANGE_Selected_PRE = -1;
	RANGE_Selected = 0;

	//-

	load_range(RANGE_Selected);

	//--

	// Preview rectangle
	bUseCustomPreviewPosition = true;

	path_rect = path_Folder + "ofxSurfingMoods_";

	/*
	//TODO:
	//crashes sometimes if no file present..
	bool b = rectPreview.loadSettings("_PreviewRect", path_rect, true);
	if (!b) rectPreview.setRect(25, 650, 700, 50);//initialize when no settings file created yet.
	rectPreview.disableEdit();
	*/

	//--

	// Load bank targets
	//if (bMode_Edit.get())
	{
		if (!loadBanks(path_Folder))
		{
			// If settings file not found, we reset and create a default bank
			resetBank(false, true);//correlative sort
		}
	}

	// Load panel settings
	//if (bMode_Edit.get())
	{
		loadSettings(path_Folder);
	}

	//TODO:
	//setGui_AdvancedVertical_MODE(false);

	//--

	refresh_MOOD_Color();

	//--

	// Link to toggles controller

#ifdef USE_TOGGLE_TRIGGERS
	index.makeReferenceTo(TARGET_Selected);
	doBuildMidiNotes();
#endif

	//--

	bMode_Edit = bMode_Edit;
}

//--------------------------------------------------------------
void ofxSurfingMoods::update(ofEventArgs& args)
{
	if (bGui) update_PreviewColors();

	//-

	//if (MODE_MarkovChain)
	//{
	//	i++;
	//}

	//-

	if (!bModeExternalClock)
	{
		timer_Range.update();
		timer_Progress = 100 * timer_Range.getNormalizedProgress();
		if (!MODE_Manual) {
			timer_ProgressComplete = ofMap(counterStepFromOne, 1, countToDuration + 1, 0, 100, true);
			timer_ProgressComplete += timer_Progress / countToDuration;
		}
	}
	else
	{
		if (!MODE_Manual) {
			timer_Progress = ofMap(counterStepFromOne, 1, countToDuration, 0, 100, true);
		}
		else {
		}
	}

	//-

	//timer_Progress = ofMap(counterStep, 0, countToDuration, 0, 100, true);
	//timer_Progress = ofMap(counterStepFromOne, 1, counterStepFromOne.getMax() + 1, 0, 100, true);
}

////--------------------------------------------------------------
//void ofxSurfingMoods::draw(ofEventArgs& args)
//{
//	//if (!bGui_Main) return;
//
//	//// Preview widget
//	//if (bGui_PreviewWidget) draw_PreviewWidget();
//
//	//// Help
//	//if (ui.bHelp) textBoxWidget.draw();
//
//	//-
//
//	//// ImGui
//	//drawGui(); 
//	// -> TODO: fails when other ImGui instances...
//	// We will call this manually from parent scope.
//}

//-

// Keys
//--------------------------------------------------------------
void ofxSurfingMoods::keyPressed(ofKeyEventArgs& eventArgs)
{
	const int& key = eventArgs.key;

	//--

	// Key enabler
	if (0)
		if (key == 'k')
		{
			bKeys = !bKeys;
			ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "KEYS: " << (bKeys ? "ON" : "OFF");

			if (!bKeys)
			{
				ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "ALL KEYS DISABLED. PRESS 'k' TO ENABLE GAIN!";
			}
			else
			{
				ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "KEYS ENABLED BACK";
			}
		}

	//--

	// Disabler for all keys. (independent from bActive)
	if (!bKeys) return;

	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << (char)key << " [" << key << "]";

	// Modifiers
	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

	if (0) // debug
	{
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "mod_COMMAND: " << (mod_COMMAND ? "ON" : "OFF");
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "mod_CONTROL: " << (mod_CONTROL ? "ON" : "OFF");
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "mod_ALT: " << (mod_ALT ? "ON" : "OFF");
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "mod_SHIFT: " << (mod_SHIFT ? "ON" : "OFF");
	}

	//-

	if (key == ' ')
	{
		if (!bKeySpace) setTogglePlay();
	}

	//else if (key == 'p')
	//{
	//	play();
	//}
	//else if (key == 's')
	//{
	//	stop();
	//}

	else if (key == 'g')
	{
		setGui_ToggleVisible();
	}

	else if (key == '1')
	{
		setRange(0);
	}

	else if (key == '2')
	{
		setRange(1);
	}

	else if (key == '3')
	{
		setRange(2);
	}

	else if (key == 'B')
	{
		if (bModeExternalClock) doBeatTick();
	}
	else if (key == '-')
	{
		countToDuration--;
	}
	else if (key == '+')
	{
		countToDuration++;
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::keyReleased(ofKeyEventArgs& eventArgs)
{
	const int& key = eventArgs.key;
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << (char)key << " [" << key << "]";

	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);
}

//--------------------------------------------------------------
void ofxSurfingMoods::addKeysListeners()
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__);
	ofAddListener(ofEvents().keyPressed, this, &ofxSurfingMoods::keyPressed);
}

//--------------------------------------------------------------
void ofxSurfingMoods::removeKeysListeners()
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__);
	ofRemoveListener(ofEvents().keyPressed, this, &ofxSurfingMoods::keyPressed);
}

//--------------------------------------------------------------
void ofxSurfingMoods::windowResized(int w, int h)
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << w << ", " << h;

	// workaround
	bResetLayout = true;
}

//--------------------------------------------------------------
void ofxSurfingMoods::exit()
{
	// Preview rectangle
	/*
	rectPreview.saveSettings("_PreviewRect", path_rect, true);
	*/

	//-

	// Save settings
	saveSettings(path_Folder);
	if (bMode_Edit)
	{
		saveBanks(path_Folder);
	}

	//-

	ofRemoveListener(params_Listeners.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);
	ofRemoveListener(parameters_ranges.parameterChangedE(), this, &ofxSurfingMoods::Changed_Ranges);

	ofRemoveListener(timer_Range.TIMER_COMPLETE, this, &ofxSurfingMoods::timer_Range_Complete);
	ofRemoveListener(timer_Range.TIMER_STARTED, this, &ofxSurfingMoods::timer_Range_Started);
}

//--------------------------------------------------------------
void ofxSurfingMoods::refresh_MOOD_Color()
{
	ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__);

	// Mood color preview label
	switch (RANGE_Selected.get())
	{

	case 0:
	{
		colorLabel.set(color_MOOD1);
	}
	break;

	case 1:
	{
		colorLabel.set(color_MOOD2);
	}
	break;

	case 2:
	{
		colorLabel.set(color_MOOD3);
	}
	break;

	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::update_PreviewColors()
{
	const int NUM_Ranges = (int)NUM_RANGES;

	int aBg = 160;
	int aRg = 24;
	int aSel = 48;

	cBg.set(ofColor(ofColor::black, aBg));
	cBord.set(ofColor(ofColor::black, 164));

	c1.set(ofColor(color_MOOD1, aRg));
	c2.set(ofColor(color_MOOD2, aRg));
	c3.set(ofColor(color_MOOD3, aRg));

	//-

	//TODO:
	if (RANGE_Selected == 0)
	{
		cRange = color_MOOD1;
	}
	else if (RANGE_Selected == 1)
	{
		cRange = color_MOOD2;
	}
	else if (RANGE_Selected == 2)
	{
		cRange = color_MOOD3;
	}

	//-

	// Color for manual control value
	const float rangeSz = 1.f / (float)NUM_Ranges;

	if (controlManual < rangeSz)
	{
		cRangeRaw = color_MOOD1;
	}
	else if (controlManual < 2 * rangeSz)
	{
		cRangeRaw = color_MOOD2;
	}
	else if (controlManual < 3 * rangeSz)
	{
		cRangeRaw = color_MOOD3;
	}

	//// flip
	//if (controlManual < rangeSz)
	//{
	//	cRangeRaw = color_MOOD3;
	//}
	//else if (controlManual < 2 * rangeSz)
	//{
	//	cRangeRaw = color_MOOD2;
	//}
	//else if (controlManual < 3 * rangeSz)
	//{
	//	cRangeRaw = color_MOOD1;
	//}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_PreviewWidget()
{
	float gx, gy, gw, gh, ww, hh, pad;
	pad = 10;

	// Custom
	// Preview rectangle
	if (bUseCustomPreviewPosition)
	{
		gx = rectPreview.getX();
		gy = rectPreview.getY();
		ww = rectPreview.getWidth();
		hh = rectPreview.getHeight();
	}

	// Default position layout
	else
	{
		gw = ofGetWidth() - 2 * pad;
		gx = pad;
		gy = pad;
		ww = gw;
		hh = 50;
	}

	//TODO:
	////horizontal by default
	//if (!MODE_vertical && !bUseCustomPreviewPosition)
	//{
	//	gx = gx + gw;
	//}
	//else
	//{
	//}

	//-

	draw_PreviewWidget(gx, gy, ww, hh);
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_PreviewWidget(int x, int  y, int  w, int  h) // customize position and size
{
	ofPushStyle();
	{
		// Default widget position without draggable rectangle
		if (!bUseCustomPreviewPosition)
		{
			float gx, gy, gw, gh, ww, hh, pad;
			pad = 10;
			gw = ofGetWidth() - 2 * pad;
			gx = pad;
			gy = pad;
			gh = 50;
			ofPushStyle();
			ofFill();
			ofSetColor(0, 0, 0, 190);
			ofRectangle r = ofRectangle(gx, gy, gw, gh);
			// expand bigger
			float ro = 4.0f;
			float pp = 5;
			ofRectangle rr = ofRectangle(
				r.getX() - pp, r.getY() - pp,
				r.getWidth() + 2 * pp, r.getHeight() + 2 * pp);
			ofDrawRectRounded(rr, ro);
			ofPopStyle();
		}

		//-

		// States bar vs progress bar
		float hratio = 0.7f;
		float h1 = h * hratio;
		float h2 = h * (1 - hratio);
		float x2, y2, ww2, hh2;

		const int NUM_Ranges = (int)NUM_RANGES;

		// For 0.20f: if fps is 60. duration will be 60/5 frames = 12frames
		blinkDuration = 0.15f * ofGetFrameRate();

		float sizes = w / (float)NUM_TARGETS;
		float ro = 4.0f;
		float line = 2.0f;

		float padBg, padBox, padSel;
		padBg = h1 * 0.05f;
		padBox = h1 * 0.1f;
		padSel = h1 * 0.1f;

		// Alphas
		int aBg = 140;
		int aRg = 24;
		int aSel = 48;

		//-

		// 0. Main Bg expanded
		///*
		if (bUseCustomPreviewPosition)
		{
			ofFill();
			ofSetColor(0, 0, 0, 190);
			//ofSetColor(cBg);
			float pp = 5;
			ofRectangle rr = ofRectangle(
				rectPreview.getX() - pp, rectPreview.getY() - pp,
				rectPreview.getWidth() + 2 * pp, rectPreview.getHeight() + 2 * pp);
			ofDrawRectRounded(rr, ro);
		}
		//*/

		//-

		// Rect A

		// States boxes
		{
			float x1, x2, x3, xEnd;
			x1 = x;
			x2 = x1 + ((rLimit1)*sizes);
			x3 = x2 + ((rLimit2 - rLimit1) * sizes);
			xEnd = x3 + ((NUM_TARGETS - rLimit2) * sizes);

			for (int rg = 0; rg < NUM_Ranges; rg++)
			{
				float _w;
				float _x;

				switch (rg)
				{
				case 0:
				{
					ofSetColor(RANGE_Selected != 0 ? c1 : color_MOOD1);// disable alpha when range is selected
					_x = x1;
					_w = x2 - x1;
				}
				break;

				case 1:
				{
					ofSetColor(RANGE_Selected != 1 ? c2 : color_MOOD2);
					_x = x2;
					_w = x3 - x2;
				}
				break;

				case 2:
				{
					ofSetColor(RANGE_Selected != 2 ? c3 : color_MOOD3);
					_x = x3;
					_w = xEnd - x3;
				}
				break;
				}

				// Range box
				ofFill();
				ofDrawRectRounded(_x, y, _w, h1, ro);
			}
		}

		//-

		// 2. Target Boxes with labels

		for (int t = 0; t < NUM_TARGETS; t++)
		{
			if (t >= 0 && t < rLimit1)
			{
				ofSetColor(c1);
			}
			else if (t >= rLimit1 && t < rLimit2)
			{
				ofSetColor(c2);
			}
			else if (t >= rLimit2 && t < NUM_TARGETS)
			{
				ofSetColor(c3);
			}

			float xb, yb, wb, hb;
			xb = x + t * sizes + 0.5f * padBox;
			yb = y + 0.5f * padBox;
			wb = sizes - padBox;
			hb = h1 - padBox;

			//-

			// 2.1 Target Box

			ofFill();
			ofDrawRectRounded(xb, yb, wb, hb, ro);

			//-

			// 2.2 Text Label

			ofSetColor(0);
			float xOff = 3;
			float yOff = 5;

			// workaround to start from 1 (to 9)  instead of 0 (to 8).
			// slider indexes are not changed/correlated!
			bool startAt1InsteadOfZero = true;
#ifdef PREVIEW_START_AT_ZERO
			startAt1InsteadOfZero = false;
#endif
			string s = startAt1InsteadOfZero ? ofToString(t + 1) : ofToString(t);

			if (myFont.isLoaded())
			{
				myFont.drawString(s, xb + wb * 0.5f - xOff, yb + 0.5f * hb + yOff);
			}
			else
			{
				ofDrawBitmapString(s, xb + wb * 0.5f - xOff, yb + 0.5f * hb + yOff);
			}
		}

		//--

		// 3. Selected Box / Target

		// Blink disabling box draw

		if (bBlink)
		{
			blinkCounterFrames++;
			if (blinkCounterFrames >= blinkDuration)
			{
				bBlink = false;
			}
		}

		//-

		// 3.1. Filled box

		ofFill();
		float blinkFactor = 0.4f;
		if (!bBlink) ofSetColor(0, aSel);
		else ofSetColor(0, blinkFactor * aSel);
		ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f * padSel, y + 0.5f * padSel, sizes - padSel, h1 - padSel, ro);

		//-

		// 3.2. Border

		ofNoFill();
		ofSetLineWidth(line);
		if (!bBlink) ofSetColor(cBord);
		else ofSetColor(cBord.r, cBord.g, cBord.b, cBord.a * blinkFactor);
		ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f * padSel, y + 0.5f * padSel, sizes - padSel, h1 - padSel, ro);

		//----

		// B. Box

		// 4. Completed timer progress

		x2 = x;
		y2 = y + h1 + padBg;
		ww2 = w;
		hh2 = h2 - 2 * padBg;

		if (RANGE_Selected >= 0 && RANGE_Selected < 3)
		{
			float _w = 1;
			float wStep = 1;

			if (!MODE_Manual) // Mode A Range or B Markov
			{
				if (bModeExternalClock) // external clock
				{
					wStep = ww2 / MAX(1, (float)(countToDuration.get()));//width of any step
					_w = counterStepFromOne * wStep;
				}
				else // internal clock
				{
					wStep = ww2 / (float)MAX(1, (counterStepFromOne.getMax()));//width of any step
					_w = ofMap(counterStepFromOne, 1, counterStepFromOne.getMax() + 1, 0, ww2, true);
				}
			}

			// Manual mode don't have steps counter bc it waits the user commands!
			// The we just counT all the bar a single step.

			else // Mode Manual
			{
				_w = ofMap(timer_Progress, 0, 100, 0, ww2, true);
			}

			//-

			if (!bModeExternalClock)
			{
				float wTimer = ofMap(timer_Progress, 0, 100, 0, wStep, true);
				// scale by step timer to make it analog-continuous
				_w = _w + wTimer;//add step timer
			}

			//--

			// 4.1 Bg

			ofFill();
			ofSetColor(cBg);
			ofDrawRectRounded(x2, y2, ww2, hh2, ro);

			//-

			// 4. 2 Complete progress range 
			// Colored
			{
				if (!(MODE_Manual && bModeExternalClock))
				{
					if (RANGE_Selected == 0)
					{
						cRange = color_MOOD1;
					}
					else if (RANGE_Selected == 1)
					{
						cRange = color_MOOD2;
					}
					else if (RANGE_Selected == 2)
					{
						cRange = color_MOOD3;
					}
					ofSetColor(cRange);

					//--

					// 4.3 Bar bg rectangle

					ofFill();
					ofDrawRectRounded(x2, y2, _w, hh2, ro);

					// 4.4 Mark all range steps with vertical lines

					if (!MODE_Manual)
					{
						ofNoFill();
						ofSetLineWidth(line + 1.0f);
						float xStep;
						for (int m = 1; m < counterStepFromOne.getMax(); m++)
						{
							xStep = wStep * m;

							ofDrawLine(x2 + xStep, y2, x2 + xStep, y2 + hh2);
						}
					}
				}
			}

			//----

			// 5. Manual control line
			{
				if (MODE_Manual)
				{
					ofNoFill();
					float xx = x2 + controlManual * w;
					{
						float __x, __y, __w, __h;
						__w = 5;
						__h = hh2;
						__x = xx;
						__y = y2;
						ofRectangle r(__x, __y, __w, __h);

						ofPushMatrix();
						ofTranslate(-__w / 2.0, 0);
						ofFill();
						ofSetColor(cRangeRaw);
						ofDrawRectRounded(r, 2);
						ofPopMatrix();
					}
				}
			}
		}

		//--

		// Preview rectangle Bg when editing
		///*
		if (bUseCustomPreviewPosition)
		{
			if (bEdit_PreviewWidget)
			{
				/*
				ofPushStyle();
				ofSetColor(128, 64);
				ofDrawRectangle(rectPreview.getRectangle());
				//ofDrawRectangle(rectPreview);
				rectPreview.draw();
				ofPopStyle();
				*/

				rectPreview.draw();
			}
		}
		//*/

		//--

		//TODO:
		//// Markov debug preview
		//float ww =300;
		//float hh =300;
		//if(i == 0){
		//    ofSetColor(ofColor::red);
		//    ofDrawRectangle(ww, hh + 10, 10, 10);
		//}
		//
		//markov.draw(ww + 35, hh + 20);
		//
		//ofSetColor(ofColor::white);
		//ofDrawBitmapString("You can change the \ntransition matrix in \n'data/transitionMatrix.txt'", ww + 10, hh + 50);
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofxSurfingMoods::load_range(int r) // recall current range margins from data vector
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << r;

	if ((r < 0) || (r >= NUM_RANGES))
	{
		ofLogError("ofxSurfingMoods") << (__FUNCTION__) << "load_range. OUT OF RANGE";
	}
	else
	{
		//// Clamp ranges
		//if (r < 0)
		//	r = 0;
		//else if (r >= NUM_RANGES)
		//	r = NUM_RANGES - 1;

		if (ranges.size() > 0 && r < NUM_RANGES && r >= 0)
		{
			myRange.name = ranges[r].name;
			myRange.min = ranges[r].min;
			myRange.max = ranges[r].max;

			//-

			// load to the engine too
			Range_Min = myRange.min;
			Range_Max = myRange.max;
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::save_range(int r)
{
	if ((r < 0) || (r >= NUM_RANGES))
	{
		if (r != -1) ofLogError("ofxSurfingMoods") << (__FUNCTION__) << "OUT OF RANGE !";
	}
	else
	{
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << r;

		//// Clamp ranges
		//if (r < 0)
		//	r = 0;
		//else if (r >= NUM_RANGES)
		//	r = NUM_RANGES - 1;

		if (ranges.size() > 0 && r < NUM_RANGES && r >= 0)
		{
			ranges[r].min = myRange.min;
			ranges[r].max = myRange.max;
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::stopMachine()
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "stopMachine";

	counterStep = 0;
	directionUp = true;
	timer_Range.stop();

	TARGET_Selected_PRE = -1;
	RANGE_Selected_PRE = -1;

	RANGE_Selected = 0;

	// set the target to the first target pos of the range
	if (RANGE_Selected < ranges.size())
		TARGET_Selected = ranges[RANGE_Selected].min;

	// markov
	if (bMarkovFileFound)
	{
		ofxMC::Matrix mat(path_markovMatrix);
		markov.setup(mat, 0);
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::resetClock()
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "resetClock";

	//timer_Range.stop();
	bpmLenghtBars = 1;
	bpmSpeed = 120;
	//timer = 0;

	//workflow
	//stopMachine();
	//stop();
}

//--------------------------------------------------------------
void ofxSurfingMoods::resetBank(bool RANDOMIZED, bool SORT_RELATIVE)
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "resetBank";

	// erase bank targets
	for (int p = 0; p < NUM_TARGETS; p++)
	{
		if (p >= MAX_ITEMS) return;//avoid crashes

		if (SORT_RELATIVE)
		{
			presets_A[p] = MIN(p, NUM_PRESETS_A - 1);
			presets_B[p] = MIN(p, NUM_PRESETS_B - 1);
			presets_C[p] = MIN(p, NUM_PRESETS_C - 1);
		}
		else
		{
			if (!RANDOMIZED)
			{
				presets_A[p] = 0;
				presets_B[p] = 0;
				presets_C[p] = 0;
			}
			else
			{
				presets_A[p] = (int)ofRandom(0, NUM_PRESETS_A);
				presets_B[p] = (int)ofRandom(0, NUM_PRESETS_B);
				presets_C[p] = (int)ofRandom(0, NUM_PRESETS_C);
			}
		}
	}

	RANGE_Selected_PRE = -1;
	TARGET_Selected_PRE = -1;
	if (stopBack)
	{
		stopMachine();
	}
}


// player
//--------------------------------------------------------------
void ofxSurfingMoods::stop()
{
	if (bEnable && bPLAY)
	{
		bPLAY = false;

		if (stopBack)
		{
			stopMachine();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::play()
{
	if (bEnable && !bPLAY)
	{
		bPLAY = true;
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::setTogglePlay()
{
	if (bEnable)
	{
		if (bPLAY)
		{
			stop();

			if (stopBack)
			{
				stopMachine();
			}
		}
		else
		{
			play();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::setGui_Visible(bool b)
{
	bGui = b;
}

//--------------------------------------------------------------
void ofxSurfingMoods::setGui_ToggleVisible()
{
	bGui = !bGui;
}

//TODO:
////--------------------------------------------------------------
//void ofxSurfingMoods::setGui_AdvancedVertical_MODE(bool enable)
//{
//	MODE_vertical = enable;
//}

//--------------------------------------------------------------
void ofxSurfingMoods::setBpm(float bpm)
{
	bpmSpeed = bpm;
}

//--------------------------------------------------------------
void ofxSurfingMoods::setBarsScale(int bars)
{
	bpmLenghtBars = bars;
}

//--------------------------------------------------------------
void ofxSurfingMoods::doRunEngineStep()
{
	// This function can be used to trig ranges jumps externally without using the internal timer.

	int _RANGE_Selected_PRE = RANGE_Selected.get();

	//-

	// 1. Mode random

	//-

	// 2. Mode back loop:

	// Count times and cycle
	counterStep++;
	counterStep = counterStep % countToDuration;
	counterStepFromOne = counterStep + 1;// for gui user

	if (counterStep == 0)
	{
		//1. routine type A: 0-1-2 .. 2-1-0 .. 0-1-2 ..

		if (directionUp && (RANGE_Selected == NUM_RANGES - 1))
		{
			directionUp = false;
		}
		else if (!directionUp && (RANGE_Selected == 0))
		{
			directionUp = true;
		}

		//TODO:
		// 2. routine type B:  0-1-2 .. 0-1-2 ..

		//-

		// move on
		if (directionUp)
		{
			RANGE_Selected = RANGE_Selected + 1;
		}
		else
		{
			RANGE_Selected = RANGE_Selected - 1;
		}

		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "RANGE_Selected: " << RANGE_Selected;
	}

	//-

	// do randomize between min/max ranges

	if (!MODE_StartLocked.get()) // allows start from any target
	{
		if (!MODE_AvoidRepeat.get()) // allows repeat target
		{
			TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
		}
		else // avoids repeat same target
		{
			int _pre = TARGET_Selected.get();
			TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
			int count = 0;

			while (TARGET_Selected.get() == _pre)//not changed
			{
				TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);

				count++;
				if (count > 5) // max attemps to avoid infinite loops...
				{
					if (Range_Min != _pre) TARGET_Selected = Range_Min;
					else TARGET_Selected = Range_Max;

					break;
				}
			}
		}

	}

	// force start from first
	// when changed range, first step will be locked to the min from range
	else
	{
		// range changed
		if (RANGE_Selected.get() != _RANGE_Selected_PRE)
		{
			TARGET_Selected = Range_Min.get();
		}
		else
		{
			if (!MODE_AvoidRepeat.get()) // allows repeat target
			{
				TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
			}
			else // avoids repeat same target
			{
				int _pre = TARGET_Selected.get();
				TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);

				int count = 0;
				int MAX_TRIES = 5;

				while (TARGET_Selected.get() == _pre) // not changed
				{
					TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);

					count++;
					if (count > MAX_TRIES) // max attemps to avoid infinite loops...
					{
						if (Range_Min != _pre) TARGET_Selected = Range_Min;
						else TARGET_Selected = Range_Max;

						break;
					}
				}
			}
		}
	}

	//-

	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "TARGET RANDOM: " << TARGET_Selected;

	//-
}

//--------------------------------------------------------------
void ofxSurfingMoods::doBeatTick()
{
	if (bModeExternalClock && bExternalLocked) doRunStep();
}

//--------------------------------------------------------------
void ofxSurfingMoods::clone()
{
	ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << "clone targets";
	for (int i = TARGET_Selected; i < NUM_TARGETS; i++)
	{
		presets_A[i] = PRESET_A_Selected;
		presets_B[i] = PRESET_B_Selected;
		presets_C[i] = PRESET_C_Selected;
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::doRunStep(bool bforced)
{
	if (bforced ||
		(bPLAY || (bModeExternalClock && bExternalLocked)))
	{
		// Restart

		if (!bModeExternalClock) timer_Range.start(false);

		//-

		// Modes

		if (MODE_Ranged.get())
		{
			doRunEngineStep();
		}

		//-

		else if (MODE_Manual)
		{
			int _RANGE_Selected_PRE = RANGE_Selected.get();

			//-

			if (controlManual < 1.f / 3.f) {
				RANGE_Selected = 0;
			}
			else if (controlManual < 2.f / 3.f) {
				RANGE_Selected = 1;
			}
			else if (controlManual <= 1.f) {
				RANGE_Selected = 2;
			}

			//// flip
			//if (controlManual < 1.f / 3.f) {
			//	RANGE_Selected = 2;
			//}
			//else if (controlManual < 2.f / 3.f) {
			//	RANGE_Selected = 1;
			//}
			//else if (controlManual <= 1.f) {
			//	RANGE_Selected = 0;
			//}

			//-

			// Do randomize between min/max ranges

			//int _RANGE_Selected_PRE = RANGE_Selected.get();

			if (!MODE_StartLocked.get())//allows start from any target
			{
				if (!MODE_AvoidRepeat.get())// allows repeat target
				{
					TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
				}
				else//avoids repeat same target
				{
					int _pre = TARGET_Selected.get();
					TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
					int count = 0;

					while (TARGET_Selected.get() == _pre)//not changed
					{
						TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);

						count++;
						if (count > 5)//max attemps to avoid infinite loops...
						{
							if (Range_Min != _pre) TARGET_Selected = Range_Min;
							else TARGET_Selected = Range_Max;

							break;
						}
					}
				}

			}
			// when changed range, first step will be locked to the min from range
			else
			{
				// range changed
				if (_RANGE_Selected_PRE != RANGE_Selected.get())
				{
					TARGET_Selected = Range_Min.get();
				}
				else
				{
					if (!MODE_AvoidRepeat.get()) // allows repeat target
					{
						TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
					}
					else // avoids repeat same target
					{
						int _pre = TARGET_Selected.get();
						TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
						int count = 0;

						while (TARGET_Selected.get() == _pre) // not changed
						{
							TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);

							count++;
							if (count > 5) // max attemps to avoid infinite loops...
							{
								if (Range_Min != _pre) TARGET_Selected = Range_Min;
								else TARGET_Selected = Range_Max;

								break;
							}
						}
					}
				}
			}
		}

		//-

		else if (MODE_MarkovChain.get())
		{
			if (bMarkovFileFound) {
				int _RANGE_Selected_PRE = RANGE_Selected.get();

				// Count times and cycle
				counterStep++;
				counterStep = counterStep % countToDuration;
				counterStepFromOne = counterStep + 1;// for gui user

				// Type A: mode stay amount of counter
				//if (counterStep == 0)
				// Type B: ignoring counter. just 1
				{
					markov.update();
					TARGET_Selected = markov.getState();

					//--

					// Range not changed
					if (RANGE_Selected.get() == _RANGE_Selected_PRE)
					{
						if (MODE_AvoidRepeat.get()) // avoids repeat same target
						{
							int _pre = TARGET_Selected.get();

							markov.update();
							TARGET_Selected = markov.getState();

							int count = 0;
							int MAX_TRIES = 100;

							while (TARGET_Selected.get() == _pre) // not changed
							{
								markov.update();
								TARGET_Selected = markov.getState();

								count++;
								if (count > MAX_TRIES) // max attemps to avoid infinite loops...
								{
									// do not renew
									break;
								}
							}
						}
					}

					//--

					ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "TARGET: " << TARGET_Selected;

					// TODO: only implemented to 3 ranges..
					if (TARGET_Selected >= ranges[0].min && TARGET_Selected <= ranges[0].max) {
						RANGE_Selected = 0;
					}
					else if (TARGET_Selected >= ranges[1].min && TARGET_Selected <= ranges[1].max) {
						RANGE_Selected = 1;
					}
					else if (TARGET_Selected >= ranges[2].min && TARGET_Selected <= ranges[2].max) {
						RANGE_Selected = 2;
					}

					//ofLogNotice("ofxSurfingMoods")<<(__FUNCTION__) << "RANGE_Selected: " << RANGE_Selected;
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::timer_Range_Complete(int& args)
{
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "\n";

	ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << "timer_Range_Complete";
	doRunStep();
}

//--------------------------------------------------------------
void ofxSurfingMoods::timer_Range_Started(int& args)
{
	ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << "timer_Range_Started";
}

//--------------------------------------------------------------
void ofxSurfingMoods::saveBanks(std::string path)
{
	// Store bank, each target to their preset/presets_ABC settings
	ofJson pt;
	js_targets.clear();

	for (int i = 0; i < NUM_TARGETS; i++)
	{
		// Clamp min
		int prst_A = (int)MAX(presets_A[i], 0);
		int prst_B = (int)MAX(presets_B[i], 0);
		int prst_C = (int)MAX(presets_C[i], 0);

		// Clamp max
		prst_A = (int)MIN(prst_A, NUM_PRESETS_A);
		prst_B = (int)MIN(prst_B, NUM_PRESETS_B);
		prst_C = (int)MIN(prst_C, NUM_PRESETS_C);

		pt["preset_A"] = prst_A;
		pt["preset_B"] = prst_B;
		pt["preset_C"] = prst_C;

		//-

		js_targets.push_back(pt);

		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "presets: " << pt;
	}
	bool bSaved = ofSavePrettyJson(path + filename_Bank, js_targets);

	if (bSaved)
	{
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "ofSaveJson: " << js_targets;
	}
	else
	{
		ofLogError("ofxSurfingMoods") << (__FUNCTION__) << "CAN'T SAVE FILES INTO: " << path;
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::saveSettings(std::string path)
{
	// store app state
	ofXml _settings;
	ofSerialize(_settings, params_AppSettings);
	std::string _path = path + filename_AppSettings + ".xml";
	_settings.save(_path);
	ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << _path;
}

//--------------------------------------------------------------
bool ofxSurfingMoods::loadBanks(std::string path)
{
	// 2. bank targets presets_A/presets_C
	std::string pathBank = path + filename_Bank;
	ofFile file(pathBank);

	if (file.exists())
	{
		// parse json
		file >> js_targets;

		ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << js_targets;
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "LOADED FILE: " << pathBank;
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << js_targets;

		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << pathBank + " json file must be present and formatted as expected!";

		// avoid crashes
		int p = 0;
		for (auto& js_tar : js_targets)
		{
			if ((!js_tar.empty()) && (p < NUM_TARGETS))
			{
				ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << endl << js_tar;

				int prst_A;
				int prst_B;
				int prst_C;
				prst_A = js_tar["preset_A"];
				prst_B = js_tar["preset_B"];
				prst_C = js_tar["preset_C"];

				// Clamp min
				prst_A = (int)MAX(prst_A, 0);
				prst_B = (int)MAX(prst_B, 0);
				prst_C = (int)MAX(prst_C, 0);

				// Clamp max
				presets_A[p] = (int)MIN(prst_A, NUM_PRESETS_A);
				presets_B[p] = (int)MIN(prst_B, NUM_PRESETS_B);
				presets_C[p] = (int)MIN(prst_C, NUM_PRESETS_C);

				ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << presets_A[p];
				ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << presets_B[p];
				ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << presets_C[p];
				p++;
			}
		}

		return true;
	}
	else
	{
		ofLogError("ofxSurfingMoods") << (__FUNCTION__) << pathBank << " NOT FOUND!";
		return false;
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::loadSettings(std::string path)
{
	//TODO:
	//ofSetDataPathRoot();
	//ofToDataPath();

	//-

	// 1. load panel settings
	ofXml _settings;

	std::string _path = path + filename_AppSettings + ".xml";
	bool bLoaded = _settings.load(_path);

	if (bLoaded)
	{
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << "LOADED: " << _path;
		ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << endl << endl << _settings.toString();

		ofDeserialize(_settings, params_AppSettings);
	}
	else
	{
		ofLogError("ofxSurfingMoods") << (__FUNCTION__) << "FILE NOT FOUND: " << _path;

		// workaround
		// Should check rect settings file instead..
		// Set Default layout
		doResetPreviewWidget();
		doResetManualSlider();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::Changed_Params_Listeners(ofAbstractParameter& e)
{
	if (bDISABLE_CALLBACKS) return;
	{
		std::string name = e.getName();

		if (name != "COMPLETE") ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << name << " : " << e;

		if (0) {}

		else if (name == countToDuration.getName())
		{
			countToDuration = ofClamp(countToDuration, countToDuration.getMin(), countToDuration.getMax());

			counterStepFromOne.setMin(1);
			counterStepFromOne.setMax(countToDuration.get());
		}

		else if (name == bPLAY.getName())
		{
			if (bPLAY)
			{
				// 60,000 / bpmSpeed = MS
				timer = bpmLenghtBars * (BPM_BAR_RATIO * (60000 / bpmSpeed));
				timer_Range.start(false);
				bIsPlaying = true;

				//-

				counterStep = 0;
				counterStepFromOne = counterStep + 1; // for gui user

				// workflow
				// enable some mode
				if (!MODE_MarkovChain && !MODE_Manual && !MODE_Ranged)
				{
					MODE_Ranged = true;
					MODE_MarkovChain = false;
					MODE_Manual = false;
				}
			}
			else
			{
				timer_Range.stop();
				bIsPlaying = false;

				counterStep = 0;
				counterStepFromOne = 0;
				RANGE_Selected = 0;
			}
		}

		else if (name == bModeExternalClock.getName())
		{
			bExternalLocked = bModeExternalClock;//?
		}

		else if (name == bClone_TARGETS.getName())
		{
			if (bClone_TARGETS)
			{
				bClone_TARGETS = false;
				clone();
			}
		}

		else if (name == TARGET_Selected.getName())
		{
			ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << " " << name << ":" << TARGET_Selected;

			// Blink
			bBlink = true;
			blinkCounterFrames = 0;

			//-

			// changed
			if (TARGET_Selected != TARGET_Selected_PRE)
			{
				// Limited target
				if (TARGET_Selected > NUM_TARGETS - 1)
				{
					TARGET_Selected = NUM_TARGETS - 1;
				}
				else if (TARGET_Selected < 0)
				{
					TARGET_Selected = 0;
				}

				//-

				// Targets management

				//TODO:
				// Save not required if target contents (selected A-B-C) not changed..

				if (bAutoSave_Target && TARGET_Selected_PRE >= 0)
				{
					//TODO:
					// should clamp!
					// auto save
					if (PRESET_A_Enable) presets_A[TARGET_Selected_PRE] = PRESET_A_Selected;
					if (PRESET_B_Enable) presets_B[TARGET_Selected_PRE] = PRESET_B_Selected;
					if (PRESET_C_Enable) presets_C[TARGET_Selected_PRE] = PRESET_C_Selected;
				}

				//-

				if (bAutoLoad_Target)
				{
					// auto load
					if (PRESET_A_Enable) PRESET_A_Selected = presets_A[TARGET_Selected];
					if (PRESET_B_Enable) PRESET_B_Selected = presets_B[TARGET_Selected];
					if (PRESET_C_Enable) PRESET_C_Selected = presets_C[TARGET_Selected];
				}

				//--

				// TODO: workflow when user has selected a target would load name of the range where it is

				//if (TARGET_Selected >= 0 && TARGET_Selected < rLimit1)
				//{
				//	load_range(0);
				//}
				//else if (TARGET_Selected >= rLimit1 && TARGET_Selected < rLimit2)
				//{
				//	load_range(1);
				//}
				//else if (TARGET_Selected >= rLimit2 && TARGET_Selected <= NUM_TARGETS - 1)
				//{
				//	load_range(2);
				//}

				//-

				//updateLabels();

				//-

				TARGET_Selected_PRE = TARGET_Selected;
			}

			// not changed. re trig
			else
			{
				if (bAutoLoad_Target)
				{
					// auto load
					if (PRESET_A_Enable) PRESET_A_Selected = presets_A[TARGET_Selected];
					if (PRESET_B_Enable) PRESET_B_Selected = presets_B[TARGET_Selected];
					if (PRESET_C_Enable) PRESET_C_Selected = presets_C[TARGET_Selected];
				}
			}
		}

		//else if (name == "PRESET A" || name == "PRESET B" || name == "PRESET C")
		//{
		//	updateLabels();
		//}

		else if (name == bpmSpeed.getName())
		{
			// 60,000 / bpmSpeed = MS
			timer = bpmLenghtBars * (BPM_BAR_RATIO * (60000 / bpmSpeed));
		}

		else if (name == timer.getName())
		{
			if (bPLAY)
			{
				timer_Range.setTime(timer);
			}
		}

		else if (name == bpmLenghtBars.getName())
		{
			// 60,000 / bpmSpeed = MS
			timer = bpmLenghtBars * (BPM_BAR_RATIO * (60000 / bpmSpeed));
		}

		// Resets
		else if (name == bResetClockSettings.getName() && bResetClockSettings)
		{
			bResetClockSettings = false;
			resetClock();
		}
		else if (name == bReset_Bank.getName() && bReset_Bank)
		{
			bReset_Bank = false;
			resetBank(false, false);//all to 0
			stop();
		}
		else if (name == bResetSort_Bank.getName() && bResetSort_Bank)
		{
			bResetSort_Bank = false;
			resetBank(false, true);//correlative sort
			stop();
		}
		else if (name == bRandomize_Bank.getName() && bRandomize_Bank)
		{
			bRandomize_Bank = false;
			resetBank(true, false);//relative random
			stop();
		}

		/*
		else if (name == bEdit_PreviewWidget.getName())
		{
			if (bEdit_PreviewWidget)
			{
				rectPreview.enableEdit();
			}
			else
			{
				rectPreview.disableEdit();
			}
		}
		*/

		// Modes
		else if (name == MODE_MarkovChain.getName())
		{
			if (MODE_MarkovChain)
			{
				MODE_Manual = false;
				MODE_Ranged = false;
			}
			else refresModeshWorkflow();
		}
		else if (name == MODE_Ranged.getName())
		{
			if (MODE_Ranged)
			{
				MODE_Manual = false;
				MODE_MarkovChain = false;
			}
			else refresModeshWorkflow();
		}
		else if (name == MODE_Manual.getName())
		{
			if (MODE_Manual)
			{
				MODE_Ranged = false;
				MODE_MarkovChain = false;
			}
			else refresModeshWorkflow();
		}

		// Manual control
		else if (name == controlManual.getName())
		{
			// workflow
			if (!bModeExternalClock)
			{
				if (bModeAutomatic)
				{
					if (!bPLAY.get())
					{
						bPLAY = true;
					}
				}
				else
				{
					bool bforce = false;

					//// flip
					//if (controlManual < 1.f / 3.f) {
					//	if (RANGE_Selected.get() != 2) bforce = true;
					//}
					//else if (controlManual < 2.f / 3.f) {
					//	if (RANGE_Selected.get() != 1) bforce = true;
					//}
					//else if (controlManual <= 1.f) {
					//	if (RANGE_Selected.get() != 0) bforce = true;
					//}

					if (controlManual < 1.f / 3.f) {
						if (RANGE_Selected.get() != 0) bforce = true;
					}
					else if (controlManual < 2.f / 3.f) {
						if (RANGE_Selected.get() != 1) bforce = true;
					}
					else if (controlManual <= 1.f) {
						if (RANGE_Selected.get() != 2) bforce = true;
					}

					if (!bforce)
					{
						if (!timer_Range.bIsRunning) {
							doRunStep(true);
						}
					}
					else { // don't wait to the timer ends to force change to the new range
						doRunStep(true);
					}
				}
			}
			else if (!bExternalLocked.get())
			{
				bExternalLocked = true;
			}
		}

		// workflow
		else if (name == bMode_Edit.getName())
		{
			if (bMode_Edit.get())
			{
				bAutoSave_Target = true;
				bAutoSave_Range = true;
			}
			else
			{
				bAutoSave_Target = false;
				bAutoSave_Range = false;
			}
		}

		// workflow
		else if (name == bModeAutomatic.getName())
		{
			if (!bModeAutomatic)
				if (bPLAY.get())
				{
					bPLAY = false;
				}
		}

		else if (name == ui.bMinimize.getName())
		{
			if (ui.bMinimize) {

				if (bGui_Advanced)bGui_Advanced = false;
				//bExpand = false;
			}
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::Changed_Ranges(ofAbstractParameter& e)
{
	if (bDISABLE_CALLBACKS) return;
	{
		std::string name = e.getName();
		ofLogVerbose("ofxSurfingMoods") << (__FUNCTION__) << name << " : " << e;

		if (name == RANGE_Selected.getName())
		{
			ofLogNotice("ofxSurfingMoods") << (__FUNCTION__) << RANGE_Selected;

			//TODO
			// split to functions to disable autosave when playing mode..

			// auto save previous
			if (RANGE_Selected != RANGE_Selected_PRE)
			{
				int r;
				if (bAutoSave_Range)
				{
					// save range
					r = RANGE_Selected_PRE;
					save_range(r);
				}

				//-

				if (bAutoLoad_Range)
				{
					// load range
					r = RANGE_Selected;
					load_range(r);
				}

				//-

				RANGE_Selected_PRE = RANGE_Selected;
			}

			//--

			refresh_MOOD_Color();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::setupGui()
{
	ui.setName("ofxSurfingMoods");
	ui.setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	ui.setup();

	ui.addWindowSpecial(bGui);
	ui.addWindowSpecial(bGui_Advanced);
	ui.addWindowSpecial(bGui_Matrices);

	ui.startup();

	//--

	//TODO:
	// fixing..
	static bool bCustom2 = true;
	if (bCustom2)
	{
		ui.clearStyles();
		ui.AddStyle(bPLAY, OFX_IM_TOGGLE_BIG, 1, false);

		ui.AddStyle(MODE_Ranged, OFX_IM_TOGGLE_BIG, 3, true);
		ui.AddStyle(MODE_MarkovChain, OFX_IM_TOGGLE_BIG, 3, true);
		ui.AddStyle(MODE_Manual, OFX_IM_TOGGLE_BIG, 3, false);
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::doResetManualSlider()
{
	// panels sizes
	static float ww;
	static float hh;
	static float xx;
	static float yy;
	int padx = 10;
	int pady = 10;
	int pady2 = 50;

	ImGuiCond flagsCond = ImGuiCond_None;
	flagsCond |= ImGuiCond_FirstUseEver;

	ww = 150;
	xx = ofGetWidth() - ww - padx;
	yy = rectPreview.getRectangle().getBottomRight().y + pady;
	hh = ofGetHeight() - yy - pady;

	flagsCond = ImGuiCond_None;

	ImGui::SetNextWindowSize(ImVec2(ww, hh), flagsCond);
	ImGui::SetNextWindowPos(ImVec2(xx, yy), flagsCond);
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_ManualSlider()
{
	if (bGui_ManualSlider && MODE_Manual)
	{
		if (bResetSlider)
		{
			bResetSlider = false;

			doResetManualSlider();
		}

		// workaround
		if (bResetLayout)
		{
			bResetLayout = false;

			doResetPreviewWidget();
			doResetManualSlider();
		}

		//--

		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(40, 200));
		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
			window_flags |= ImGuiWindowFlags_NoBackground;
			window_flags |= ImGuiWindowFlags_NoScrollbar;
			if (!bGui_ManualSliderHeader) window_flags |= ImGuiWindowFlags_NoTitleBar;

			if (ui.BeginWindow(bGui_ManualSlider, window_flags))
			{
				// markers zones
				float x1, x2, gap, yy, ww, hh;
				ww = ofxImGuiSurfing::getPanelWidth();
				hh = ofxImGuiSurfing::getPanelHeight();
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				float linew = 2.f;
				float linea = 0.15f;
				ImVec4 cm = ImVec4(0.0f, 0.0f, 0.0f, linea);

				gap = 10;
				ww -= 2 * gap;
				x1 = p.x + gap;
				x2 = x1 + ww;

				yy = p.y + 0.33 * hh;
				draw_list->AddLine(ImVec2(x1, yy), ImVec2(x2, yy), ImGui::GetColorU32(cm), linew);

				yy = p.y + 0.66 * hh;
				draw_list->AddLine(ImVec2(x1, yy), ImVec2(x2, yy), ImGui::GetColorU32(cm), linew);

				//-

				// v slider

				auto c = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
				ImVec4 _cBg = ImVec4(c.x, c.y, c.z, c.w * 0.2);
				ImGui::PushStyleColor(ImGuiCol_SliderGrab, cRange);
				ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, cRangeRaw);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, _cBg);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, _cBg);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, _cBg);
				{
					ImVec2 sz = ImVec2(-1.f, -1.f);
					bool bNoName = true;
					bool bNoNumber = true;
					ofxImGuiSurfing::AddVSlider(controlManual, sz, bNoName, bNoNumber);
				}
				ImGui::PopStyleColor(5);

				ui.EndWindow();
			}
		}
		ImGui::PopStyleVar();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_GameMode()
{
	//--

	// Colorize MOODS
	float a;
	ImVec4 c;

	if (RANGE_Selected == 0) c = color_MOOD1;
	else if (RANGE_Selected == 1) c = color_MOOD2;
	else if (RANGE_Selected == 2) c = color_MOOD3;

	//external
	if (bModeExternalClock) a = 1.0f;
	//internal playing
	else a = ofMap(1 - timer_Range.getNormalizedProgress(), 0, 1, 0.35, 1, true);

	//ImVec4 ca = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * a);
	//ImVec4 ca2 = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * (a * 0.5));//lower
	ImVec4 ca = (ImVec4)ImColor(c.x, c.y, c.z, c.w * a);
	ImVec4 ca2 = (ImVec4)ImColor(c.x, c.y, c.z, c.w * (a * 0.5));//lower

	//--

	//// Text + Main Controls
	//ImGui::PushStyleColor(ImGuiCol_Text, ca);
	//{
	//	//std::string s;
	//	//if (MODE_Ranged) s = MODE_Ranged.getName();
	//	//else if (MODE_MarkovChain) s = MODE_MarkovChain.getName();
	//	//else if (MODE_Manual) s = MODE_Manual.getName();
	//	//ui.AddSpacingSeparated();
	//	//ui.AddLabelHuge(s.c_str(), false, true);
	//	//ui.AddSpacing();
	//	//ui.AddSpacing();

	//	// Mood
	//	ui.pushStyleFont(OFX_IM_FONT_BIG);
	//	ui.Add(RANGE_Selected, OFX_IM_DRAG);
	//	//ui.Add(RANGE_Selected, OFX_IM_INACTIVE);
	//	ui.popStyleFont();

	//	//ui.AddSpacing();

	//	//// State
	//	//ui.Add(TARGET_Selected, OFX_IM_HSLIDER_SMALL);
	//	////ui.Add(TARGET_Selected, OFX_IM_DEFAULT, 1, false);
	//}
	//ImGui::PopStyleColor();

	//ui.AddSpacing();

	//--

	//bool b = false;
	////bool b = ui.bMinimize;
	////float h = (b ? 1 : 2) * ui.getWidgetsHeightUnit();

	float h = 2 * ui.getWidgetsHeightUnit();

	string toolTip = "";
	bool bFlip = true;
	int amountButtonsPerRowClicker = 3;
	bool bResponsiveButtonsClicker = true;

	//--

	// State / Target

	ui.AddLabelBig(TARGET_Selected.getName());
	//ui.AddLabelBig(TARGET_Selected.getName(), false, b);

	if (!bUseColorizedMatrices) ofxImGuiSurfing::AddMatrixClicker(TARGET_Selected, h);
	else ofxImGuiSurfing::AddMatrixClickerLabels(TARGET_Selected, keyCommandsChars, colors, bResponsiveButtonsClicker, amountButtonsPerRowClicker, true, h, toolTip, bFlip);
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_Matrices()
{
	if (bGui_Matrices)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_SMALL;

		if (ui.BeginWindowSpecial(bGui_Matrices))
		{
			//bool b = ui.bMinimize;
			//float h = (b ? 1 : 2) * ui.getWidgetsHeightUnit();

			float h = 2 * ui.getWidgetsHeightUnit();

			string toolTip = "";
			bool bFlip = true;
			int amountButtonsPerRowClicker = 3;
			bool bResponsiveButtonsClicker = true;

			//--

			// State / Target

			ui.AddLabelBig(TARGET_Selected.getName(), false);

			if (!bUseColorizedMatrices) ofxImGuiSurfing::AddMatrixClicker(TARGET_Selected, h);
			else ofxImGuiSurfing::AddMatrixClickerLabels(TARGET_Selected, keyCommandsChars, colors, bResponsiveButtonsClicker, amountButtonsPerRowClicker, true, h, toolTip, bFlip);

			if (!ui.bMinimize) {
				ui.AddSpacing();
				ui.AddSpacing();
				ui.Add(bExpand, OFX_IM_TOGGLE_ROUNDED_SMALL);
				ui.AddSpacing();
			}

			//--

			//if (!ui.bMinimize)
			if (bExpand)
			{
				ui.AddSeparated();

				h = 1 * ui.getWidgetsHeightUnit();

				//--

				// Presets / Devices

				if (PRESET_A_Enable)
				{
					ui.AddLabelBig(PRESET_A_Selected.getName(), false);

					if (!bUseColorizedMatrices) ofxImGuiSurfing::AddMatrixClicker(PRESET_A_Selected, h);
					else ofxImGuiSurfing::AddMatrixClickerLabels(PRESET_A_Selected, keyCommandsChars, colors, bResponsiveButtonsClicker, amountButtonsPerRowClicker, true, h, toolTip, bFlip);

					ui.AddSpacingSeparated();
				}

				if (PRESET_B_Enable)
				{
					ui.AddLabelBig(PRESET_B_Selected.getName(), false);

					if (!bUseColorizedMatrices) ofxImGuiSurfing::AddMatrixClicker(PRESET_B_Selected, h);
					else ofxImGuiSurfing::AddMatrixClickerLabels(PRESET_B_Selected, keyCommandsChars, colors, bResponsiveButtonsClicker, amountButtonsPerRowClicker, true, h, toolTip, bFlip);

					ui.AddSpacingSeparated();
				}

				if (PRESET_C_Enable)
				{
					ui.AddLabelBig(PRESET_C_Selected.getName(), false);

					if (!bUseColorizedMatrices) ofxImGuiSurfing::AddMatrixClicker(PRESET_C_Selected, h);
					else ofxImGuiSurfing::AddMatrixClickerLabels(PRESET_C_Selected, keyCommandsChars, colors, bResponsiveButtonsClicker, amountButtonsPerRowClicker, true, h, toolTip, bFlip);
				}

				if (!ui.bMinimize) {
					ui.AddSpacingBigSeparated();
					ui.Add(bResetSort_Bank, OFX_IM_TOGGLE_SMALL);
				}
			}

			ui.EndWindowSpecial();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_Main()
{
	if (bGui)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_SMALL;

		if (ui.BeginWindowSpecial(bGui))
		{
			//--

			// Minimize
			ui.Add(ui.bMinimize, OFX_IM_TOGGLE_ROUNDED);

			ui.AddSpacingSeparated();

			//--

			static bool bOpen = false;
			ImGuiTreeNodeFlags _flagt;

			//-

			// Colorize MOODS
			float a;
			ImVec4 c;

			if (RANGE_Selected == 0) c = color_MOOD1;
			else if (RANGE_Selected == 1) c = color_MOOD2;
			else if (RANGE_Selected == 2) c = color_MOOD3;

			//external
			if (bModeExternalClock) a = 1.0f;
			//internal playing
			else a = ofMap(1 - timer_Range.getNormalizedProgress(), 0, 1, 0.35, 1, true);

			//ImVec4 ca = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * a);
			//ImVec4 ca2 = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * (a * 0.5));//lower
			ImVec4 ca = (ImVec4)ImColor(c.x, c.y, c.z, c.w * a);
			ImVec4 ca2 = (ImVec4)ImColor(c.x, c.y, c.z, c.w * (a * 0.5));//lower

			//-

			// Play
			{
				ui.AddSpacing();

				//ui.Add(bModeExternalClock, OFX_IM_TOGGLE_SMALL);
				float _h = ui.getWidgetsHeightUnit();

				// External clock
				ofxImGuiSurfing::AddBigToggleNamed(bModeExternalClock, -1, _h, bModeExternalClock.getName(), "CLOCK INTERNAL");

				bool b = bPLAY.get();
				if (b)
				{
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ca);
					ImGui::PushStyleColor(ImGuiCol_Button, ca);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ca2);
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Border, ca);
					//ImGui::PushStyleColor(ImGuiCol_BorderShadow, ca);
				}

				{
					// internal
					if (!bModeExternalClock)
					{
						ofxImGuiSurfing::AddBigToggleNamed(bPLAY, -1, -1, "PLAYING", "PLAY");
					}

					// external
					else
					{
						ui.Add(bExternalLocked, OFX_IM_TOGGLE_BIG);
					}
				}

				if (b)
				{
					ImGui::PopStyleColor(3);
				}
				else
				{
					ImGui::PopStyleColor(1);
				}

				//--

				if (bModeExternalClock)
				{
					static ofParameter<bool> _bBeat{ "Force Beat", false };
					if (ui.Add(_bBeat, OFX_IM_BUTTON_SMALL))
					{
						if (_bBeat) {
							_bBeat = false;

							doBeatTick();
						}
					}
				}

				//ui.AddSpacing();
			}

			//--

			if (!ui.bMinimize)
			{
				ui.AddSpacingSeparated();
				ui.AddSpacing();
				//ui.AddLabel("WIDGETS");
				//ui.AddSpacing();

				ui.Add(bGui_PreviewWidget, OFX_IM_TOGGLE_BUTTON_ROUNDED);
				if (bGui_PreviewWidget) {
					ui.Indent();
					ui.Add(bUseCustomPreviewPosition, OFX_IM_TOGGLE_BUTTON_ROUNDED_MINI);
					ui.Unindent();
				}

				if (MODE_Manual) ui.Add(bGui_ManualSlider, OFX_IM_TOGGLE_BUTTON_ROUNDED);

				ui.Add(bGui_Matrices, OFX_IM_TOGGLE_BUTTON_ROUNDED);

				ui.AddSpacingSeparated();

				// Edit mode
				float _h = ui.getWidgetsHeightUnit();
				ofxImGuiSurfing::AddBigToggleNamed(bMode_Edit, -1, _h, "MODE EDIT", "MODE LOCKED", true);
			}

			//--

			ui.AddSpacingSeparated();

			//if (bInternalClock) 
			{
				// Clock

				// minimized
				if (ui.bMinimize && bModeExternalClock) {
					ui.Add(bpmSpeed, OFX_IM_SLIDER);
				}
				else if (ui.bMinimize)
				{
					bOpen = false;
					_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
					_flagt |= ImGuiTreeNodeFlags_Framed;

					if (ImGui::TreeNodeEx("CLOCK", _flagt))
					{
						ui.refreshLayout();

						// Mini Clock

						float _w3 = ui.getWidgetsWidth(3);
						float _w2 = ui.getWidgetsWidth(2);
						float _h = ui.getWidgetsHeightUnit();

						ui.Add(bpmSpeed, OFX_IM_SLIDER);
						if (!bModeExternalClock)
						{
							if (ImGui::Button("/ 2", ImVec2(_w3, _h))) {
								bpmSpeed = bpmSpeed / 2.0f;
							}
							ImGui::SameLine();

							if (ImGui::Button("* 2", ImVec2(_w3, _h))) {
								bpmSpeed = bpmSpeed * 2.0f;
							}
							ImGui::SameLine();

							if (ImGui::Button("RESET", ImVec2(_w3, _h))) {//to change the name..
								bResetClockSettings = true;
							}
						}

						ImGui::TreePop();
					}
					ui.refreshLayout();
				}

				// expanded
				else
				{
					// Clock

					bOpen = false;
					_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
					_flagt |= ImGuiTreeNodeFlags_Framed;

					if (ImGui::TreeNodeEx("CLOCK", _flagt))
					{
						ui.refreshLayout();

						float _w1 = ui.getWidgetsWidth(1);
						float _w2 = ui.getWidgetsWidth(2);
						float _h = ui.getWidgetsHeightUnit();

						ui.Add(bpmSpeed, OFX_IM_SLIDER, 1, false);

						if (!bModeExternalClock)
						{
							ui.Add(bpmSpeed, OFX_IM_STEPPER);

							if (ImGui::Button("/ 2", ImVec2(_w2, _h)))
							{
								bpmSpeed = bpmSpeed / 2.0f;
							}

							ImGui::SameLine();

							if (ImGui::Button("* 2", ImVec2(_w2, _h)))
							{
								bpmSpeed = bpmSpeed * 2.0f;
							}

							ui.Add(bResetClockSettings, OFX_IM_BUTTON_SMALL);
						}

						ImGui::TreePop();
					}
					ui.refreshLayout();
				}

				ui.AddSpacingSeparated();
			}

			//--

			// Modes

			// A. Ranged
			// B. Markov
			// C. Manual

			if (!ui.bMinimize)
			{
				bool bOpen2 = false;
				ImGuiTreeNodeFlags _flagt2 = (bOpen2 ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt2 |= ImGuiTreeNodeFlags_Framed;

				if (ImGui::TreeNodeEx("MODES", _flagt2))
				{
					ui.refreshLayout();

					// Modes
					ui.Add(MODE_Ranged, OFX_IM_TOGGLE_MEDIUM);
					if (bMarkovFileFound) {
						ui.Add(MODE_MarkovChain, OFX_IM_TOGGLE_MEDIUM);
					}
					else {
						ui.AddLabel("ERROR! MARKOV FILES NOT FOUND ON /DATA");
					}

					ui.Add(MODE_Manual, OFX_IM_TOGGLE_MEDIUM);

					ImGui::TreePop();
				}
				ui.refreshLayout();

				ui.AddSpacingSeparated();
			}

			//if (!ui.bMinimize) ui.AddSpacingSeparated();

			//--

			// Action

			bOpen = false;
			_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
			_flagt |= ImGuiTreeNodeFlags_Framed;

			if (ImGui::TreeNodeEx("ACTION", _flagt))
			{
				ui.refreshLayout();

				if (MODE_Manual)
				{
					ui.AddSpacing();
					ImGui::PushStyleColor(ImGuiCol_Text, ca);

					//ui.Add(controlManual, OFX_IM_SLIDER);
					ui.Add(controlManual, OFX_IM_HSLIDER_SMALL);

					ImGui::PopStyleColor();
				}
				else
				{
				}

				//--

				if (!MODE_Manual)
				{
					ui.Add(countToDuration, OFX_IM_STEPPER); // user setter
				}

				//--

				// For monitor only
				//if (!ui.bMinimize)
				if (!MODE_Manual)
				{
					if (countToDuration != 1)
					{
						ui.Add(counterStepFromOne, OFX_IM_INACTIVE);
					}
				}

				// Progress Bar
				if (!bModeExternalClock && !ui.bMinimize)
				{
					ui.Add(timer_Progress, OFX_IM_PROGRESS_BAR_NO_TEXT);

					if (!MODE_Manual && !bModeExternalClock && countToDuration != 1)
						ui.Add(timer_ProgressComplete, OFX_IM_PROGRESS_BAR_NO_TEXT);
				}

				//----

				ui.AddSpacing();

				// Text + Main Controls
				ImGui::PushStyleColor(ImGuiCol_Text, ca);
				{
					std::string s;
					if (MODE_Ranged) s = MODE_Ranged.getName();
					else if (MODE_MarkovChain) s = MODE_MarkovChain.getName();
					else if (MODE_Manual) s = MODE_Manual.getName();

					ui.AddSpacingSeparated();

					ui.AddLabelHuge(s.c_str(), false, true);

					ui.AddSpacing();
					ui.AddSpacing();

					// Mood
					ui.Add(RANGE_Selected, OFX_IM_DRAG);
					//ui.Add(RANGE_Selected, OFX_IM_INACTIVE);

					ui.AddSpacing();

					// State
					ui.Add(TARGET_Selected, OFX_IM_HSLIDER_SMALL);
					//ui.Add(TARGET_Selected, OFX_IM_DEFAULT, 1, false);
				}
				ImGui::PopStyleColor();

				ui.AddSpacing();

				//-

				if (!ui.bMinimize)
				{
					ui.AddSpacing();
					ui.AddLabel("TARGET > PRESETS", false, true);

					ui.AddSpacingSeparated();
					ui.AddSpacing();
				}

				if (ImGui::TreeNodeEx("PRESETS", ImGuiTreeNodeFlags_Framed))
				{
					ui.refreshLayout();

					if (!ui.bMinimize)
					{
						ui.Add(PRESET_A_Enable, OFX_IM_TOGGLE_SMALL);
						ui.Add(PRESET_B_Enable, OFX_IM_TOGGLE_SMALL);
						ui.Add(PRESET_C_Enable, OFX_IM_TOGGLE_SMALL);

						ui.AddSpacing();

						ui.AddSpacingSeparated();
					}
					ui.AddSpacing();

					if (PRESET_A_Enable) ui.Add(PRESET_A_Selected, OFX_IM_DEFAULT);
					if (PRESET_B_Enable) ui.Add(PRESET_B_Selected, OFX_IM_DEFAULT);
					if (PRESET_C_Enable) ui.Add(PRESET_C_Selected, OFX_IM_DEFAULT);

					//ui.AddSpacing();

					//ui.Add(bGui_Matrices, OFX_IM_TOGGLE_BUTTON_ROUNDED);

					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ui.refreshLayout();

			//--

			if (!ui.bMinimize)
			{
				ui.AddSpacingSeparated();

				ui.Add(bGui_Advanced, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
			}

			ui.EndWindowSpecial();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::doResetPreviewWidget()
{
	float gx, gy, gw, gh, ww, hh, pad;
	pad = 10;
	gw = ofGetWidth() - 2 * pad;
	gx = pad;
	gy = pad;
	ww = gw;
	hh = 50;
	rectPreview.setRectangle(ofRectangle(gx, gy, gw, hh)); // initialize
}

//--------------------------------------------------------------
void ofxSurfingMoods::drawGui()
{
	ui.Begin();
	{
		if (bGui) {
			draw_ImGui_Main();
			draw_ImGui_Matrices();
			draw_ImGui_Advanced();
		}

		// visible also when gui hidden!
		draw_ImGui_ManualSlider();
	}
	ui.End();

	//--

	if (!bGui) return;

	// Preview widget
	if (bGui_PreviewWidget) draw_PreviewWidget();

	// Help
	if (ui.bHelp) textBoxWidget.draw();
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_Advanced()
{
	// Advanced
	if (bGui_Advanced)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW
		{
			static bool bOpen = true;
			ImGuiTreeNodeFlags _flagt;

			if (ui.BeginWindowSpecial(bGui_Advanced))
			{
				//--

				//if (!ui.bMinimize) 
				{
					//ui.refreshLayout();
					//ui.AddSpacing();
					ui.Add(MODE_StartLocked, OFX_IM_TOGGLE_SMALL);
					ui.Add(MODE_AvoidRepeat, OFX_IM_TOGGLE_SMALL);
					//ui.AddSpacing();

					ui.AddSpacingSeparated();
				}

				//--
				// 
				// Tools
				{
					bOpen = false;
					_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
					_flagt |= ImGuiTreeNodeFlags_Framed;

					if (ImGui::TreeNodeEx("TOOLS", _flagt))
					{
						ui.refreshLayout();

						// Target panel
						ui.Add(bClone_TARGETS, OFX_IM_TOGGLE_SMALL);
						ui.Add(bResetSort_Bank, OFX_IM_TOGGLE_SMALL);
						ui.Add(bReset_Bank, OFX_IM_TOGGLE_SMALL);
						ui.Add(bRandomize_Bank, OFX_IM_TOGGLE_SMALL, 1, false);

						ImGui::TreePop();
					}
					ui.refreshLayout();

					ui.AddSpacingSeparated();
				}

				//--

				// Ranges
				{
					if (ImGui::TreeNodeEx("DEBUG LIMITS", _flagt))
					{
						ui.refreshLayout();
						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
						flags |= ImGuiTreeNodeFlags_Framed;
						flags |= ImGuiTreeNodeFlags_DefaultOpen;

						ui.AddGroup(parameters_ranges, flags);

						ImGui::TreePop();
					}
					ui.refreshLayout();

					ui.AddSpacingSeparated();
				}

				//--

				// Extra
				if (!ui.bMinimize)
				{
					//ui.AddSpacingSeparated();

					ui.AddSpacing();

					ui.Add(ui.bExtra, OFX_IM_TOGGLE_ROUNDED_MEDIUM);

					if (ui.bExtra)
					{
						ui.Indent();
						{
							//--

							ui.AddSpacing();

							ui.Add(bResetSort_Bank, OFX_IM_TOGGLE_SMALL);
							ui.AddSpacing();
							ui.Add(bKeys, OFX_IM_TOGGLE_ROUNDED_SMALL);
							ui.Add(ui.bHelp, OFX_IM_TOGGLE_ROUNDED_SMALL);

							if (MODE_Manual) ui.Add(bModeAutomatic, OFX_IM_TOGGLE_ROUNDED);

							ImGui::Separator();

							//--

							// Preview

							ui.Add(bGui_PreviewWidget, OFX_IM_TOGGLE_ROUNDED_SMALL);
							if (bGui_PreviewWidget)
							{
								ui.Indent();
								{
									ui.Add(bUseCustomPreviewPosition, OFX_IM_TOGGLE_ROUNDED_SMALL);
									if (bUseCustomPreviewPosition) {
										ui.Add(bEdit_PreviewWidget, OFX_IM_TOGGLE_ROUNDED_SMALL);
										if (ui.Add(bResetPreviewWidget, OFX_IM_TOGGLE_ROUNDED_SMALL))
										{
											bResetPreviewWidget = false;
											doResetPreviewWidget();
										}
									}
								}
								ui.Unindent();
							}

							//--

							// Slider

							if (MODE_Manual)
							{
								ImGui::Separator();

								ui.Add(bGui_ManualSlider, OFX_IM_TOGGLE_ROUNDED_SMALL);
								if (bGui_ManualSlider)
								{
									ui.Indent();
									{
										ui.Add(bGui_ManualSliderHeader, OFX_IM_TOGGLE_ROUNDED_SMALL);
										if (ui.Add(bResetSlider, OFX_IM_TOGGLE_ROUNDED_SMALL))
										{
											bResetSlider = true;
										}
									}
									ui.Unindent();
								}
							}
						}
						ui.Unindent();
					}
				}

				ui.EndWindowSpecial();
			}
		}
	}
}
