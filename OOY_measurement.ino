/**
 * Update measurement data (string) for xively 
 */
void updateData() {
  getAllTemperatureValues(&actualReadings[0]);
  actualReadings[chan_heater_power] = measurePower();

  if (actualReadings[chan_inlet_temp] < lowest_inlet) {
    lowest_inlet = actualReadings[chan_inlet_temp];
  }

  actualReadings[chan_heater_power_cum] += actualReadings[chan_heater_power]/240000;

  if (1) {
    char channel_data[7];
    
    dtostrf(actualReadings[chan_boiler_temp_top], 4, 2, channel_data);
    dataString = "boiler_temp_top,";   
    dataString += channel_data;

    dtostrf(actualReadings[chan_boiler_temp_bottom], 4, 2, channel_data);
    dataString += "\nboiler_temp_bottom,";   
    dataString += channel_data;

    dtostrf(actualReadings[chan_boiler_temp_top]-actualReadings[chan_boiler_temp_bottom], 5, 2, channel_data);
    dataString += "\nboiler_temp_delta,";   
    dataString += channel_data;
  
    dtostrf(actualReadings[chan_inlet_temp], 4, 2, channel_data);
    dataString += "\ninlet_temp,";
    dataString += channel_data;

    dtostrf(actualReadings[chan_outlet_temp], 4, 2, channel_data);
    dataString += "\noutlet_temp,";
    dataString += channel_data;

    dtostrf(actualReadings[chan_shell_temp], 4, 2, channel_data);
    dataString += "\nshell_temp,";
    dataString += channel_data;
    
    dtostrf(actualReadings[chan_heater_power], 1, 0, channel_data);
    dataString += "\nheater_power,";
    dataString += channel_data;
    
    dtostrf(actualReadings[chan_heater_power_cum], 4, 2, channel_data);
    dataString += "\nheater_power_cum,";
    dataString += channel_data;
    
    if (sendLastDayResult) {
      dtostrf(lowest_inlet, 4, 2, channel_data);
      dataString += "\nwater_temp,";
      dataString += channel_data;
      lowest_inlet = 99.9;
      
      sendLastDayResult = false;
    }
  }
}

/**
 * Reading the actual mains current to calculate the heater power consumption
 */
double measurePower() {
  double power = emon.calcIrms(NMBR_OF_MEASUREMENTS) * MAINS_VOLTAGE;
  if ( power > MIN_POWER_LEVEL) {
    return power;
  } else {
    return 0.0;
  }
}

/**
 * Reading all connected DS18B20 temperature sensors
 * 
 * @param *values floats array to store the measured values
 */
void getAllTemperatureValues(float *values) {
  int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;

  byte sensorId = 0;
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  while ( ds.search(addr)) {

    if ( OneWire::crc8( addr, 7) != addr[7]) {
      //CRC is not valid!
      return;
    }

    if ( addr[0] == 0x28) {
      //device is a DS18B20 family device
      ds.reset();
      ds.select(addr);
      ds.write(0x44);         // start conversion
      present = ds.reset();
      ds.select(addr);    
      ds.write(0xBE);         // Read Scratchpad

      for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
      }

      LowByte = data[0];
      HighByte = data[1];
      TReading = (HighByte << 8) + LowByte;
      SignBit = TReading & 0x8000;  // test most sig bit

      if (SignBit) // negative
      {
        TReading = (TReading ^ 0xffff) + 1; // 2's comp
      }
      values[sensorId] = TReading * 0.0625;
      values[sensorId] = (!SignBit) ? values[sensorId] : -1 * values[sensorId];

      sensorId++;
    }
  }
  ds.reset_search();
}

