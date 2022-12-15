/*
  Library to retrieve weather data from OpenWeatherMap.org using the One Call API 3.0.
    https://openweathermap.org/api/one-call-3

  The One Call API can return the current forecast, minute by minute for 60 minutes, hourly for 48 hours,
  and daily forecasts for 8 days.  Utilizes ArduinoJson (https://arduinojson.org) to process the API return
  and populate data structures

  Alert reports are not implemented
  
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


// DEBUG Flags to print status information to the Serial monitor remove comment markers to enable
// #define DEBUG    // Function processes and errors
//#define DEBUG1 // API Response

#include "OWMOneCall.h"


OWMOneCall::OWMOneCall()
{
  // Set check variables to the max number of forecasts returned by the API
  _maxMinRpts = 60;
  _maxHrRpts = 48;
  _maxDlyRpts = 8;
}

OWMOneCall::~OWMOneCall()
{
  delete [] minWx;
  delete [] hrWx;
  delete [] dlyWx;
}

void OWMOneCall::begin(String apiKey, uint8_t cur, uint8_t min, uint8_t hr, uint8_t dly, OWMUnits units)
{
    _units = units;
    _apiKey = apiKey;

    if (cur > 0) {
      _currentSize = 511;
      _currentRpt = true;
    }
    else {
      _currentSize = 0;
      _currentRpt = false;
    }
    
    if (min > 0) {
      _minuteSize = 2970;
      if (min > _maxMinRpts) {
        _numMinRpts = _maxMinRpts;
      } else {
        _numMinRpts = min;
      }
      minWx = new minuteWeather[_numMinRpts];
    } else {
      _minuteSize = 0;
    }

    if (hr > 0) {
      _hourSize = 16479;
      if (hr > _maxHrRpts) {
        _numHrRpts = _maxHrRpts;
      } else {
        _numHrRpts = hr;
      }
      hrWx = new hourlyWeather[_numHrRpts];
    } else {
      _hourSize = 0;
    }

    if (dly > 0) {
      _dailySize = 4815;
      if (dly > _maxDlyRpts) {
        _numDlyRpts = _maxDlyRpts;
      } else {
        _numDlyRpts = dly;
      }
      dlyWx = new dailyWeather[_numDlyRpts];
    } else {
      _dailySize = 0;
    }

    #ifdef DEBUG
      Serial.print(F("Report requested for: "));
      if (cur > 0) Serial.print(F("Current and "));
      Serial.print(_numMinRpts);
      Serial.print(F(" minutes, "));
      Serial.print(_numHrRpts);
      Serial.print(F(" hours and "));
      Serial.print(_numDlyRpts);
      Serial.println(F(" days."));
    #endif
}

void OWMOneCall::setLocation(float latitude, float longitude) {

  if (latitude > 90 || latitude < -90) {  // Range check
   _lat = 0.0;
   #ifdef DEBUG
     Serial.println(F("Latitude out of range, setting to 0"));
   #endif
  } else {
    _lat = latitude;
  };

  if (longitude > 180 || longitude < -180) {  // Range check
    _lon = 0;
    #ifdef DEBUG
     Serial.println(F("Longitude out of range, setting to 0"));
   #endif
  } else {
    _lon = longitude;
  }

  #ifdef DEBUG
    Serial.print(F("Location set: "));
    Serial.print(_lat);
    Serial.print(F(", "));
    Serial.println(_lon);
  #endif
}

uint16_t OWMOneCall::getJsonSize() {
    uint16_t size;

    size = _positionSize + _currentSize + _minuteSize + _hourSize + _dailySize;

    #ifdef DEBUG
      Serial.print(F("File Size from sum: "));
      Serial.println(size);
    #endif
    
    uint8_t exponent = log2(size) + 1;
    
    size = pow(2, exponent);

    #ifdef DEBUG
      Serial.print(F("Calculated exponent: "));
      Serial.println(exponent);
      Serial.print(F("File Size: "));
      Serial.println(size);
    #endif

    return size;

}

String OWMOneCall::getApiCall() {
    String apiCall;
    String excludes = "&exclude=alerts";
    String unitString;

    if (!_currentRpt) excludes += ",current";
    if (_numDlyRpts < 1) excludes += ",daily";
    if (_numHrRpts < 1) excludes += ",hourly";
    if (_numMinRpts < 1) excludes += ",minutely";

   switch (_units) {
    case IMPERIAL:
      unitString = "&units=imperial&APPID=";
      break;
    case METRIC:
      unitString = "&units=metric&APPID=";
      break;
    case STANDARD:
    default:
      unitString = "&APPID=";
      break;
   }

    apiCall = "http://api.openweathermap.org/data/3.0/onecall?lat=" + String(_lat,2) + "&lon=" + String(_lon, 2) + excludes + unitString + _apiKey;

    return apiCall; 

}

bool OWMOneCall::getWeather() {
  
  String payload;
  String apiCall = getApiCall();
  #ifdef DEBUG
    Serial.println(apiCall);
  #endif
  uint16_t jsonSize = getJsonSize();
  HTTPClient http;
  DynamicJsonDocument wxData(jsonSize);

  http.begin(apiCall);
  uint16_t httpCode = http.GET();
  
    if (httpCode > 0) {
	    payload = http.getString();
        #ifdef DEBUG1
          Serial.println(payload);
        #endif
        DeserializationError wxError = deserializeJson(wxData, payload);
        if (!wxError) {
          
          // Store Current Weather
          if (_currentRpt) {
            currWx.time = wxData["current"]["dt"];
            currWx.sunrise = wxData["current"]["sunrise"];
            currWx.sunset = wxData["current"]["sunset"];
            currWx.temp = wxData["current"]["temp"];
            currWx.feelsLike = wxData["current"]["feels_like"];
            currWx.pressure = wxData["current"]["pressure"];
            currWx.humidity = wxData["current"]["humidity"];
            currWx.dewPoint = wxData["current"]["dew_point"];
            currWx.clouds = wxData["current"]["clouds"];
            currWx.uvi = wxData["current"]["uvi"];
            currWx.visibility = wxData["current"]["visibility"];
            currWx.windSpeed = wxData["current"]["wind_speed"];
            currWx.windGust = wxData["current"]["wind_gust"];
            currWx.windDir = wxData["current"]["wind_deg"];
            currWx.rain = wxData["current"]["rain"]["1hr"];
            currWx.snow = wxData["current"]["snow"]["1hr"];
            currWx.id = wxData["current"]["weather"][0]["id"];
            currWx.main = wxData["current"]["weather"][0]["main"].as<String>();
            currWx.description = wxData["current"]["weather"][0]["description"].as<String>();
            currWx.ico = convertIcon(wxData["current"]["weather"][0]["icon"].as<String>());
          }

          // Store Minutely Weather
          if (_numMinRpts > 0) {
            for (int i = 0; i < _numMinRpts; i++) {
              minWx[i].time = wxData["minutely"][i]["dt"];
              minWx[i].precip = wxData["minutely"][i]["precipitation"];
            }
          }

          // Store Hourly Weather
          if (_numHrRpts > 0) {
            for (int i = 0; i < _numHrRpts; i++) {
               hrWx[i].time = wxData["hourly"][i]["dt"];
               hrWx[i].temp = wxData["hourly"][i]["temp"];
               hrWx[i].feelsLike = wxData["hourly"][i]["feels_like"];
               hrWx[i].pressure = wxData["hourly"][i]["pressure"];
               hrWx[i].humidity = wxData["hourly"][i]["humidity"];
               hrWx[i].dewPoint = wxData["hourly"][i]["dew_point"];
               hrWx[i].clouds = wxData["hourly"][i]["clouds"];
               hrWx[i].uvi = wxData["hourly"][i]["uvi"];
               hrWx[i].visibility = wxData["hourly"][i]["visibility"];
               hrWx[i].windSpeed = wxData["hourly"][i]["wind_speed"];
               hrWx[i].windGust = wxData["hourly"][i]["wind_gust"];
               hrWx[i].windDir = wxData["hourly"][i]["wind_deg"];
               hrWx[i].precipProb = wxData["hourly"][i]["pop"];
               hrWx[i].rain = wxData["hourly"][i]["rain"]["1hr"];
               hrWx[i].snow = wxData["hourly"][i]["snow"]["1hr"];
               hrWx[i].id = wxData["hourly"][i]["weather"][0]["id"];
               hrWx[i].main = wxData["hourly"][i]["weather"][0]["main"].as<String>();
               hrWx[i].description = wxData["hourly"][i]["weather"][0]["description"].as<String>();
               hrWx[i].ico = convertIcon(wxData["hourly"][i]["weather"][0]["icon"].as<String>()); 
            }
          }

          // Store Daily Weather
          if (_numDlyRpts > 0) {
            for (int i = 0; i < _numDlyRpts; i++) {
               dlyWx[i].time = wxData["daily"][i]["dt"];
               dlyWx[i].sunrise = wxData["daily"][i]["sunrise"];
               dlyWx[i].sunset = wxData["daily"][i]["sunset"];
               dlyWx[i].moonrise = wxData["daily"][i]["moonrise"];
               dlyWx[i].moonset = wxData["daily"][i]["moonset"];
               dlyWx[i].moonPhase = wxData["daily"][i]["moon_phase"];
               dlyWx[i].morningTemp = wxData["daily"][i]["temp"]["morn"];
               dlyWx[i].dayTemp = wxData["daily"][i]["temp"]["day"];
               dlyWx[i].eveTemp = wxData["daily"][i]["temp"]["eve"];
               dlyWx[i].nightTemp = wxData["daily"][i]["temp"]["night"];
               dlyWx[i].maxTemp = wxData["daily"][i]["temp"]["max"];
               dlyWx[i].minTemp = wxData["daily"][i]["temp"]["min"];
               dlyWx[i].mornFeels = wxData["daily"][i]["feels_like"]["morn"];
               dlyWx[i].dayFeels = wxData["daily"][i]["feels_like"]["day"];
               dlyWx[i].eveFeels = wxData["daily"][i]["feels_like"]["eve"];
               dlyWx[i].nightFeels = wxData["daily"][i]["feels_like"]["night"];
               dlyWx[i].pressure = wxData["daily"][i]["pressure"];
               dlyWx[i].humidity = wxData["daily"][i]["humidity"];
               dlyWx[i].dewPoint = wxData["daily"][i]["dew_point"];
               dlyWx[i].clouds = wxData["daily"][i]["clouds"];
               dlyWx[i].uvi = wxData["daily"][i]["uvi"];
               dlyWx[i].windSpeed = wxData["daily"][i]["wind_speed"];
               dlyWx[i].windGust = wxData["daily"][i]["wind_gust"];
               dlyWx[i].windDir = wxData["daily"][i]["wind_deg"];
               dlyWx[i].precipProb = wxData["daily"][i]["pop"];
               dlyWx[i].rain = wxData["daily"][i]["rain"];
               dlyWx[i].snow = wxData["daily"][i]["snow"];
               dlyWx[i].id = wxData["daily"][i]["weather"][0]["id"];
               dlyWx[i].main = wxData["daily"][i]["weather"][0]["main"].as<String>();
               dlyWx[i].description = wxData["daily"][i]["weather"][0]["description"].as<String>();
               dlyWx[i].ico = convertIcon(wxData["daily"][i]["weather"][0]["icon"].as<String>());    
            }
          }

        } else {
        #ifdef DEBUG
          Serial.print(F("Weather data JSON error: "));
          Serial.println(wxError.c_str());
        #endif
        return false;
        }
     
    } else {
      #ifdef DEBUG
        Serial.print(F("API Call error"));
      #endif
      return false;
    }
  http.end();
  return true; 
}

Icon OWMOneCall::convertIcon(String str) {
    if (str == "01d") return i01d;
    else if(str == "01n") return i01n;
    else if(str == "02d") return i02d;
    else if(str == "02n") return i02n;
    else if(str == "03d") return i03d;
    else if(str == "03n") return i03n;
    else if(str == "04d") return i04d;
    else if(str == "04n") return i04n;
    else if(str == "09d") return i09d;
    else if(str == "09n") return i09n;
    else if(str == "10d") return i10d;
    else if(str == "10n") return i10n;
    else if(str == "11d") return i11d;
    else if(str == "11n") return i11n;
    else if(str == "13d") return i13d;
    else if(str == "13n") return i13n;
    else if(str == "50d") return i50d;
    else if(str == "50n") return i50n;
    else return ierr;
}

