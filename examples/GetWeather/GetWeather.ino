
#include <Arduino.h>
#include <OWMOneCall.h>
#include <WiFi.h>

OWMOneCall weather;

OWMUnits units = IMPERIAL;


// Replace with your network information and API key
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PW";
const char* API_KEY = YOUR_API_KEY";

// Which and number of forecasts to store - 0 to disable
// Max: current: 1, minute: 60, hour: 48, daily: 8
uint8_t currentCount = 1;
uint8_t minuteCount = 4;
uint8_t hourCount = 12;
uint8_t dailyCount = 5;

float latitude = 39.76;
float longitude = -74.89;

// Function prototypes for printing forecast info
void printCurr();
void printMin();
void printHour();
void printDaily();


void setup() {
  Serial.begin(115200);

  // Enable the builtin led - lit when connected to WiFi
  pinMode(LED_BUILTIN, OUTPUT);

  uint8_t count = 0;
  WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println(ssid);
    Serial.print(F("Connecting to WiFi .."));

    while (WiFi.status() != WL_CONNECTED && count < 20) // Try for 20 seconds and move on if not connected
    { 
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print('.');
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      count++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print(F("WiFi Connected IP: "));
      Serial.println(WiFi.localIP());
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print(F("WiFi connect error"));
    }

  weather.begin(API_KEY, currentCount, minuteCount, hourCount, dailyCount, IMPERIAL);
  weather.setLocation(latitude, longitude);
  
  weather.getWeather();

  if (currentCount > 0) printCurr();
  if (minuteCount > 0) printMin();
  if (hourCount > 0) printHour();
  if (dailyCount > 0) printDaily();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void printCurr() {
  //Print Current weather
  Serial.println(F("Current Conditions"));
  Serial.print(F("Time : "));
  Serial.println(weather.currWx.time);
  Serial.print(F("Sunrise : "));
  Serial.println(weather.currWx.sunrise);
  Serial.print(F("Sunset : "));
  Serial.println(weather.currWx.sunset);
  Serial.print(F("Temp : "));
  Serial.println(weather.currWx.temp);
  Serial.print(F("Feels Like : "));
  Serial.println(weather.currWx.feelsLike);
  Serial.print(F("Pressure : "));
  Serial.println(weather.currWx.pressure);
  Serial.print(F("Humidity : "));
  Serial.println(weather.currWx.humidity);
  Serial.print(F("Dew Point : "));
  Serial.println(weather.currWx.dewPoint);
  Serial.print(F("Clouds : "));
  Serial.println(weather.currWx.clouds);
  Serial.print(F("UVI : "));
  Serial.println(weather.currWx.uvi);
  Serial.print(F("Visibility : "));
  Serial.println(weather.currWx.visibility);
  Serial.print(F("Wind Speed : "));
  Serial.println(weather.currWx.windSpeed);
  Serial.print(F("Wind Gust : "));
  Serial.println(weather.currWx.windGust);
  Serial.print(F("Wind Dir : "));
  Serial.println(weather.currWx.windDir);
  Serial.print(F("Rain : "));
  Serial.println(weather.currWx.rain);
  Serial.print(F("Snow : "));
  Serial.println(weather.currWx.snow);
  Serial.print(F("ID : "));
  Serial.println(weather.currWx.id);
  Serial.print(F("Main : "));
  Serial.println(weather.currWx.main);
  Serial.print(F("Description : "));
  Serial.println(weather.currWx.description);
  Serial.print(F("Icon : "));
  Serial.println(weather.currWx.ico);
  Serial.println();
}

void printMin() {
  // Print Minute weather
  Serial.print(F("Minute by minute forecast for ")) ;
  Serial.print(minuteCount);
  Serial.println(F(" Min."));
  for (uint8_t i = 0; i < minuteCount; i++) {
    Serial.print(F("Time: "));
    Serial.print(weather.minWx[i].time);
    Serial.print(F("  Precip: "));
    Serial.println(weather.minWx[i].precip);
  }
  Serial.println();
}

void printHour() {
  // Print Hourly Weather
  Serial.println(F("Hourly forecast for "));
  Serial.print(hourCount);
  Serial.println(F(" Hours."));
   for (uint8_t i = 0; i < hourCount; i++) {
    Serial.print(F("Time : "));
    Serial.println(weather.hrWx[i].time);
    Serial.print(F("Temp : "));
    Serial.println(weather.hrWx[i].temp);
    Serial.print(F("Feels Like : "));
    Serial.println(weather.hrWx[i].feelsLike);
    Serial.print(F("Pressure : "));
    Serial.println(weather.hrWx[i].pressure);
    Serial.print(F("Humidity : "));
    Serial.println(weather.hrWx[i].humidity);
    Serial.print(F("Dew Point : "));
    Serial.println(weather.hrWx[i].dewPoint);
    Serial.print(F("Clouds : "));
    Serial.println(weather.hrWx[i].clouds);
    Serial.print(F("UVI : "));
    Serial.println(weather.hrWx[i].uvi);
    Serial.print(F("Visibility : "));
    Serial.println(weather.hrWx[i].visibility);
    Serial.print(F("Wind Speed : "));
    Serial.println(weather.hrWx[i].windSpeed);
    Serial.print(F("Wind Gust : "));
    Serial.println(weather.hrWx[i].windGust);
    Serial.print(F("Wind Dir : "));
    Serial.println(weather.hrWx[i].windDir);
    Serial.print(F("Probability of precip: "));
    Serial.println(weather.hrWx[i].precipProb);
    Serial.print(F("Rain : "));
    Serial.println(weather.hrWx[i].rain);
    Serial.print(F("Snow : "));
    Serial.println(weather.hrWx[i].snow);
    Serial.print(F("ID : "));
    Serial.println(weather.hrWx[i].id);
    Serial.print(F("Main : "));
    Serial.println(weather.hrWx[i].main);
    Serial.print(F("Description : "));
    Serial.println(weather.hrWx[i].description);
    Serial.print(F("Icon : "));
    Serial.println(weather.hrWx[i].ico);
    Serial.println();
 }
}

void printDaily() {
  // Print Daily Weather
  Serial.println(F("Daily forecast for "));
  Serial.print(dailyCount);
  Serial.println(F(" Days."));
   for (uint8_t i = 0; i < dailyCount; i++) {
    Serial.print(F("Time : "));
    Serial.println(weather.dlyWx[i].time);
    Serial.print(F("Sunrise : "));
    Serial.println(weather.dlyWx[i].sunrise);
    Serial.print(F("Sunset : "));
    Serial.println(weather.dlyWx[i].sunset);
    Serial.print(F("Moonrise : "));
    Serial.println(weather.dlyWx[i].moonrise);
    Serial.print(F("Moonset : "));
    Serial.println(weather.dlyWx[i].moonset);
    Serial.print(F("Moon Phase : "));
    Serial.println(weather.dlyWx[i].moonPhase);
    Serial.print(F("Morning Temp : "));
    Serial.println(weather.dlyWx[i].morningTemp);
    Serial.print(F("Day Temp : "));
    Serial.println(weather.dlyWx[i].dayTemp);
    Serial.print(F("Evening Temp : "));
    Serial.println(weather.dlyWx[i].eveTemp);
    Serial.print(F("Night Temp : "));
    Serial.println(weather.dlyWx[i].nightTemp);
    Serial.print(F("Low Temp : "));
    Serial.println(weather.dlyWx[i].minTemp);
    Serial.print(F("High Temp : "));
    Serial.println(weather.dlyWx[i].maxTemp);
    Serial.print(F("Morning Feels Like : "));
    Serial.println(weather.dlyWx[i].mornFeels);
    Serial.print(F("Day Feels Like : "));
    Serial.println(weather.dlyWx[i].dayFeels);
    Serial.print(F("Evening Feels Like : "));
    Serial.println(weather.dlyWx[i].eveFeels);
    Serial.print(F("Night Feels Like : "));
    Serial.println(weather.dlyWx[i].nightFeels);
    Serial.print(F("Pressure : "));
    Serial.println(weather.dlyWx[i].pressure);
    Serial.print(F("Humidity : "));
    Serial.println(weather.dlyWx[i].humidity);
    Serial.print(F("Dew Point : "));
    Serial.println(weather.dlyWx[i].dewPoint);
    Serial.print(F("Clouds : "));
    Serial.println(weather.dlyWx[i].clouds);
    Serial.print(F("UVI : "));
    Serial.println(weather.dlyWx[i].uvi);
    Serial.print(F("Wind Speed : "));
    Serial.println(weather.dlyWx[i].windSpeed);
    Serial.print(F("Wind Gust : "));
    Serial.println(weather.dlyWx[i].windGust);
    Serial.print(F("Wind Dir : "));
    Serial.println(weather.dlyWx[i].windDir);
    Serial.print(F("Probability of precip: "));
    Serial.println(weather.dlyWx[i].precipProb);
    Serial.print(F("Rain : "));
    Serial.println(weather.dlyWx[i].rain);
    Serial.print(F("Snow : "));
    Serial.println(weather.dlyWx[i].snow);
    Serial.print(F("ID : "));
    Serial.println(weather.dlyWx[i].id);
    Serial.print(F("Main : "));
    Serial.println(weather.dlyWx[i].main);
    Serial.print(F("Description : "));
    Serial.println(weather.dlyWx[i].description);
    Serial.print(F("Icon : "));
    Serial.println(weather.dlyWx[i].ico);
    Serial.println();
 }
}
