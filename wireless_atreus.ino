#include <SPI.h>
#include <SoftwareSerial.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "bluefruit-config.h"
#include "hid-keycode.h"

typedef struct
{
  uint8_t modifier;   /**< Keyboard modifier keys  */
  uint8_t reserved;   /**< Reserved for OEM use, always set to 0. */
  uint8_t keycode[6]; /**< Key codes of the currently pressed keys. */
} hid_keyboard_report_t;

int row_0 = 10;
int row_1 = 9;
int row_2 = 6;
int row_3 = 5;

int col_0 = 13;
int col_1 = 12;
int col_2 = 0;
int col_3 = 3;
int col_4 = 2;
int col_5 = 11;

int col_8 = 18;
int col_7 = 19;
int col_6 = 20;
int col_10 = 21;
int col_9 = 22;

/* layer 1
 *  !    @     up     {    }        ||     pgup    7     8     9    *
 *  #  left   down  right  $        ||     pgdn    4     5     6    +
 *  [    ]      (     )    &        ||       `     1     2     3    \
 * L2  insert super shift bksp ctrl || alt space   fn    .     0    =
 */


int layers[4][11] =
  {// layer 0
    {K_Q, K_W, K_E, K_R, K_T, K_NONE, K_Y, K_U, K_I, K_O, K_P},
    {K_A, K_S, K_D, K_F, K_G, K_NONE, K_H, K_J, K_K, K_L, K_SEMICOLON},
    {K_Z, K_X, K_C, K_V, K_B, K_CONTROL_LEFT, K_N, K_M, K_COMMA, K_PERIOD, K_SLASH},
    {K_ESCAPE, K_TAB, K_GUI_LEFT, K_SHIFT_LEFT, K_BACKSPACE, K_ALT_LEFT,K_SPACE,K_NONE,K_MINUS,K_APOSTROPHE,K_RETURN},
  };

hid_keyboard_report_t keyReport = { 0, 0, { 0 } };

// Report sent previously. This is used to prevent sending the same report over time.
// Notes: HID Central intepretes no new report as no changes, which is the same as
// sending very same report multiple times. This will help to reduce traffic especially
// when most of the time there is no keys pressed.
// - Init to different with keyReport
hid_keyboard_report_t previousReport = { 0, 0, { 0 } };

/*
central device won't be able to reconnect.
MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
-----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE         0
/*=========================================================================*/


/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
Serial.println(err);
while (1);
}

// I know I could construct these as literal ints, but I like having them named for purposes of easy debugging

int columns[] = {col_0, col_1, col_2, col_3, col_4, col_5, col_6, col_7, col_8, col_9, col_10};

int rows[] = {row_0, row_1, row_2, row_3};


bool key_state;

void setup() {
  delay(500);

  Serial.begin(57600);
  Serial.println(F("Adafruit Bluefruit HID Keyboard Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Enable HID Service if not enabled */
  int32_t hid_en = 0;

  ble.sendCommandWithIntReply( F("AT+BleHIDEn"), &hid_en);

  if ( !hid_en )
  {
    Serial.println(F("Enable HID Service (including Keyboard): "));
    ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ));

    /* Add or remove service requires a reset */
    Serial.println(F("Performing a SW reset (service changes require a reset): "));
    !ble.reset();
  }

  Serial.println();
  Serial.println(F("Go to your phone's Bluetooth settings to pair your device"));
  Serial.println(F("then open an application that accepts keyboard input"));
  Serial.println();
}

void loop() {
  if ( ble.isConnected() ){
    get_keys();
    ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 8);
    keyReport = previousReport;
  }
}

void get_keys() {
  int index = 0;
  for (size_t column = 0; column < 11; column++) { // column loop
    for (size_t row = 0; row < 4; row++) {
      pinMode(rows[row], OUTPUT);
      pinMode(columns[column], INPUT_PULLUP);
      digitalWrite(rows[row], LOW);
      key_state = digitalRead(columns[column]);
      if (!key_state) {
        if ( ble.isConnected() )
        {
          int code = layers[row][column];
          if (code > 0xdf){
            keyReport.modifier = modifier(code);
          }else{
            keyReport.keycode[index] = layers[row][column];
            index++;
          }
        }
      }
      pinMode(rows[row], INPUT); //
      pinMode(columns[column], INPUT); // we turn off the pullup to save power between cycles
    }
  }
}

int modifier(int code){
  int mod;
  if (code == 0xe0) {
    mod = 0x01;
  } else if (code == 0xe1) {
    mod = 0x02;
  } else if (code == 0xe2) {
    mod = 0x04;
  } else if (code == 0xe3) {
    mod = 0x08;
  }
  return keyReport.modifier | mod;
}
