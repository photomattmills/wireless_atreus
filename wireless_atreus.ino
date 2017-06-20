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
*  !    @     up      {    }        ||     pgup    7     8     9    *
 *  #  left   down  right  $        ||     pgdn    4     5     6    +
 *  [    ]      (     )    &        ||       `     1     2     3    \
 * L2  insert super shift bksp ctrl || alt space   fn    .     0    =
 */

typedef struct {
  uint8_t modifier;
  uint8_t keycode;
} key_stroke_t;

// key_stroke_t shift(uint8_t key){
//   return {K_SHIFT_LEFT, key};
// }

int layers[2][4][11][2] = {
  {// layer 0
    {{K_NONE,K_Q}, {K_NONE,K_W}, {K_NONE,K_E}, {K_NONE,K_R}, {K_NONE,K_T}, {K_NONE,K_NONE}, {K_NONE,K_Y}, {K_NONE,K_U}, {K_NONE,K_I}, {K_NONE,K_O}, {K_NONE,K_P}},
    {{K_NONE,K_A}, {K_NONE,K_S}, {K_NONE,K_D}, {K_NONE,K_F}, {K_NONE,K_G}, {K_NONE,K_NONE}, {K_NONE,K_H}, {K_NONE,K_J}, {K_NONE,K_K}, {K_NONE,K_L}, {K_NONE,K_SEMICOLON}},
    {{K_NONE,K_Z}, {K_NONE,K_X}, {K_NONE,K_C}, {K_NONE,K_V}, {K_NONE,K_B}, {K_CONTROL_LEFT, K_NONE}, {K_NONE,K_N}, {K_NONE,K_M}, {K_NONE,K_COMMA}, {K_NONE,K_PERIOD}, {K_NONE,K_SLASH}},
    {{K_NONE,K_ESCAPE}, {K_NONE,K_TAB}, {K_GUI_LEFT,K_NONE}, {K_SHIFT_LEFT,K_NONE}, {K_NONE,K_BACKSPACE}, {K_ALT_LEFT, K_NONE}, {K_NONE,K_SPACE},{K_NONE,K_NONE},{K_NONE,K_MINUS},{K_NONE,K_APOSTROPHE},{K_NONE,K_RETURN}},
  },{
     {
       {K_SHIFT_LEFT,K_1}, {K_SHIFT_LEFT,K_2}, {K_NONE,K_ARROW_UP}, {K_SHIFT_LEFT,K_BRACKET_LEFT}, {K_SHIFT_LEFT,K_BRACKET_RIGHT}, {K_NONE,K_NONE}, {K_NONE,K_PAGE_UP}, {K_NONE,K_7}, {K_NONE,K_8}, {K_NONE,K_9}, {K_SHIFT_LEFT,K_8}
     },{
       {K_SHIFT_LEFT,K_3}, {K_NONE,K_ARROW_LEFT}, {K_NONE,K_ARROW_DOWN}, {K_NONE,K_ARROW_RIGHT}, {K_SHIFT_LEFT,K_4},{K_NONE,K_NONE}, {K_NONE,K_PAGE_DOWN}, {K_NONE,K_4},{K_NONE,K_5},{K_NONE,K_6},{K_SHIFT_LEFT,K_EQUAL}
     },{
        {K_NONE,K_BRACKET_LEFT}, {K_NONE,K_BRACKET_RIGHT}, {K_SHIFT_LEFT,K_9}, {K_SHIFT_LEFT,K_0}, {K_SHIFT_LEFT,K_7}, {K_CONTROL_LEFT, K_NONE}, {K_NONE,K_GRAVE}, {K_NONE,K_1}, {K_NONE,K_2}, {K_NONE,K_3}, {K_NONE,K_BACKSLASH}
     },{
       {K_NONE,K_ESCAPE}, {K_NONE,K_TAB}, {K_GUI_LEFT,K_NONE}, {K_SHIFT_LEFT,K_NONE}, {K_NONE,K_BACKSPACE}, {K_ALT_LEFT, K_NONE}, {K_NONE,K_SPACE}, {K_NONE,K_NONE}, {K_NONE,K_PERIOD}, {K_NONE,K_0}, {K_NONE,K_KEYPAD_EQUAL}
     }
  }
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
  int layer = key_pressed(3,7) ? 1 : 0;
  for (size_t column = 0; column < 11; column++) { // column loop
    for (size_t row = 0; row < 4; row++) {
      bool state = key_pressed(row, column);
      if (state) {
        if ( ble.isConnected() )
        {
          int* key_code = layers[layer][row][column];
          keyReport.modifier = modifier(key_code[0]);
          keyReport.keycode[index] = key_code[1];
          (key_code[1] != 0) ? (index++) : 0;
        }
      }
      pinMode(rows[row], INPUT); //
      pinMode(columns[column], INPUT); // we turn off the pullup to save power between cycles
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
