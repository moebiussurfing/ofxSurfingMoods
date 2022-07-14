
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
		ofLogError(__FUNCTION__) << "Markov file " << path_markovMatrix << " not found!";
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
		ofLogError(__FUNCTION__) << "ofTrueTypeFont FONT FILE '" << myTTF << "' NOT FOUND!";
		ofLogError(__FUNCTION__) << "Load default font.";

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

	//--------

	// startup
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

	bGui_Main.set("MOODS", true);
	bGui_Matrices.set("MATRICES", false);
	bGui_Advanced.set("ADVANCED", false);
	bGui_PreviewWidget.set("Preview Widget", false);
	bGui_ManualSlider.set("Manual Slider", false);
	bGui_ManualSliderHeader.set("Slider Header", false);

	bPLAY.set("PLAY", false);

	bpmSpeed.set("BPM", 120.f, 30.f, 400.f);//bmp
	bpmLenghtBars.set("BARS LEN", 1, 1, 16);//bars
	timer.set("TIMER", 1000, 1, 60000);//ms
	timer.setSerializable(false);

	TARGET_Selected.set("TARGET", 0, 0, NUM_TARGETS - 1);//target

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
	bEdit_PreviewWidget.set("Edit Preview Widget", false);

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
	bKeys.makeReferenceTo(guiManager.bKeys);
	//bHelp.makeReferenceTo(guiManager.bHelp);

	//---

	// Group params outside gui but to use listeners and xml settings
	parameters_ranges.setName("RANGES");
	parameters_ranges.add(RANGE_Selected);
	parameters_ranges.add(myRange.min);
	parameters_ranges.add(myRange.max);

	//---

	// Store params (grouped only to save/load, not to allow on gui or callbacks)
	params_AppSettings.setName("ofxSurfingMoods_AppSettings");
	params_AppSettings.add(bGui_Main);
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
	params_AppSettings.add(bModeClockExternal);
	params_AppSettings.add(guiManager.params_Advanced);

	//-

	// Group params for callback listener only
	params_Listeners.setName("MoodMachine_params");

	params_Listeners.add(bPLAY);
	params_Listeners.add(bModeClockExternal);
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
	params_Listeners.add(guiManager.bMinimize);

	// Exclude from file settings
	//bPLAY.setSerializable(false);
	counterStepFromOne.setSerializable(false);
	bGui_ManualSliderHeader.setSerializable(false);
	autoSaveLoad_settings.setSerializable(false);
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

	//TODO: 
	//crashes sometimes if no file present..
	bool b = rectPreview.loadSettings("_PreviewRect", path_rect, true);
	if (!b) rectPreview.setRect(25, 650, 700, 50);//initialize when no settings file created yet.
	rectPreview.disableEdit();

	//--

	// Load bank targets
	if (autoSaveLoad_settings.get())
	{
		if (!loadBanks(path_Folder))
		{
			// If settings file not found, we reset and create a default bank
			resetBank(false, true);//correlative sort
		}
	}

	// Load panel settings
	if (autoSaveLoad_settings.get())
	{
		loadSettings(path_Folder);
	}

	//TODO:
	//setGui_AdvancedVertical_MODE(false);

	//--

	refresh_MOOD_Color();
}

