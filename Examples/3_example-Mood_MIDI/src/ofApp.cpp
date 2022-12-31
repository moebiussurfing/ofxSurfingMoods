#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetCircleResolution(100);

	gradient.addColor(ofColor::red);
	gradient.addColor(ofColor::yellow);
	gradient.addColor(ofColor::green);

	//--

	setupMoods();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Circle will be filled with gradient colors to visualize Preset A index
	ofPushStyle();
	ofSetColor(colorCircle);
	ofFill();
	float r = ofGetHeight() * 0.2;
	float x = ofGetWidth() * 0.5;
	float y = ofGetHeight() * 0.5;
	ofDrawCircle(x, y, r);
	ofSetLineWidth(2.0);
	ofNoFill();
	ofSetColor(64, 200);
	ofDrawCircle(x, y, r);
	ofPopStyle();

	//--

	// Gui

	//--

	ui.Begin();

	ImVec2 size_min = ImVec2(500, -1);
	ImVec2 size_max = ImVec2(600, -1);
	ImGui::SetNextWindowSizeConstraints(size_min, size_max);

	if (ui.BeginWindow("ofApp"))
	{
		//if (ui.BeginTree("LOCAL", true, false))
		{
			//ui.Add(moods.TARGET_Selected);
			//ui.AddSpacing();
			//ui.Add(moods.PRESET_A_Selected);
			//ui.Add(moods.PRESET_B_Selected);
			//ui.Add(moods.PRESET_C_Selected);

			float sz = 0.25f;
			SurfingGuiTypes t = OFX_IM_VSLIDER;
			ImGui::Columns(4, "##s4", false);
			ui.AddSpacing();
			ui.Add(moods.TARGET_Selected, t, 4, sz);
			ImGui::NextColumn();
			ui.AddSpacing();
			ui.Add(moods.PRESET_A_Selected, t, 4, sz);
			ImGui::NextColumn();
			ui.AddSpacing();
			ui.Add(moods.PRESET_B_Selected, t, 4, sz);
			ImGui::NextColumn();
			ui.AddSpacing();
			ui.Add(moods.PRESET_C_Selected, t, 4, sz);
			ImGui::Columns(1);

			//ui.EndTree(false);
		}

		ui.AddSpacingSeparated();

		ImGui::Columns(4, "##t4", false);
		ui.AddGroup(moods.getTogglesTarget());
		ImGui::NextColumn();
		ui.AddGroup(moods.getTogglesPresetA());
		ImGui::NextColumn();
		ui.AddGroup(moods.getTogglesPresetB());
		ImGui::NextColumn();
		ui.AddGroup(moods.getTogglesPresetC());
		ImGui::Columns(1);

		// styles
		{
			static bool bDone = false;
			if (!bDone)
			{
				bDone = true;
				//ui.ClearStyles();

				SurfingGuiTypes t = OFX_IM_TOGGLE_BIG_XXL_BORDER_BLINK;

				auto& g1 = moods.getTogglesTarget();
				ui.AddStyleGroupForBools(g1, t);

				auto& g2 = moods.getTogglesPresetA();
				ui.AddStyleGroupForBools(g2, t);

				auto& g3 = moods.getTogglesPresetB();
				ui.AddStyleGroupForBools(g3, t);

				auto& g4 = moods.getTogglesPresetC();
				ui.AddStyleGroupForBools(g4, t);
			}
		}

		ui.EndWindow();
	};

	ui.End();

	//-

	moods.drawGui();

	mMidiParams.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == OF_KEY_F1) { bGui = !bGui; }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
	moods.windowResized(w, h);
}

//--------------------------------------------------------------
void ofApp::exit()
{
	exitMoods();
}


//----


//--------------------------------------------------------------
// 
// Full Snippet to copy / paste to your project ->
//
// Surfing Moods
//
// setupMoods();
// exitMoods();

//--------------------------------------------------------------
void ofApp::setupMoods()
{
	// Callbacks listeners from Moods
	moods.RANGE_Selected.addListener(this, &ofApp::Changed_Mood_RANGE);
	moods.TARGET_Selected.addListener(this, &ofApp::Changed_Mood_TARGET);

	moods.PRESET_A_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_A);
	moods.PRESET_B_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_B);
	moods.PRESET_C_Selected.addListener(this, &ofApp::Changed_Mood_PRESET_C);

	//--

	moods.setup();

	// Default structure is:
	// 3 Moods aka Ranges (with 3 targets each)
	// 9 States aka Targets.
	// 9 Presets for each preset receiver A-B-C.
	// Splitting the 3 Moods:
	// Limit 0-1 target 3, limit 1-2 target 6.

	//--

	mMidiParams.connect();

	// -> Add selector indexes
	mMidiParams.add(moods.TARGET_Selected);
	mMidiParams.add(moods.PRESET_A_Selected);
	mMidiParams.add(moods.PRESET_B_Selected);
	mMidiParams.add(moods.PRESET_C_Selected);

	// -> Add groups
	mMidiParams.add(moods.getTogglesTarget());
	mMidiParams.add(moods.getTogglesPresetA());
	mMidiParams.add(moods.getTogglesPresetB());
	mMidiParams.add(moods.getTogglesPresetC());

	mMidiParams.setVisible(true);
}

//--------------------------------------------------------------

// Callbacks to listen inside the moods
// We can apply current targets and presets to whatever we want into our OF_APP.
// We can modify other things depending on mood range too.

//--------------------------------------------------------------
void ofApp::Changed_Mood_TARGET(int& index)
{
	ofLogNotice(__FUNCTION__) << index;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_A(int& index)
{
	ofLogNotice(__FUNCTION__) << index;

	// Change the circle color reflecting the Preset A index
	float val = ofMap(index, 0, 8, 0.f, 1.f);
	colorCircle = gradient.getColorAtPercent(val);
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_B(int& index)
{
	ofLogNotice(__FUNCTION__) << index;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_PRESET_C(int& index)
{
	ofLogNotice(__FUNCTION__) << index;
}

//--------------------------------------------------------------
void ofApp::Changed_Mood_RANGE(int& index)
{
	ofLogNotice(__FUNCTION__) << index;

	// Change the background color reflecting the Mood/Range

	if (index == 0)
	{
		ofBackground(ofColor::indianRed);
	}
	else if (index == 1)
	{
		ofBackground(ofColor::yellow);
	}
	else if (index == 2)
	{
		ofBackground(ofColor::lightGreen);
	}
}

//--------------------------------------------------------------
void ofApp::exitMoods()
{
	// Callbacks listeners from Moods

	moods.RANGE_Selected.removeListener(this, &ofApp::Changed_Mood_RANGE);

	moods.TARGET_Selected.removeListener(this, &ofApp::Changed_Mood_TARGET);

	moods.PRESET_A_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_A);
	moods.PRESET_B_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_B);
	moods.PRESET_C_Selected.removeListener(this, &ofApp::Changed_Mood_PRESET_C);
}
