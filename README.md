# Wristmate

![Wristmate 3D](https://github.com/Ponti17/wristmate/blob/master/wristmateDocuments/wristmate_3d.PNG)

Wristmate is a complete embedded smartwatch solution developed as a university project.

In response to the complexity of modern smartwatches, which are now so powerful that they are comparable to 5 year old smartphones, we recognized the need for a sripped down, streamlined solution. 

Many people find themselves resorting to their smartphone anyway for anything beoynd basic tasks like checking daily step count or reading simple push notifications, rendering the advanced features of modern smartwatches redundant. 

Wristmate was conceived as a "smartwatch lite". An experience that prioritizes only core, essential functionality, with an increased focus on battery life and usability. 

## PlatformIO
The development of Wristmate was carried out using the PlatformIO extension in Visual Studio Code (VSCode). This repository contains the PlatformIO directory of the project with an additional folder of supplementary files.

The source code files, including the main.cpp files, can be found in the /src/ directory.

Header files are located in the /include/ directory.

## wristmateLib
A custom library was written for wristmate. Core functionality such as initialization, WiFi, animations and the drawing of menus are all handled through a wristmate object. 

## bitmaps
For displaying more intricate graphics, the Wristmate software employs the use of simple bitmaps (display is mono). Bitmaps are declared in bitmaps.h and defined in bitmaps.cpp. Adding additional bitmaps can be easily done with the help of [this page](https://javl.github.io/image2cpp/).

## Schematic
The schematic diagram of Wristmate can be found in /wristmateDocuments/.