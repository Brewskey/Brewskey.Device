#include "application.h"

//#define DEBUG 100
//#define NDEF_DEBUG 1
#include "Tappt/Pins.h"
#include "Tappt/Display/Display.h"
#include "Tappt/led/LED.h"
#include "Tappt/KegeratorState/KegeratorState.h"
#include "Tappt/NfcClient/NfcClient.h"
#include "Tappt/Sensors/Sensors.h"
#include "Tappt/ServerLink/ServerLink.h"
#include "Tappt/TapptTimer/TapptTimer.h"
#include "TOTP/TOTP.h"

PRODUCT_ID(BREWSKEY_PRODUCT_ID);
PRODUCT_VERSION(BREWSKEY_PRODUCT_VERSION);

#define MILLISECONDS_IN_DAY 86400000

Display* display;
LED led;
KegeratorState* state;
NfcClient* nfcClient;
PacketReader reader;
Sensors sensors = Sensors(reader);
TapptTimer timeSync = TapptTimer(MILLISECONDS_IN_DAY);

void setup(void) {
  RGB.control(true);
  RGB.color(255, 255, 255);
  display = new Display();
  Serial.begin(115200);
  Serial1.begin(19200);
  //Serial1.halfDuplex(true);

  Serial.println("Starting");
  /*
      while(!Serial.available()) {
        Spark.process();
      }
  */
  while (Serial1.available()) {
    Serial1.read();
  }

  nfcClient = new NfcClient();
  state = new KegeratorState(display, nfcClient, &sensors);
}

void loop(void) {
  state->Tick();
  timeSync.Tick();

  if (timeSync.ShouldTrigger()) {
    Particle.syncTime();
  }
}

// Override the Serial1 buffer we use to communicate with expansion boxes
#define SERIA1_BUFFER_SIZE 129
HAL_USB_USART_Config acquireUSBSerial1Buffer()
{
  HAL_USB_USART_Config conf = { 0 };

  // The usable buffer size will be 128
  static uint8_t usbserial1_rx_buffer[SERIA1_BUFFER_SIZE];
  static uint8_t usbserial1_tx_buffer[SERIA1_BUFFER_SIZE];

  conf.rx_buffer = usbserial1_rx_buffer;
  conf.tx_buffer = usbserial1_tx_buffer;
  conf.rx_buffer_size = SERIA1_BUFFER_SIZE;
  conf.tx_buffer_size = SERIA1_BUFFER_SIZE;

  return conf;
}

void serialEvent1()
{
  // Make sure that we don't misread some packets when the device switches
  // states.
  // switch (state->GetState()) {
  //   case KegeratorState::INITIALIZING:
  //   case KegeratorState::CLEANING:
  //   case KegeratorState::UNLOCKED:
  //   case KegeratorState::INACTIVE: {
  //     while (Serial1.available()) {
  //       Serial1.read();
  //     }
  //     return;
  //   }
  // }
  sensors.ReadMultitap();
}
