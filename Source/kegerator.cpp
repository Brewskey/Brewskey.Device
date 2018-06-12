#include "application.h"

//#define DEBUG 100
//#define NDEF_DEBUG 1
#include "Tappt/Pins.h"
#include "Tappt/Display/Display.h"
#include "Tappt/led/LED.h"
#include "Tappt/KegeratorStateMachine/KegeratorStateMachine.h"
#include "Tappt/NfcClient/NfcClient.h"
#include "Tappt/Sensors/Sensors.h"
#include "Tappt/ServerLink/ServerLink.h"
#include "Tappt/TapptTimer/TapptTimer.h"
#include "TOTP/TOTP.h"

#ifdef TEST_MODE
SYSTEM_MODE(SEMI_AUTOMATIC);
#endif

PRODUCT_ID(BREWSKEY_PRODUCT_ID);
PRODUCT_VERSION(BREWSKEY_PRODUCT_VERSION);

#define MILLISECONDS_IN_DAY 86400000

Display* display;
LED led;
KegeratorStateMachine* stateMachine;
NfcClient* nfcClient;
#ifdef EXPANSION_BOX_PIN
PacketReader reader;
Sensors sensors = Sensors(reader);
#else
Sensors sensors = Sensors();
#endif
TapptTimer timeSync = TapptTimer(MILLISECONDS_IN_DAY);

void setup(void) {
  RGB.control(true);
  RGB.color(255, 255, 255);
  display = new Display();
  Serial.begin(115200);
  Serial1.begin(19200);
//Serial1.halfDuplex(true);
  Serial.println("Starting");

  // while(!Serial.available()) {
  //  Spark.process();
  // }

  // while (Serial1.available()) {
  //   Serial1.read();
  // }

  nfcClient = new NfcClient();
  stateMachine = new KegeratorStateMachine(display, nfcClient, &sensors);

  #ifdef TEST_MODE
  stateMachine->TestInitialization(
    "~2~asdfasdfasdf~1,2,3,4,5,~1~10313,10313,10313,10313,10313,"
  );
  #endif
}

void loop(void) {
  stateMachine->Tick();
  timeSync.Tick();

  if (timeSync.ShouldTrigger()) {
    Particle.syncTime();
  }
}

#ifdef EXPANSION_BOX_PIN
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
  sensors.ReadMultitap();
}
#endif
