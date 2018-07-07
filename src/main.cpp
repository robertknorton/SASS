/*
Author: Robert Norton
Project: Senior Design Project: Smart Automated Shower System (SASS)
Date Last Updated: 6/13/2018

Using GUISlice version 0.10.0

ARDUINO NOTES:
- GUIslice_config.h must be edited to match the pinout connections
between the Arduino CPU and the display controller (see ADAGFX_PIN_*).
   */

#include <Arduino.h>
// ToF sensor includes
#include "Adafruit_VL53L0X.h"
// Temp sensor includes
#include <OneWire.h>
#include <DallasTemperature.h>
// Servo motor includes
#include <Servo.h>
// GUIslice Defines for LCD display
#include "display.h"


void sweep(Servo myservo)
{
  int ustep = 0;
  for (ustep = 500; ustep <= 2500; ustep += 10)
  { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.writeMicroseconds(ustep);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (ustep = 2500; ustep >= 500; ustep -= 10)
  { // goes from 180 degrees to 0 degrees
    myservo.writeMicroseconds(ustep);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

// void updateServo(Servo myservo, char type, int val)
// {
//     int setVal = 0;
//     switch (type) {
//         case 't':
//             setVal = map(val, 0, 110, 500, 2500);
//             Serial.print(setVal);
//             myservo.writeMicroseconds(setVal);
//             break;
//         case 'f':
//             setVal = map(val, 0, 100, 500, 2500);
//             Serial.print(setVal);
//             myservo.writeMicroseconds(map(Set_Flow_Value, 0, 100, 500, 2500));
//             Serial.print(myservo.readMicroseconds());
//             break;
//     }
// }

/*
--------------------------------------------------------------------------------
                         _____      _
                        / ____|    | |
                       | (___   ___| |_ _   _ _ __
                        \___ \ / _ \ __| | | | '_ \
                        ____) |  __/ |_| |_| | |_) |
                       |_____/ \___|\__|\__,_| .__/
                                             | |
                                             |_|
--------------------------------------------------------------------------------
*/
const int temp_set_down_button =    28;
const int temp_set_up_button   =    30;
const int water_on_toggle      =    32;
const int flow_control_toggle  =    34;
const int flow_up_button       =    36;
const int flow_down_button     =    38;

Adafruit_VL53L0X lox = Adafruit_VL53L0X(); // create a TOF sensor object

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSensor(&oneWire);
/********************************************************************/
#define FLOW_SERVO_PIN 45
#define TEMP_SERVO_PIN 44
// Creating Servo objects
Servo servoFlow;
Servo servoTemp;

void setup()
{
  // Buttons and Inputs
  pinMode(temp_set_down_button, INPUT);
  pinMode(temp_set_up_button, INPUT);
  pinMode(water_on_toggle, INPUT);
  pinMode(flow_control_toggle, INPUT);
  pinMode(flow_up_button, INPUT);
  pinMode(flow_down_button, INPUT);

  // Servo motors Initialize
  servoFlow.attach(FLOW_SERVO_PIN);
  servoFlow.writeMicroseconds(500);
  servoTemp.attach(TEMP_SERVO_PIN);
  servoTemp.writeMicroseconds(500);

  Serial.begin(115200);

  // Initialize debug output for ToF sensor
  gslc_InitDebug(&DebugOut);
  delay(1000);  // NOTE: Some devices require a delay after Serial.begin() before serial port can be used
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  // Start water temperature sensor
  tempSensor.begin();

  // Initialize
  if (!gslc_Init(&m_gui,&m_drv,m_asPage,MAX_PAGE,m_asFont,MAX_FONT)) { return; }

  // Load Fonts
  #ifdef USE_EXTRA_FONTS
    // Demonstrate the use of additional fonts (must have #include)
    if (!gslc_FontAdd(&m_gui,E_FONT_BTN,GSLC_FONTREF_PTR,&FreeSansBold12pt7b,1)) { return; }
  #else
    // Use default font
    if (!gslc_FontAdd(&m_gui,E_FONT_BTN,GSLC_FONTREF_PTR,NULL,1)) { return; }
  #endif
  if (!gslc_FontAdd(&m_gui,E_FONT_TXT,GSLC_FONTREF_PTR,NULL,1)) { return; }

  // Create graphic elements
  InitOverlays();

  // Start up display on main page
  gslc_SetPageCur(&m_gui,DISP_PG_MAIN);

  m_bQuit = false;
}

/*
------------------------------------------------------------------------------------------
            __  __       _         _
           |  \/  |     (_)       | |
           | \  / | __ _ _ _ __   | |     ___   ___  _ __
           | |\/| |/ _` | | '_ \  | |    / _ \ / _ \| '_ \
           | |  | | (_| | | | | | | |___| (_) | (_) | |_) |
           |_|  |_|\__,_|_|_| |_| |______\___/ \___/| .__/
                                                    | |
                                                    |_|
------------------------------------------------------------------------------------------
*/
// The below integers are used for changing how often sensor values are checked
  const int proxCheckDelay = 1000;
  const int tempCheckDelay = 3000;
// Timers for checking when to check sensors
  long proxTimer = millis();
  long tempTimer = millis();
// Initialize final measurement variables
  int ToF_Measurement = NULL;
  int Water_Temp_Measurement = NULL;
// Initialize setting variables
  int Set_Temp_Value = 110;
  int Old_Set_Temp_Value = 110;
  int Set_Flow_Value = 0;
// Initialize input and input state variables
  int Set_Temp_Down_State = 0;
  int Set_Temp_Up_State = 0;
  int Set_Flow_Down_State = 0;
  int Set_Flow_Up_State = 0;
  int Water_On_State = 0;
  int old_Water_On_State = 0;
  int Flow_Control_State = 0;

  char distMeasureTxt[MAX_STR]; // string for updating proximity distance range
  char flowMeasureTxt[MAX_STR]; // string for updating flow range
  char offMeasureTxt[MAX_STR]; // string for updating flow range
  char tempMeasureTxt[MAX_STR]; // string for updating temp range

void loop()
{
// Initialize VL53L0X measurement data variable
VL53L0X_RangingMeasurementData_t measure;

/********************************************************************/
  // Check Water On Button and Flow Control Button Logic
Water_On_State = digitalRead(water_on_toggle);
Flow_Control_State = digitalRead(flow_control_toggle);
if (Water_On_State == HIGH) // Water On
{
  // Since water is on, request update from Temp Sensor
  if (millis() - tempTimer > tempCheckDelay)
  {
    tempTimer = millis(); // resets timer value
    tempSensor.requestTemperatures();
    Water_Temp_Measurement = (tempSensor.getTempCByIndex(0)*1.8)+32;
    Serial.print("Water Temperature is: ");
    Serial.println(Water_Temp_Measurement);
  }
  /********************************************************************/
    // Set Temperature Up Button Logic
    Set_Temp_Up_State = digitalRead(temp_set_up_button);
    if (Set_Temp_Up_State == HIGH)
    {
      if (Set_Temp_Value < 110)
      {
        Set_Temp_Value = ++Set_Temp_Value;
        Serial.print("Temp Set Value: "); Serial.println(Set_Temp_Value);
        delay(50);
      }
    }
  /********************************************************************/
    // Set Temperature Down Button Logic
    Set_Temp_Down_State = digitalRead(temp_set_down_button);
    if (Set_Temp_Down_State == HIGH)
    {
      if (Set_Temp_Value > 68)
      {
        Set_Temp_Value = --Set_Temp_Value;
        Serial.print("Temp Set Value: "); Serial.println(Set_Temp_Value);
        delay(50);
      }
    }
  /********************************************************************/
  /*
          Handle Flow controls logic
  */
  if (Flow_Control_State == HIGH) // Proximity Sensor On, manual flow control Off
  {
    // TODO - insert logic and assignment of Set_Flow_Value from proximity sensor
    if ((millis() - proxTimer) > proxCheckDelay)
    {
      proxTimer = millis(); // resets timer value
      Serial.print("Reading a measurement... ");
      lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
      ToF_Measurement = measure.RangeMilliMeter;

      if (measure.RangeStatus != 4) {  // phase failures have incorrect data
        Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
      } else {
        Serial.println(" out of range ");
      }
    }
  }
  else // Proximity Sensor Off, manual flow control On
  {
    /********************************************************************/
      // Set Flow Up Button Logic
      Set_Flow_Up_State = digitalRead(flow_up_button);
      if (Set_Flow_Up_State == HIGH)
      {
        if (Set_Flow_Value < 100)
        {
          Set_Flow_Value = Set_Flow_Value + 10;
          Serial.print("Flow Set Value: "); Serial.println(Set_Flow_Value);
          // Update Servo Positions
          servoFlow.writeMicroseconds(map(Set_Flow_Value, 0, 100, 500, 2500));
          delay(50);
        }
      }
    /********************************************************************/
      // Set Flow Down Button Logic
      Set_Flow_Down_State = digitalRead(flow_down_button);
      if (Set_Flow_Down_State == HIGH)
      {
        if (Set_Flow_Value > 0)
        {
          Set_Flow_Value = Set_Flow_Value - 10;
          Serial.print("Flow Set Value: "); Serial.println(Set_Flow_Value);
          // Update Servo Positions
          servoFlow.writeMicroseconds(map(Set_Flow_Value, 0, 100, 500, 2500));
          delay(50);
        }
      }
    /********************************************************************/
  }
}
else // Water Off
{
  Set_Flow_Value = 0;
  // TODO - add checkbox saying water is off
  // No input or changing of water flow is allowed.


  // Changing of water temp allowed. So when the user does turn on water
  // it will automaticlly be adjusted to that temperature
  /********************************************************************/
    // Set Temperature Up Button Logic
    Set_Temp_Up_State = digitalRead(temp_set_up_button);
    if (Set_Temp_Up_State == HIGH)
    {
      if (Set_Temp_Value < 110)
      {
        Set_Temp_Value = ++Set_Temp_Value;
        Serial.print("Temp Set Value: "); Serial.println(Set_Temp_Value);
        delay(50);
      }
    }
  /********************************************************************/
    // Set Temperature Down Button Logic
    Set_Temp_Down_State = digitalRead(temp_set_down_button);
    if (Set_Temp_Down_State == HIGH)
    {
      if (Set_Temp_Value > 68)
      {
        Set_Temp_Value = --Set_Temp_Value;
        Serial.print("Temp Set Value: "); Serial.println(Set_Temp_Value);
        delay(50);
      }
    }
  /********************************************************************/
}


  // Update distance values from TOF on GUI
  // Limit the numerical sensor value of the ToF sensor from 0 to 1000
  // if (ToF_Measurement>1000)
  // {
  //   ToF_Measurement = 1000;
  //   snprintf(distMeasureTxt,MAX_STR,"%u",ToF_Measurement);
  //   gslc_ElemSetTxtStr(&m_gui,m_pElemCnt,distMeasureTxt);
  // }
  // else if (ToF_Measurement<0)
  // {
  //   ToF_Measurement = 0;
  //   snprintf(distMeasureTxt,MAX_STR,"%u",ToF_Measurement);
  //   gslc_ElemSetTxtStr(&m_gui,m_pElemCnt,distMeasureTxt);
  // }
  // else
  // {
  //   snprintf(distMeasureTxt,MAX_STR,"%u",ToF_Measurement);
  //   gslc_ElemSetTxtStr(&m_gui,m_pElemCnt,distMeasureTxt);
  // }
    // do nothing

/********************************************************************/

  // Update elements on active page

  // Updating distance graph in gui
  //gslc_ElemXGaugeUpdate(&m_gui,m_pElemProgress,(ToF_Measurement));



  // Updating Flow percentage text in GUI  -  All of this nasty GUI resetting
  // is to ensure proper redrawing and get rid of artifacts
  if (Water_On_State == LOW && old_Water_On_State == HIGH)
  {
    gslc_ElemSetRedraw(&m_gui,ref_DISP_ELEM_FLOW_PROGRESS, GSLC_REDRAW_FULL);
    snprintf(flowMeasureTxt,MAX_STR,"%s","  ");
    gslc_ElemSetTxtStr(&m_gui,ref_DISP_ELEM_FLOW_TXT,flowMeasureTxt);
    gslc_ElemSetRedraw(&m_gui,ref_DISP_ELEM_FLOW_TXT, GSLC_REDRAW_FULL);
  }
  else if (Water_On_State == LOW)
  {
    snprintf(flowMeasureTxt,MAX_STR,"%s","OFF");
    gslc_ElemSetTxtStr(&m_gui,ref_DISP_ELEM_FLOW_TXT,flowMeasureTxt);
  }
  else if (Water_On_State == HIGH && old_Water_On_State == LOW)
  {
    gslc_ElemSetRedraw(&m_gui,ref_DISP_ELEM_FLOW_PROGRESS, GSLC_REDRAW_FULL);
    Set_Flow_Value = 0;
    snprintf(flowMeasureTxt,MAX_STR,"%d%%",Set_Flow_Value);
    gslc_ElemSetTxtStr(&m_gui,ref_DISP_ELEM_FLOW_TXT,flowMeasureTxt);
    gslc_ElemSetRedraw(&m_gui,ref_DISP_ELEM_FLOW_TXT, GSLC_REDRAW_FULL);
  }
  else
  {
    snprintf(flowMeasureTxt,MAX_STR,"%d%%",Set_Flow_Value);
    gslc_ElemSetTxtStr(&m_gui,ref_DISP_ELEM_FLOW_TXT,flowMeasureTxt);
  }
  old_Water_On_State = Water_On_State;

  // Updating Flow percentage graph in GUI
  gslc_ElemXGaugeUpdate(&m_gui,ref_DISP_ELEM_FLOW_PROGRESS,(Set_Flow_Value*2.5));

  // Updating Temp degree graph in GUI
  // Logic below is used for determining graph color
  if (Set_Temp_Value == 102 && (Old_Set_Temp_Value == 101 || Old_Set_Temp_Value == 103))
  {gslc_ElemXGaugeSetIndicator(&m_gui,ref_DISP_ELEM_TEMP_PROGRESS,GSLC_COL_RED,1,1,true);}
  else if (Set_Temp_Value == 100 && (Old_Set_Temp_Value == 101 || Old_Set_Temp_Value == 99))
  {gslc_ElemXGaugeSetIndicator(&m_gui,ref_DISP_ELEM_TEMP_PROGRESS,GSLC_COL_RED_LT1,1,1,true);}
  else if (Set_Temp_Value == 90 && (Old_Set_Temp_Value == 91 || Old_Set_Temp_Value == 89))
  {gslc_ElemXGaugeSetIndicator(&m_gui,ref_DISP_ELEM_TEMP_PROGRESS,GSLC_COL_RED_LT2,1,1,true);}
  else if (Set_Temp_Value == 80 && (Old_Set_Temp_Value == 81|| Old_Set_Temp_Value == 79))
  {gslc_ElemXGaugeSetIndicator(&m_gui,ref_DISP_ELEM_TEMP_PROGRESS,GSLC_COL_RED_LT3,1,1,true);}
  else if (Set_Temp_Value == 70 && (Old_Set_Temp_Value == 71|| Old_Set_Temp_Value == 89))
  {gslc_ElemXGaugeSetIndicator(&m_gui,ref_DISP_ELEM_TEMP_PROGRESS,GSLC_COL_RED_LT4,1,1,true);}
  else{ }
  // Updates the actual level of the verticle progress bar
  gslc_ElemXGaugeUpdate(&m_gui,ref_DISP_ELEM_TEMP_PROGRESS,(Set_Temp_Value*2.5));
  // Saves last Set_Temp_Value
  Old_Set_Temp_Value = Set_Temp_Value;

  // Updating Temp degree text in GUI
  snprintf(tempMeasureTxt,MAX_STR,"%d",Set_Temp_Value);
  gslc_ElemSetTxtStr(&m_gui,ref_DISP_ELEM_TEMP_TXT,tempMeasureTxt);

  // Setting the checkbox if the ToF sensor measures an object within 300mm
  // if (ToF_Measurement<300)
  // {
  //   //gslc_ElemXCheckboxSetState (gslc_tsGui pGui, gslc_tsElemRef pElemRef, bool bChecked)
  //   gslc_ElemXCheckboxSetState(&m_gui, m_pElemCheckBox1, true);
  // }
  // else
  // {
  //   gslc_ElemXCheckboxSetState(&m_gui, m_pElemCheckBox1, false);
  // }

  // Periodically call GUIslice update function
  gslc_Update(&m_gui);

  // Slow down updates
  //delay(50);

}
