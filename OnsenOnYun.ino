#include <OneWire.h>
#include <Wire.h>
#include <Console.h>
#include <stdlib.h>
#include <Process.h>
#include <stdbool.h>
#include "EmonLib.h"
#include "RTClib.h"

// Program settings
#define SAMPLE_INTERVAL         15000
#define CURRENT_CAL             28.95
#define NMBR_OF_MEASUREMENTS    1480
#define MAINS_VOLTAGE           230
#define MIN_POWER_LEVEL         50.0
#define ISO_VALUE               10

// Xively connection parameters
#define APIKEY        "uIP9aTS6lbSZZ9PYpStFrBUgsw0zqVIudkSBG3VWOihkP62R"//Xively API key
#define FeedId        "1234611996"  //Xively Feed ID

// DS1307 Real Time Clock
RTC_DS1307 RTC;

// DS18S20 Temperature chip i/o
OneWire ds(7);  // on pin 7

// Create an Energy Monitor instance
EnergyMonitor emon;

// Date
Process date;
int day;       
int lastDay = -1;
bool sendLastDayResult = false;

float actualReadings[10];

#define chan_boiler_temp_bottom 0
#define chan_outlet_temp        1
#define chan_shell_temp         2
#define chan_boiler_temp_top    3
#define chan_inlet_temp         4
#define chan_boiler_temp_delta  5
#define chan_heater_power       6
#define chan_heater_power_cum   7
#define chan_boiler_energy_in   8
#define chan_boiler_energy_out  9

unsigned long lastSample = 0;      // when your last send a sample

float lowest_inlet = 99.9;
float lastBoilerTemp = 0.0;


String dataString;
//static char channel_data[7];  // Channel data string buffer 

/**
 * Program setup
 */
void setup(void) {
  Wire.begin();

  // Start the date process
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%u"); //get the day of week (1..7)
    date.run();
  }

  // Initialize Console and wait for port to open:
  Bridge.begin();
  Console.begin();

  // Initialize power monitor
  emon.current(0, CURRENT_CAL);

  // Get started
  delay(10000);
  
  Console.print("Starting ");
  getAllTemperatureValues(&actualReadings[0]); //do one reading to clear sensor buffer
  Console.print(".");
  actualReadings[chan_heater_power] = measurePower();
  Console.println(".\n");
}

/**
 * Main program loop
 */
void loop()
{
  if (millis() - lastSample >= SAMPLE_INTERVAL) {
    lastSample = millis();
        
    if(lastDay != day) {
      actualReadings[chan_heater_power_cum] = (float)0; // reset cumulative power on a new day
      actualReadings[chan_boiler_energy_in] = (float)0;
      actualReadings[chan_boiler_energy_out] = (float)0;
      sendLastDayResult = true;
		Console.println("A new day is here!");
    }
    
    if (!date.running())  {
      date.begin("date");
      date.addParameter("+%u"); //get the day of week (1..7)
      date.run();
    }
        
    updateData();
    sendData();
  }

  while (date.available() > 0) {
    lastDay = day;
    day = date.readString().toInt();
  }
}
