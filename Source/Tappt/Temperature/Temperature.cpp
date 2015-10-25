#include "Temperature.h"

Temperature::Temperature():
  sensors(new OneWire(TEMPERATURE_PIN)) {

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(this->sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (this->sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Method 1:
  // search for devices on the bus and assign based on an index.  ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  if (!sensors.getAddress(this->insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  this->PrintAddress(this->insideThermometer);
  Serial.println();

  Serial.print("Device 0 Resolution: ");
  Serial.print(this->sensors.getResolution(this->insideThermometer), DEC);
  Serial.println();
}


int Temperature::Tick()
{
  this->timer.Tick();

  if (!this->timer.ShouldTrigger) {
    return 0;
  }

  this->sensors.requestTemperatures();
  float temperature = this->sensors.getTempF(this->insideThermometer);

  // Sometimes the temperature isn't read correctly and is a large
  // negative number so don't send it.
  if (temperature < -50) {
    return -1;
  }

  Serial.print("Temperature: "); Serial.println(temperature);

  sprintf(
    json,
    "{\"t\":\"%f\"}",
    temperature
  );

  Serial.print("Json: ");Serial.println(json);

  Particle.publish("tappt_temperature", json, 5, PRIVATE);

  return 0;
}

// function to print a device address
void Temperature::PrintAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
