#include "application.h"

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
  //nfcClient.Tick();

  Serial.println("Tick");
  delay(5000);
}
