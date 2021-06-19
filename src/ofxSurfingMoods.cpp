
#include "ofxSurfingMoods.h"

//--------------------------------------------------------------
void ofxSurfingMoods::setup(int numTargets, int numPresets, int limit1, int limit2)
{
	NUM_TARGETS = numTargets; // TARGETS
	NUM_PRESETS_A = numPresets; // PRESETS
	NUM_PRESETS_B = numPresets; // PRESETS
	NUM_PRESETS_C = numPresets; // PRESETS

	rLimit1 = limit1;
	rLimit2 = limit2;

	setup();//default sizes
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup()//default sizes
{
	//ofSetLogLevel(OF_LOG_VERBOSE);
	//ofSetLogLevel(OF_LOG_SILENT);

	//-

	//erase arrays
	for (int i = 0; i < MAX_ITEMS; i++)
	{
		presets_A[i] = 0;
		presets_B[i] = 0;
		presets_C[i] = 0;
	}

	//-

	//settings paths
	path_Folder = "ofxSurfingMoods/";//default folder
	filename_Settings = "moods_Settings";//settings
	filename_Bank = "moods_Bank.json";//ranges bank: any target to relatives preset/pattern

	//-

	//markov
	path_markovMatrix = path_Folder + "markov/" + "transitionMatrix.txt";
	ofxMC::Matrix mat(path_markovMatrix);
	markov.setup(mat, 0);

	//--

	//font to draw preview boxes
	fname = "overpass-mono-bold.otf";
	myTTF = "assets/fonts/" + fname;
	sizeTTF = 9;//font size
	bool isLoaded = myFont.load(myTTF, sizeTTF, true, true);
	if (!isLoaded)
	{
		ofLogError(__FUNCTION__) << "ofTrueTypeFont FONT FILE '" << myTTF << "' NOT FOUND!";
	}

	//--

	//params
	setup_Params();

	//gui
#ifdef USE_ofxGuiExtended
	setup_GUI_Main();
#endif

	//--

	//callbacks

	//1.
	ofAddListener(params_Listeners.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);
	ofAddListener(params_USER.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);

	//2.
	ofAddListener(parameters_ranges.parameterChangedE(), this, &ofxSurfingMoods::Changed_Ranges);

	//-

	//timer
	//60,000 / BPM = MS
	timer = LEN_BARS * (BPM_BAR_RATIO * (60000 / BPM));
	ofAddListener(timer_Range.TIMER_COMPLETE, this, &ofxSurfingMoods::timer_Range_Complete);
	ofAddListener(timer_Range.TIMER_STARTED, this, &ofxSurfingMoods::timer_Range_Started);

	//-

	//RESET
	////ERASE BANK TARGETS
	//resetBank(false);
	////REST CLOCK
	//resetClock();

	//-

	setGui_Visible(true);

	//--

	ENABLED_MoodMachine = true;
	bIsPlaying = false;
	timer = 0;
	PLAY = false;

	TARGET_Selected_PRE = -1;
	RANGE_Selected_PRE = -1;
	RANGE_Selected = 0;

	//-

	load_range(RANGE_Selected);

	//-

	setup_ImGui();

	//--

	startup();
}

//--------------------------------------------------------------
void ofxSurfingMoods::startup()
{
	//-------

	// startup

	//-

	// preview rectangle
	//bUseCustomPreviewPosition = false;
	bUseCustomPreviewPosition = true;

	path_rect = path_Folder + "ofxSurfingMoods_";

	//TODO: crash
	//bool b = rectPreview.loadSettings("", path_rect, true);
	//if (!b) rectPreview.setRect(25, 650, 700, 50); // initialize when no settings file created yet.

	//--

	// load bank targets
	if (autoSaveLoad_settings.get())
	{
		loadBanks(path_Folder);
	}

	// load panel settings
	if (autoSaveLoad_settings.get())
	{
		loadSettings(path_Folder);
	}

	//setGui_AdvancedVertical_MODE(false);
	//setPosition(20, 20);//gui panel position

	// workflow
#ifdef USE_ofxGuiExtended
	group_Advanced->getVisible().set(SHOW_AdvancedRanges);
#endif

	//--

	refresh_MOOD_Color();

	//updateLabels();

	//--

	/*
#ifdef USE_ofxGuiExtended
//theme
	path_Theme = "assets/theme/";
	path_Theme += "theme_ofxGuiExtended2.json";
	loadTheme(path_Theme);
	//group_RANGES;
	//group_TARGETS;
	//group_CLOCK;
#endif
	*/

	//--
}

//--------------------------------------------------------------
void ofxSurfingMoods::update()
{
	//if (Mode_MarkovChain)
	//{
	//	i++;
	//}

	//-

	timer_Range.update();
	timer_Progress = 100 * timer_Range.getNormalizedProgress();
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw()
{
	if (!bGui) return;

	if (SHOW_Preview) drawPreview();

	draw_ImGui();
}

//--------------------------------------------------------------
void ofxSurfingMoods::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofxSurfingMoods::exit()
{
	//-

	// preview rectangle
	rectPreview.saveSettings("", path_rect, true);

	//-

	/*
#ifdef USE_ofxGuiExtended
	positionGui_Engine = glm::vec2(group_USER->getPosition().x, group_USER->getPosition().y);
#endif
	*/

	if (autoSaveLoad_settings)
	{
		// save panel settings
		saveSettings(path_Folder);
		saveBanks(path_Folder);
	}

	//-

	ofRemoveListener(params_Listeners.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);
	ofRemoveListener(params_USER.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);
	ofRemoveListener(parameters_ranges.parameterChangedE(), this, &ofxSurfingMoods::Changed_Ranges);

	// timer
	ofRemoveListener(timer_Range.TIMER_COMPLETE, this, &ofxSurfingMoods::timer_Range_Complete);
	ofRemoveListener(timer_Range.TIMER_STARTED, this, &ofxSurfingMoods::timer_Range_Started);
}

//--------------------------------------------------------------
void ofxSurfingMoods::refresh_MOOD_Color()
{
	ofLogVerbose(__FUNCTION__);

	// mood color preview label
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
void ofxSurfingMoods::drawPreview() // put to the rigth-top of user panel
{
	float gx, gy, gw, gh, ww, hh, pad;

	////custom
	//if (bUseCustomPreviewPosition)
	//{
	//	gx = positionPreviewBoxes.x;
	//	gy = positionPreviewBoxes.y;
	//	ww = positionPreviewBoxes_Width;
	//	hh = positionPreviewBoxes_Height;
	//}

	//// custom
	// preview rectangle
	if (bUseCustomPreviewPosition)
	{
		gx = rectPreview.getX();
		gy = rectPreview.getY();
		ww = rectPreview.getWidth();
		hh = rectPreview.getHeight();
	}

	// default
	else
	{
		pad = 10;
		gw = ofGetWidth() - 2 * pad;
		gx = pad;
		gy = pad;
		ww = gw;
		hh = 50;

		/*
		#ifdef USE_ofxGuiExtended
				//advanced panel to the right
				gw = group_USER->getWidth() + 5;
				gx = group_USER->getPosition().x + gw + 5;
				gy = group_USER->getPosition().y + 4;
				ww = 400;
				hh = 30;
		#endif
		*/
	}

	////horizontal by default
	//if (!MODE_vertical && !bUseCustomPreviewPosition)
	//{
	//	gx = gx + gw;
	//}
	//else
	//{
	//}

	//-

	drawPreview(gx, gy, ww, hh);

	//-

	// preview rectangle
	if (Edit_Preview)
	{
		ofPushStyle();
		ofSetColor(128, 64);
		ofDrawRectangle(rectPreview);
		rectPreview.draw();
		ofPopStyle();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::drawPreview(int x, int  y, int  w, int  h) // custom position and size
{
	//TODO: there's a little offset...

	//if (bGui || SHOW_Preview)
	if (SHOW_Preview)
	{
		ofPushStyle();

		//-

		int NUM_Ranges = (int)NUM_RANGES;

		//for 0.20f: if fps is 60. duration will be 60/5 frames = 12frames
		blinkDuration = 0.20f*ofGetFrameRate();

		float sizes = w / (float)NUM_TARGETS;
		float ro = 4.0f;
		float line = 2.0f;

		float padBg, padBox, padSel;
		padBg = h * 0.2f;
		padBox = h * 0.1f;
		padSel = h * 0.1f;

		int aBg = 140;
		int aRg = 24;
		int aSel = 48;

		ofColor cBg{ ofColor(ofColor::black, aBg) };
		ofColor cBord{ ofColor(ofColor::black, 164) };

		ofColor c1, c2, c3;
		c1.set(ofColor(color_MOOD1, aRg));
		c2.set(ofColor(color_MOOD2, aRg));
		c3.set(ofColor(color_MOOD3, aRg));

		//progress bar
		float padH = padBg + 2.0f;

		//-

		//bg
		ofFill();
		ofSetColor(cBg);
		//ofDrawRectangle(x - padBg * 0.5f, y - padBg * 0.5f, w + padBg, h + padBg);//out expand
		ofDrawRectRounded(x - padBg * 0.5f, y - padBg * 0.5f, w + padBg, h + padBg, ro);//out expand

		float x1, x2, x3, xEnd;
		x1 = x;
		x2 = x1 + ((rLimit1)* sizes);
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
				ofSetColor(RANGE_Selected != 0 ? c1 : color_MOOD1);//disable alpha when range is selected
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

			// range box
			ofFill();
			ofDrawRectRounded(_x, y, _w, h, ro);

			//border
			//ofSetLineWidth(line);
			//ofSetColor(cBord);
			//ofNoFill();
			//ofDrawRectRounded(_x, y, _w, h, ro);
			////ofDrawRectangle(_x, y, _w, h);
		}

		// target boxes
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
			xb = x + t * sizes + 0.5f*padBox;
			yb = y + 0.5f*padBox;
			wb = sizes - padBox;
			hb = h - padBox;

			// target box
			ofFill();
			ofDrawRectRounded(xb, yb, wb, hb, ro);

			// text label
			ofSetColor(0);
			float xOff = 3;
			float yOff = 5;
			if (myFont.isLoaded())
			{
				myFont.drawString(ofToString(t), xb + wb * 0.5f - xOff, yb + 0.5f*hb + yOff);
			}
			else
			{
				ofDrawBitmapString(ofToString(t), xb + wb * 0.5f - xOff, yb + 0.5f*hb + yOff);
			}
		}

		//-

		// selected box

		// blink disabling box draw
		if (bBlink)
		{
			blinkCounterFrames++;
			if (blinkCounterFrames >= blinkDuration)
			{
				bBlink = false;
			}
		}

		// filled box
		ofFill();
		float blinkFactor = 0.4f;
		if (!bBlink)
			ofSetColor(0, aSel);
		else
			ofSetColor(0, blinkFactor*aSel);

		ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f*padSel, y + 0.5f*padSel,
			sizes - padSel, h - padSel, ro);

		// border
		ofNoFill();
		ofSetLineWidth(line);
		if (!bBlink)
			ofSetColor(cBord);
		else
			ofSetColor(cBord.r, cBord.g, cBord.b, cBord.a * blinkFactor);
		ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f*padSel, y + 0.5f*padSel,
			sizes - padSel, h - padSel, ro);

		//--

		// complete timer progress
		//TODO: hardcoded limits
		if (RANGE_Selected >= 0 && RANGE_Selected < 3)
		{
			//float padH = 6.0f;//joined
			float _h = h * 0.25f;
			float ro2 = 0.5f*ro;

			//TODO:
			float _w = ofMap(COUNTER_step_FromOne, 1, COUNTER_step_FromOne.getMax() + 1, 0, w, true);
			//float _w = ofMap(COUNTER_step_FromOne, 1, (4.0f/COUNT_Duration) * (COUNTER_step_FromOne.getMax() + 1), 0, w, true);

			float wStep = w / (float)(COUNTER_step_FromOne.getMax());//width of any step

			//TODO:
			//only works for count duration 4..
			float wTimer = ofMap(timer_Progress, 0, 100, 0, wStep);//scale by step timer
																   //float wTimer = ofMap(timer_Progress, 0, 100, 0, wStep));//scale by step timer

			_w = _w + wTimer;//add step timer

			//bg
			ofFill();
			ofSetColor(cBg);
			ofDrawRectRounded(x - padBg * 0.5f, y - padBg * 0.5f + h + padH,
				w + padBg, _h + padBg, ro);

			////border
			//ofSetColor(cBord);
			//ofNoFill();
			//ofDrawRectRounded(x - padBg * 0.5f, y - padBg * 0.5f + h + padH, w + padBg, _h + padBg, ro2);

			// complete progress range
			if (RANGE_Selected == 0)
			{
				ofSetColor(color_MOOD1);
			}
			else if (RANGE_Selected == 1)
			{
				ofSetColor(color_MOOD2);
			}
			else if (RANGE_Selected == 2)
			{
				ofSetColor(color_MOOD3);
			}

			// colored progress
			ofFill();
			ofDrawRectRounded(x, y + h + padH, _w, _h, ro2);

			// mark all range steps with vertical lines
			ofNoFill();
			//ofSetColor(255);
			ofSetLineWidth(line + 1.0f);
			float xStep;
			for (int m = 1; m < COUNTER_step_FromOne.getMax(); m++)
			{
				xStep = wStep * m;
				ofDrawLine(x + xStep, (y + h + padH), x + xStep, (y + h + padH) + _h);
			}

			//-

			// manual control line
			if (Mode_Manual)
			{
				ofNoFill();
				ofSetColor(255, 150);
				ofSetLineWidth(line + 2.0f);
				float xx = x + controlManual * w;
				_h = h * 0.25f;
				ofDrawLine(xx, (y + h + padH), xx, (y + h + padH) + _h);
			}
		}

		//-

		//// markov debug preview
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

		//-

		ofPopStyle();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup_Params()
{
	// colors to show moods (ranges)
	int a = 128;
	color_MOOD1.set(ofColor(ofColor::red, a));
	color_MOOD2.set(ofColor(ofColor::yellow, a));
	color_MOOD3.set(ofColor(ofColor::green, a));

	std::string spacer = " ";
	//std::string spacer = "           ";

	//positionGui_Engine.set("Gui Panel Position", glm::vec2(500, 500), glm::vec2(0), glm::vec2(1920, 1080));

	//-

	// 1. setup_GUI_Target

	PLAY.set("PLAY", false);

	BPM.set("BPM", 120, 30, 400);//bmp
	LEN_BARS.set("BARS LEN", 1, 1, 16);//bars
	timer.set("TIMER", 1000, 1, 60000);//ms
	timer.setSerializable(false);

	TARGET_Selected.set("STATE" + spacer + "TARGET", 0, 0, NUM_TARGETS - 1);//target

	PRESET_A_Enable.set("ENABLE A", true);
	PRESET_B_Enable.set("ENABLE B", true);
	PRESET_C_Enable.set("ENABLE C", true);

	// presets margins

	PRESET_A_Selected.set("PRESET A", 0, 0, NUM_PRESETS_A - 1);
	PRESET_B_Selected.set("PRESET B", 0, 0, NUM_PRESETS_B - 1);
	PRESET_C_Selected.set("PRESET C", 0, 0, NUM_PRESETS_C - 1);

	Range_Min.set("MIN TARGET", 0, 0, NUM_TARGETS - 1);
	Range_Max.set("MAX TARGET", NUM_TARGETS - 1, 0, NUM_TARGETS - 1);
	bReset_Settings.set("RESET CLOCK", false);
	bReset_Settings.setSerializable(false);
	clone_TARGETS.set("BANK CLONE>", false);
	clone_TARGETS.setSerializable(false);
	bGui.set("MOOD SURFER", true);

	SHOW_GuiUser.set("SHOW USER", true);
	SHOW_AdvancedRanges.set("SHOW ADVANCED", false);
	SHOW_Clocks.set("SHOW CLOCKS", false);
	SHOW_Preview.set("SHOW PREVIEW", false);
	bUseCustomPreviewPosition.set("Custom", false);

	Edit_Preview.set("EDIT PREVIEW", false);

	// disabled
	////labels to monitor
	////MONITOR1.set("", "");//monitor
	////MONITOR1.setSerializable(false);
	//MONITOR2.set("", "");//monitor
	//MONITOR2.setSerializable(false);

	//MOOD_Color_Preview.setName("MOOD RANGE");

	//----

	// 3. setup_GUI_Ranges
	//init ranges vector
	ranges.resize(NUM_RANGES);

	//ranges

	//hardcoded for NUM_RANGES = 3
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

	// RANGES

	// define params
	timer_Progress.set("COMPLETE", 0, 0, 100);//%
	RANGE_Selected.set("MOOD" + spacer + "RANGE", 0, 0, NUM_RANGES - 1);
	//myRange.min.set("MIN PRESET", 0, 0, NUM_TARGETS - 1);//?
	//myRange.max.set("MAX PRESET", 0, 0, NUM_TARGETS - 1);
	myRange.name.set("MOOD", ranges[RANGE_Selected].name);
	COUNT_Duration.set("STAY COUNT", 4, 1, 8);
	COUNTER_step_FromOne.set("COUNTER", 0, 0, COUNT_Duration);

	bRandomize_Bank.set("BANK RANDOMIZE", false);
	bRandomize_Bank.setSerializable(false);
	bReset_Bank.set("BANK RESET CLEAR", false);
	bReset_Bank.setSerializable(false);
	bResetSort_Bank.set("BANK RESET SORTED", false);
	bResetSort_Bank.setSerializable(false);

	//---

	myRange.min.set("TARGET MIN", 0, 0, NUM_TARGETS - 1);
	myRange.max.set("TARGET MAX", 0, 0, NUM_TARGETS - 1);
	//myRange.min.set("RANGE TARGETS", 0, 0, NUM_TARGETS - 1);//range sliders
	//myRange.max.set(NUM_TARGETS - 1);

	//---

	// group params outside gui but to use listeners and xml settings
	parameters_ranges.setName("RANGES PARAMS");
	parameters_ranges.add(RANGE_Selected);
	parameters_ranges.add(myRange.min);
	parameters_ranges.add(myRange.max);
	parameters_ranges.add(COUNT_Duration);
	parameters_ranges.add(bReset_Bank);
	parameters_ranges.add(bResetSort_Bank);
	parameters_ranges.add(bRandomize_Bank);

	//---

	// store params (grouped only to save/load, not to allow on gui or callbacks)
	params_STORE.setName("MoodMachine_params_STORE");
	params_STORE.add(COUNT_Duration);
	params_STORE.add(BPM);
	params_STORE.add(LEN_BARS);
	params_STORE.add(bGui);
	params_STORE.add(SHOW_GuiUser);
	params_STORE.add(SHOW_Clocks);
	params_STORE.add(SHOW_AdvancedRanges);
	params_STORE.add(SHOW_Preview);
	params_STORE.add(bUseCustomPreviewPosition);
	params_STORE.add(PRESET_A_Enable);
	params_STORE.add(PRESET_B_Enable);
	params_STORE.add(PRESET_C_Enable);
	params_STORE.add(TARGET_Selected);
	params_STORE.add(Mode_MarkovChain);
	params_STORE.add(Mode_Ranged);
	params_STORE.add(Mode_StartLocked);
	params_STORE.add(Mode_AvoidRepeat);
	params_STORE.add(Mode_Manual);
	params_STORE.add(controlManual);
	//params_STORE.add(positionGui_Engine);
	params_STORE.add(guiManager.params_Advanced);
	//params_STORE.add(autoSaveLoad_settings);

	autoSaveLoad_settings.setSerializable(false);

	//-

	// group params for callback listener only
	params_Listeners.setName("MoodMachine_params");

	params_Listeners.add(PLAY);
	params_Listeners.add(BPM);
	params_Listeners.add(LEN_BARS);
	params_Listeners.add(bReset_Settings);
	params_Listeners.add(bRandomize_Bank);
	params_Listeners.add(bResetSort_Bank);
	params_Listeners.add(bReset_Bank);
	params_Listeners.add(bGui);
	params_Listeners.add(SHOW_GuiUser);
	params_Listeners.add(SHOW_AdvancedRanges);
	params_Listeners.add(Edit_Preview);
	params_Listeners.add(SHOW_Preview);
	params_Listeners.add(TARGET_Selected);
	params_Listeners.add(clone_TARGETS);
	params_Listeners.add(PRESET_A_Selected);
	params_Listeners.add(PRESET_B_Selected);
	params_Listeners.add(PRESET_C_Selected);
	params_Listeners.add(Range_Min);
	params_Listeners.add(Range_Max);
	params_Listeners.add(timer);
	params_Listeners.add(Mode_MarkovChain);
	params_Listeners.add(Mode_Ranged);
	params_Listeners.add(Mode_StartLocked);
	params_Listeners.add(Mode_AvoidRepeat);
	params_Listeners.add(Mode_Manual);
	params_Listeners.add(controlManual);
	//params_Listeners.add(positionGui_Engine);

	//----

	// 2. setup_GUI_User

	//user
	//params_USER.setName("USER");
	params_USER.setName("MOOD MACHINE");//change display name

	params_USER.add(PLAY);

	params_USER.add(Mode_Ranged);
	params_USER.add(Mode_MarkovChain);
	params_USER.add(Mode_Manual);

	params_USER.add(controlManual);

	params_USER.add(Mode_StartLocked);
	params_USER.add(Mode_AvoidRepeat);

	//params_USER.add(params_Ranged);
	//params_USER.add(params_Manual);

	params_USER.add(COUNT_Duration);
	params_USER.add(timer_Progress);
	params_USER.add(COUNTER_step_FromOne);

	//params_USER.add(labelRange);
	//params_USER.add(MOOD_Color_Preview);
	params_USER.add(RANGE_Selected);

	params_USER.add(TARGET_Selected);

	//params_USER.add(labelTarget);
	//params_USER.add(MONITOR1);
	//params_USER.add(MONITOR2);

	params_USER.add(PRESET_A_Selected);
	params_USER.add(PRESET_B_Selected);
	params_USER.add(PRESET_C_Selected);

	params_USER.add(PRESET_A_Enable);
	params_USER.add(PRESET_B_Enable);
	params_USER.add(PRESET_C_Enable);

	params_USER.add(Edit_Preview);

	params_USER.add(SHOW_Preview);
	params_USER.add(SHOW_AdvancedRanges);

	//params_USER.add(autoSaveLoad_settings);
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
		////clamp ranges
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

			//load to the engine too
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

		////clamp ranges
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

////--------------------------------------------------------------
//void ofxSurfingMoods::updateLabels()
//{
//	//disabled everywhere, not required to show the monitor label on gui..
//	ofLogNotice(__FUNCTION__) << "updateLabels";
//
//	std::string pad;
//	//pad = " ";
//	//pad = "\t";
//	pad = "\t\t";
//	//pad = "\t\t\t";
//
//	//MONITOR1 = "TARGET:" + ofToString(TARGET_Selected);
//	//MONITOR1 += pad;
//
//	//std::string rName;
//	//rName = ofToString(myRange.name.get());
//	//MONITOR1 = "RANGE:" + rName;
//	//if (rName.size() < 2)
//	//	MONITOR1 += " ";
//
//	MONITOR2 = "";
//	MONITOR2 += "A" + ofToString(PRESET_A_Selected);
//	//if (PRESET_A_Selected < 10)
//	//	MONITOR2 += " ";
//	MONITOR2 += pad;
//	MONITOR2 += "B" + ofToString(PRESET_B_Selected);
//	MONITOR2 += pad;
//	MONITOR2 += "C" + ofToString(PRESET_C_Selected);
//}

//--------------------------------------------------------------
void ofxSurfingMoods::stopMachine()
{
	ofLogNotice(__FUNCTION__) << "stopMachine";

	COUNTER_step = 0;
	directionUp = true;
	timer_Range.stop();

	TARGET_Selected_PRE = -1;
	RANGE_Selected_PRE = -1;

	RANGE_Selected = 0;
	TARGET_Selected = ranges[RANGE_Selected].min; // set the target to the first target pos of the range

	// markov
	ofxMC::Matrix mat(path_markovMatrix);
	markov.setup(mat, 0);
}

//--------------------------------------------------------------
void ofxSurfingMoods::resetClock()
{
	ofLogNotice(__FUNCTION__) << "resetClock";

	//timer_Range.stop();
	LEN_BARS = 1;
	BPM = 120;
	//timer = 0;

	stopMachine();
	//stop();
}

//--------------------------------------------------------------
void ofxSurfingMoods::resetBank(bool RANDOMIZED, bool SORT_RELATIVE)
{
	ofLogNotice(__FUNCTION__) << "resetBank";

	//erase bank targets
	for (int p = 0; p < NUM_TARGETS; p++)
	{
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
	if (ENABLED_MoodMachine && PLAY)
	{
		PLAY = false;

		if (stopBack)
		{
			stopMachine();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::play()
{
	if (ENABLED_MoodMachine && !PLAY)
	{
		PLAY = true;
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::setTogglePlay()
{
	if (ENABLED_MoodMachine)
	{
		if (PLAY)
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
	////TODO: global gui enabler. not implemented..
	bGui = b;
	//gui.getVisible().set(bGui);

	SHOW_GuiUser = b;

	//workflow
	if (!SHOW_GuiUser && SHOW_AdvancedRanges) SHOW_AdvancedRanges = false;
}

//--------------------------------------------------------------
void ofxSurfingMoods::setGui_ToggleVisible()
{
	//bGui = !bGui;
	//gui.getVisible().set(bGui);

	SHOW_GuiUser = !SHOW_GuiUser;
}

////--------------------------------------------------------------
//void ofxSurfingMoods::setGui_AdvancedVertical_MODE(bool enable)
//{
//	MODE_vertical = enable;
//}

//--------------------------------------------------------------
void ofxSurfingMoods::setBpm(float bpm)
{
	BPM = bpm;
}

//--------------------------------------------------------------
void ofxSurfingMoods::setBarsScale(int bars)
{
	LEN_BARS = bars;
}

//--------------------------------------------------------------
void ofxSurfingMoods::runEngineModeRange()
{
	// this function can be used to trig ranges jumps externally without using the internal timer.

	int _RANGE_Selected_PRE = RANGE_Selected.get();

	//-

	// 1. mode random

	//-

	// 2. mode back loop:

	// count times and cycle
	COUNTER_step++;
	COUNTER_step = COUNTER_step % COUNT_Duration;
	COUNTER_step_FromOne = COUNTER_step + 1;// for gui user

	if (COUNTER_step == 0)
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

	if (!Mode_StartLocked.get()) // allows start from any target
	{
		if (!Mode_AvoidRepeat.get()) // allows repeat target
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
			if (!Mode_AvoidRepeat.get())// allows repeat target
			{
				TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);
			}
			else// avoids repeat same target
			{
				int _pre = TARGET_Selected.get();
				TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);

				int count = 0;
				int MAX_TRIES = 5;

				while (TARGET_Selected.get() == _pre)// not changed
				{
					TARGET_Selected = ofRandom(Range_Min, Range_Max + 1);

					count++;
					if (count > MAX_TRIES)// max attemps to avoid infinite loops...
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
void ofxSurfingMoods::timer_Range_Complete(int &args)
{
	ofLogNotice(__FUNCTION__) << "\n";

	ofLogVerbose(__FUNCTION__) << "timer_Range_Complete";

	if (PLAY)
	{
		//RESTART
		timer_Range.start(false);

		//-

		//modes

		if (Mode_Ranged.get())
		{
			runEngineModeRange();
		}

		//-

		else if (Mode_Manual)
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

			//do randomize between min/max ranges

			//int _RANGE_Selected_PRE = RANGE_Selected.get();

			if (!Mode_StartLocked.get())//allows start from any target
			{
				if (!Mode_AvoidRepeat.get())// allows repeat target
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
			//when changed range, first step will be locked to the min from range
			else
			{
				//range changed
				if (_RANGE_Selected_PRE != RANGE_Selected.get())
				{
					TARGET_Selected = Range_Min.get();
				}
				else
				{

					if (!Mode_AvoidRepeat.get())// allows repeat target
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
			}
		}

		//-

		else if (Mode_MarkovChain.get())
		{
			int _RANGE_Selected_PRE = RANGE_Selected.get();

			//count times and cycle
			COUNTER_step++;
			COUNTER_step = COUNTER_step % COUNT_Duration;
			COUNTER_step_FromOne = COUNTER_step + 1;// for gui user

			//type A: mode stay amount of counter
			//if (COUNTER_step == 0)
			//type B: ignoring counter. just 1
			{
				markov.update();
				TARGET_Selected = markov.getState();

				//--

				//range not changed
				if (RANGE_Selected.get() == _RANGE_Selected_PRE)
				{
					if (Mode_AvoidRepeat.get())//avoids repeat same target
					{
						int _pre = TARGET_Selected.get();

						markov.update();
						TARGET_Selected = markov.getState();

						int count = 0;
						int MAX_TRIES = 100;

						while (TARGET_Selected.get() == _pre)//not changed
						{
							markov.update();
							TARGET_Selected = markov.getState();

							count++;
							if (count > MAX_TRIES)//max attemps to avoid infinite loops...
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

//--------------------------------------------------------------
void ofxSurfingMoods::timer_Range_Started(int &args)
{
	ofLogVerbose(__FUNCTION__) << "timer_Range_Started";
}


//--------------------------------------------------------------
void ofxSurfingMoods::saveBanks(std::string path)
{
	//store bank, each target to their preset/presets_ABC settings
	ofJson pt;
	js_targets.clear();

	for (int i = 0; i < NUM_TARGETS; i++)
	{
		//clamp min
		int prst_A = (int)MAX(presets_A[i], 0);
		int prst_B = (int)MAX(presets_B[i], 0);
		int prst_C = (int)MAX(presets_C[i], 0);
		//clamp max
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
	//store app state
	//save gui settings

	ofXml _settings;
	ofSerialize(_settings, params_STORE);

	std::string _path = path + filename_Settings + ".xml";

	_settings.save(_path);

	ofLogNotice(__FUNCTION__) << _path;
}

//--------------------------------------------------------------
void ofxSurfingMoods::loadBanks(std::string path)
{
	// 2. bank targets presets_A/presets_C
	std::string pathBank = path + filename_Bank;
	ofFile file(pathBank);
	if (file.exists())
	{
		//parse json
		file >> js_targets;

		//cout << js_targets;
		ofLogNotice(__FUNCTION__) << "LOADED FILE: " << pathBank;
		ofLogNotice(__FUNCTION__) << js_targets;

		ofLogNotice(__FUNCTION__) << pathBank + " json file must be present and formatted as expected!";

		//avoid crashes
		int p = 0;
		for (auto &js_tar : js_targets)
		{
			if ((!js_tar.empty()) && (p < NUM_TARGETS))
			{
				ofLogVerbose(__FUNCTION__) << endl << js_tar;
				//cout << js_tar << endl;

				int prst_A;
				int prst_B;
				int prst_C;
				prst_A = js_tar["preset_A"];
				prst_B = js_tar["preset_B"];
				prst_C = js_tar["preset_C"];

				//clamp min
				prst_A = (int)MAX(prst_A, 0);
				prst_B = (int)MAX(prst_B, 0);
				prst_C = (int)MAX(prst_C, 0);
				//clamp max
				presets_A[p] = (int)MIN(prst_A, NUM_PRESETS_A);
				presets_B[p] = (int)MIN(prst_B, NUM_PRESETS_B);
				presets_C[p] = (int)MIN(prst_C, NUM_PRESETS_C);

				ofLogVerbose(__FUNCTION__) << presets_A[p];
				ofLogVerbose(__FUNCTION__) << presets_B[p];
				ofLogVerbose(__FUNCTION__) << presets_C[p];
				p++;
			}
		}
	}
	else
	{
		ofLogError(__FUNCTION__) << pathBank << " NOT FOUND!";
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::loadSettings(std::string path)
{
	//ofLogNotice(__FUNCTION__) << path;

	//TODO:
	//ofSetDataPathRoot();
	//ofToDataPath();

	//-

	// 1. load panel settings
	ofXml _settings;

	std::string _path = path + filename_Settings + ".xml";
	bool bLoaded = _settings.load(_path);

	if (bLoaded)
	{
		ofLogNotice(__FUNCTION__) << "LOADED: " << _path;
		ofLogNotice(__FUNCTION__) << endl << endl << _settings.toString();

		ofDeserialize(_settings, params_STORE);
	}
	else
	{
		ofLogError(__FUNCTION__) << "FILE NOT FOUND: " << _path;
	}

/*
//#ifdef USE_ofxGuiExtended
//	group_USER->setPosition(positionGui_Engine.get().x, positionGui_Engine.get().y);
//#endif
*/
}


//--------------------------------------------------------------
void ofxSurfingMoods::Changed_Params_Listeners(ofAbstractParameter &e)
{
	if (!BLOCK_CALLBACK_Feedback)
	{
		std::string WIDGET = e.getName();

		if (WIDGET != "COMPLETE")
			ofLogVerbose(__FUNCTION__) << WIDGET << " : " << e;

		if (WIDGET == PLAY.getName())
		{
			if (PLAY)
			{
				//  60,000 / BPM = MS
				timer = LEN_BARS * (BPM_BAR_RATIO * (60000 / BPM));
				timer_Range.start(false);
				bIsPlaying = true;

				//-

				COUNTER_step = 0;
				COUNTER_step_FromOne = COUNTER_step + 1;// for gui user
/*
#ifdef USE_ofxGuiExtended
				(group_USER->getIntSlider("COUNTER"))->setEnabled(true);//hidden
				(group_USER->getIntSlider("COMPLETE"))->setEnabled(true);//hidden
#endif
*/
//workflow
//enable some mode
				if (!Mode_MarkovChain && !Mode_Manual && !Mode_Ranged)
				{
					Mode_Ranged = true;
					Mode_MarkovChain = false;
					Mode_Manual = false;
				}
			}
			else
			{
				timer_Range.stop();
				bIsPlaying = false;

				COUNTER_step = 0;
				COUNTER_step_FromOne = 0;
				RANGE_Selected = 0;
				/*
				#ifdef USE_ofxGuiExtended
								(group_USER->getIntSlider("COUNTER"))->setEnabled(false);//hidden
								(group_USER->getIntSlider("COMPLETE"))->setEnabled(false);//hidden
				#endif
				*/
			}
		}

		else if (WIDGET == "BANK CLONE>")
		{
			if (clone_TARGETS)
			{
				clone_TARGETS = false;
				clone();
			}
		}

		else if (WIDGET == TARGET_Selected.getName())
		{
			ofLogNotice(__FUNCTION__) << TARGET_Selected;

			//blink
			bBlink = true;
			blinkCounterFrames = 0;

			//-

			if (TARGET_Selected != TARGET_Selected_PRE)
			{
				//limited target
				if (TARGET_Selected > NUM_TARGETS - 1)
				{
					TARGET_Selected = NUM_TARGETS - 1;
				}
				else if (TARGET_Selected < 0)
				{
					TARGET_Selected = 0;
				}

				//-

				//targets management

				//TODO:
				//save not required if target contents (selected A-B-C) not changed..

				if (target_autoSave && TARGET_Selected_PRE >= 0)
				{
					//TODO:
					//should clamp!
					//auto save
					if (PRESET_A_Enable) presets_A[TARGET_Selected_PRE] = PRESET_A_Selected;
					if (PRESET_B_Enable) presets_B[TARGET_Selected_PRE] = PRESET_B_Selected;
					if (PRESET_C_Enable) presets_C[TARGET_Selected_PRE] = PRESET_C_Selected;
				}

				//-

				if (target_autoLoad)
				{
					//auto load
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

		//else if (WIDGET == "PRESET A" || WIDGET == "PRESET B" || WIDGET == "PRESET C")
		//{
		//	updateLabels();
		//}

		else if (WIDGET == "TIMER")
		{
			if (PLAY)
			{
				timer_Range.setup(timer);
			}
		}

		else if (WIDGET == "BPM")
		{
			//  60,000 / BPM = MS
			timer = LEN_BARS * (BPM_BAR_RATIO * (60000 / BPM));

			//TODO:
			//must solve jumps..
			//stop();
		}

		else if (WIDGET == "BARS LEN")
		{
			//  60,000 / BPM = MS
			timer = LEN_BARS * (BPM_BAR_RATIO * (60000 / BPM));
		}

		//resets
		else if (WIDGET == "RESET CLOCK")
		{
			bReset_Settings = false;
			resetClock();
			stop();
		}
		else if (WIDGET == "BANK RESET CLEAR")
		{
			bReset_Bank = false;
			resetBank(false, false);//all to 0
			stop();
		}
		else if (WIDGET == "BANK RESET SORTED")
		{
			bResetSort_Bank = false;
			resetBank(false, true);//relative random
			stop();
		}
		else if (WIDGET == "BANK RANDOMIZE")
		{
			bRandomize_Bank = false;
			resetBank(true, false);
			stop();
		}

		//gui
		else if (WIDGET == SHOW_AdvancedRanges.getName())
		{
			/*
	#ifdef USE_ofxGuiExtended
				group_Advanced->getVisible().set(SHOW_AdvancedRanges);

				//workflow
				auto p = group_USER->getShape().getTopRight();
				if (!MODE_vertical)//advanced panel to the right
				{
					setGui_AdvancedPositon(p.x + 5, p.y);
				}
				else//advanced panel to the botton
				{
					auto p = group_USER->getShape().getBottomLeft();
					setGui_AdvancedPositon(p.x, p.y + 10);
				}
	#endif
			*/
		}
		else if (WIDGET == SHOW_GuiUser.getName())
		{
			/*
	#ifdef USE_ofxGuiExtended
				group_USER->getVisible().set(SHOW_GuiUser);
	#endif
			*/
		}
		else if (WIDGET == "SHOW PREVIEW")
		{

		}
		else if (WIDGET == Edit_Preview.getName())
		{
			if (Edit_Preview)
			{
				rectPreview.enableEdit();
			}
			else
			{
				rectPreview.disableEdit();
			}
		}

		// modes
		else if (WIDGET == Mode_MarkovChain.getName())
		{
			if (Mode_MarkovChain)
			{
				Mode_Manual = false;
				Mode_Ranged = false;
			}
			else refresModeshWorkflow();
		}
		else if (WIDGET == Mode_Ranged.getName())
		{
			if (Mode_Ranged)
			{
				Mode_Manual = false;
				Mode_MarkovChain = false;
			}
			else refresModeshWorkflow();
		}
		else if (WIDGET == Mode_Manual.getName())
		{
			if (Mode_Manual)
			{
				Mode_Ranged = false;
				Mode_MarkovChain = false;
			}
			else refresModeshWorkflow();
			/*
#ifdef USE_ofxGuiExtended
			(group_USER->getFloatSlider(controlManual.getName()))->setEnabled(Mode_Manual.get());
#endif
			*/
		}
		else if (WIDGET == controlManual.getName())
		{
			// workflow
			if (!PLAY.get()) {
				PLAY = true;
			}
		}

		// workflow
		/*
#ifdef USE_ofxGuiExtended
		else if (WIDGET == PRESET_A_Enable.getName())
		{
			(group_USER->getIntSlider(PRESET_A_Selected.getName()))->setEnabled(PRESET_A_Enable.get());
		}
		else if (WIDGET == PRESET_B_Enable.getName())
		{
			(group_USER->getIntSlider(PRESET_B_Selected.getName()))->setEnabled(PRESET_B_Enable.get());
		}
		else if (WIDGET == PRESET_C_Enable.getName())
		{
			(group_USER->getIntSlider(PRESET_C_Selected.getName()))->setEnabled(PRESET_C_Enable.get());
		}
#endif
		*/
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::Changed_Ranges(ofAbstractParameter &e)
{
	if (!BLOCK_CALLBACK_Feedback)
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

		else if (name == "STAY COUNT")
		{
			/*
#ifdef USE_ofxGuiExtended
			//re scale slider limit
			(group_USER->getIntSlider("COUNTER"))->setMax(COUNT_Duration);
#endif
			*/

			// avoid rescale timers error on preview

			// workflow
			bool bPre = isPlaying();
			stop();
			if (bPre)play();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::keyPressed(int key)
{
	if (key == ' ')
	{
		setTogglePlay();
	}

	else if (key == 'p')
	{
		play();
	}

	else if (key == 's')
	{
		stop();
	}

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
}


//--------------------------------------------------------------
void ofxSurfingMoods::setup_ImGui()
{
	guiManager.setImGuiAutodraw(bAutoDraw);
	guiManager.setup();
	//guiManager.setup(gui);

	static bool bCustom2 = true;
	if (bCustom2)
	{
		widgetsManager.AddWidgetConf(PLAY, SurfingWidgetTypes::IM_TOGGLE_BIG, false, 1, 5);

		widgetsManager.AddWidgetConf(Mode_Ranged, SurfingWidgetTypes::IM_TOGGLE_BIG, true, 3);
		widgetsManager.AddWidgetConf(Mode_MarkovChain, SurfingWidgetTypes::IM_TOGGLE_BIG, true, 3);
		widgetsManager.AddWidgetConf(Mode_Manual, SurfingWidgetTypes::IM_TOGGLE_BIG, false, 3, 5);


		//	widgetsManager.AddWidgetConf(bPrevious, SurfingWidgetTypes::IM_BUTTON_SMALL, true, 2);
		//	widgetsManager.AddWidgetConf(bNext, SurfingWidgetTypes::IM_BUTTON_SMALL, false, 2, 20);
		//	widgetsManager.AddWidgetConf(separation, SurfingWidgetTypes::IM_STEPPER);
		//	widgetsManager.AddWidgetConf(speed, SurfingWidgetTypes::IM_DRAG, false, 1, 10);
		//	widgetsManager.AddWidgetConf(shapeType, SurfingWidgetTypes::IM_SLIDER);
		//	widgetsManager.AddWidgetConf(size, SurfingWidgetTypes::IM_STEPPER);
		//	widgetsManager.AddWidgetConf(amount, SurfingWidgetTypes::IM_DRAG, false, 1, 10);
		//	widgetsManager.AddWidgetConf(bMode1, SurfingWidgetTypes::IM_TOGGLE_BIG, true, 2);
		//	widgetsManager.AddWidgetConf(bMode2, SurfingWidgetTypes::IM_TOGGLE_BIG, false, 2);
		//	widgetsManager.AddWidgetConf(bMode3, SurfingWidgetTypes::IM_TOGGLE_BIG, true, 2);
		//	widgetsManager.AddWidgetConf(bMode4, SurfingWidgetTypes::IM_TOGGLE_BIG, false, 2);
		//	//widgetsManager.AddWidgetConf(lineWidth3, SurfingWidgetTypes::IM_DRAG); // not works?
		//	// hide some params from any on-param-group appearance
		//	widgetsManager.AddWidgetConf(speed3, SurfingWidgetTypes::IM_HIDDEN, false, -1, 50);
		//	widgetsManager.AddWidgetConf(size2, SurfingWidgetTypes::IM_HIDDEN, false, -1, 50);
		//	widgetsManager.AddWidgetConf(bPrevious, SurfingWidgetTypes::IM_HIDDEN);
		//	widgetsManager.AddWidgetConf(bNext, SurfingWidgetTypes::IM_HIDDEN);
		//  widgetsManager.AddWidgetConf(lineWidth, SurfingWidgetTypes::IM_HIDDEN);

	}
	//guiManager.bAutoResize = false;
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_User()
{
	// USER

	if (SHOW_GuiUser)
	{
		static bool bOpen2 = false;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

		//guiManager.beginWindow("USER GROUP", NULL, window_flags);
		//{
		//	// group
		//	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
		//	flags |= ImGuiTreeNodeFlags_Framed;
		//	//flags |= ImGuiTreeNodeFlags_DefaultOpen;
		//	ofxImGuiSurfing::AddGroup(params_USER, flags);
		//}
		//guiManager.endWindow();

		//-

		guiManager.beginWindow("SURFING MOODS", &bOpen2, window_flags);
		{
			widgetsManager.refreshPanelShape();

			// play
			widgetsManager.Add(PLAY, SurfingWidgetTypes::IM_TOGGLE_BIG);
			ImGui::Dummy(ImVec2(0, 2));

			static bool bOpen = false;
			ImGuiTreeNodeFlags _flagt;
			_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
			_flagt |= ImGuiTreeNodeFlags_Framed;

			if (ImGui::TreeNodeEx("PANELS", _flagt))
			{
				widgetsManager.refreshPanelShape();

				widgetsManager.Add(SHOW_AdvancedRanges, SurfingWidgetTypes::IM_TOGGLE_SMALL);
				widgetsManager.Add(SHOW_Clocks, SurfingWidgetTypes::IM_TOGGLE_SMALL);
				if (ImGui::TreeNodeEx("PREVIEW", _flagt))
				{
					widgetsManager.refreshPanelShape();
					float _w100 = ofxImGuiSurfing::getWidgetsWidth(1);
					float _w50 = ofxImGuiSurfing::getWidgetsWidth(2);
					float _h = BUTTON_BIG_HEIGHT;

					widgetsManager.Add(SHOW_Preview, SurfingWidgetTypes::IM_TOGGLE_SMALL);
					widgetsManager.Add(Edit_Preview, SurfingWidgetTypes::IM_TOGGLE_SMALL);

					if (ImGui::Button("Reset", ImVec2(_w100, _h / 2)))
					{
						//bUseCustomPreviewPosition = false;

						float gx, gy, gw, gh, ww, hh, pad;
						pad = 10;
						gw = ofGetWidth() - 2 * pad;
						gx = pad;
						gy = pad;
						ww = gw;
						hh = 50;
						rectPreview.setRect(gx, gy, gw, hh); // initialize
					}
					//ImGui::SameLine();
					//ofxImGuiSurfing::ToggleRoundedButton("Custom", &bUseCustomPreviewPosition);
					ofxImGuiSurfing::AddToggleRoundedButton(bUseCustomPreviewPosition);
					//if (ImGui::Button("Lock", ImVec2(_w50, _h / 2)))
					//{}
				}
				ImGui::Dummy(ImVec2(0, 5));
				ImGui::TreePop();
			}

			widgetsManager.Add(Mode_Ranged, SurfingWidgetTypes::IM_TOGGLE_BIG);
			widgetsManager.Add(Mode_MarkovChain, SurfingWidgetTypes::IM_TOGGLE_BIG);
			widgetsManager.Add(Mode_Manual, SurfingWidgetTypes::IM_TOGGLE_BIG);

			//widgetsManager.Add(Mode_Ranged, SurfingWidgetTypes::IM_TOGGLE_BIG, true, 3);
			//widgetsManager.Add(Mode_MarkovChain, SurfingWidgetTypes::IM_TOGGLE_BIG, true, 3);
			//widgetsManager.Add(Mode_Manual, SurfingWidgetTypes::IM_TOGGLE_BIG, false, 3);

			if (Mode_Manual) widgetsManager.Add(controlManual, SurfingWidgetTypes::IM_SLIDER);

			ImGui::Dummy(ImVec2(0, 5));

			widgetsManager.Add(COUNT_Duration, SurfingWidgetTypes::IM_SLIDER);
			widgetsManager.Add(timer_Progress, SurfingWidgetTypes::IM_SLIDER);
			widgetsManager.Add(COUNTER_step_FromOne, SurfingWidgetTypes::IM_SLIDER, false, 1, 4);

			widgetsManager.Add(Mode_StartLocked, SurfingWidgetTypes::IM_TOGGLE_SMALL, true, 2);
			widgetsManager.Add(Mode_AvoidRepeat, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 2);

			ImGui::Dummy(ImVec2(0, 2));

			//widgetsManager.Add(MOOD_Color_Preview, SurfingWidgetTypes::IM_DEFAULT);

			ImGui::Text("RANGE | STATE");
			widgetsManager.Add(RANGE_Selected, SurfingWidgetTypes::IM_DEFAULT);
			widgetsManager.Add(TARGET_Selected, SurfingWidgetTypes::IM_DEFAULT, false, 1, 4);

			widgetsManager.Add(PRESET_A_Enable, SurfingWidgetTypes::IM_TOGGLE_SMALL);
			widgetsManager.Add(PRESET_B_Enable, SurfingWidgetTypes::IM_TOGGLE_SMALL);
			widgetsManager.Add(PRESET_C_Enable, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 1, 4);

			if (PRESET_A_Enable) widgetsManager.Add(PRESET_A_Selected, SurfingWidgetTypes::IM_DEFAULT);
			if (PRESET_B_Enable) widgetsManager.Add(PRESET_B_Selected, SurfingWidgetTypes::IM_DEFAULT);
			if (PRESET_C_Enable) widgetsManager.Add(PRESET_C_Selected, SurfingWidgetTypes::IM_DEFAULT, false, 1, 4);

			//--

			ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bAdvanced);
			guiManager.drawAdvancedSubPanel();
		}
		guiManager.endWindow();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui()
{
	if (!bGui) return;

	guiManager.begin();
	{
		draw_ImGui_User();

		//--

		// RANGES

		if (SHOW_AdvancedRanges)
		{
			static bool bOpen1 = true;
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
			if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

			guiManager.beginWindow("MOODS ADVANCED", &bOpen1, window_flags);
			{
				//widgetsManager.refreshPanelShape();

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
				flags |= ImGuiTreeNodeFlags_Framed;
				flags |= ImGuiTreeNodeFlags_DefaultOpen;

				ofxImGuiSurfing::AddGroup(parameters_ranges, flags);
				//ofxImGuiSurfing::AddGroup(params_STORE, flags);
				//ofxImGuiSurfing::AddGroup(params_Listeners, flags);
			}
			guiManager.endWindow();
		}

		//--

		// TARGETS
		{
			if (SHOW_Clocks)
			{
				// panels sizes
				float xx = 10;
				float yy = 10;
				float ww = PANEL_WIDGETS_WIDTH;
				float hh = 20;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(ww, hh));
				{
					static bool bOpen = true;
					ImGuiTreeNodeFlags _flagt;
					ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
					if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

					std::string n = "MOODS CLOCKS";
					guiManager.beginWindow(n.c_str(), &bOpen, window_flags);
					{
						widgetsManager.Add(SHOW_GuiUser, SurfingWidgetTypes::IM_TOGGLE_BIG, false, 1);

						//-

						bOpen = true;
						_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
						_flagt |= ImGuiTreeNodeFlags_Framed;

						if (ImGui::TreeNodeEx("CLOCK", _flagt))
						{
							widgetsManager.refreshPanelShape();

							// clock panel
							widgetsManager.AddWidgetConf(BPM, SurfingWidgetTypes::IM_DRAG, false, 1, -1);
							widgetsManager.AddWidgetConf(BPM, SurfingWidgetTypes::IM_STEPPER);
							//widgetsManager.AddWidgetConf(LEN_BARS);

							ofxImGuiSurfing::AddParameter(BPM);
							ofxImGuiSurfing::AddParameter(LEN_BARS);

							widgetsManager.Add(bReset_Settings, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 1);

							ImGui::TreePop();
						}

						//SHOW_timer = true;
						//if (SHOW_timer)
						//ofxImGuiSurfing::AddParameter(timer); // hide timer

						bOpen = false;
						_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
						_flagt |= ImGuiTreeNodeFlags_Framed;

						if (ImGui::TreeNodeEx("TOOLS", _flagt))
						{
							widgetsManager.refreshPanelShape();

							// target panel
							widgetsManager.Add(clone_TARGETS, SurfingWidgetTypes::IM_TOGGLE_SMALL);
							widgetsManager.Add(bResetSort_Bank, SurfingWidgetTypes::IM_TOGGLE_SMALL);
							widgetsManager.Add(bReset_Bank, SurfingWidgetTypes::IM_TOGGLE_SMALL);
							widgetsManager.Add(bRandomize_Bank, SurfingWidgetTypes::IM_TOGGLE_SMALL, false, 1, 5);

							ImGui::TreePop();
						}

						//--

						ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bAdvanced);
						guiManager.drawAdvancedSubPanel();
					}
					guiManager.endWindow();
				}
				ImGui::PopStyleVar();
			}
		}
	}
	guiManager.end();
}

//-

// ofxGuiExtended
/*
#ifdef USE_ofxGuiExtended
//--------------------------------------------------------------
void ofxSurfingMoods::setup_GUI_Main()
{
	//-

	//ofxGuiExtended panels

	group_USER = gui.addPanel("__USER__");
	//group_USER = gui.addGroup("__USER__");

	group_Advanced = gui.addGroup("ADVANCED");
	group_RANGES = group_Advanced->addGroup("RANGES LIMITS");
	group_TARGETS = group_Advanced->addGroup("BANK TOOLS");
	group_CLOCK = group_Advanced->addGroup("CLOCK");

	//---

	////params
	//setup_Params();

	//--

	//user
	setup_GUI_User();

	//advanced

	//targets
	setup_GUI_Target();

	//ranges
	setup_GUI_Ranges();

	//-

	//customize
	setup_GUI_Customize();

	//-
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup_GUI_User()
{
	group_USER->add(params_USER);
	group_USER->setShowHeader(true);
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup_GUI_Customize()
{
	//panels
	gui_w = 200;
	//widgets
	gui_slider_mini_h = 18;
	gui_slider_big_h = 30;
	gui_button_big_h = 40;

	////highlight important items
	//ofColor cUser;
	//cUser.set(64, 64, 64, 128);
	//std::string cUserStr = ofxGui::colorToString(cUser);

	//--

	////gui font

	//sizeTTF_Gui = 8;//font size
	////fname = "overpass-mono-bold.otf";
	//fname = "telegrama_render.otf";
	////myTTF_Gui = path_Folder + "fonts/" + fname;
	//myTTF_Gui = "assets/fonts/" + fname;

	//ofFile fileF(myTTF_Gui);
	//bool bLoadedGuiFont = fileF.exists();

	//--

	////all gui
	//j_Gui =
	//{
	//	{"font-family", myTTF_Gui},
	//	{"font-size", sizeTTF_Gui},
	//	//{"padding", 50},//widgets height
	//};

	//sliders mini
	j_itemMini =
	{
		{"type", "fullsize"},
		{"height", gui_slider_mini_h},
	};

	//big buttons
	j_itemFat =
	{
		//{"fill-color", "rgba(128,128,128,0.4)" },
		{"type", "fullsize"},
		{"height", gui_button_big_h},
		{"text-align", "center"},
	};

	//medium buttons
	j_itemMedium =
	{
		//{"fill-color", "rgba(128,128,128,0.4)" },
		{"type", "fullsize"},
		{"height", 22},
		{"text-align", "right"},
	};

	//--

	//widgets
	//(group_USER->get("m1"))->;//TODO: to disable name display of MONITOR

	(group_USER->getToggle("PLAY"))->setConfig(j_itemFat);
	(group_USER->getIntSlider("COUNTER"))->unregisterMouseEvents();//disable user mouse
	 //(group_USER->getIntSlider(RANGE_Selected.getName()))->setConfig(j_itemMini);

	(group_USER->getToggle("MOOD RANGE"))->setConfig({
		{"type", "fullsize"},
		{"text-align", "center"},
		{"text-color", "rgba(0,0,0,1.0)"},
		{"border-radius", 5.0f},
		{"height", gui_slider_big_h},
		});
	(group_USER->getToggle("MOOD RANGE"))->unregisterMouseEvents();
	//(group_USER->getToggle("MOOD RANGE"))->setShowName(false);
	//(group_USER->getToggle("MOOD RANGE"))->setEnabled(false);//hidden

	refresh_MOOD_Color();

	//mini
	(group_RANGES->getIntSlider(RANGE_Selected.getName()))->setConfig(j_itemMini);

	(group_RANGES->getIntSlider("TARGET MIN"))->setConfig(j_itemMini);
	(group_RANGES->getIntSlider("TARGET MAX"))->setConfig(j_itemMini);

	//(group_CLOCK->getFloatSlider("BPM"))->setConfig(jConf_BigBut2);
	(group_CLOCK->getFloatSlider("BPM"))->setConfig({ { "precision", 2 } });
	//(group_CLOCK->getIntSlider("BARS LEN"))->setConfig(jHigh);

	//(group_USER->getToggle("PLAY"))->setConfig(jHigh);
	//(group_USER->getIntSlider("STAY COUNT"))->setConfig(jHigh);
	//(group_USER->getIntSlider(RANGE_Selected.getName()))->setConfig(jHigh);

	(group_USER->getToggle("ENABLE A"))->setConfig(j_itemMedium);
	(group_USER->getToggle("ENABLE B"))->setConfig(j_itemMedium);
	(group_USER->getToggle("ENABLE C"))->setConfig(j_itemMedium);

	//big
	(group_USER->getIntSlider(TARGET_Selected.getName()))->setConfig({
		{"height", 30}
		});

	//--

	////disable
	////custom gui font to all inside gui panels
	//if (bLoadedGuiFont)
	//{
	//	//gui.setConfig(j_Gui);
	//}
	//else
	//{
	//	ofLogError(__FUNCTION__) << "setup_GUI_Customize() FILE '" << myTTF_Gui << "' NOT FOUND!";
	//}

	//-

	//folders expanding
	//group_CLOCK->minimize();
	group_TARGETS->minimize();
	group_RANGES->minimize();
}

//--------------------------------------------------------------
void ofxSurfingMoods::refresh_MOOD_Color()
{
	ofLogVerbose(__FUNCTION__) << "refresh_MOOD_Color";

	//mood color preview label
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
	//ofLogVerbose(__FUNCTION__) << "colorLabel: " << ofToString(colorLabel);

	//-

	//colorize
	std::string cStr = ofxGui::colorToString(colorLabel);
	//ofLogVerbose(__FUNCTION__) << "mood color: " << cStr;
	//"rgba(0,255,255,0.4)"

	ofJson jMood;
	jMood =
	{
		{"background-color", cStr},
		{"border-color", cStr},
		{"fill-color", cStr},
	};
	(group_USER->getToggle("MOOD RANGE"))->setConfig(jMood);

	(group_RANGES->getIntSlider(RANGE_Selected.getName()))->setConfig({
		{"background-color", cStr},
		//{"border-color", cStr},
		//{"fill-color", cStr},
		});

	(group_RANGES->getIntSlider("TARGET MIN"))->setConfig({
		{"background-color", cStr},
		//{"border-color", cStr},
		//{"fill-color", cStr},
		});

	(group_RANGES->getIntSlider("TARGET MAX"))->setConfig({
		{"background-color", cStr},
		//{"border-color", cStr},
		//{"fill-color", cStr},
		});

	(group_USER->getIntSlider(TARGET_Selected.getName()))->setConfig({
		//{"background-color", cStr},
		{"border-color", cStr},
		//{"fill-color", cStr},
		});

	(group_USER->getIntSlider(RANGE_Selected.getName()))->setConfig({
		//{"background-color", cStr},
		{"border-color", cStr},
		//{"fill-color", cStr},
		});
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup_GUI_Target()
{
	//target panel

	group_TARGETS->add(clone_TARGETS);
	group_TARGETS->add(bResetSort_Bank);
	group_TARGETS->add(bReset_Bank);
	group_TARGETS->add(bRandomize_Bank);

	//-

	//clock panel

	group_CLOCK->add(BPM);
	group_CLOCK->add(LEN_BARS);
	//SHOW_timer = true;
	//if (SHOW_timer)
	//	group_CLOCK->add(timer);//hide timer
	group_CLOCK->add(bReset_Settings);
	group_CLOCK->add(SHOW_GuiUser);
}


//ranges
//--------------------------------------------------------------
void ofxSurfingMoods::setup_GUI_Ranges()
{
	group_RANGES->add(RANGE_Selected);
	group_RANGES->add<ofxGuiIntSlider>(myRange.min);
	group_RANGES->add<ofxGuiIntSlider>(myRange.max);
	//group_RANGES->addSpacer(0, 1);
	//group_RANGES->add(bReset_Bank);
}

//--------------------------------------------------------------
void ofxSurfingMoods::setPosition(int _x, int _y)
{
	setGui_UserPositon(_x, _y);

	if (!MODE_vertical)//advanced panel to the right
	{
		setGui_AdvancedPositon(_x + 205, _y);
	}
	else//advanced panel to the botton
	{
		float h = group_USER->getHeight();
		setGui_AdvancedPositon(_x, _y + h + 20);
	}
}
#endif
*/