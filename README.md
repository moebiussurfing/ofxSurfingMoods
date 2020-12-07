ofxSurfingMoods
=============================

# Overview
**ofxSurfingMoods** is an **openFrameworks** addon that works as my personal **Mood-Machine** to randomize'n'walk around some *Moods/States*.  
Every State is named *Target*, and every Target can be linked to three *Presets* (A-B-C).  
Also, there's three *Moods/States* called *Ranges*.  
There is an internal **BPM Clock** and customizable timers, and counters to configure the global speeds.  
In your *ofApp*, you can receive the callbacks when *Presets* (A-B-C) changed, linking them to any *parameter/States/Preset Selections...etc* of your *Scene*.

## Screenshots
![image](/readme_images/Capture.gif?raw=true "image")
![image](/readme_images/Capture1.PNG?raw=true "image")

## Features
- Three different **Modes**:
1. **Mode Range**: looped walking from **Moods/Ranges** **1 > 2 > 3** and **3 > 2 > 1**, doing randomize to select around the contained Targets into the **Mood/Range**.
2. **Mode Manual Control**: Available float to force select a **Mood/Range**, and to walk locked inside.
3. **Mode Markov Chain**: **Editable matrix** to distribute graph probabilities. Customizable file '*transitionMatrix.txt*' with your own text editor. 
- Avoid repeat the same *Target/State* on the next step State, in case randomize gets the same previous **Target**.
- Force to *Start-from-First* **Target/State** of the **Mood Range** when arriving to one of the three **Moods**. (Then if you are triggering a kind of music patterns, you can create a transitional pattern to use when arriving to a Mood).
- Colored Preview panel to display the runtime. Also cusomizable layout using the mouse.

## Usage
 - Look int **/example_Basic**.

## Dependencies
Already included in *OF_ADDON/libs*. No need to add manually.
- **ofxMarkovChain** (https://github.com/elaye/ofxMarkovChain)
- **ofxSimpleTimer** (https://github.com/HeliosInteractive/ofxSimpleTimer)
- **ofxScaleDragRect** (https://github.com/roymacdonald/ofxScaleDragRect)

## Tested systems
- **Windows10** / **VS2017** / **OF ~0.11**
- **macOS High Sierra** / **Xcode 9/10** / **OF ~0.11**

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*

## License
*MIT License.*