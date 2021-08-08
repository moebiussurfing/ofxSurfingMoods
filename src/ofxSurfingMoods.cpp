
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

	//-------

	// startup

	startup();

	//--
}

//--------------------------------------------------------------
void ofxSurfingMoods::startup()
{
	// preview rectangle
	//bUseCustomPreviewPosition = false;
	bUseCustomPreviewPosition = true;

	path_rect = path_Folder + "ofxSurfingMoods_";

	//TODO: crashes sometime if no file present..
	bool b = rectPreview.loadSettings("", path_rect, true);
	if (!b) rectPreview.setRect(25, 650, 700, 50); // initialize when no settings file created yet.

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
	group_Advanced->getVisible().set(bGui_Advanced);
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
void ofxSurfingMoods::update(ofEventArgs & args)
{
	//if (MODE_MarkovChain)
	//{
	//	i++;
	//}

	//-

	if (!bTickMode)
	{
		timer_Range.update();
		timer_Progress = 100 * timer_Range.getNormalizedProgress();
	}
	else
	{
		if (!MODE_Manual) {
			timer_Progress = ofMap(COUNTER_step_FromOne, 1, COUNT_Duration, 0, 100, true);
		}
		else {

		}
	}

	//timer_Progress = ofMap(COUNTER_step, 0, COUNT_Duration, 0, 100, true);
	//timer_Progress = ofMap(COUNTER_step_FromOne, 1, COUNTER_step_FromOne.getMax() + 1, 0, 100, true);
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw(ofEventArgs & args)
{
	if (!bGui) return;

	if (bGui_PreviewWidget) draw_PreviewWidget();

	draw_ImGui();
}

//--------------------------------------------------------------
void ofxSurfingMoods::windowResized(int w, int h)
{
	doResetPreviewWidget();
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
void ofxSurfingMoods::draw_PreviewWidget() // put to the rigth-top of user panel
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

	draw_PreviewWidget(gx, gy, ww, hh);

	//-

	// preview rectangle
	if (bEdit_PreviewWidget)
	{
		ofPushStyle();
		ofSetColor(128, 64);
		ofDrawRectangle(rectPreview);
		rectPreview.draw();
		ofPopStyle();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_PreviewWidget(int x, int  y, int  w, int  h) // custom position and size
{
	//if (bGui || bGui_PreviewWidget)
	//if (bGui_PreviewWidget)
	{
		ofPushStyle();

		//-

		const int NUM_Ranges = (int)NUM_RANGES;

		//for 0.20f: if fps is 60. duration will be 60/5 frames = 12frames
		blinkDuration = 0.20f * ofGetFrameRate();

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

		// color for manual control value
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

		//-

		// progress bar
		float padH = padBg + 2.0f;

		//-

		// bg

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

		//-

		// target boxes with labels

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

		// selected box/target

		// blink disabling box draw

		if (bBlink)
		{
			blinkCounterFrames++;
			if (blinkCounterFrames >= blinkDuration)
			{
				bBlink = false;
			}
		}

		// 1. filled box

		ofFill();
		float blinkFactor = 0.4f;
		if (!bBlink) ofSetColor(0, aSel);
		else ofSetColor(0, blinkFactor*aSel);

		ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f*padSel, y + 0.5f*padSel, sizes - padSel, h - padSel, ro);

		// 2. border

		ofNoFill();
		ofSetLineWidth(line);
		if (!bBlink) ofSetColor(cBord);
		else ofSetColor(cBord.r, cBord.g, cBord.b, cBord.a * blinkFactor);
		ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f*padSel, y + 0.5f*padSel, sizes - padSel, h - padSel, ro);

		//----

		// 2. completed timer progress

		//TODO: hardcoded limits

		if (RANGE_Selected >= 0 && RANGE_Selected < 3)
		{
			//float padH = 6.0f;//joined
			float _h = h * 0.25f;
			float ro2 = 0.5f * ro;

			float _w = 1;
			float wStep = 1;

			if (!MODE_Manual) {
				if (!bTickMode) {
					wStep = w / (float)MAX(1, (COUNTER_step_FromOne.getMax()));//width of any step
					_w = ofMap(COUNTER_step_FromOne, 1, COUNTER_step_FromOne.getMax() + 1, 0, w, true);
				}
				else {
					wStep = w / MAX(1, (float)(COUNT_Duration.get()));//width of any step
					_w = COUNTER_step_FromOne * wStep;
				}
			}
			// manual mode don't have steps counter bc it waits the user commands!
			// the we just cound all the bar a single step.
			else {
				_w = ofMap(timer_Progress, 0, 100, 0, w, true);
			}

			//-

			if (!bTickMode) {
				float wTimer = ofMap(timer_Progress, 0, 100, 0, wStep, true);//scale by step timer to make it analog-continuous
				_w = _w + wTimer;//add step timer
			}

			{
				// bg
				ofFill();
				ofSetColor(cBg);
				ofDrawRectRounded(x - padBg * 0.5f, y - padBg * 0.5f + h + padH, w + padBg, _h + padBg, ro);

				//// border
				//ofSetColor(cBord);
				//ofNoFill();
				//ofDrawRectRounded(x - padBg * 0.5f, y - padBg * 0.5f + h + padH, w + padBg, _h + padBg, ro2);

				//--

				// complete progress range

				// colored progress

				if (!(MODE_Manual && bTickMode)) {

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

					// bg rectangle
					ofFill();
					ofDrawRectRounded(x, y + h + padH, _w, _h, ro2);

					// mark all range steps with vertical lines
					if (!MODE_Manual) {
						//ofSetColor(255);
						ofNoFill();
						ofSetLineWidth(line + 1.0f);
						float xStep;
						for (int m = 1; m < COUNTER_step_FromOne.getMax(); m++)
						{
							xStep = wStep * m;
							ofDrawLine(x + xStep, (y + h + padH), x + xStep, (y + h + padH) + _h);
						}
					}
				}
			}

			//----

			// 3. manual control line
			{
				if (MODE_Manual)
				{
					ofNoFill();
					float xx = x + controlManual * w;

					////style1
					//{
					//	//black
					//	ofSetColor(0, 255);
					//	ofSetLineWidth(2.0f);
					//	ofDrawLine(xx - 1, (y + h + padH), xx - 1, (y + h + padH) + _h);

					//	//white
					//	ofSetColor(255, 150);
					//	ofSetLineWidth(line + 2.0f);
					//	_h = h * 0.25f;
					//	ofDrawLine(xx, (y + h + padH), xx, (y + h + padH) + _h);
					//}

					//style2
					{
						float __x, __y, __w, __h;
						__w = 5;
						__h = _h + 10;
						//__h = _h * 1.25;
						__x = xx;
						__y = (y - padBg * 0.5f + h + padH) + _h;
						ofRectangle r(__x, __y, __w, __h);

						ofFill();
						ofSetColor(cRangeRaw);
						//ofSetColor(255, 255, 255, 225);
						//ofDrawCircle(__x, __y, 3);
						ofSetRectMode(OF_RECTMODE_CENTER);
						ofDrawRectRounded(r, 2);
						ofSetRectMode(OF_RECTMODE_CORNER);
					}

					//_h = h * .5f;
					//ofDrawLine(xx, (y + h - padH), xx, (y + h + 2 * padH) + _h);
				}
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

	//-

	// 1. setup_GUI_Target

	PLAY.set("PLAY", false);

	BPM.set("BPM", 120, 30, 400);//bmp
	LEN_BARS.set("BARS LEN", 1, 1, 16);//bars
	timer.set("TIMER", 1000, 1, 60000);//ms
	timer.setSerializable(false);

	TARGET_Selected.set("STATE TARGET", 0, 0, NUM_TARGETS - 1);//target

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
	bClone_TARGETS.set("BANK CLONE>", false);
	bClone_TARGETS.setSerializable(false);
	bGui.set("MOODS SURFING", true);

	bGui_Advanced.set("Moods Advanced", false);
	bGui_ManualSlider.set("Manual Slider", false);
	bGui_ManualSliderHeader.set("Slider Header", true);
	bGui_PreviewWidget.set("Show Preview Widget", false);
	bUseCustomPreviewPosition.set("Custom", false);
	bEdit_PreviewWidget.set("Edit Preview Widget", false);

	//SHOW_GuiUser.set("SHOW USER", true);
	//SHOW_Clocks.set("SHOW CLOCKS", false);

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
	RANGE_Selected.set("RANGE MOOD", 0, 0, NUM_RANGES - 1);
	//myRange.min.set("MIN PRESET", 0, 0, NUM_TARGETS - 1);//?
	//myRange.max.set("MAX PRESET", 0, 0, NUM_TARGETS - 1);
	myRange.name.set("MOOD", ranges[RANGE_Selected].name);

	// main counters
	COUNT_Duration.set("STAY COUNT", 4, 1, 8);
	COUNTER_step_FromOne.set("COUNTER", 1, 0, COUNT_Duration);

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
	parameters_ranges.setName("RANGES");
	parameters_ranges.add(RANGE_Selected);
	parameters_ranges.add(myRange.min);
	parameters_ranges.add(myRange.max);
	parameters_ranges.add(COUNT_Duration);
	//parameters_ranges.add(bReset_Bank);
	//parameters_ranges.add(bResetSort_Bank);
	//parameters_ranges.add(bRandomize_Bank);

	//---

	// store params (grouped only to save/load, not to allow on gui or callbacks)
	params_STORE.setName("MoodMachine_params_STORE");
	params_STORE.add(COUNT_Duration);
	params_STORE.add(BPM);
	params_STORE.add(LEN_BARS);
	params_STORE.add(bGui);
	//params_STORE.add(SHOW_GuiUser);
	//params_STORE.add(SHOW_Clocks);
	params_STORE.add(bGui_Advanced);
	params_STORE.add(bGui_ManualSlider);
	params_STORE.add(bGui_ManualSliderHeader);
	params_STORE.add(bGui_PreviewWidget);
	params_STORE.add(bUseCustomPreviewPosition);
	params_STORE.add(PRESET_A_Enable);
	params_STORE.add(PRESET_B_Enable);
	params_STORE.add(PRESET_C_Enable);
	params_STORE.add(TARGET_Selected);
	params_STORE.add(MODE_MarkovChain);
	params_STORE.add(MODE_Ranged);
	params_STORE.add(MODE_StartLocked);
	params_STORE.add(MODE_AvoidRepeat);
	params_STORE.add(MODE_Manual);
	params_STORE.add(controlManual);
	//params_STORE.add(positionGui_Engine);
	params_STORE.add(guiManager.params_Advanced);
	//params_STORE.add(autoSaveLoad_settings);

	autoSaveLoad_settings.setSerializable(false);

	PLAY.setSerializable(false);

	//-

	// group params for callback listener only
	params_Listeners.setName("MoodMachine_params");

	params_Listeners.add(PLAY);
	params_Listeners.add(bTickMode);
	params_Listeners.add(BPM);
	params_Listeners.add(LEN_BARS);
	params_Listeners.add(bReset_Settings);
	params_Listeners.add(bRandomize_Bank);
	params_Listeners.add(bResetSort_Bank);
	params_Listeners.add(bReset_Bank);
	params_Listeners.add(bGui);
	//params_Listeners.add(SHOW_GuiUser);
	params_Listeners.add(bGui_Advanced);
	params_Listeners.add(bEdit_PreviewWidget);
	params_Listeners.add(bGui_PreviewWidget);
	params_Listeners.add(TARGET_Selected);
	params_Listeners.add(bClone_TARGETS);
	params_Listeners.add(PRESET_A_Selected);
	params_Listeners.add(PRESET_B_Selected);
	params_Listeners.add(PRESET_C_Selected);
	params_Listeners.add(Range_Min);
	params_Listeners.add(Range_Max);
	params_Listeners.add(timer);
	params_Listeners.add(MODE_MarkovChain);
	params_Listeners.add(MODE_Ranged);
	params_Listeners.add(MODE_StartLocked);
	params_Listeners.add(MODE_AvoidRepeat);
	params_Listeners.add(MODE_Manual);
	params_Listeners.add(controlManual);
	//params_Listeners.add(positionGui_Engine);

	//----

	// 2. setup_GUI_User

	//user
	//params_USER.setName("USER");
	params_USER.setName("MOOD MACHINE");//change display name

	params_USER.add(PLAY);

	params_USER.add(MODE_Ranged);
	params_USER.add(MODE_MarkovChain);
	params_USER.add(MODE_Manual);

	params_USER.add(controlManual);

	params_USER.add(MODE_StartLocked);
	params_USER.add(MODE_AvoidRepeat);

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

	params_USER.add(bEdit_PreviewWidget);

	params_USER.add(bGui_PreviewWidget);
	params_USER.add(bGui_Advanced);

	//params_USER.add(autoSaveLoad_settings);

//eclude
	COUNTER_step_FromOne.setSerializable(false);

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

	//SHOW_GuiUser = b;

	//workflow
	//if (!SHOW_GuiUser && bGui_Advanced) bGui_Advanced = false;
}

//--------------------------------------------------------------
void ofxSurfingMoods::setGui_ToggleVisible()
{
	bGui = !bGui;
	//gui.getVisible().set(bGui);

	//SHOW_GuiUser = !SHOW_GuiUser;
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
void ofxSurfingMoods::doRunEngineStep()
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
			if (!MODE_AvoidRepeat.get())// allows repeat target
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
void ofxSurfingMoods::doBeatTick()
{
	if (bTickMode && bExternalLocked) doRunStep();
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
void ofxSurfingMoods::doRunStep()
{
	if (PLAY || (bTickMode && bExternalLocked))
	{
		//RESTART
		if (!bTickMode) timer_Range.start(false);

		//-

		//modes

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

			//do randomize between min/max ranges

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
			}
		}

		//-

		else if (MODE_MarkovChain.get())
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
					if (MODE_AvoidRepeat.get())//avoids repeat same target
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
void ofxSurfingMoods::timer_Range_Complete(int &args)
{
	ofLogNotice(__FUNCTION__) << "\n";

	ofLogVerbose(__FUNCTION__) << "timer_Range_Complete";
	doRunStep();
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

		else if (WIDGET == bTickMode.getName())
		{

		}

		else if (WIDGET == bClone_TARGETS.getName())
		{
			if (bClone_TARGETS)
			{
				bClone_TARGETS = false;
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

		else if (WIDGET == timer.getName())
		{
			if (PLAY)
			{
				timer_Range.setup(timer);
			}
		}

		else if (WIDGET == BPM.getName())
		{
			//  60,000 / BPM = MS
			timer = LEN_BARS * (BPM_BAR_RATIO * (60000 / BPM));

			//TODO:
			//must solve jumps..
			//stop();
		}

		else if (WIDGET == LEN_BARS.getName())
		{
			//  60,000 / BPM = MS
			timer = LEN_BARS * (BPM_BAR_RATIO * (60000 / BPM));
		}

		//resets
		else if (WIDGET == bReset_Settings.getName() && bReset_Settings)
		{
			bReset_Settings = false;
			resetClock();
			stop();
		}
		else if (WIDGET == bReset_Bank.getName() && bReset_Bank)
		{
			bReset_Bank = false;
			resetBank(false, false);//all to 0
			stop();
		}
		else if (WIDGET == bResetSort_Bank.getName() && bResetSort_Bank)
		{
			bResetSort_Bank = false;
			resetBank(false, true);//relative random
			stop();
		}
		else if (WIDGET == bRandomize_Bank.getName() && bRandomize_Bank)
		{
			bRandomize_Bank = false;
			resetBank(true, false);
			stop();
		}

		//gui
	//	else if (WIDGET == bGui_Advanced.getName())
	//	{
	//		/*
	//#ifdef USE_ofxGuiExtended
	//			group_Advanced->getVisible().set(bGui_Advanced);

	//			//workflow
	//			auto p = group_USER->getShape().getTopRight();
	//			if (!MODE_vertical)//advanced panel to the right
	//			{
	//				setGui_AdvancedPositon(p.x + 5, p.y);
	//			}
	//			else//advanced panel to the botton
	//			{
	//				auto p = group_USER->getShape().getBottomLeft();
	//				setGui_AdvancedPositon(p.x, p.y + 10);
	//			}
	//#endif
	//		*/
	//	}

		//	else if (WIDGET == SHOW_GuiUser.getName())
	//	{
	//		/*
	//#ifdef USE_ofxGuiExtended
	//			group_USER->getVisible().set(SHOW_GuiUser);
	//#endif
	//		*/
	//	}

		else if (WIDGET == bGui_PreviewWidget.getName())
		{
		}

		else if (WIDGET == bEdit_PreviewWidget.getName())
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

		// modes
		else if (WIDGET == MODE_MarkovChain.getName())
		{
			if (MODE_MarkovChain)
			{
				MODE_Manual = false;
				MODE_Ranged = false;
			}
			else refresModeshWorkflow();
		}
		else if (WIDGET == MODE_Ranged.getName())
		{
			if (MODE_Ranged)
			{
				MODE_Manual = false;
				MODE_MarkovChain = false;
			}
			else refresModeshWorkflow();
		}
		else if (WIDGET == MODE_Manual.getName())
		{
			if (MODE_Manual)
			{
				MODE_Ranged = false;
				MODE_MarkovChain = false;
			}
			else refresModeshWorkflow();
			/*
#ifdef USE_ofxGuiExtended
			(group_USER->getFloatSlider(controlManual.getName()))->setEnabled(MODE_Manual.get());
#endif
			*/
		}
		else if (WIDGET == controlManual.getName())
		{
			// workflow
			if (!bTickMode)
			{
				if (!PLAY.get())
				{
					PLAY = true;
				}
			}
			else if (!bExternalLocked.get())
			{
				bExternalLocked = true;
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

		else if (name == COUNT_Duration.getName())
		{
			COUNT_Duration = ofClamp(COUNT_Duration, COUNT_Duration.getMin(), COUNT_Duration.getMax());

			COUNTER_step_FromOne.setMin(1);
			COUNTER_step_FromOne.setMax(COUNT_Duration.get());

			/*
#ifdef USE_ofxGuiExtended
			//re scale slider limit
			(group_USER->getIntSlider("COUNTER"))->setMax(COUNT_Duration);
#endif
			*/

			// avoid rescale timers error on preview

			//// workflow
			//bool bPre = isPlaying();
			//stop();
			//if (bPre)play();
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

	//--

	//TODO:
	//fixing..
	static bool bCustom2 = true;
	if (bCustom2)
	{
		guiManager.AddStyle(PLAY, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, false, 1, 5);

		guiManager.AddStyle(MODE_Ranged, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, true, 3);
		guiManager.AddStyle(MODE_MarkovChain, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, true, 3);
		guiManager.AddStyle(MODE_Manual, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, false, 3, 5);


		//	guiManager.AddStyle(bPrevious, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL, true, 2);
		//	guiManager.AddStyle(bNext, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL, false, 2, 20);
		//	guiManager.AddStyle(separation, SurfingImGuiTypes::OFX_IM_STEPPER);
		//	guiManager.AddStyle(speed, SurfingImGuiTypes::OFX_IM_DRAG, false, 1, 10);
		//	guiManager.AddStyle(shapeType, SurfingImGuiTypes::OFX_IM_SLIDER);
		//	guiManager.AddStyle(size, SurfingImGuiTypes::OFX_IM_STEPPER);
		//	guiManager.AddStyle(amount, SurfingImGuiTypes::OFX_IM_DRAG, false, 1, 10);
		//	guiManager.AddStyle(bMode1, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, true, 2);
		//	guiManager.AddStyle(bMode2, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, false, 2);
		//	guiManager.AddStyle(bMode3, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, true, 2);
		//	guiManager.AddStyle(bMode4, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, false, 2);
		//	//guiManager.AddStyle(lineWidth3, SurfingImGuiTypes::OFX_IM_DRAG); // not works?
		//	// hide some params from any on-param-group appearance
		//	guiManager.AddStyle(speed3, SurfingImGuiTypes::OFX_IM_HIDDEN, false, -1, 50);
		//	guiManager.AddStyle(size2, SurfingImGuiTypes::OFX_IM_HIDDEN, false, -1, 50);
		//	guiManager.AddStyle(bPrevious, SurfingImGuiTypes::OFX_IM_HIDDEN);
		//	guiManager.AddStyle(bNext, SurfingImGuiTypes::OFX_IM_HIDDEN);
		//  guiManager.AddStyle(lineWidth, SurfingImGuiTypes::OFX_IM_HIDDEN);

	}
	//guiManager.bAutoResize = false;
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_ManualSlider()
{
	if (bGui_ManualSlider && MODE_Manual)
	{
		// panels sizes
		float ww = 20;
		float hh = 100;
		float xx = ofGetWidth() / 2 - ww / 2;
		float yy = ofGetHeight() / 2 - hh / 2;

		ImGuiCond flagsCond = ImGuiCond_None;
		flagsCond |= ImGuiCond_FirstUseEver;
		ImGui::SetNextWindowSize(ImVec2(ww, hh), flagsCond);
		ImGui::SetNextWindowPos(ImVec2(xx, yy), flagsCond);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(ww, hh));
		{
			static bool bOpen = true;
			ImGuiTreeNodeFlags _flagt;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
			window_flags |= ImGuiWindowFlags_NoBackground;
			window_flags |= ImGuiWindowFlags_NoScrollbar;
			if (!bGui_ManualSliderHeader) window_flags |= ImGuiWindowFlags_NoTitleBar;
			//window_flags |= ImGuiWindowFlags_NoCollapse;
			//window_flags |= ImGuiWindowFlags_NoDecoration;
			//window_flags |= ImGuiWindowFlags_NoDocking;
			//if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

			//std::string n = bGui_ManualSlider.getName();
			std::string n = "Control";
			guiManager.beginWindow(n.c_str(), (bool*)&bGui_ManualSlider.get(), window_flags);
			//guiManager.beginWindow(bGui_ManualSlider, window_flags);
			{
				auto c = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
				ImVec4 cBg = ImVec4(c.x, c.y, c.z, c.w * 0.2);
				ImGui::PushStyleColor(ImGuiCol_SliderGrab, cRange);
				ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, cRangeRaw);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, cBg);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, cBg);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, cBg);
				ofxImGuiSurfing::AddVSlider(controlManual);
				ImGui::PopStyleColor(5);
			}
			guiManager.endWindow();
		}
		ImGui::PopStyleVar();
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_User()
{
	// USER

	//if (SHOW_GuiUser)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		static bool bMinimize = false;

		//-

		guiManager.beginWindow(bGui.getName().c_str(), (bool*)&bGui.get(), window_flags);
		{
			guiManager.refresh();

			static bool bOpen = false;
			ImGuiTreeNodeFlags _flagt;
			_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
			_flagt |= ImGuiTreeNodeFlags_Framed;

			// colorize moods
			float a;
			ImVec4 c;
			if (RANGE_Selected == 0) c = color_MOOD1;
			else if (RANGE_Selected == 1) c = color_MOOD2;
			else if (RANGE_Selected == 2) c = color_MOOD3;

			if (bTickMode) a = 1.0f;
			else a = ofMap(1 - timer_Range.getNormalizedProgress(), 0, 1, 0.25, 1, true);

			ImVec4 ca = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * a);

			//-

			// play / lock

			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ca);
			ImGui::PushStyleColor(ImGuiCol_Button, ca);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ca);
			if (!bTickMode) guiManager.Add(PLAY, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
			else
			{
				if (guiManager.Add(bExternalLocked, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG))
				{
					//bExternalLocked = true;
				}
			}
			ImGui::PopStyleColor(3);

			//-

			ImGui::Dummy(ImVec2(0, 2));

			ofxImGuiSurfing::ToggleRoundedButton("Minimize", &bMinimize);

			//--

			// clock
			if (!bMinimize)
			{
				bOpen = false;
				_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt |= ImGuiTreeNodeFlags_Framed;

				ofxImGuiSurfing::AddToggleRoundedButton(bTickMode);

				if (ImGui::TreeNodeEx("CLOCK", _flagt))
				{
					guiManager.refresh();

					// clock panel
					guiManager.Add(BPM, SurfingImGuiTypes::OFX_IM_DRAG, false, 1, 0);
					guiManager.Add(BPM, SurfingImGuiTypes::OFX_IM_STEPPER);

					guiManager.Add(bReset_Settings, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1, 0);

					ImGui::TreePop();
				}
			}

			//--

			/*if (ImGui::TreeNodeEx("PANELS", _flagt))
			{
				guiManager.refresh();

				guiManager.Add(bGui_Advanced, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
				//guiManager.Add(SHOW_Clocks, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);

				if (ImGui::TreeNodeEx("PREVIEW WIDGET", _flagt))
				{
					guiManager.refresh();
					float _w100 = ofxImGuiSurfing::getWidgetsWidth(1);
					float _w50 = ofxImGuiSurfing::getWidgetsWidth(2);
					float _h = BUTTON_BIG_HEIGHT;

					guiManager.Add(bGui_PreviewWidget, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
					guiManager.Add(bEdit_PreviewWidget, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);

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

					ImGui::TreePop();
				}
				ImGui::Dummy(ImVec2(0, 5));

				ImGui::TreePop();
			}*/

			if (!bMinimize) {
				bool bOpen2 = false;
				ImGuiTreeNodeFlags _flagt2 = (bOpen2 ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt2 |= ImGuiTreeNodeFlags_Framed;
				if (ImGui::TreeNodeEx("MODES", _flagt2))
				{
					guiManager.refresh();

					guiManager.Add(MODE_Ranged, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
					guiManager.Add(MODE_MarkovChain, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
					guiManager.Add(MODE_Manual, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
					ImGui::TreePop();
				}

				guiManager.refresh();
			}

			//guiManager.Add(MODE_Ranged, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, true, 3);
			//guiManager.Add(MODE_MarkovChain, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, true, 3);
			//guiManager.Add(MODE_Manual, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, false, 3);

			if (MODE_Manual) {
				ImGui::PushStyleColor(ImGuiCol_Text, ca);
				guiManager.Add(controlManual, SurfingImGuiTypes::OFX_IM_SLIDER);
				ImGui::PopStyleColor();
			}
			else {
				//ofxImGuiSurfing::AddVoidWidget();
			}

			ImGui::Dummy(ImVec2(0, 2));

			//--

			if (MODE_Manual) {//ofxImGuiSurfing::AddVoidWidget();
			}
			else if (!(bTickMode && MODE_Manual)) {
				guiManager.Add(COUNT_Duration, SurfingImGuiTypes::OFX_IM_STEPPER); // user setter
			}
			else {
				//ofxImGuiSurfing::AddVoidWidget();
			}

			//guiManager.Add(COUNT_Duration, SurfingImGuiTypes::OFX_IM_SLIDER);

			ImGui::Dummy(ImVec2(0, 2));

			//-

			// for monitor only
			if (MODE_Manual) {//ofxImGuiSurfing::AddVoidWidget();
			}
			else if (!(bTickMode && MODE_Manual)) {
				if (COUNT_Duration != 1) {
					guiManager.Add(COUNTER_step_FromOne, SurfingImGuiTypes::OFX_IM_INACTIVE);
				}
				else {
					//ofxImGuiSurfing::AddVoidWidget();
				}
			}
			else {
				//ofxImGuiSurfing::AddVoidWidget();
			}

			// progress
			if (MODE_Manual) {//ofxImGuiSurfing::AddVoidWidget();
			}
			else if (!(bTickMode && MODE_Manual))
			{
				guiManager.Add(timer_Progress, SurfingImGuiTypes::OFX_IM_PROGRESS_BAR);
			}
			else {//ofxImGuiSurfing::AddVoidWidget();
			}

			//--

			if (!bMinimize) {
				ImGui::Dummy(ImVec2(0, 2));

				guiManager.Add(MODE_StartLocked, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, true, 2);
				guiManager.Add(MODE_AvoidRepeat, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 2);

			}
			ImGui::Dummy(ImVec2(0, 5));

			//guiManager.Add(MOOD_Color_Preview, SurfingImGuiTypes::OFX_IM_DEFAULT);

			//----

			ImGui::PushStyleColor(ImGuiCol_Text, ca);

			string s;

			s = ofToString(BPM.get(), 2) + " BPM";
			ImGui::Text(s.c_str());

			if (MODE_Ranged) s = MODE_Ranged.getName();
			else if (MODE_MarkovChain) s = MODE_MarkovChain.getName();
			else if (MODE_Manual) s = MODE_Manual.getName();
			ImGui::Text(s.c_str());

			ImGui::Text("STATES:RANGE");
			ImGui::Dummy(ImVec2(0, 2));

			guiManager.Add(RANGE_Selected, SurfingImGuiTypes::OFX_IM_INACTIVE);
			guiManager.Add(TARGET_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT, false, 1, 4);

			ImGui::PopStyleColor();

			//-

			if (!bMinimize) {
				ImGui::Dummy(ImVec2(0, 5));

				ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "TARGET > PRESETS");
				ImGui::Dummy(ImVec2(0, 2));

				guiManager.Add(PRESET_A_Enable, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
				guiManager.Add(PRESET_B_Enable, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
				guiManager.Add(PRESET_C_Enable, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1, 4);
			}

			if (PRESET_A_Enable) guiManager.Add(PRESET_A_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT);
			if (PRESET_B_Enable) guiManager.Add(PRESET_B_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT);
			if (PRESET_C_Enable) guiManager.Add(PRESET_C_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT, false, 1, 4);

			//--

			if (!bMinimize) {

				ImGui::Dummy(ImVec2(0, 5));

				static bool bExtra;
				ofxImGuiSurfing::ToggleRoundedButton("Extra", &bExtra);
				if (bExtra)
				{
					ImGui::Indent();
					{
						ofxImGuiSurfing::AddToggleRoundedButton(bGui_Advanced);
						ofxImGuiSurfing::AddToggleRoundedButton(bGui_PreviewWidget);
						if (bGui_PreviewWidget)
						{
							//if (ImGui::TreeNodeEx("PREVIEW WIDGET", _flagt))
							ImGui::Indent();
							{
								guiManager.refresh();
								float _w100 = ofxImGuiSurfing::getWidgetsWidth(1);
								float _w50 = ofxImGuiSurfing::getWidgetsWidth(2);
								float _h = BUTTON_BIG_HEIGHT;

								//guiManager.Add(bGui_PreviewWidget, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);

								ofxImGuiSurfing::AddToggleRoundedButton(bEdit_PreviewWidget);
								//guiManager.Add(bEdit_PreviewWidget, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
								if (ImGui::Button("Reset"/*, ImVec2(_w100, _h / 2)*/)) { doResetPreviewWidget(); }

								//ImGui::SameLine();
								//ofxImGuiSurfing::ToggleRoundedButton("Custom", &bUseCustomPreviewPosition);
								ofxImGuiSurfing::AddToggleRoundedButton(bUseCustomPreviewPosition);

								//ImGui::TreePop();
							}
							ImGui::Unindent();
						}
						//ImGui::Dummy(ImVec2(0, 5));

						if (MODE_Manual)ofxImGuiSurfing::AddToggleRoundedButton(bGui_ManualSlider);
						if (MODE_Manual && bGui_ManualSlider)ofxImGuiSurfing::AddToggleRoundedButton(bGui_ManualSliderHeader);

						ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bAdvanced);
						guiManager.drawAdvancedSubPanel(false);
					}
					ImGui::Unindent();
				}
			}
		}
	}
	guiManager.endWindow();
}

//--------------------------------------------------------------
void ofxSurfingMoods::doResetPreviewWidget()
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

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui()
{
	if (!bGui) return;

	guiManager.begin();
	{
		draw_ImGui_User();
		draw_ImGui_Advanced();
		draw_ImGui_ManualSlider();
	}
	guiManager.end();
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw_ImGui_Advanced()
{
	if (!bGui) return;

	//--

	// advanced
	{
		if (bGui_Advanced)
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

				std::string n = "MOODS ADVANCED";
				guiManager.beginWindow(n.c_str(), (bool*)&bGui_Advanced.get(), window_flags);
				{
					guiManager.refresh();

					//guiManager.Add(SHOW_GuiUser, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1);

					//--

					// TOOLS
					{
						bOpen = false;
						_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
						_flagt |= ImGuiTreeNodeFlags_Framed;

						if (ImGui::TreeNodeEx("TOOLS", _flagt))
						{
							guiManager.refresh();

							// target panel
							guiManager.Add(bClone_TARGETS, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
							guiManager.Add(bResetSort_Bank, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
							guiManager.Add(bReset_Bank, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
							guiManager.Add(bRandomize_Bank, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1, 5);

							ImGui::TreePop();
						}
					}

					//--

					// RANGES
					{
						if (ImGui::TreeNodeEx("DEBUG LIMITS", _flagt))
						{
							guiManager.refresh();

							ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
							flags |= ImGuiTreeNodeFlags_Framed;
							flags |= ImGuiTreeNodeFlags_DefaultOpen;

							ofxImGuiSurfing::AddGroup(parameters_ranges, flags);
							//ofxImGuiSurfing::AddGroup(params_STORE, flags);
							//ofxImGuiSurfing::AddGroup(params_Listeners, flags);

							ImGui::TreePop();
						}
					}

					guiManager.refresh();

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

	group_TARGETS->add(bClone_TARGETS);
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