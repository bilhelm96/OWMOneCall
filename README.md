# OWMOneCall Library for Ardiuno

Simple library for getting weather data from OpenWeatherMap.org via their One Call 3.0 API. The One Call API can return the current forcast, precipication forcast for the next 60 minutes, hourly forcasts for the next 48 hours and daily forecasts for eight days. While the One Call API can also return regional alerts the functionality is not implemented in the library.  
The number of reports processed can be reduced from the maximum provided in order to save memory and the different types of forcasts can be included or not included as the user desires.  
The library relies on the [ArduinoJson](https://github.com/bblanchon/ArduinoJson) for parsing the JSON data returned from OpenWeatherMap. The OWMOneCall libray Version 1.2.0 and higher requires ArduinoJson version 7 to work properly. 

### Using the library
There are only three functions to use the OWMOnCall library:
- begin(): Initializes the library with the required API key and what and how many forecasts to save
- setLoctaion(): Sets the latitude and longitude for the requested forecast
- getWeather(): Populates the data structures 

Forecast data is retrieved by accessing the data structures directly.