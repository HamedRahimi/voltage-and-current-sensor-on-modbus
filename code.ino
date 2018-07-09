#include "EEPROM.h"
#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

modbusDevice regBank;
modbusSlave slave;
Adafruit_INA219 ina219;

void setup()
{
   uint32_t currentFrequency;
   
  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  ina219.begin();
 
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();
  //Assign the modbus device ID.
  regBank.setId(1);


  /*
    modbus registers follow the following format
    00001-09999  Digital Outputs, A master device can read and write to these registers
    10001-19999  Digital Inputs, A master device can only read the values from these registers
    30001-39999  Analog Inputs, A master device can only read the values from these registers
    40001-49999  Analog Outputs, A master device can read and write to these registers

    Analog values are 16 bit unsigned words stored with a range of 0-32767
    Digital values are stored as bytes, a zero value is OFF and any nonzer value is ON

    It is best to configure registers of like type into contiguous blocks.  this
    allows for more efficient register lookup and and reduces the number of messages
    required by the master to retrieve the data
  */

  //Add Analog Input registers 30001-10010 to the register bank
  regBank.add(30001);
  regBank.add(30002);
  regBank.add(30003);
  regBank.add(30004);
  /*
     Assign the modbus device object to the protocol handler
     This is where the protocol handler will look to read and write
     register data.  Currently, a modbus slave protocol handler may
     only have one device assigned to it.
  */
  slave._device = &regBank;
  // Initialize the serial port for coms at 9600 baud
  slave.setBaud(9600);
  wdt_enable(WDTO_8S);
  delay(100);

}

void loop()
{
 
  wdt_enable(WDTO_8S);

   word shuntvoltage =0;
   word busvoltage = 0;
   word current_mA = 0;
   word loadvoltage = 0;

  shuntvoltage = ina219.getShuntVoltage_mV()*1000;
  busvoltage = ina219.getBusVoltage_V()*1000;
  current_mA = ina219.getCurrent_mA()*1000;
  loadvoltage = busvoltage + (shuntvoltage / 1000)*1000;

  //put some data into the registers
  regBank.set(30001, shuntvoltage );
  regBank.set(30002, busvoltage);
  regBank.set(30003, current_mA );
  regBank.set(30004, loadvoltage);

  slave.run();
  wdt_reset();

}
