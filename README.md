ofxSurfingMoods
=============================

# Overview
**ofxSurfingMoods** is an **openFrameworks** addon that works as my personal Mood-Machine to randomize-walk around some states.  
Every state is called Target, and every Target can be linked to 3 presets (ABC). Also, there's 3 Mood States called Ranges.  
There are an internal Bpm clock and customizable timers to define the global speed. 

## Screenshot
![image](/readme_images/Capture.gif?raw=true "image")
![image](/readme_images/Capture1.PNG?raw=true "image")

## Features
- Avoid repeat the same state on the next step mode.
- Force to start from first state of the Range Mood when arriving.
- Three different modes:
1. Range Mode
2. Manual Control with a float to force select a Range, and to walk locked inside.
3. Markov chain. Editable matrix to distribute graph probabilities.

## Usage
 - Look **example_Basic**.

## Dependencies
Already included in *OF_ADDON/libs*. No need to add manually.
- **ofxMarkovChain**
- **ofxSimpleTimer**

## Tested systems
- **Windows10** / **VS2017** / **OF ~0.11**
- **macOS High Sierra** / **Xcode 9/10** / **OF ~0.11**

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*

## License
*MIT License.*