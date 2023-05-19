/*
	* File: bitmaps.h
	* Contains bitmaps for all images.
	* Andreas Pedersen
*/

#include <Arduino.h>
#ifndef BITMAPS_H
#define BITMAPS_H

// '1', 240x240px
extern const unsigned char vaultboy_thumbsup1 [] PROGMEM;

// '2', 240x240px
extern const unsigned char vaultboy_thumbsup2 [] PROGMEM;

// '3', 240x240px
extern const unsigned char vaultboy_thumbsup3 [] PROGMEM;

// '4', 240x240px
extern const unsigned char vaultboy_thumbsup4 [] PROGMEM;

// '5', 240x240px
extern const unsigned char vaultboy_thumbsup5 [] PROGMEM;

// '6', 240x240px
extern const unsigned char vaultboy_thumbsup6 [] PROGMEM;

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 43296)
extern const int vaultboy_thumbsupallArray_LEN;
extern const unsigned char* vaultboy_thumbsupallArray[];

// analog watchface
// 'watch_face', 240x240px
extern const unsigned char bitmap_analog_face [] PROGMEM;

// run bitmap 30x30
extern const unsigned char bitmap_run [] PROGMEM;

// standing bitmap 30x30
extern const unsigned char bitmap_stand [] PROGMEM;

// walking bitmap 30x30
extern const unsigned char bitmap_walking [] PROGMEM;

// footsteps bitmap 25x25
extern const unsigned char bitmap_footsteps [] PROGMEM;

// wifi bitmap 30x30
extern const unsigned char bitmap_wifi [] PROGMEM;

// 02d 60x60 - 
extern const unsigned char bitmap_02d [] PROGMEM;

// 01d 60x60
extern const unsigned char bitmap_01d [] PROGMEM;

// 03d 60x60
extern const unsigned char bitmap_03d [] PROGMEM;

// 04d 60x60
extern const unsigned char bitmap_04d [] PROGMEM;

// 09d 60x60
extern const unsigned char bitmap_09d [] PROGMEM;

// 10d 60x60
extern const unsigned char bitmap_10d [] PROGMEM;

// 11d 60x60
extern const unsigned char bitmap_11d [] PROGMEM;

// 13d 60x60
extern const unsigned char bitmap_13d [] PROGMEM;

// 50d 60x60
extern const unsigned char bitmap_50d [] PROGMEM;

#endif