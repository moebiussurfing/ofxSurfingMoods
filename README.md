# Overview
An **openFrameworks** add-on that works as my personal **MOOD-MACHINE**.  
To *Walk'n'Randomize* around some *Moods and States*.  

1. There are **Three Moods** that we also name as **Ranges**. (Red-Yellow-Green)  

2. Each Range has Three States. Each **State** can be named also **Target**.  

3. Every **Target** can be linked to three **Presets Receivers (A-B-C)**.  

4. Each **Preset receiver (A-B-C)** has 9 available destination **Presets/Indexes**.  

We will get this index changes using **callbacks** in our **ofApp**, to **apply the States to our Scene**,  
linking them to change some *Parameters/States/Scenes/Preset Selections/Colors/Triggering Video/Sounds...etc*  

There is an internal **BPM Clock** and customizable timers and counters to configure the global speeds or how much time we want to stay or walking around into the *Mood States*.  

## Screenshots
![image](/readme_images/Capture1.PNG?raw=true "image")  

## Features

### THREE DIFFERENT **MODES**

1. **MODE RANGED**:  
Looped walking through the **Moods/Ranges** : **123** **321** **123** ..., doing randomize to select a **Target/State** around the contained Targets into the **Mood/Range**.

2. **MODE MARKOV CHAIN**:  
![image](/readme_images/MarcovGraph.png?raw=true "image")  
Editable matrix to distribute graph probabilities. Customizable file '*transitionMatrix.txt*' with your own text editor. (Each row represents each State, and the odds (%) of jumping to the other States, or "to rest into itself". All row values must add up to 1.0 (100%) to be stochastic.):  
![image](/readme_images/MarkovMatrix.PNG?raw=true "image")  

3. **MODE MANUAL CONTROL**:  
Available float to force select a **Mood/Range**, and to *walk'n'randomize* **States/Targets** locked inside the **Mood/Range**.

#### MORE FEATURES
- Optionally avoid repeating the same Target/State on the next step State, in case randomize gets the same previous Target/State.
- Optionally Force to *Start-from-First* Target/State of the Mood/Range when arriving in one of the three Moods.  
(Then, for example, if you are triggering a kind of music patterns, you can create a transitional pattern to use when arriving at a Mood).
- Colored Preview panel to display the workflow during runtime. Also, customizable layout using the mouse.

## Usage
 - Look into **OF_ADDON/examples/**.

## Dependencies
Add to **PROJECT GENERATOR**:  
* [ofxSurfingHelpers](https://github.com/moebiussurfing/ofxSurfingHelpers)  
* [ofxSurfingImGui](https://github.com/moebiussurfing/ofxSurfingImGui)
* [ofxImGui](https://github.com/Daandelange/ofxImGui/tree/ofParameters-Helpers-Test) / _Fork_ from @**Daandelange**  

Already included in *OF_ADDON/libs* or into other add-ons. No need to add them manually:
- [ofxMarkovChain](https://github.com/elaye/ofxMarkovChain)
- [ofxSimpleTimer](https://github.com/HeliosInteractive/ofxSimpleTimer)
* [ofxScaleDragRect](https://github.com/moebiussurfing/ofxScaleDragRect) / _Fork_  
* [ofxMSAInteractiveObject](https://github.com/moebiussurfing/ofxMSAInteractiveObject) / _Fork_  

## Tested Systems
* **Windows 10** / **VS 2022** / **OF ~0.12**

## Author
An addon by **@moebiusSurfing**  
*( ManuMolina ) 2022*  

## License
[**MIT License**](https://github.com/LICENSE)