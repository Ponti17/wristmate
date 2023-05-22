/*
    * wristmateLib.h
    * Header file for wristmateLib.cpp
    * Creade by Andreas Pedersen.
*/

#include <Arduino.h>

#ifndef WRISTMATE_H
#define WRISTMATE_H

class Wristmate
{
    public:
        Wristmate(const char* ssid, const char* password, const char* openWeatherApiKey);
        void begin();
        void getData(bool on_boot);
        void menu1();
        void menu2();
        void menu3();
        void animationThumbsUp();
        bool hasTime;
        bool hasWeather;
    private:
        Adafruit_SharpMem display;
        Adafruit_BMP280 bmp;
        BMA400 accelerometer;
        const char* _openWeatherApiKey;
        const char* _ssid;
        const char* _password;
        const int BLACK = 0;
        const int WHITE = 1;
        String weather_temp;
        String weather_icon;
};

#endif