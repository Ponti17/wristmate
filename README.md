# Wristmate

Software for wristmate. 

![Wristmate 3D](https://github.com/Ponti17/wristmate/blob/master/wristmateDocuments/wristmate_3d.PNG)

## PlatformIO
---
The development of wristmate was done in VSCode PlatformIO. This repository contains the platformio directory of the project. 

.cpp files (including main.cpp) are located in /src/. 

Header files are located in /include/.

## wristmateLib
---
/src/wristmateLib.cpp

/include/wristmateLib.h

A custom library was written to initialize wristmate. Core functionality such as initialization, WiFi, animations and the drawing of menus are all handled through a wristmate object. 

## bitmaps
---
/src/bitmaps.cpp

/include/bitmaps.h


To display more complex graphics bitmaps are used. All bitmaps are declared in bitmaps.h and defined in bitmaps.cpp. More bitmaps are easily added using [this page](https://javl.github.io/image2cpp/).

## Schematic
---
The schematic can be found in /wristmateDocuments/.