//--------------------------------------------------------------
void ofxSurfingMoods::update(ofEventArgs& args)
{
	if (bGui_Main) update_PreviewColors();

	//-

	//if (MODE_MarkovChain)
	//{
	//	i++;
	//}

	//-

	if (!bModeClockExternal)
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
//	//if (guiManager.bHelp) textBoxWidget.draw();
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
			ofLogNotice(__FUNCTION__) << "KEYS: " << (bKeys ? "ON" : "OFF");

			if (!bKeys)
			{
				ofLogNotice(__FUNCTION__) << "ALL KEYS DISABLED. PRESS 'k' TO ENABLE GAIN!";
			}
			else
			{
				ofLogNotice(__FUNCTION__) << "KEYS ENABLED BACK";
			}
		}

	//--

	// Disabler for all keys. (independent from bActive)
	if (!bKeys) return;

	ofLogNotice(__FUNCTION__) << (char)key << " [" << key << "]";

	// Modifiers
	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

	if (0) // debug
	{
		ofLogNotice(__FUNCTION__) << "mod_COMMAND: " << (mod_COMMAND ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_CONTROL: " << (mod_CONTROL ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_ALT: " << (mod_ALT ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_SHIFT: " << (mod_SHIFT ? "ON" : "OFF");
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
		if (bModeClockExternal) doBeatTick();
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
	ofLogNotice(__FUNCTION__) << (char)key << " [" << key << "]";

	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);
}

//--------------------------------------------------------------
void ofxSurfingMoods::addKeysListeners()
{
	ofLogNotice(__FUNCTION__);
	ofAddListener(ofEvents().keyPressed, this, &ofxSurfingMoods::keyPressed);
}

//--------------------------------------------------------------
void ofxSurfingMoods::removeKeysListeners()
{
	ofLogNotice(__FUNCTION__);
	ofRemoveListener(ofEvents().keyPressed, this, &ofxSurfingMoods::keyPressed);
}

//--------------------------------------------------------------
void ofxSurfingMoods::windowResized(int w, int h)
{
	ofLogNotice(__FUNCTION__) << w << ", " << h;

	// workaround
	bResetLayout = true;
}

//--------------------------------------------------------------
void ofxSurfingMoods::exit()
{
	// Preview rectangle
	rectPreview.saveSettings("_PreviewRect", path_rect, true);

	//-

	// Save settings
	if (autoSaveLoad_settings)
	{
		saveSettings(path_Folder);
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
	ofLogVerbose(__FUNCTION__);

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
		blinkDuration = 0.20f * ofGetFrameRate();

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
				if (bModeClockExternal) // external clock
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

			if (!bModeClockExternal)
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
				if (!(MODE_Manual && bModeClockExternal))
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
		if (bUseCustomPreviewPosition)
		{
			if (bEdit_PreviewWidget)
			{
				ofPushStyle();
				ofSetColor(128, 64);
				ofDrawRectangle(rectPreview);
				rectPreview.draw();
				ofPopStyle();
			}
		}

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
	ofLogNotice(__FUNCTION__) << r;

	if ((r < 0) || (r >= NUM_RANGES))
	{
		ofLogError(__FUNCTION__) << "load_range. OUT OF RANGE";
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
		if (r != -1) ofLogError(__FUNCTION__) << "OUT OF RANGE !";
	}
	else
	{
		ofLogNotice(__FUNCTION__) << r;

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
	ofLogNotice(__FUNCTION__) << "stopMachine";

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
	ofLogNotice(__FUNCTION__) << "resetClock";

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
	ofLogNotice(__FUNCTION__) << "resetBank";

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
	bGui_Main = b;
}

//--------------------------------------------------------------
void ofxSurfingMoods::setGui_ToggleVisible()
{
	bGui_Main = !bGui_Main;
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

		ofLogNotice(__FUNCTION__) << "RANGE_Selected: " << RANGE_Selected;
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

	ofLogNotice(__FUNCTION__) << "TARGET RANDOM: " << TARGET_Selected;

	//-
}

//--------------------------------------------------------------
void ofxSurfingMoods::doBeatTick()
{
	if (bModeClockExternal && bExternalLocked) doRunStep();
}

//--------------------------------------------------------------
void ofxSurfingMoods::clone()
{
	ofLogVerbose(__FUNCTION__) << "clone targets";
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
		(bPLAY || (bModeClockExternal && bExternalLocked)))
	{
		// Restart

		if (!bModeClockExternal) timer_Range.start(false);

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

					ofLogNotice(__FUNCTION__) << "TARGET: " << TARGET_Selected;

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

					//ofLogNotice(__FUNCTION__) << "RANGE_Selected: " << RANGE_Selected;
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::timer_Range_Complete(int& args)
{
	ofLogNotice(__FUNCTION__) << "\n";

	ofLogVerbose(__FUNCTION__) << "timer_Range_Complete";
	doRunStep();
}

//--------------------------------------------------------------
void ofxSurfingMoods::timer_Range_Started(int& args)
{
	ofLogVerbose(__FUNCTION__) << "timer_Range_Started";
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

		ofLogNotice(__FUNCTION__) << "presets: " << pt;
	}
	bool bSaved = ofSavePrettyJson(path + filename_Bank, js_targets);

	if (bSaved)
	{
		ofLogNotice(__FUNCTION__) << "ofSaveJson: " << js_targets;
	}
	else
	{
		ofLogError(__FUNCTION__) << "CAN'T SAVE FILES INTO: " << path;
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
	ofLogNotice(__FUNCTION__) << _path;
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

		ofLogVerbose(__FUNCTION__) << js_targets;
		ofLogNotice(__FUNCTION__) << "LOADED FILE: " << pathBank;
		ofLogNotice(__FUNCTION__) << js_targets;

		ofLogNotice(__FUNCTION__) << pathBank + " json file must be present and formatted as expected!";

		// avoid crashes
		int p = 0;
		for (auto& js_tar : js_targets)
		{
			if ((!js_tar.empty()) && (p < NUM_TARGETS))
			{
				ofLogVerbose(__FUNCTION__) << endl << js_tar;

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

				ofLogVerbose(__FUNCTION__) << presets_A[p];
				ofLogVerbose(__FUNCTION__) << presets_B[p];
				ofLogVerbose(__FUNCTION__) << presets_C[p];
				p++;
			}
		}

		return true;
	}
	else
	{
		ofLogError(__FUNCTION__) << pathBank << " NOT FOUND!";
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
		ofLogNotice(__FUNCTION__) << "LOADED: " << _path;
		ofLogNotice(__FUNCTION__) << endl << endl << _settings.toString();

		ofDeserialize(_settings, params_AppSettings);
	}
	else
	{
		ofLogError(__FUNCTION__) << "FILE NOT FOUND: " << _path;

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

		if (name != "COMPLETE") ofLogVerbose(__FUNCTION__) << name << " : " << e;

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

		else if (name == bModeClockExternal.getName())
		{
			bExternalLocked = bModeClockExternal;//?
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
			ofLogNotice(__FUNCTION__) << name << ":" << TARGET_Selected;

			// Blink
			bBlink = true;
			blinkCounterFrames = 0;

			//-

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

				if (target_autoSave && TARGET_Selected_PRE >= 0)
				{
					//TODO:
					// should clamp!
					// auto save
					if (PRESET_A_Enable) presets_A[TARGET_Selected_PRE] = PRESET_A_Selected;
					if (PRESET_B_Enable) presets_B[TARGET_Selected_PRE] = PRESET_B_Selected;
					if (PRESET_C_Enable) presets_C[TARGET_Selected_PRE] = PRESET_C_Selected;
				}

				//-

				if (target_autoLoad)
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
			if (!bModeClockExternal)
			{
				if (bModeAutomatic)
				{
					if (!bPLAY.get())
					{
						bPLAY = true;
					}
				}
				else {
					bool bforce = false;
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
		else if (name == bModeAutomatic.getName())
		{
			if (!bModeAutomatic)
				if (bPLAY.get())
				{
					bPLAY = false;
				}
		}

		else if (name == guiManager.bMinimize.getName())
		{
			if (guiManager.bMinimize)
				if (bGui_Advanced)bGui_Advanced = false;
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::Changed_Ranges(ofAbstractParameter& e)
{
	if (bDISABLE_CALLBACKS) return;
	{
		std::string name = e.getName();
		ofLogVerbose(__FUNCTION__) << name << " : " << e;

		if (name == RANGE_Selected.getName())
		{
			ofLogNotice(__FUNCTION__) << RANGE_Selected;

			//TODO
			// split to functions to disable autosave when playing mode..

			// auto save previous
			if (RANGE_Selected != RANGE_Selected_PRE)
			{
				int r;
				if (range_autoSave)
				{
					// save range
					r = RANGE_Selected_PRE;
					save_range(r);
				}

				//-

				if (range_autoLoad)
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
	guiManager.setName("ofxSurfingMoods");
	guiManager.setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	guiManager.setup();

	guiManager.addWindowSpecial(bGui_Main);
	guiManager.addWindowSpecial(bGui_Advanced);
	guiManager.addWindowSpecial(bGui_Matrices);

	guiManager.startup();

	//--

	//TODO:
	// fixing..
	static bool bCustom2 = true;
	if (bCustom2)
	{
		guiManager.clearStyles();
		guiManager.AddStyle(bPLAY, OFX_IM_TOGGLE_BIG, 1, false);

		guiManager.AddStyle(MODE_Ranged, OFX_IM_TOGGLE_BIG, 3, true);
		guiManager.AddStyle(MODE_MarkovChain, OFX_IM_TOGGLE_BIG, 3, true);
		guiManager.AddStyle(MODE_Manual, OFX_IM_TOGGLE_BIG, 3, false);
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
	yy = rectPreview.getBottomRight().y + pady;
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

			if (guiManager.beginWindow(bGui_ManualSlider, window_flags))
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
					ofxImGuiSurfing::AddVSlider(controlManual);
				}
				ImGui::PopStyleColor(5);

				guiManager.endWindow();
			}
		}
		ImGui::PopStyleVar();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_Matrices()
{
	if (bGui_Matrices)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_SMALL;

		if (guiManager.beginWindowSpecial(bGui_Matrices))
		{
			bool b = true;
			//bool b = guiManager.bMinimize;

			float h = (b ? 1 : 2) * guiManager.getWidgetsHeightUnit();

			if (PRESET_A_Enable)
			{
				guiManager.AddLabel(PRESET_A_Selected.getName(), false, b);
				ofxImGuiSurfing::AddMatrixClicker(PRESET_A_Selected, h);
				if (!b) guiManager.AddSpacingSeparated();
			}

			if (PRESET_B_Enable)
			{
				guiManager.AddLabel(PRESET_B_Selected.getName(), false, b);
				ofxImGuiSurfing::AddMatrixClicker(PRESET_B_Selected, h);
				if (!b) guiManager.AddSpacingSeparated();
			}

			if (PRESET_C_Enable)
			{
				guiManager.AddLabel(PRESET_C_Selected.getName(), false, b);
				ofxImGuiSurfing::AddMatrixClicker(PRESET_C_Selected, h);
			}

			guiManager.endWindowSpecial();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_Main()
{
	if (bGui_Main)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_SMALL;

		if (guiManager.beginWindowSpecial(bGui_Main))
		{
			//--

			// Minimize
			guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_ROUNDED_MEDIUM);

			guiManager.AddSpacingSeparated();

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
			if (bModeClockExternal) a = 1.0f;//external
			else a = ofMap(1 - timer_Range.getNormalizedProgress(), 0, 1, 0.35, 1, true);//internal playing
			ImVec4 ca = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * a);
			ImVec4 ca2 = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * (a * 0.5));//lower

			//-

			// Play
			{
				guiManager.AddSpacing();

				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ca);
				ImGui::PushStyleColor(ImGuiCol_Button, ca);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ca2);
				{
					// internal
					if (!bModeClockExternal)
					{
						ofxImGuiSurfing::AddBigToggleNamed(bPLAY, -1, -1, "PLAYING", "PLAY");
					}

					// external
					else
					{
						guiManager.Add(bExternalLocked, OFX_IM_TOGGLE_BIG);
					}
				}
				ImGui::PopStyleColor(3);

				if (bModeClockExternal)
				{
					static ofParameter<bool> _bBeat{ "Force Beat", false };
					if (guiManager.Add(_bBeat, OFX_IM_BUTTON_SMALL))
					{
						if (_bBeat) {
							_bBeat = false;

							doBeatTick();
						}
					}
				}

				// External clock
				guiManager.Add(bModeClockExternal, OFX_IM_TOGGLE_BUTTON_ROUNDED_MEDIUM);
				guiManager.AddSpacing();
			}

			//--

			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();

				guiManager.Add(bGui_PreviewWidget, OFX_IM_TOGGLE_BUTTON_ROUNDED);
				if (MODE_Manual) guiManager.Add(bGui_ManualSlider, OFX_IM_TOGGLE_BUTTON_ROUNDED);
			}

			//--

			guiManager.AddSpacingSeparated();

			//if (bInternalClock) 
			{
				// minimized
				if (guiManager.bMinimize)
				{
					// Clock

					bOpen = false;
					_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
					_flagt |= ImGuiTreeNodeFlags_Framed;

					if (ImGui::TreeNodeEx("CLOCK", _flagt))
					{
						guiManager.refreshLayout();

						// Mini Clock

						float _w3 = guiManager.getWidgetsWidth(3);
						float _w2 = guiManager.getWidgetsWidth(2);
						float _h = guiManager.getWidgetsHeightUnit();

						guiManager.Add(bpmSpeed, OFX_IM_SLIDER);
						if (!bModeClockExternal)
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
					guiManager.refreshLayout();
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
						guiManager.refreshLayout();

						float _w1 = guiManager.getWidgetsWidth(1);
						float _w2 = guiManager.getWidgetsWidth(2);
						float _h = guiManager.getWidgetsHeightUnit();

						guiManager.Add(bpmSpeed, OFX_IM_SLIDER, 1, false);

						if (!bModeClockExternal)
						{
							guiManager.Add(bpmSpeed, OFX_IM_STEPPER);

							if (ImGui::Button("/ 2", ImVec2(_w2, _h)))
							{
								bpmSpeed = bpmSpeed / 2.0f;
							}

							ImGui::SameLine();

							if (ImGui::Button("* 2", ImVec2(_w2, _h)))
							{
								bpmSpeed = bpmSpeed * 2.0f;
							}

							guiManager.Add(bResetClockSettings, OFX_IM_BUTTON_SMALL);
						}

						ImGui::TreePop();
					}
					guiManager.refreshLayout();
				}

				guiManager.AddSpacingSeparated();
			}

			//--

			// Modes

			// A. Ranged
			// B. Markov
			// C. Manual

			if (!guiManager.bMinimize)
			{
				bool bOpen2 = false;
				ImGuiTreeNodeFlags _flagt2 = (bOpen2 ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt2 |= ImGuiTreeNodeFlags_Framed;

				if (ImGui::TreeNodeEx("MODES", _flagt2))
				{
					guiManager.refreshLayout();

					// Modes
					guiManager.Add(MODE_Ranged, OFX_IM_TOGGLE_MEDIUM);
					if (bMarkovFileFound) {
						guiManager.Add(MODE_MarkovChain, OFX_IM_TOGGLE_MEDIUM);
					}
					else {
						guiManager.AddLabel("ERROR! MARKOV FILES NOT FOUND ON /DATA");
					}

					guiManager.Add(MODE_Manual, OFX_IM_TOGGLE_MEDIUM);

					ImGui::TreePop();
				}
				guiManager.refreshLayout();

				guiManager.AddSpacingSeparated();
			}

			//if (!guiManager.bMinimize) guiManager.AddSpacingSeparated();

			//--

			// Action

			bOpen = false;
			_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
			_flagt |= ImGuiTreeNodeFlags_Framed;

			if (ImGui::TreeNodeEx("ACTION", _flagt))
			{
				guiManager.refreshLayout();

				if (MODE_Manual)
				{
					guiManager.AddSpacing();
					ImGui::PushStyleColor(ImGuiCol_Text, ca);

					guiManager.Add(controlManual, OFX_IM_SLIDER);

					ImGui::PopStyleColor();
				}
				else
				{
				}

				//--

				if (!MODE_Manual)
				{
					guiManager.Add(countToDuration, OFX_IM_STEPPER); // user setter
				}

				//--

				// For monitor only
				//if (!guiManager.bMinimize)
				if (!MODE_Manual)
				{
					if (countToDuration != 1)
					{
						guiManager.Add(counterStepFromOne, OFX_IM_INACTIVE);
					}
				}

				// Progress Bar
				if (!bModeClockExternal && !guiManager.bMinimize)
				{
					guiManager.Add(timer_Progress, OFX_IM_PROGRESS_BAR_NO_TEXT);

					if (!MODE_Manual && !bModeClockExternal && countToDuration != 1)
						guiManager.Add(timer_ProgressComplete, OFX_IM_PROGRESS_BAR_NO_TEXT);
				}

				//----

				guiManager.AddSpacing();

				// Text + Main Controls
				ImGui::PushStyleColor(ImGuiCol_Text, ca);
				{
					std::string s;
					if (MODE_Ranged) s = MODE_Ranged.getName();
					else if (MODE_MarkovChain) s = MODE_MarkovChain.getName();
					else if (MODE_Manual) s = MODE_Manual.getName();

					guiManager.AddSpacingSeparated();

					guiManager.AddLabelHuge(s.c_str(), false, true);

					guiManager.AddSpacing();
					guiManager.AddSpacing();

					guiManager.Add(RANGE_Selected, OFX_IM_DEFAULT);

					guiManager.AddSpacing();

					guiManager.Add(TARGET_Selected, OFX_IM_DEFAULT, 1, false);
				}
				ImGui::PopStyleColor();

				guiManager.AddSpacing();

				//-

				if (!guiManager.bMinimize)
				{
					guiManager.AddSpacing();
					guiManager.AddLabel("TARGET > PRESETS", false, true);

					guiManager.AddSpacingSeparated();
					guiManager.AddSpacing();
				}

				if (ImGui::TreeNodeEx("PRESETS", ImGuiTreeNodeFlags_Framed))
				{
					guiManager.refreshLayout();

					if (!guiManager.bMinimize)
					{
						guiManager.Add(PRESET_A_Enable, OFX_IM_TOGGLE_SMALL);
						guiManager.Add(PRESET_B_Enable, OFX_IM_TOGGLE_SMALL);
						guiManager.Add(PRESET_C_Enable, OFX_IM_TOGGLE_SMALL);

						guiManager.AddSpacing();
					}

					guiManager.AddSpacingSeparated();
					guiManager.AddSpacing();

					if (PRESET_A_Enable) guiManager.Add(PRESET_A_Selected, OFX_IM_DEFAULT);
					if (PRESET_B_Enable) guiManager.Add(PRESET_B_Selected, OFX_IM_DEFAULT);
					if (PRESET_C_Enable) guiManager.Add(PRESET_C_Selected, OFX_IM_DEFAULT);

					guiManager.AddSpacing();

					guiManager.Add(bGui_Matrices, OFX_IM_TOGGLE_BUTTON_ROUNDED);

					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			guiManager.refreshLayout();

			//--

			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();

				guiManager.Add(bGui_Advanced, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
			}

			guiManager.endWindowSpecial();
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
	rectPreview.setRect(gx, gy, gw, hh); // initialize
}

//--------------------------------------------------------------
void ofxSurfingMoods::drawGui()
{
	if (!bGui_Main) return;

	guiManager.begin();
	{
		draw_ImGui_Main();
		draw_ImGui_Matrices();
		draw_ImGui_Advanced();
		draw_ImGui_ManualSlider();
	}
	guiManager.end();

	//--

	// Preview widget
	if (bGui_PreviewWidget) draw_PreviewWidget();

	// Help
	if (guiManager.bHelp) textBoxWidget.draw();
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

			if (guiManager.beginWindowSpecial(bGui_Advanced))
			{
				//--

				//if (!guiManager.bMinimize) 
				{
					//guiManager.refreshLayout();
					//guiManager.AddSpacing();
					guiManager.Add(MODE_StartLocked, OFX_IM_TOGGLE_SMALL);
					guiManager.Add(MODE_AvoidRepeat, OFX_IM_TOGGLE_SMALL);
					//guiManager.AddSpacing();

					guiManager.AddSpacingSeparated();
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
						guiManager.refreshLayout();

						// Target panel
						guiManager.Add(bClone_TARGETS, OFX_IM_TOGGLE_SMALL);
						guiManager.Add(bResetSort_Bank, OFX_IM_TOGGLE_SMALL);
						guiManager.Add(bReset_Bank, OFX_IM_TOGGLE_SMALL);
						guiManager.Add(bRandomize_Bank, OFX_IM_TOGGLE_SMALL, 1, false);

						ImGui::TreePop();
					}
					guiManager.refreshLayout();

					guiManager.AddSpacingSeparated();
				}

				//--

				// Ranges
				{
					if (ImGui::TreeNodeEx("DEBUG LIMITS", _flagt))
					{
						guiManager.refreshLayout();
						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
						flags |= ImGuiTreeNodeFlags_Framed;
						flags |= ImGuiTreeNodeFlags_DefaultOpen;

						guiManager.AddGroup(parameters_ranges, flags);

						ImGui::TreePop();
					}
					guiManager.refreshLayout();

					guiManager.AddSpacingSeparated();
				}

				//--

				// Extra
				if (!guiManager.bMinimize)
				{
					//guiManager.AddSpacingSeparated();

					guiManager.AddSpacing();

					guiManager.Add(guiManager.bExtra, OFX_IM_TOGGLE_ROUNDED_MEDIUM);

					if (guiManager.bExtra)
					{
						guiManager.Indent();
						{
							//--

							guiManager.AddSpacing();

							guiManager.Add(bResetSort_Bank, OFX_IM_TOGGLE_SMALL);
							guiManager.AddSpacing();
							guiManager.Add(bKeys, OFX_IM_TOGGLE_ROUNDED_SMALL);
							guiManager.Add(guiManager.bHelp, OFX_IM_TOGGLE_ROUNDED_SMALL);

							if (MODE_Manual) guiManager.Add(bModeAutomatic, OFX_IM_TOGGLE_ROUNDED);

							ImGui::Separator();

							//--

							// Preview

							guiManager.Add(bGui_PreviewWidget, OFX_IM_TOGGLE_ROUNDED_SMALL);
							if (bGui_PreviewWidget)
							{
								guiManager.Indent();
								{
									guiManager.Add(bUseCustomPreviewPosition, OFX_IM_TOGGLE_ROUNDED_SMALL);
									if (bUseCustomPreviewPosition) {
										guiManager.Add(bEdit_PreviewWidget, OFX_IM_TOGGLE_ROUNDED_SMALL);
										if (guiManager.Add(bResetPreviewWidget, OFX_IM_TOGGLE_ROUNDED_SMALL))
										{
											bResetPreviewWidget = false;
											doResetPreviewWidget();
										}
									}
								}
								guiManager.Unindent();
							}

							//--

							// Slider

							if (MODE_Manual)
							{
								ImGui::Separator();

								guiManager.Add(bGui_ManualSlider, OFX_IM_TOGGLE_ROUNDED_SMALL);
								if (bGui_ManualSlider)
								{
									guiManager.Indent();
									{
										guiManager.Add(bGui_ManualSliderHeader, OFX_IM_TOGGLE_ROUNDED_SMALL);
										if (guiManager.Add(bResetSlider, OFX_IM_TOGGLE_ROUNDED_SMALL))
										{
											bResetSlider = true;
										}
									}
									guiManager.Unindent();
								}
							}
						}
						guiManager.Unindent();
					}
				}

				guiManager.endWindowSpecial();
			}
		}
	}
}