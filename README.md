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
- simple window style for specifying that a window is "nested" rather than being a separate native desktop window;
- "text_edit" widget supports multiple fonts, text colours (with optional gradient effects), colour emojis and rendering text in columns;
- GPU shader rendered CSS3 compliant gradients with optional gaussian smoothing;
- MVC (model-view-controller) related classes supporting robust and fast app design;
- optional MDI support utilizing "nested" windows;
- 2D (initially) game support: sprites and 2D physics;
-flexible asset management: texture images (e.g PNGs) can be stored in .zip archives which can be optionally embedded inside the program binary accessible using simple URLs;
- support for OpenGL, DirectX and Vulkan.


# Roadmap

Version | ETA           | Features
--------|---------------|---------------------------------------------------
v0.99   | Spring 2018   | Windows OpenGL; widget library feature complete; GUI designer tool
v1.00   | Summer 2018   | Linux OpenGL; Vulkan; scripting engine
v1.10   | Winter 2018   | Android support; physics engine
v1.20   | Summer 2019   | macOS support

# Implementation Progress

![](http://neogfx.org/temp/bugfix.png)
![](http://neogfx.org/temp/emoji.png)
![](http://neogfx.org/temp/colour_picker_finished.png)

Feature                   | % done  |     TODO
--------------------------|---------|-------------------------------------------------------------------------------
Window                    | 95      |     Drop shadow
Dialog                    | 90      |     Default button
Message Box               | 100     |
Menu                      | 100     | 
Nested windows            | 25      |     
MDI                       | 0       |
Tabs                      | 90      |     Tabs below and on the side
Toolbars                  | 90      |     Drop-down button support
Layout Managers           | 95      |     Need to fully support size policies
Label                     | 100     |     
Button                    | 100     |     
Table View                | 85      |     Selection; icon
Tree View                 | 0       |
List View                 | 50      |
Text Edit                 | 85      |     Columns; rich text (HTML); undo/redo
Line Edit                 | 100     |     
Spin Box                  | 100     |
Slider                    | 90      |     Keyboard
Check Box                 | 100     |     
Radio Button              | 100     |
Combo Box                 | 75      |
Group Box                 | 100     |
Tool Tips                 | 0       |
Colour Gradient Selector  | 100     |
Colour Picker             | 100     |
File Browsers (Native)    | 0       |
Drag and drop             | 10      |
XML Resource Description  | 0       |
i18n                      | 0       |
l10n                      | 0       |     UK English, US English, French, German, Chinese
Skins                     | 0       |

Last updated: 02/02/2018
