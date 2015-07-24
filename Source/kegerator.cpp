#include "application.h"

//#define DEBUG 1

#include "KegeratorState.h"
#include "NfcClient.h"
#include "DallasTemperature.h"

NfcClient* nfcClient;
int state = KegeratorState::LISTENING;

DallasTemperature sensors(new OneWire(D2));

// arrays to hold device address
DeviceAddress insideThermometer;

void setupThermometer();

void setup(void) {
    Serial.begin(115200);

    while(!Serial.available()) {
      Spark.process();
    }

    //nfcClient = new NfcClient();

    setupThermometer();
}

void loop(void) {
//  delay(6000);
  //RestClient RestClient = RestClient("http://tappt.io");
  sensors.requestTemperatures();
  float celsius = sensors.getTempC(insideThermometer);
  Serial.print("Temperature: "); Serial.println(celsius) ;

return;
  switch (state) {
    case KegeratorState::LISTENING:
      {
        int nfcState = nfcClient->Tick();
        if (nfcState == NfcState::NO_MESSAGE) {
          delay(1000);
          break;
        }


        // handle status. switch state
      }
      break;
    case KegeratorState::POURING:
      break;
    case KegeratorState::DONE_POURING:
      break;
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void setupThermometer() {
  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // assign address manually.  the addresses below will beed to be changed
  // to valid device addresses on your bus.  device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };

  // Method 1:
  // search for devices on the bus and assign based on an index.  ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them.  It might be a good idea to
  // check the CRC to make sure you didn't get garbage.  The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();
}

/*
OneWire one = OneWire(D3);
uint8_t resp[9];
char myIpAddress[24];
char tempfStr[16];

//unsigned int lastTime = 0;

TempSensor sensors[NUM_SENSORS];
int checkIndex = 0;



void findDevices() {
    Serial.println("waiting 5 seconds...");
    delay(5000);

    uint8_t addr[12];
    int found = 0;
    while(one.search(addr)) {

        Serial.print("Found device: ");

        char *tempID = new char[16];
        sprintf(tempID, "%x%x%x%x%x%x%x%x%x",
            addr[0],  addr[0], addr[2] , addr[3] , addr[4] , addr[5], addr[6], addr[7] , addr[8]
        );
        sensors[found].id = tempID;

        for(int i=0;i<9;i++)
        {
            sensors[found].rom[i] = addr[i];
        }
        sensors[found].updated = 0;

        Serial.print(tempID);
        Serial.println("");
        found++;
    }
}

void setup() {
    Serial.begin(9600);

    findDevices();

    Spark.variable("temperatureF", &tempfStr, STRING);

    Spark.variable("ipAddress", myIpAddress, STRING);
    IPAddress myIp = Network.localIP();
    sprintf(myIpAddress, "%d.%d.%d.%d", myIp[0], myIp[1], myIp[2], myIp[3]);
}



void loop() {
    if (checkIndex >= NUM_SENSORS) {
        checkIndex = 0;
    }

    uint8_t *rom = sensors[checkIndex].rom;

    delay(1000);

    //select ROM address
    // Get the temp
    one.reset();
    one.write(0x55);
    one.write_bytes(rom,8);
    one.write(0x44);
    delay(10);

    //ask for the temperature from
    one.reset();
    one.write(0x55);
    one.write_bytes(rom, 8);
    one.write(0xBE);
    one.read_bytes(resp, 9);



    byte MSB = resp[1];
    byte LSB = resp[0];

    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    float TemperatureSum = tempRead / 16;

    //Multiply by 9, then divide by 5, then add 32
    float fahrenheit =  ((TemperatureSum * 9) / 5) + 32;

    if (fahrenheit > 7000) {
        fahrenheit = 7404 - fahrenheit;
    }
    sensors[checkIndex].value = fahrenheit;


    Serial.print("Thermometer ID: ");
    Serial.println(sensors[checkIndex].id);

    Serial.println("Value: " + String(fahrenheit));


    unsigned int now = millis();

    if ((now - sensors[checkIndex].updated) > 60000) {
        sprintf(tempfStr, "%f", sensors[checkIndex].value);

        Spark.publish(String("Temperature/") + sensors[checkIndex].id,  tempfStr );
        sensors[checkIndex].updated = now;
    }

    checkIndex++;

}
*/
