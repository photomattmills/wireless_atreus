#include <SPI.h>
//#include <SoftwareSerial.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLEBattery.h"

#include "bluefruit-config.h"
#include "hid-keycode.h"

//#define VBATPIN A9

typedef struct
{
  uint8_t modifier;   /**< Keyboard modifier keys  */
  uint8_t reserved;   /**< Reserved for OEM use, always set to 0. */
  uint8_t keycode[6]; /**< Key codes of the currently pressed keys. */
} hid_keyboard_report_t;

int row_0 = 13;
int row_1 = 12;
int row_2 = 11;
int row_3 = 0;
int row_4 = 1;
int row_5 = 38;
int row_6 = 20;
int row_7 = 21;
int row_8 = 5;
int row_9 = 6;
int row_10 = 9;
int row_11 = 10;

int col_0 = 14;
int col_1 = 15;
int col_2 = 16;
int col_3 = 17;
int col_4 = 18;
int col_5 = 19;



// I know I could construct these as literal ints, but I like having them named for purposes of easy debugging

int columns[] = {col_0, col_1, col_2, col_3, col_4, col_5};

int rows[] = {row_0, row_1, row_2, row_3, row_4, row_5, row_6, row_7, row_8, row_9, row_10, row_11};

int battery_percent;

typedef struct {
  uint8_t modifier;
  uint8_t keycode;
} key_stroke_t;

// The 'keys' in this keymap have a modifier and a keycode; I tried doing this as a struct but the compiler got mad. :shrug:
int layers[12][6][2] = {
  {
    {K_NONE, K_EQUAL}, {K_NONE, K_TAB}, {K_CONTROL_LEFT, K_NONE},{K_SHIFT_LEFT, K_NONE}, {K_NONE, K_ESCAPE}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_1},{K_NONE, K_Q},{K_NONE, K_A},{K_NONE, K_Z},{K_NONE, K_GRAVE}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_2},{K_NONE, K_W},{K_NONE, K_S},{K_NONE, K_X},{K_NONE, K_BRACKET_LEFT}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_3},{K_NONE, K_E},{K_NONE, K_D},{K_NONE, K_C},{K_NONE, K_BRACKET_RIGHT}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_4},{K_NONE, K_R},{K_NONE, K_F},{K_NONE, K_V},{K_SHIFT_LEFT, K_NONE}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_5},{K_NONE, K_T},{K_NONE, K_G},{K_NONE, K_B},{K_GUI_LEFT, K_NONE}, {K_NONE, K_BACKSPACE}
  },{
    {K_NONE, K_6},{K_NONE, K_Y},{K_NONE, K_H},{K_NONE, K_N},{K_NONE, K_SPACE}, {K_NONE, K_RETURN}
  },{
    {K_NONE, K_7},{K_NONE, K_U},{K_NONE, K_J},{K_NONE, K_M},{K_NONE, K_ARROW_LEFT}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_8},{K_NONE, K_I},{K_NONE, K_K},{K_NONE, K_COMMA},{K_NONE, K_ARROW_UP}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_9},{K_NONE, K_O},{K_NONE, K_L},{K_NONE, K_PERIOD},{K_NONE, K_ARROW_RIGHT}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_0},{K_NONE, K_P},{K_NONE, K_SEMICOLON},{K_NONE, K_SLASH},{K_NONE, K_ARROW_DOWN}, {K_NONE, K_NONE}
  },{
    {K_NONE, K_MINUS},{K_NONE, K_BACKSLASH},{K_NONE, K_APOSTROPHE},{K_SHIFT_LEFT, K_NONE},{K_ALT_LEFT, K_NONE}, {K_NONE, K_NONE}
  }
};

hid_keyboard_report_t keyReport = { 0, 0, { 0 } };

// Used to re-initalize the report when keys are released/change
hid_keyboard_report_t previousReport = { 0, 0, { 0 } };

/*
central device won't be able to reconnect.
MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
-----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE        0
/*=========================================================================*/


/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
// Adafruit_BLEBattery battery(ble);

// A small helper
void error(const __FlashStringHelper*err) {
Serial.println(err);
while (1);
}


bool key_state;

void setup() {
  delay(1000);

  Serial.begin(57600);
  Serial.println("Hello");

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  Serial.println( F("OK!") );
  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=Matt's iPad KB" )) ) {
    error(F("Could not set device name?"));
  }

  /* Enable HID Service i6f not enabled */
  // int32_t hid_en = 1;
  // ble.atcommand("AT+GATTCLEAR");
  // ble.atcommand("AT+GATTADDSERVICE=UUID=0x180F");
  // ble.atcommand("AT+GATTADDCHAR=UUID=0x2A19,PROPERTIES=0x10,MIN_LEN=1,VALUE=100");
  // ble.atcommand("AT+GATTCHAR=1,55");
  // !ble.reset();

  // ble.sendCommandWithIntReply( F("AT+BleHIDEn"), &hid_en);
  //
  // if ( !hid_en )
  // {
  //   Serial.println(F("Enable HID Service (including Keyboard): "));
  //   ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ));
  //
  //   /* Add or remove service requires a reset */
  //   Serial.println(F("Performing a SW reset (service changes require a reset): "));
  //   !ble.reset();
  // }
  // ble.atcommand( F("AT+BleBATTEn=On") );
  // !ble.reset();
  /* Enable Battery service if not enabled */

  // battery.begin(true);

// Add a battery service (UUID = 0x180F) to the peripheral

// Add a battery measurement characteristic (UUID = 0x2A19), notify enabled
}

void loop() {
  if ( ble.isConnected() ){
    get_battery();
    get_keys();
    ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 8);
    keyReport = previousReport;
  }
}

void get_battery(){}

void get_keys() {
  int index = 0;
  int layer = key_pressed(3,7) ? 1 : 0;
  for (size_t column = 0; column < 6; column++) { // column loop
    for (size_t row = 0; row < 12; row++) {
        bool state = key_pressed(row, column);
        if (state) {
          // Serial.print("column: ");
          // Serial.print(column);
          // Serial.print(", row: ");
          // Serial.println(row);
         if ( ble.isConnected() && index < 6)
         {
           int* key_code = layers[row][column];
           keyReport.modifier = modifier(key_code[0]);
           keyReport.keycode[index] = key_code[1];
           (key_code[1] != 0) ? (index++) : 0;
         }

        // pinMode(rows[row], INPUT); //
        // pinMode(columns[column], INPUT); // we turn off the pullup to save power between cycles
      }
    }
  }
}

bool key_pressed(uint8_t row, uint8_t column){
  pinMode(rows[row], OUTPUT);
  pinMode(columns[column], INPUT_PULLUP);
  digitalWrite(rows[row], LOW);
  key_state = digitalRead(columns[column]);
  pinMode(rows[row], INPUT); //
  pinMode(columns[column], INPUT); // we turn off the pullup to save power between cycles
  return !key_state;
}

int modifier(int code){
  int mod = 0;
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
