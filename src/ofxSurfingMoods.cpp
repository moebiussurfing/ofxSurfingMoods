
#include "ofxSurfingMoods.h"


// keys
//--------------------------------------------------------------
void ofxSurfingMoods::keyPressed(ofKeyEventArgs &eventArgs)
{
	const int &key = eventArgs.key;

	//--

	// key enabler
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

	// disabler for all keys. (independent from bActive)
	if (!bKeys) return;

	ofLogNotice(__FUNCTION__) << (char)key << " [" << key << "]";

	// modifiers
	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

	static bool debug = false;
	if (debug)
	{
		ofLogNotice(__FUNCTION__) << "mod_COMMAND: " << (mod_COMMAND ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_CONTROL: " << (mod_CONTROL ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_ALT: " << (mod_ALT ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_SHIFT: " << (mod_SHIFT ? "ON" : "OFF");
	}

	//-

	if (!bKeys) return;

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
void ofxSurfingMoods::keyReleased(ofKeyEventArgs &eventArgs)
{
	const int &key = eventArgs.key;
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

//-

//--------------------------------------------------------------
void ofxSurfingMoods::setup(int numTargets, int numPresets, int limit1, int limit2)
{
	NUM_TARGETS = numTargets; // TARGETS
	NUM_PRESETS_A = numPresets; // PRESETS
	NUM_PRESETS_B = numPresets; // PRESETS
	NUM_PRESETS_C = numPresets; // PRESETS

	rLimit1 = limit1;
	rLimit2 = limit2;

	setup(); // default sizes
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup() // default sizes
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
	path_Folder = "ofxSurfingMoods/"; // default folder
	filename_Settings = "moods_Settings"; // settings
	filename_Bank = "moods_Bank.json"; // ranges bank: any target to relatives preset/pattern

	//-

	//markov
	path_markovMatrix = path_Folder + "markov/" + "transitionMatrix.txt";

	//avoid crash
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

	// font to draw preview boxes
	fname = "overpass-mono-bold.otf";
	myTTF = "assets/fonts/" + fname;
	sizeTTF = 9; // font size
	bool isLoaded = myFont.load(myTTF, sizeTTF, true, true);
	if (!isLoaded)
	{
		ofLogError(__FUNCTION__) << "ofTrueTypeFont FONT FILE '" << myTTF << "' NOT FOUND!";
	}

	//--

	// Params
	setup_Params();

	//--

	// callbacks

	// 1.
	ofAddListener(params_Listeners.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);
	ofAddListener(params_USER.parameterChangedE(), this, &ofxSurfingMoods::Changed_Params_Listeners);

	// 2.
	ofAddListener(parameters_ranges.parameterChangedE(), this, &ofxSurfingMoods::Changed_Ranges);

	//-

	// timer
	// 60,000 / bpmSpeed = MS
	timer = bpmLenghtBars * (BPM_BAR_RATIO * (60000 / bpmSpeed));
	timer_Range.setup(timer);
	ofAddListener(timer_Range.TIMER_COMPLETE, this, &ofxSurfingMoods::timer_Range_Complete);
	ofAddListener(timer_Range.TIMER_STARTED, this, &ofxSurfingMoods::timer_Range_Started);

	//-

	// RESET
	//// ERASE BANK TARGETS
	//resetBank(false);
	////REST CLOCK
	//resetClock();

	//-

	setGui_Visible(true);

	//--

	ENABLED_MoodMachine = true;
	bIsPlaying = false;
	timer = 0;
	bPLAY = false;

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
	bUseCustomPreviewPosition = true;

	path_rect = path_Folder + "ofxSurfingMoods_";

	//TODO: crashes sometime if no file present..
	bool b = rectPreview.loadSettings("_PreviewRect", path_rect, true);
	//bool b = rectPreview.loadSettings("", path_rect, true);
	if (!b) rectPreview.setRect(25, 650, 700, 50); // initialize when no settings file created yet.
	rectPreview.disableEdit();

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

	//--

	refresh_MOOD_Color();

	//updateLabels();
}

//--------------------------------------------------------------
void ofxSurfingMoods::update(ofEventArgs & args)
{
	update_PreviewColors();

	//if (MODE_MarkovChain)
	//{
	//	i++;
	//}

	//-

	if (!bModeClockExternal)
	{
		timer_Range.update();
		timer_Progress = 100 * timer_Range.getNormalizedProgress();
	}
	else
	{
		if (!MODE_Manual) {
			timer_Progress = ofMap(counterStepFromOne, 1, countStayDuration, 0, 100, true);
		}
		else {

		}
	}

	//timer_Progress = ofMap(COUNTER_step, 0, countStayDuration, 0, 100, true);
	//timer_Progress = ofMap(counterStepFromOne, 1, counterStepFromOne.getMax() + 1, 0, 100, true);
}

//--------------------------------------------------------------
void ofxSurfingMoods::draw(ofEventArgs & args)
{
	//TODO: debug
	if (0)
	{
		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(2);
		ofSetColor(0, 0, 255, 255);
		ofDrawRectangle(rectPreview);
		float xx = rectPreview.getBottomLeft().x;
		float yy = rectPreview.getCenter().y;
		float ww = rectPreview.getWidth();
		ofDrawLine(xx, yy, xx + ww, yy);
		rectPreview.draw();
		ofPopStyle();
	}

	if (!bGui) return;

	//update_PreviewColors();
	if (bGui_PreviewWidget) draw_PreviewWidget();

	//-

	//// ImGui
	//draw_ImGui(); // -> TODO: fails when other ImGui instances...
}

//--------------------------------------------------------------
void ofxSurfingMoods::windowResized(int w, int h)
{
	if (!bUseCustomPreviewPosition) doResetPreviewWidget();
}

//--------------------------------------------------------------
void ofxSurfingMoods::exit()
{
	//-

	// preview rectangle
	//rectPreview.saveSettings("", path_rect, true);
	rectPreview.saveSettings("_PreviewRect", path_rect, true);

	//-

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
	pad = 10;

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
		gw = ofGetWidth() - 2 * pad;
		gx = pad;
		gy = pad;
		ww = gw;
		hh = 50;
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

	//// Preview rectangle Bg
	//if (bUseCustomPreviewPosition)
	//{
	//	if (bEdit_PreviewWidget)
	//	{
	//		ofPushStyle();
	//		ofSetColor(128, 64);
	//		ofDrawRectangle(rectPreview);
	//		rectPreview.draw();
	//		ofPopStyle();
	//	}
	//}
	//else {//TODO:
	//	ofPushStyle();
	//	ofSetColor(128, 64);
	//	ofRectangle r = ofRectangle(gx, gy, gw, gh);;
	//	ofDrawRectangle(r);
	//	ofPopStyle();
	//}
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
}


//--------------------------------------------------------------
void ofxSurfingMoods::draw_PreviewWidget(int x, int  y, int  w, int  h) // customize position and size
{
	ofPushStyle();

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

	float hratio = 0.7f;// states bar vs progress bar

	float h1 = h * hratio;
	float h2 = h * (1 - hratio);

	float x2, y2, ww2, hh2;

	const int NUM_Ranges = (int)NUM_RANGES;

	// for 0.20f: if fps is 60. duration will be 60/5 frames = 12frames
	blinkDuration = 0.20f * ofGetFrameRate();

	float sizes = w / (float)NUM_TARGETS;
	float ro = 4.0f;
	float line = 2.0f;

	float padBg, padBox, padSel;
	padBg = h1 * 0.05f;
	padBox = h1 * 0.1f;
	padSel = h1 * 0.1f;

	// alphas
	int aBg = 140;
	int aRg = 24;
	int aSel = 48;

	//-

	// 0. Main bg expanded
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

	// 1. Bg

	//ofFill();
	//ofSetColor(cBg);
	//ofDrawRectRounded(x, y, w, h1, ro);

	// States boxes
	{
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

	// 2. Target boxes with labels

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
		hb = h1 - padBox;

		//-

		// 2.1 Target box

		ofFill();
		ofDrawRectRounded(xb, yb, wb, hb, ro);

		//-

		// 2.2 Text label

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

	//--

	// 3. Selected box/target

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
	ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f*padSel, y + 0.5f*padSel, sizes - padSel, h1 - padSel, ro);

	//-

	// 3.2. Border

	ofNoFill();
	ofSetLineWidth(line);
	if (!bBlink) ofSetColor(cBord);
	else ofSetColor(cBord.r, cBord.g, cBord.b, cBord.a * blinkFactor);
	ofDrawRectRounded(x + TARGET_Selected * sizes + 0.5f*padSel, y + 0.5f*padSel, sizes - padSel, h1 - padSel, ro);

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

		if (!MODE_Manual) {
			if (!bModeClockExternal) {
				wStep = ww2 / (float)MAX(1, (counterStepFromOne.getMax()));//width of any step
				_w = ofMap(counterStepFromOne, 1, counterStepFromOne.getMax() + 1, 0, ww2, true);
			}
			else {
				wStep = ww2 / MAX(1, (float)(countStayDuration.get()));//width of any step
				_w = counterStepFromOne * wStep;
			}
		}
		// Manual mode don't have steps counter bc it waits the user commands!
		// The we just cound all the bar a single step.
		else {
			_w = ofMap(timer_Progress, 0, 100, 0, ww2, true);
		}

		//-

		if (!bModeClockExternal) {
			float wTimer = ofMap(timer_Progress, 0, 100, 0, wStep, true);//scale by step timer to make it analog-continuous
			_w = _w + wTimer;//add step timer
		}

		//-

		{
			// 4.1 Bg

			ofFill();
			ofSetColor(0, 0, 0, 32);
			//ofSetColor(cBg);
			ofDrawRectRounded(x2, y2, ww2, hh2, ro);

			//-

			// 4. 2 Complete progress range 
			// Colored

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

	//-

	ofPopStyle();

	//-

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

	// 1. Setup_GUI_Target

	bPLAY.set("PLAY", false);

	bpmSpeed.set("BPM", 120, 30, 400);//bmp
	bpmLenghtBars.set("BARS LEN", 1, 1, 16);//bars
	timer.set("TIMER", 1000, 1, 60000);//ms
	timer.setSerializable(false);

	TARGET_Selected.set("STATE TARGET", 0, 0, NUM_TARGETS - 1);//target

	PRESET_A_Enable.set("ENABLE A", true);
	PRESET_B_Enable.set("ENABLE B", true);
	PRESET_C_Enable.set("ENABLE C", true);

	// Presets margins

	PRESET_A_Selected.set("PRESET A", 0, 0, NUM_PRESETS_A - 1);
	PRESET_B_Selected.set("PRESET B", 0, 0, NUM_PRESETS_B - 1);
	PRESET_C_Selected.set("PRESET C", 0, 0, NUM_PRESETS_C - 1);

	Range_Min.set("MIN TARGET", 0, 0, NUM_TARGETS - 1);
	Range_Max.set("MAX TARGET", NUM_TARGETS - 1, 0, NUM_TARGETS - 1);

	bResetClockSettings.set("RESET CLOCK", false);
	bResetClockSettings.setSerializable(false);
	bClone_TARGETS.set("BANK CLONE>", false);
	bClone_TARGETS.setSerializable(false);
	bGui.set("MOODS SURFING", true);
	bKeys.set("Keys", true);

	bGui_Advanced.set("Moods Advanced", false);
	bGui_ManualSlider.set("Show Manual Slider", false);
	bGui_ManualSliderHeader.set("Slider Header", false);
	bGui_PreviewWidget.set("Show Preview Widget", false);
	bUseCustomPreviewPosition.set("Custom", false);
	bEdit_PreviewWidget.set("Edit Preview Widget", false);

	//----

	// 3. Setup_GUI_Ranges
	// init ranges vector
	ranges.resize(NUM_RANGES);

	// Ranges

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

	// Ranges

	// Define params
	timer_Progress.set("COMPLETE", 0, 0, 100);//%
	RANGE_Selected.set("RANGE MOOD", 0, 0, NUM_RANGES - 1);
	//myRange.min.set("MIN PRESET", 0, 0, NUM_TARGETS - 1);//?
	//myRange.max.set("MAX PRESET", 0, 0, NUM_TARGETS - 1);
	myRange.name.set("MOOD", ranges[RANGE_Selected].name);

	// Main counters
	countStayDuration.set("STAY COUNT", 4, 1, 8);
	counterStepFromOne.set("COUNTER", 1, 0, countStayDuration);

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

	// Group params outside gui but to use listeners and xml settings
	parameters_ranges.setName("RANGES");
	parameters_ranges.add(RANGE_Selected);
	parameters_ranges.add(myRange.min);
	parameters_ranges.add(myRange.max);
	//parameters_ranges.add(countStayDuration);
	//parameters_ranges.add(bReset_Bank);
	//parameters_ranges.add(bResetSort_Bank);
	//parameters_ranges.add(bRandomize_Bank);

	//---

	// Store params (grouped only to save/load, not to allow on gui or callbacks)
	params_STORE.setName("ofxSurfingMoods_Settings");
	params_STORE.add(countStayDuration);
	params_STORE.add(bpmSpeed);
	params_STORE.add(bpmLenghtBars);
	params_STORE.add(bGui);
	params_STORE.add(bGui_Advanced);
	params_STORE.add(bGui_ManualSlider);
	params_STORE.add(bGui_ManualSliderHeader);
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
	params_STORE.add(guiManager.params_Advanced);
	params_STORE.add(bGui_PreviewWidget);
	params_STORE.add(bUseCustomPreviewPosition);

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
	params_Listeners.add(bGui);
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

	//----

	// 2. setup_GUI_User

	params_USER.setName("MOOD MACHINE");//change display name

	params_USER.add(bPLAY);

	params_USER.add(MODE_Ranged);
	params_USER.add(MODE_MarkovChain);
	params_USER.add(MODE_Manual);
	params_USER.add(controlManual);

	params_USER.add(MODE_StartLocked);
	params_USER.add(MODE_AvoidRepeat);

	params_USER.add(countStayDuration);
	params_USER.add(timer_Progress);
	params_USER.add(counterStepFromOne);

	params_USER.add(RANGE_Selected);
	params_USER.add(TARGET_Selected);
	params_USER.add(PRESET_A_Selected);
	params_USER.add(PRESET_B_Selected);
	params_USER.add(PRESET_C_Selected);
	params_USER.add(PRESET_A_Enable);
	params_USER.add(PRESET_B_Enable);
	params_USER.add(PRESET_C_Enable);

	params_USER.add(bEdit_PreviewWidget);
	params_USER.add(bGui_PreviewWidget);
	params_USER.add(bGui_Advanced);

	// Exclude from file settings
	counterStepFromOne.setSerializable(false);
	bGui_ManualSliderHeader.setSerializable(false);
	autoSaveLoad_settings.setSerializable(false);
	bPLAY.setSerializable(false);
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

	COUNTER_step = 0;
	directionUp = true;
	timer_Range.stop();

	TARGET_Selected_PRE = -1;
	RANGE_Selected_PRE = -1;

	RANGE_Selected = 0;
	TARGET_Selected = ranges[RANGE_Selected].min; // set the target to the first target pos of the range

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
	if (ENABLED_MoodMachine && bPLAY)
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
	if (ENABLED_MoodMachine && !bPLAY)
	{
		bPLAY = true;
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::setTogglePlay()
{
	if (ENABLED_MoodMachine)
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
	////TODO: global gui enabler. not implemented..
	bGui = b;
	//gui.getVisible().set(bGui);

	//SHOW_GuiUser = b;

	// workflow
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
	// this function can be used to trig ranges jumps externally without using the internal timer.

	int _RANGE_Selected_PRE = RANGE_Selected.get();

	//-

	// 1. mode random

	//-

	// 2. mode back loop:

	// count times and cycle
	COUNTER_step++;
	COUNTER_step = COUNTER_step % countStayDuration;
	counterStepFromOne = COUNTER_step + 1;// for gui user

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
	if (bModeClockExternal && bExternalLocked)
		doRunStep();
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
	if (bforced || (bPLAY || (bModeClockExternal && bExternalLocked)))
	{
		// restart

		if (!bModeClockExternal) timer_Range.start(false);

		//-

		// modes

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

			// do randomize between min/max ranges

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

				// count times and cycle
				COUNTER_step++;
				COUNTER_step = COUNTER_step % countStayDuration;
				counterStepFromOne = COUNTER_step + 1;// for gui user

				// type A: mode stay amount of counter
				//if (COUNTER_step == 0)
				// type B: ignoring counter. just 1
				{
					markov.update();
					TARGET_Selected = markov.getState();

					//--

					// range not changed
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

		// workaround
		// Should check rect settings file instead..
		// Set Default layout
		doResetPreviewWidget();
		doResetManualSlider();
	}
}


//--------------------------------------------------------------
void ofxSurfingMoods::Changed_Params_Listeners(ofAbstractParameter &e)
{
	if (!BLOCK_CALLBACK_Feedback)
	{
		std::string name = e.getName();

		if (name != "COMPLETE")
			ofLogVerbose(__FUNCTION__) << name << " : " << e;

		if (name == bPLAY.getName())
		{
			if (bPLAY)
			{
				// 60,000 / bpmSpeed = MS
				timer = bpmLenghtBars * (BPM_BAR_RATIO * (60000 / bpmSpeed));
				timer_Range.start(false);
				bIsPlaying = true;

				//-

				COUNTER_step = 0;
				counterStepFromOne = COUNTER_step + 1; // for gui user

				// workflow
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
				counterStepFromOne = 0;
				RANGE_Selected = 0;
			}
		}

		else if (name == bModeClockExternal.getName())
		{

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

		//else if (name == "PRESET A" || name == "PRESET B" || name == "PRESET C")
		//{
		//	updateLabels();
		//}

		else if (name == bpmSpeed.getName())
		{
			// 60,000 / bpmSpeed = MS
			timer = bpmLenghtBars * (BPM_BAR_RATIO * (60000 / bpmSpeed));

			//TODO:
			//must solve jumps..
			//stop();
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

		//resets
		else if (name == bResetClockSettings.getName() && bResetClockSettings)
		{
			bResetClockSettings = false;
			resetClock();
			//workflow
			//stop();
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
			resetBank(false, true);//relative random
			stop();
		}
		else if (name == bRandomize_Bank.getName() && bRandomize_Bank)
		{
			bRandomize_Bank = false;
			resetBank(true, false);
			stop();
		}

		else if (name == bGui_PreviewWidget.getName())
		{
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

		// modes
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

		else if (name == countStayDuration.getName())
		{
			countStayDuration = ofClamp(countStayDuration, countStayDuration.getMin(), countStayDuration.getMax());

			counterStepFromOne.setMin(1);
			counterStepFromOne.setMax(countStayDuration.get());

			// avoid rescale timers error on preview

			//// workflow
			//bool bPre = isPlaying();
			//stop();
			//if (bPre)play();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingMoods::setup_ImGui()
{
	//guiManager.setImGuiAutodraw(bAutoDraw);
	//guiManager.setup();
	////guiManager.setup(gui);

	guiManager.setSettingsPathLabel("ofxSurfingMoods");
	guiManager.setup(IM_GUI_MODE_INSTANTIATED);

	//--

	//TODO:
	//fixing..
	static bool bCustom2 = true;
	if (bCustom2)
	{
		guiManager.clearStyles();
		guiManager.AddStyle(bPLAY, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, false, 1, 5);

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

		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(40, 200));
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

			std::string n = "Control";

			guiManager.beginWindow(n.c_str(), (bool*)&bGui_ManualSlider.get(), window_flags);
			{
				auto c = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
				ImVec4 _cBg = ImVec4(c.x, c.y, c.z, c.w * 0.2);
				ImGui::PushStyleColor(ImGuiCol_SliderGrab, cRange);
				ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, cRangeRaw);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, _cBg);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, _cBg);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, _cBg);
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
	if (bGui)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		//static bool bMinimize = false;

		//-

		guiManager.beginWindow(bGui.getName().c_str(), (bool*)&bGui.get(), window_flags);
		{
			guiManager.refreshLayout();

			static bool bOpen = false;
			ImGuiTreeNodeFlags _flagt;
			_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
			_flagt |= ImGuiTreeNodeFlags_Framed;

			// Colorize moods
			float a;
			ImVec4 c;
			if (RANGE_Selected == 0) c = color_MOOD1;
			else if (RANGE_Selected == 1) c = color_MOOD2;
			else if (RANGE_Selected == 2) c = color_MOOD3;

			if (bModeClockExternal) a = 1.0f;
			else a = ofMap(1 - timer_Range.getNormalizedProgress(), 0, 1, 0.25, 1, true);

			ImVec4 ca = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * a);
			ImVec4 ca2 = (ImVec4)ImColor::ImColor(c.x, c.y, c.z, c.w * (a * 0.1));//lower

			//-

			// Play / lock

			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ca);
			ImGui::PushStyleColor(ImGuiCol_Button, ca);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ca2);
			if (!bModeClockExternal) guiManager.Add(bPLAY, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
			else
			{
				if (guiManager.Add(bExternalLocked, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG))
				{
					//bExternalLocked = true;
				}
			}
			ImGui::PopStyleColor(3);

			//-

			ImGui::Spacing();

			ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bMinimize);

			//--

			// Clock
			if (guiManager.bMinimize) {
				guiManager.Add(bpmSpeed, SurfingImGuiTypes::OFX_IM_SLIDER);
			}
			else if (!guiManager.bMinimize)
			{
				bOpen = false;
				_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt |= ImGuiTreeNodeFlags_Framed;

				ofxImGuiSurfing::AddToggleRoundedButton(bModeClockExternal);

				if (ImGui::TreeNodeEx("CLOCK", _flagt))
				{
					guiManager.refreshLayout();

					float _w100 = ofxImGuiSurfing::getWidgetsWidth(1);
					float _w50 = ofxImGuiSurfing::getWidgetsWidth(2);
					float _h = 1 * ofxImGuiSurfing::getWidgetsHeightUnit();

					// Clock panel
					//guiManager.Add(bpmSpeed, SurfingImGuiTypes::OFX_IM_DRAG, false, 1, 0);
					guiManager.Add(bpmSpeed, SurfingImGuiTypes::OFX_IM_SLIDER, false, 1, 0);
					if (!bModeClockExternal) {
						guiManager.Add(bpmSpeed, SurfingImGuiTypes::OFX_IM_STEPPER);

						if (ImGui::Button("HALF", ImVec2(_w50, _h))) {
							bpmSpeed = bpmSpeed / 2.0f;
						}
						ImGui::SameLine();
						if (ImGui::Button("DOUBLE", ImVec2(_w50, _h))) {
							bpmSpeed = bpmSpeed * 2.0f;
						}

						guiManager.Add(bResetClockSettings, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL, false, 1, 0);
					}

					ImGui::TreePop();
				}
			}

			//--

			/*if (ImGui::TreeNodeEx("PANELS", _flagt))
			{
				guiManager.refreshLayout();

				guiManager.Add(bGui_Advanced, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
				//guiManager.Add(SHOW_Clocks, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);

				if (ImGui::TreeNodeEx("PREVIEW name", _flagt))
				{
					guiManager.refreshLayout();
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
				ImGui::Spacing();

				ImGui::TreePop();
			}*/

			if (!guiManager.bMinimize) {
				bool bOpen2 = false;
				ImGuiTreeNodeFlags _flagt2 = (bOpen2 ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt2 |= ImGuiTreeNodeFlags_Framed;
				if (ImGui::TreeNodeEx("MODES", _flagt2))
				{
					guiManager.refreshLayout();

					guiManager.Add(MODE_Ranged, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
					if (bMarkovFileFound) guiManager.Add(MODE_MarkovChain, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
					guiManager.Add(MODE_Manual, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
					ImGui::TreePop();

					//--

					if (!guiManager.bMinimize) {

						guiManager.Add(MODE_StartLocked, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, true, 2);
						guiManager.Add(MODE_AvoidRepeat, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 2);

					}
				}

				guiManager.refreshLayout();
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

			ImGui::Spacing();

			//--

			if (MODE_Manual) {
			}
			else if (!(bModeClockExternal && MODE_Manual)) {
				guiManager.Add(countStayDuration, SurfingImGuiTypes::OFX_IM_STEPPER); // user setter
			}
			else {
			}

			ImGui::Spacing();

			//-

			// for monitor only
			if (MODE_Manual) {
			}
			else if (!(bModeClockExternal && MODE_Manual)) {
				if (countStayDuration != 1) {
					guiManager.Add(counterStepFromOne, SurfingImGuiTypes::OFX_IM_INACTIVE);
				}
				else {
				}
			}
			else {
			}

			// progress
			if (MODE_Manual) {
			}
			else if (!(bModeClockExternal && MODE_Manual))
			{
				guiManager.Add(timer_Progress, SurfingImGuiTypes::OFX_IM_PROGRESS_BAR);
			}
			else {
			}

			//guiManager.Add(MOOD_Color_Preview, SurfingImGuiTypes::OFX_IM_DEFAULT);

			//----

			ImGui::PushStyleColor(ImGuiCol_Text, ca);

			string s;

			//s = ofToString(bpmSpeed.get(), 2) + " BPM";
			//ImGui::Text(s.c_str());

			if (MODE_Ranged) s = MODE_Ranged.getName();
			else if (MODE_MarkovChain) s = MODE_MarkovChain.getName();
			else if (MODE_Manual) s = MODE_Manual.getName();
			ImGui::Text(s.c_str());

			//ImGui::Text("STATES:RANGE");
			//ImGui::Spacing();

			guiManager.Add(RANGE_Selected, SurfingImGuiTypes::OFX_IM_INACTIVE);

			ImGui::Spacing();

			guiManager.Add(TARGET_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT, false, 1, 4);

			ImGui::PopStyleColor();

			//-

			if (!guiManager.bMinimize) {

				ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "TARGET > PRESETS");
				ImGui::Spacing();

				guiManager.Add(PRESET_A_Enable, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
				guiManager.Add(PRESET_B_Enable, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
				guiManager.Add(PRESET_C_Enable, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1, 4);
			}

			if (PRESET_A_Enable) guiManager.Add(PRESET_A_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT);
			if (PRESET_B_Enable) guiManager.Add(PRESET_B_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT);
			if (PRESET_C_Enable) guiManager.Add(PRESET_C_Selected, SurfingImGuiTypes::OFX_IM_DEFAULT, false, 1, 4);

			//--

			if (!guiManager.bMinimize) {

				ImGui::Spacing();

				ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bExtra);
				if (guiManager.bExtra)
				{
					ImGui::Indent();

					{
						ofxImGuiSurfing::AddToggleRoundedButton(bGui_Advanced);
						if (MODE_Manual)
							ofxImGuiSurfing::AddToggleRoundedButton(bModeAutomatic);

						ImGui::Separator();

						ofxImGuiSurfing::AddToggleRoundedButton(bGui_PreviewWidget);
						if (bGui_PreviewWidget)
						{
							ImGui::Indent();
							{
								ofxImGuiSurfing::AddToggleRoundedButton(bUseCustomPreviewPosition);
								if (bUseCustomPreviewPosition) {
									ofxImGuiSurfing::AddToggleRoundedButton(bEdit_PreviewWidget);
									if (ofxImGuiSurfing::AddToggleRoundedButton(bResetPreviewWidget)) {
										bResetPreviewWidget = false;
										doResetPreviewWidget();
									}
								}
							}
							ImGui::Unindent();
						}

						if (MODE_Manual) {
							ImGui::Separator();
							ofxImGuiSurfing::AddToggleRoundedButton(bGui_ManualSlider);
							if (bGui_ManualSlider)
							{
								ImGui::Indent();
								{
									ofxImGuiSurfing::AddToggleRoundedButton(bGui_ManualSliderHeader);
									//if (bGui_ManualSliderHeader) 
									{
										if (ofxImGuiSurfing::AddToggleRoundedButton(bResetSlider))
										{
											bResetSlider = true;
										}
									}
								}
								ImGui::Unindent();
							}
						}

						ImGui::Separator();

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

	// Advanced
	{
		if (bGui_Advanced)
		{
			// Panels sizes
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
					//guiManager.refreshLayout();
					//guiManager.Add(SHOW_GuiUser, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1);

					//--

					// Tools
					{
						bOpen = false;
						_flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
						_flagt |= ImGuiTreeNodeFlags_Framed;

						if (ImGui::TreeNodeEx("TOOLS", _flagt))
						{
							guiManager.refreshLayout();

							// Target panel
							guiManager.Add(bClone_TARGETS, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
							guiManager.Add(bResetSort_Bank, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
							guiManager.Add(bReset_Bank, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
							guiManager.Add(bRandomize_Bank, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1);

							ImGui::TreePop();
						}
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

							ofxImGuiSurfing::AddGroup(parameters_ranges, flags);
							//ofxImGuiSurfing::AddGroup(params_STORE, flags);
							//ofxImGuiSurfing::AddGroup(params_Listeners, flags);

							ImGui::TreePop();
						}
					}

					//--

					//guiManager.refreshLayout();
					//ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bAdvanced);
					//guiManager.drawAdvancedSubPanel();
				}
				guiManager.endWindow();
			}
			ImGui::PopStyleVar();
		}
	}
}