#include "application.h"

#define DEBUG 1

#include "NfcClient.h"

NfcClient* nfcClient;

void setup(void) {
    Serial.begin(115200);

    while(!Serial.available()) {
      Spark.process();
    }

    nfcClient = new NfcClient();
}

void loop(void) {
  nfcClient->Tick();

  Serial.println("Tick");
  delay(5000);
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
