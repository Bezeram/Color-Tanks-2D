# Description

2D Muliplayer tank game played on one keyboard!

# How to Run (Windows)

#### Prerequisites
Install [Cmake](https://cmake.org/download/)  
Install [Visual Studio](https://visualstudio.microsoft.com/downloads/)

1. Clone or download the zip
2. Run the batch file in the build/ folder
3. Open the .sln file and press F5 to run or press the Local Windows Debugger Button

# Rules
If a tank takes 3 hits or falls off the map, it gets destroyed.

## Features

- Terrain Editor
- Tanks slide off hills and their speed is dependent on the slope
- Terrain automatically levels out

# Controls

#### Left Tank (Red)
- A and D to move
- W and S to adjust your aim
- Space to shoot

#### Right Tank (Blue)
- Left and Right to move
- Up and Down to adjust your aim
- Enter to shoot

# Terrain Editor

Press P to enter terrain editor mode!  
To go back, press Space and Enter at the same time.  
The terrain is made up of sine functions, divided into bumps and hills. Bumps have a lower amplitude than Hills, however the former
has a higher frequency than Hills. These are just the default and can
be adjusted any which way, or even reversed.  
During this, the tanks cannot be moved and no longer interact with each other.  
Holding shift using any of the following keys adds negative values instead of positive ones, with the exception of the resolution where 
only the rate of change is affected.  
- G = regenerate random terrain
- Up = increase terrain resolution
- Down = decrease terrain resolution
- Left = move terrain function start point right (or left with shift)
- Right = move terrain function start point right (or left with shift)
- F = move terrain floor up/down
- H = adjust terrain scalar
- Z = increase / decrease amplitude of Bumps
- X = increase / decrease amplitude of Hills
- C = increase / decrease frequency of Bumps
- V = increase / decrease frequency of Hills
- N = increase / decrease max number of Bumps sine functions
- V = increase / decrease max number of Hills sine functions
- M = increase / decrease generation randomness

![screenshot_gameplay](readme/preview_tanks.gif)
