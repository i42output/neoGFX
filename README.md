![](https://raw.githubusercontent.com/FlibbleMr/neogfx/master/neoGFX.png)

# Introduction
neoGFX is a cross-platform C++ GPU-based GUI/game library.
neoGFX is still a work in progress (incomplete) so is not yet suitable for making GUI apps or games. Currently neogfx has only been built for and tested on Windows.

# Features (at version 1.0 release)
- clean modern C++ design including full exception safety supporting various widget allocation patterns (stack, member variable or free store);
- use of standard non-proprietary C++ data types including using standard string classes (UTF-8 encoding) and containers from the C++ standard library;
- no baggage: neoGFX primarily contains only the GUI and graphics related functionality needed for creating apps and games;
- full library of widgets and layout managers specifiable in XML using the included GUI designer tool;
- CSS3 style sheet support with support for custom skins;
- scripting engine (Lua);
- sub-pixel text rendering;
- "text_edit" widget supports multiple fonts, text colours (with optional gradient effects), colour emojis and rendering text in columns;
- GPU shader rendered CSS3 compliant gradients with optional gaussian smoothing;
- MVC (model-view-controller) related classes supporting robust and fast app design;
- 2D (initially) game support: sprites and 2D physics;
-flexible asset management: texture images (e.g PNGs) can be stored in .zip archives which can be optionally embedded inside the program binary accessible using simple URLs;
- support for OpenGL, DirectX and Vulkan.


# Roadmap

Version | ETA           | Features
--------|---------------|---------------------------------------------------
v0.99   | Summer 2017   | Windows OpenGL; widget library feature complete; GUI designer tool
v1.00   | Winter 2017   | Linux OpenGL; Vulkan; scripting engine
v1.10   | Spring 2018   | Android support; physics engine
v1.20   | Summer 2018   | macOS support

# Implementation Progress

![](http://neogfx.org/temp/bugfix.png)
![](http://neogfx.org/temp/emoji.png)
![](http://neogfx.org/temp/colour_picker_finished.png)

Feature                   | % done  |     TODO
--------------------------|---------|-------------------------------------------------------------------------------
Window                    | 90      |     Icon; drop shadow
Dialog                    | 90      |     Default button
Menu                      | 100     | 
MDI                       | 0       |
Tabs                      | 100     |     
Toolbars                  | 90      |     Drop-down button support
Layout Managers           | 95      |     Need to fully support size policies
Label                     | 100     |     
Button                    | 100     |     
Table View                | 85      |     Selection; icon
Tree View                 | 0       |
List View                 | 0       |
Text Edit                 | 85      |     Columns; inline images and emoji; undo/redo
Line Edit                 | 100     |     
Spin Box                  | 100     |
Slider                    | 90      |     Keyboard
Check Box                 | 100     |     
Radio Button              | 100     |
Combo Box                 | 0       |
Group Box                 | 100     |
Colour Gradient Selector  | 100     |
Colour Picker             | 100     |
File Browsers (Native)    | 0       |
XML Resource Description  | 0       |
Skins                     | 0       |

Last updated: 25/05/2017
