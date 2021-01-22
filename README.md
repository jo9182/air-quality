Air Quality
-----------

Checks various air quality indicators and display's the result. 

Measures temperature, pressure, humidity, VOCs, CO2, and particulate matter.

Built with

* Adafruit QT Py - https://www.adafruit.com/product/4600
* Adafruit PMSA003I Air Quality Breakout - https://www.adafruit.com/product/4632
* Adafruit Adafruit SGP30 Air Quality Sensor Breakout - https://www.adafruit.com/product/3709
* Adafruit BME680 - Temperature, Humidity, Pressure and Gas Sensor - https://www.adafruit.com/product/3660
* Monochrome 1.3" 128x64 OLED graphic display - https://www.adafruit.com/product/938
* SparkFun Qwiic Cable Kit - https://www.sparkfun.com/products/15081
* Any USB-C power supply

Todo:
* understand the different PM2.5 units better
* add a button, then  
    * make the screen stay off most of the time to avoid OLED burn-in (and/or switch to a TFT)
    * create a main screen with an overall summary and additional screens that dive deeper in to specific stats
* record the some history and create a way to display it
* calculate the "feels like" temperature
* house everything in a nice container