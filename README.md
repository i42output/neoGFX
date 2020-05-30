![](https://raw.githubusercontent.com/FlibbleMr/neogfx/master/neoGFX.png)
![](https://raw.githubusercontent.com/FlibbleMr/neogfx/master/underconstruction.png)

# Introduction
neoGFX is a C++ app/game engine and development platform targeted at app and game developers that wish to leverage modern GPUs for performant application user interfaces and game graphics.
neoGFX is still a work in progress (incomplete) so is not yet suitable for making apps or games. Currently neogfx has only been built for and tested on Windows.

# Features (when version 1.0 released)
- clean modern C++ design including full exception safety supporting various widget allocation patterns (stack, member variable or free store);
- use of standard non-proprietary C++ data types including using standard string classes (UTF-8 encoding) and containers from the C++ standard library;
- simple, easy to use multi-threaded event system (an improvement over traditional signals and slots);
- no baggage: neoGFX primarily contains only the GUI and graphics related functionality needed for creating apps and games;
- full library of widgets and layout managers specifiable in an RJSON (Relaxed JSON) derived declarative UI description language drag/drop editable using the included design tool, neoGFX Design Studio;
- CSS3 style sheet support with support for custom skins;
- scripting engine, neos, that will be able to support a wide range of scripting languages (on release: neoscript, JavaScript and Lua);
- sub-pixel text rendering;
- simple window style for specifying that a window is "nested" rather than being a separate native desktop window;
- "text_edit" widget supports multiple fonts, text colours (with optional gradient effects), colour emojis and rendering text in columns;
- "Green" and "Arcade" modes to optimize CPU power consumption, fight #ClimateChange!
- GPU shader rendered CSS3 compliant gradients with optional gaussian smoothing;
- MVC (model-view-controller) related classes supporting robust and fast app design;
- optional MDI support utilizing "nested" windows;
- 2D, 2.5D and 3D game support: sprites, objects and physics;
- Pure ECS (Entity-component-system) usable by both games and apps;
- flexible asset management: texture images (e.g PNGs) can be stored in .zip archives which can be optionally embedded inside the program binary accessible using simple URLs;
- support for OpenGL, DirectX and Vulkan.


# Roadmap

Version | ETA           | Features
--------|---------------|---------------------------------------------------
v0.99   | Q3, 2021      | Windows OpenGL; widget library feature complete; UI design tool
v1.00   | Q4, 2021      | Linux OpenGL; scripting engine
v1.10   | Q2, 2022      | Vulkan; Android support; physics engine
v1.20   | Q4, 2022      | macOS support

# Implementation Progress

![](http://neogfx.org/temp/lol.png?id=1)
![](http://neogfx.org/temp/glyphgradients1.png?id=2)
![](http://neogfx.org/temp/glyphgradients2.png?id=1)
![](http://neogfx.org/temp/github1.png?id=1)
![](http://neogfx.org/temp/github2.png?id=1)
![](http://neogfx.org/temp/github3.png?id=1)



Feature                         | % done  |     TODO
--------------------------------|---------|-------------------------------------------------------------------------------
Window                          | 95      |     Drop shadow
Dialog                          | 90      |     Default button
Message Box                     | 100     |
Menu                            | 100     | 
Nested windows                  | 25      |     
MDI                             | 0       |
Tabs                            | 95      |     Vertical tabs
Toolbars                        | 90      |     Drop-down button support
Layout Managers                 | 95      |     Need to fully support size policies
Label                           | 100     |     
Button                          | 100     |     
Table View                      | 95      |     Selection; cell widget
Tree View                       | 95      |
List View                       | 95      |
Text Edit                       | 85      |     Columns; rich text (HTML); undo/redo; performance optimization
Line Edit                       | 100     |     
Spin Box                        | 100     |
Slider                          | 90      |     Keyboard
Check Box                       | 100     |     
Radio Button                    | 100     |
Combo Box                       | 95      |     Glob and regex filters; review UX.
Group Box                       | 100     |
Tool Tips                       | 0       |
Status Bar                      | 100     |
Progress Bar                    | 0       |
Docks							| 10      |
Font Picker                     | 100     |
Colour Gradient Selector        | 95      |     Swatch library
Colour Picker                   | 95      |     Eyedropper tool
Date/Time                       | 0       |
File Browsers (Native)          | 99      |
Drag and drop                   | 10      |
UI/Resource Description (RJSON) | 40      |
tool: neoGFX Design Studio      | 0       |
i18n                            | 40      |
l10n                            | 0       |     UK English, US English, French, German, Chinese
HID: Game Controllers           | 80      |     Calibration/settings UI; DirectInput button mapping
Scene Graph						| 0		  |		
Skins                           | 40      |

Last updated: 18/04/2020
