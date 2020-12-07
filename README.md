ofxSurfingMoods
=============================

# Overview
**ofxSurfingMoods** is an **openFrameworks** addon that works as my personal **Mood-Machine** to *walk'n'randomize* around some *Moods and States*.  
There is three *Moods* that we call *Ranges* too.  
Every *State* is named *Target*, and every *Target* can be linked to three *Presets* (A-B-C) receivers.  
Each *Preset* receiver have 9 possible indexes/presets. We will get this index by callbacks in our *ofApp*, to **apply the States to our Scene**.
There is an internal **BPM Clock** and customizable timers and counters, to configure the global speeds or how many time we want to stay walking into a *Mood*.  
In your *ofApp*, you can receive the callbacks when *Presets* (A-B-C) changed, linking them to any *parameter/States/Preset Selections...etc* of your *Scene*.

## Screenshots
![image](/readme_images/Capture1.PNG?raw=true "image")  

**GIF**:  
https://imgur.com/a/j5e9zwa
<img src="https://imgur.com/a/j5e9zwa.gif"/>  
!(image)[https://imgur.com/a/j5e9zwa.gif]  
<blockquote class="imgur-embed-pub" lang="en" data-id="a/j5e9zwa"  ><a href="//imgur.com/a/j5e9zwa">ofxSurfingMoods</a></blockquote><script async src="//s.imgur.com/min/embed.js" charset="utf-8"></script>

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