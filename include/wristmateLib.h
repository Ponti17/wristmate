/*
    * wristmateLib.h
    * Created by Andreas Pedersen, 19/05/23.
*/

#include <Arduino.h>

#ifndef WRISTMATE_H
#define WRISTMATE_H

class Wristmate
{
    public:
        void begin();
        void menu1();
        void menu2();
        void menu3();
    private:
        String _openWeatherApiKey;
        const char* _ssid;
        const char* _password;
};

#endif