#pragma once
#include <Arduino.h>
#include <stdint.h>

// ----- Deployment params ---------------------------------------------------------------
#define ENABLE_LOGGING						// enables logging
#define DEFAULT_BUTTON_HOLD_TIME 2000		// default time for hold to register
#define WIFI_DISCONNECT_DELAY 20			// time before reconnect attempt
// #define STANDALONE						// if on portable (5V) mode
// ---------------------------------------------------------------------------------------

// ----- DMX params ----------------------------------------------------------------------
#define UNIVERSE 8							// DMX universe
#define ADDR_OFFSET (0)						// address offset in said universe
// ---------------------------------------------------------------------------------------

// ----- LED type params -----------------------------------------------------------------
#define LED_TYPE WS2815						// ledstrip type (FASTLED)
#define LED_ORDER RGB						// led order type (FASTLED)

#define DIMENSION_1D 1						// led group 1d
#define DIMENSION_2D 2						// led group 2d

#define DIMENSION DIMENSION_1D				// use 1D or 2D indexing
// ---------------------------------------------------------------------------------------

#define NUM_GROUPS NUM_LEDS				 	// this sets number of pixels
#if DIMENSION == DIMENSION_2D				// if dimension is 2D, set grid parameters
	#define GRID_WSIZE 2					// subgrid width
	#define GRID_HSIZE 2					// subgrid height
	#define GRID_WIDTH 8					// total grid witdh
	#define GRID_HEIGHT 24					// total grid size
#endif
// ---------------------------------------------------------------------------------------


// ===== CONSTANT ========================================================================
#define BAUD_RATE 9600						// debug connection
#define DMX_SIZE 512						// dmx universe size (always 512)
#define HARDWARE_DATA_PIN 5					// hardware data pin

// ----- Wifi credentials ----------------------------------------------------------------
#define WIFI_SSID "Ledique"				 	// wifi ssid (the same network as sacn master)
#define WIFI_PASS "dasenebipovezau"		 	// wifi password
// ---------------------------------------------------------------------------------------

// ----- Standalone params for wifi ------------------------------------------------------
#ifdef STANDALONE
	#define WIFI_BRIGHTNESS 60
	#define WIFI_DELAY 50
#else
	#define WIFI_BRIGHTNESS 255
	#define WIFI_DELAY 10
#endif
// ---------------------------------------------------------------------------------------

// ----- Logging functions ---------------------------------------------------------------
#ifdef ENABLE_LOGGING
	#define LOG(pattern) Serial.println(pattern)
	#define LOGF(pattern, args...) Serial.printf(pattern, args)
#else
	#define LOG(pattern)
	#define LOGF(pattern, args...)
#endif
// ---------------------------------------------------------------------------------------

// ----- we cant have more groups than leds ----------------------------------------------
#if NUM_GROUPS > NUM_LEDS
	#define NUM_GROUPS NUM_LEDS
#endif
// ---------------------------------------------------------------------------------------
// =======================================================================================

