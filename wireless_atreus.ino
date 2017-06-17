#include <SPI.h>
#include <SoftwareSerial.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "bluefruit-config.h"
#include "hid-keycode.h"

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

int layers[2][4][11] = {
  {// layer 0
    {K_Q, K_W, K_E, K_R, K_T, K_NONE, K_Y, K_U, K_I, K_O, K_P},
    {K_A,K_S,K_D,K_F,K_G,K_NONE ,K_H,K_J,K_K,K_L,K_SEMICOLON},
    {K_Z, K_X, K_C, K_V, K_B, K_CONTROL_LEFT, K_N, K_M, K_COMMA, K_PERIOD, K_SLASH},
    {K_ESCAPE, K_TAB, K_GUI_LEFT, K_SHIFT_LEFT, K_BACKSPACE, K_ALT_LEFT,K_SPACE,K_NONE,K_MINUS,K_APOSTROPHE,K_RETURN},
  },{// layer 1
    /*
     *  !    @     up     {    }        ||     pgup    7     8     9    *
     *  #  left   down  right  $        ||     pgdn    4     5     6    +
     *  [    ]      (     )    &        ||       `     1     2     3    \
     * L2  insert super shift bksp ctrl || alt space   fn    .     0    =
     */
    {K_NONE, K_W, K_E, K_R, K_T, K_NONE, K_Y, K_U, K_I, K_O, K_P},
    {K_A,K_S,K_D,K_F,K_G,K_NONE ,K_H,K_J,K_K,K_L,K_SEMICOLON},
    {K_Z, K_X, K_C, K_V, K_B, K_CONTROL_LEFT, K_N, K_M, K_COMMA, K_PERIOD, K_SLASH},
    {K_ESCAPE, K_TAB, K_GUI_LEFT, K_SHIFT_LEFT, K_BACKSPACE, K_ALT_LEFT,K_SPACE,K_NONE,K_MINUS,K_APOSTROPHE,K_RETURN},
  }
} ;

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

typedef struct
{
uint8_t modifier;   /**< Keyboard modifier keys  */
uint8_t reserved;   /**< Reserved for OEM use, always set to 0. */
uint8_t keycode[6]; /**< Key codes of the currently pressed keys. */
} hid_keyboard_report_t;

// I know I could construct these as literal ints, but I like having them named for purposes of easy debugging

int columns[] = {col_0, col_1, col_2, col_3, col_4, col_5, col_6, col_7, col_8, col_9, col_10};

int rows[] = {row_0, row_1, row_2, row_3};


bool key_state;

void setup() {
  Serial.begin(57600);
  Serial.println("hello!");
}

void loop() {
  for (size_t column = 0; column < 11; column++) { // column loop
    for (size_t row = 0; row < 4; row++) {
      // Serial.print("Col: ") ;
      // Serial.print(column) ;
      // Serial.print(" row: ") ;
      // Serial.print(row);
      // Serial.println("");
      pinMode(rows[row], OUTPUT);
      pinMode(columns[column], INPUT_PULLUP);
      digitalWrite(rows[row], LOW);
      key_state = digitalRead(columns[column]);
      if (!key_state) {
        Serial.println(layers[0][row][column]);
        Serial.print("row:");
        Serial.print(row);
        Serial.print(" column:");
        Serial.print(column);
        Serial.println(" ");
        delay(150); // simple debounce; should probably do something better / at least copy from TMK
      }
      pinMode(rows[row], INPUT);
      pinMode(columns[column], INPUT);
    }
  }
}

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the


// Report that send to Central every scanning period
hid_keyboard_report_t keyReport = { 0, 0, { 0 } };

// Report sent previously. This is used to prevent sending the same report over time.
// Notes: HID Central intepretes no new report as no changes, which is the same as
// sending very same report multiple times. This will help to reduce traffic especially
// when most of the time there is no keys pressed.
// - Init to different with keyReport
hid_keyboard_report_t previousReport = { 0, 0, { 1 } };


/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
// void setup(void)
// {
//   //while (!Serial);  // required for Flora & Micro
//   delay(500);
//
//   Serial.begin(115200);
//   Serial.println(F("Adafruit Bluefruit HID Keyboard Example"));
//   Serial.println(F("---------------------------------------"));
//
//   /* Initialise the module */
//   Serial.print(F("Initialising the Bluefruit LE module: "));
//
//   if ( !ble.begin(VERBOSE_MODE) )
//   {
//     error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
//   }
//   Serial.println( F("OK!") );
//
//   if ( FACTORYRESET_ENABLE )
//   {
//     /* Perform a factory reset to make sure everything is in a known state */
//     Serial.println(F("Performing a factory reset: "));
//     ble.factoryReset();
//   }
//
//   /* Disable command echo from Bluefruit */
//   ble.echo(false);
//
//   Serial.println("Requesting Bluefruit info:");
//   /* Print Bluefruit information */
//   ble.info();
//
//   /* Enable HID Service if not enabled */
//   int32_t hid_en = 0;
//
//   ble.sendCommandWithIntReply( F("AT+BleHIDEn"), &hid_en);
//
//   if ( !hid_en )
//   {
//     Serial.println(F("Enable HID Service (including Keyboard): "));
//     ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ));
//
//     /* Add or remove service requires a reset */
//     Serial.println(F("Performing a SW reset (service changes require a reset): "));
//     !ble.reset();
//   }
//
//   Serial.println();
//   Serial.println(F("Go to your phone's Bluetooth settings to pair your device"));
//   Serial.println(F("then open an application that accepts keyboard input"));
//   Serial.println();
//
//   // Set up input Pins
//   for(int i=0; i< 6; i++)
//   {
//     pinMode(inputPins[i], INPUT_PULLUP);
//   }
// }
//
// /**************************************************************************/
// /*!
//     @brief  Constantly poll for new command or response data
// */
// /**************************************************************************/
// void loop(void)
// {
//   /* scan GPIO, since each report can has up to 6 keys
//    * we can just assign a slot in the report for each GPIO
//    */
//   if ( ble.isConnected() )
//   {
//     for(int i=0; i<6; i++)
//     {
//       // GPIO is active low
//       if ( digitalRead(inputPins[i]) == LOW )
//       {
//         keyReport.keycode[i] = inputKeycodes[i];
//       }else
//       {
//         keyReport.keycode[i] = 0;
//       }
//
//       // Only send if it is not the same as previous report
//       if ( memcmp(&previousReport, &keyReport, 8) )
//       {
//         // Send keyboard report
//         ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 8);
//
//         // copy to previousReport
//         memcpy(&previousReport, &keyReport, 8);
//       }
//     }
//   }
//
//   // scaning period is 10 ms
//   delay(10);
// }
