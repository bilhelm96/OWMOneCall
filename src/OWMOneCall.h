/*
  Library to retrieve weather data from OpenWeatherMap.org using the One Call API 3.0.
    https://openweathermap.org/api/one-call-3

  The One Call API can return the current forecast, minute by minute for 60 minutes, hourly for 48 hours,
  and daily forecasts for 8 days.  Utilizes ArduinoJson (https://arduinojson.org) to process the API return
  and populate data structures

  Alerts reports are not implemented
  
  Released under the MIT License
  
  Copyright ©2022 William A. Wilhelm
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
  and associated documentation files (the "Software"), to deal in the Software without restriction, 
  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do 
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
  ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
  THE SOFTWARE.
*/

#ifndef __OWMONECALL_H__
#define __OWMONECALL_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "math.h"


/* Enumeration for the OpenWeatherMap icons: 
(https://openweathermap.org/weather-conditions#How-to-get-icon-URL)
useful in using itegers to select icons for display on a screen
*/
enum Icon {
    ierr,
    i01d,
    i01n,
    i02d,
    i02n,
    i03d,
    i03n,
    i04d,
    i04n,
    i09d,
    i09n,
    i10d,
    i10n,
    i11d,
    i11n,
    i13d,
    i13n,
    i50d,
    i50n,
};

// Enumeration for available unit formats at OpenWeatherMap
enum OWMUnits {
  IMPERIAL,
  METRIC,
  STANDARD
};

// Structure for fields rerturned in the Current Weather
struct currentWeather {
  uint32_t time;       // Unix timestamp
  uint32_t sunrise;    // Unix timestamp
  uint32_t sunset;     // Unix timestamp
  float temp;          // Current temperature
  float feelsLike;     // Human perception of temperature
  uint16_t pressure;   // Pressure hPa
  uint8_t humidity;    // Humidity %
  float dewPoint;      // Dew Point
  uint8_t clouds;      // Cloudiness %
  float uvi;           // UV index
  uint16_t visibility; // Visibility in meters
  float windSpeed;     // Wind Speed
  float windGust;      // Wind Gust
  uint16_t windDir;    // Wind direction in degrees
  uint16_t rain;       // Rain volume for last hour in mm
  uint16_t snow;       // Snoe volume for last hour in mm
  uint16_t id;         // WeatherID
  String main;         // Open Weather Main parameter
  String description;  // Open Weather condition
  Icon ico ;           // Weather icon
};

// Structure for the minute by minute forecast values returned from the API
struct minuteWeather {
  uint32_t time;   // Unix timestamp
  uint32_t precip; // Precipitation in mm
};

// Structure for the hourly forecast values returned from the API
struct hourlyWeather {
  uint32_t time;       // Unix timestamp
  float temp;          // Current temperature
  float feelsLike;     // Human perception of temperature
  uint16_t pressure;   // Pressure hPa
  uint8_t humidity;    // Humidity %
  float dewPoint;      // Dew Point
  float uvi;           // UV index
  uint8_t clouds;      // Cloudiness %
  uint16_t visibility; // Visibility in meters
  float windSpeed;     // Wind Speed
  float windGust;      // Wind Gust
  uint16_t windDir;    // Wind direction in degrees
  float precipProb;    // Probability of precipitation 0 - 1
  uint16_t rain;       // Rain volume for last hour in mm
  uint16_t snow;       // Snow volume for last hour in mm
  uint16_t id;         // WeatherID
  String main;         // Open Weather Main parameter
  String description;  // Open Weather condition
  Icon ico ;           // Weather icon
};

// Structure for the daily forecast values returned from the API
struct dailyWeather {
  uint32_t time;       // Unix timestamp
  uint32_t sunrise;    // Unix timestamp
  uint32_t sunset;     // Unix timestamp
  uint32_t moonrise;   // Unix timestamp
  uint32_t moonset;    // Unix timestamp
  float moonPhase;     // 0 and 1 are new moon; .5 is full
  float morningTemp;   // Morning temperature
  float dayTemp;       // Day temperature
  float eveTemp;       // Evening temperature
  float nightTemp;     // Night temperature
  float maxTemp;       // Max daily temperature
  float minTemp;       // Min daily temperature
  float mornFeels;     // Morning feels like temperature
  float dayFeels;      // Day feels like temperature
  float eveFeels;      // Evening feels like temperature
  float nightFeels;    // Night feels like temperature
  uint16_t pressure;   // Pressure hPa
  uint8_t humidity;    // Humidity %
  float dewPoint;      // Dew Point
  float uvi;           // UV index
  uint8_t clouds;      // Cloudiness %
  float windSpeed;     // Wind Speed
  float windGust;      // Wind Gust
  uint16_t windDir;    // Wind direction in degrees
  float precipProb;    // Probability of precipitation 0 - 1
  float rain;       // Rain volume in mm
  float snow;       // Snow volume in mm
  uint16_t id;         // WeatherID
  String main;         // Open Weather Main parameter
  String description;  // Open Weather condition
  Icon ico ;           // Weather icon
};

class OWMOneCall
{
private:
    String _apiKey;  // User's API Key required to make the call
    OWMUnits _units;  // Requested units for the call to be returned in

    // Latitute and longiture to check the weather for
    float _lat;
    float _lon;
    

    // OWMO One Call provides Minute forecasts for the next 60 Minutes
    // Hourly forecasts for the next 48 hours
    // Daily forecasts for the next 8 days
    // These are check variables set in the constructor confirm the user does not 
    // request more than the max amount of forecasts returned by the API
    uint8_t _maxMinRpts;  // 60
    uint8_t _maxHrRpts;   // 48
    uint8_t _maxDlyRpts;  // 8

    // User can filter the number of reports to save memory
    bool _currentRpt;
    uint8_t _numMinRpts;
    uint8_t _numHrRpts;
    uint8_t _numDlyRpts;

    // Sizing for the JSON file to parse the API response - Sizes from ArduinoJson assistant (https://arduinojson.org/v6/assistant/#/step1)
    uint16_t _currentSize;  // min 511, recommended 768
    uint16_t _minuteSize;   // min 2970, recommended 3072
    uint16_t _hourSize;     // min 16479, recommded 24576
    uint16_t _dailySize;    // min 4815, recommended 6144
    static const uint16_t _positionSize = 114;  // Header (lat, long, timezone) min size 114, recommended 128
    
    // Fucntoin to calulate JSON Deserialization file size. Adds minimums and rounds up to the next higher power of 2 for margin
    uint16_t getJsonSize();  // Total file min size 24614, recommended 32768

    // Convert the icon name in the API response to the enumeration
    Icon convertIcon(String str);

    // Generate the http: string based on location, forecasts requested, units, and API key
    String getApiCall();
    

public:
    OWMOneCall();
    ~OWMOneCall();
    /* Begin function to set what forecasts to get, filter to the number of forecasts
       requested, set the API key and units
     */
    void begin(String apiKey, uint8_t cur, uint8_t min, uint8_t hr, uint8_t dly, OWMUnits units);
    
    // Sets the location for the API call
    void setLocation(float latitude, float longitude);

    // Call the API and populate data structures
    // Returns true if successful, false on error
    bool getWeather();
    
    // Variables to store forecast data 
    // Minute, Hour, and Daily are pointers to dynamically set size based on user quantity filter
    currentWeather currWx;
    minuteWeather *minWx;
    hourlyWeather *hrWx;
    dailyWeather *dlyWx;

};


#endif