ofxSurfingMoods
=============================

# Overview
**ofxSurfingMoods** is an **openFrameworks** addon that works as my personal **Mood-Machine** to *walk'n'randomize* around some *Moods and States*.  

There is **three Moods** that we call **Ranges** too.  
Every **State** is named **Target**, and every **Target** can be linked to three **Presets** (A-B-C) receivers.  

Each **Preset receiver (A-B-C)** has 9 available **Presets/indexes**.  
We will get this index changes using **callbacks** in our **ofApp**, to **apply the States to our Scene**.  
(Linking them to some *Parameters/States/Preset Selections/Changing Colors/Triggering Sounds...etc*)

There is an internal **BPM Clock** and customizable timers and counters, to configure the global speeds or how much time we want to stay walking into the *Mood States*.  

## Screenshots
![image](/readme_images/Capture1.PNG?raw=true "image")  

**VIDEO/GIF**:  
https://imgur.com/gallery/Li0cT6C

## Features
- Three different **Modes**:
1. **Mode Ranged**:  
Looped walking through the **Moods/Ranges** : **123** **321** **123** ..., doing randomize to select a **Target/State** around the contained Targets into the **Mood/Range**.
2. **Mode Markov Chain**:  
Editable matrix to distribute graph probabilities. Customizable file '*transitionMatrix.txt*' with your own text editor:  
![image](/readme_images/MarkovMatrix.PNG?raw=true "image")  
3. **Mode Manual Control**:  
Available float to force select a **Mood/Range**, and to *walk'n'randomize* **States/Targets** locked inside the **Mood/Range**.

- Avoid repeat the same Target/State on the next step State, in case randomize gets the same previous Target/State.
- Force to *Start-from-First* Target/State of the Mood/Range when arriving to one of the three Moods.  
(Then if you are triggering a kind of music patterns, you can create a transitional pattern to use when arriving at a Mood).
- Colored Preview panel to display the workflow during runtime. Also, customizable layout using the mouse.

## Usage
 - Look into **OF_ADDON/example_Basic**.

## Dependencies
- **ofxGuiExtended2** (https://github.com/moebiussurfing/ofxGuiExtended2)  

Already included in *OF_ADDON/libs*. No need to add manually:
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