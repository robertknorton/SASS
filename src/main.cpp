/*
Author: Robert Norton
Project: Senior Design Project: Smart Automated Shower System (SASS)
Date Last Updated: 5/18/2018

Using GUISlice version 0.10.0

ARDUINO NOTES:
- GUIslice_config.h must be edited to match the pinout connections
between the Arduino CPU and the display controller (see ADAGFX_PIN_*).
   */


#include <Arduino.h>
// Adafruit Display includes
#include <GUIslice.h>
#include <GUIslice_ex.h>
#include <GUIslice_drv.h>
#include <Adafruit_GFX.h>
// ToF sensor includes
#include "Adafruit_VL53L0X.h"
// Temp sensor includes
#include <OneWire.h>
#include <DallasTemperature.h>

/*
--------------------------------------------------------------------------------
   _____ _    _ _____   _____        __ _       _
  / ____| |  | |_   _| |  __ \      / _(_)     (_)
 | |  __| |  | | | |   | |  | | ___| |_ _ _ __  _  ___  ___
 | | |_ | |  | | | |   | |  | |/ _ \  _| | '_ \| |/ _ \/ __|
 | |__| | |__| |_| |_  | |__| |  __/ | | | | | | |  __/\__ \
  \_____|\____/|_____| |_____/ \___|_| |_|_| |_|_|\___||___/

--------------------------------------------------------------------------------
*/

// To demonstrate additional fonts, uncomment the following line:
#define USE_EXTRA_FONTS

#ifdef USE_EXTRA_FONTS
  // Note that these files are located within the Adafruit-GFX library folder:
  #include "Fonts/FreeSansBold12pt7b.h"
#endif

// Enumerations for pages, elements, fonts, images
enum {E_PG_MAIN};
enum {E_ELEM_BOX,E_ELEM_TXT_COUNT,E_ELEM_PROGRESS,E_ELEM_PROGRESS1,E_ELEM_CHECK1,E_FLOW_TXT_DISP};
enum {E_FONT_BTN,E_FONT_TXT,FLOW_PERCENT_FONT_TXT};
enum {E_GROUP1};

bool        m_bQuit = false;

// Free-running counter for display
unsigned    m_nCount = 0;


// Instantiate the GUI
#define MAX_PAGE                1
#define MAX_FONT                5

// Define the maximum number of elements per page
#define MAX_ELEM_PG_MAIN          16                                        // # Elems total
#define MAX_ELEM_PG_MAIN_RAM      MAX_ELEM_PG_MAIN                          // # Elems in RAM

gslc_tsGui                  m_gui;
gslc_tsDriver               m_drv;
gslc_tsFont                 m_asFont[MAX_FONT];
gslc_tsPage                 m_asPage[MAX_PAGE];
gslc_tsElem                 m_asPageElem[MAX_ELEM_PG_MAIN_RAM];
gslc_tsElemRef              m_asPageElemRef[MAX_ELEM_PG_MAIN];

gslc_tsXGauge               m_sXGauge,m_sXGauge1;
gslc_tsXGauge               m_sXRadial,m_sXRamp;
gslc_tsXCheckbox            m_asXCheck[3];


#define MAX_STR             25

  // Save some element references for quick access
  gslc_tsElemRef*  m_pElemCnt        = NULL;
  gslc_tsElemRef*  m_pElemProgress   = NULL;
  gslc_tsElemRef*  m_pElemRadialDistance   = NULL;
  gslc_tsElemRef*  m_pElemRamp   = NULL;
  gslc_tsElemRef*  m_pElemCheckBox1  = NULL;
  gslc_tsElemRef*  m_pElemProgress1 = NULL;
  gslc_tsElemRef*  m_pFlowDisp        = NULL;


// Define debug message function
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

// Button callbacks

// no call backs

// Create page elements
bool InitOverlays()
{
  gslc_tsElemRef* pElemRef;

  gslc_PageAdd(&m_gui,E_PG_MAIN,m_asPageElem,MAX_ELEM_PG_MAIN_RAM,m_asPageElemRef,MAX_ELEM_PG_MAIN);

  gslc_FontAdd(&m_gui,FLOW_PERCENT_FONT_TXT,GSLC_FONTREF_PTR,NULL,6);

#define bgColor           GSLC_COL_GRAY_DK3
#define textColor         GSLC_COL_GRAY_LT2
#define outlineColor      GSLC_COL_GRAY_LT2


  // Background flat color
  gslc_SetBkgndColor(&m_gui,bgColor);

  // Create background box
//  pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX,E_PG_MAIN,(gslc_tsRect){0,0,480,320});
//  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_WHITE,GSLC_COL_GRAY_LT2,GSLC_COL_BLACK);

  // Create counter
//  pElemRef = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_tsRect){120,60,50,10},
//    (char*)"",0,E_FONT_TXT);
  static char mstr1[8] = "";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TXT_COUNT,E_PG_MAIN,(gslc_tsRect){210,80,50,10},mstr1,sizeof(mstr1),E_FONT_TXT);
  gslc_ElemSetCol(&m_gui,pElemRef,outlineColor,bgColor,GSLC_COL_BLACK);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,textColor);
  m_pElemCnt = pElemRef; // Save for quick access

  // Create progress bar (horizontal)
  pElemRef = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_tsRect){20,80,50,10},(char*)"Distance(mm):",0,E_FONT_TXT);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_ORANGE,bgColor,GSLC_COL_BLACK);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,textColor);
  pElemRef = gslc_ElemXGaugeCreate(&m_gui,E_ELEM_PROGRESS,E_PG_MAIN,&m_sXGauge,(gslc_tsRect){100,80,100,10},0,1000,0,GSLC_COL_GREEN,false);
  gslc_ElemSetCol(&m_gui,pElemRef,outlineColor,bgColor,GSLC_COL_BLACK);
  m_pElemProgress = pElemRef; // Save for quick access

  // Second progress bar (vertical)
  // - Demonstration of vertical bar with offset zero-pt showing both positive and negative range
  pElemRef = gslc_ElemXGaugeCreate(&m_gui,E_ELEM_PROGRESS1,E_PG_MAIN,&m_sXGauge1,(gslc_tsRect){250,60,200,250},0,10,0,GSLC_COL_BLUE,true);
  gslc_ElemSetCol(&m_gui,pElemRef,outlineColor,GSLC_COL_GRAY,GSLC_COL_BLACK);
  m_pElemProgress1 = pElemRef; // Save for quick access

  // Create checkbox 1
  pElemRef = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_tsRect){20,100,20,20},(char*)"Under 300mm:",0,E_FONT_TXT);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_ORANGE,bgColor,GSLC_COL_BLACK);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,textColor);
  pElemRef = gslc_ElemXCheckboxCreate(&m_gui,E_ELEM_CHECK1,E_PG_MAIN,&m_asXCheck[0],(gslc_tsRect){100,100,20,20},false,GSLCX_CHECKBOX_STYLE_X,GSLC_COL_BLUE_LT2,false);
  gslc_ElemSetCol(&m_gui,pElemRef,outlineColor,bgColor,GSLC_COL_BLACK);
  m_pElemCheckBox1 = pElemRef; // Save for quick access

// Flow Display Text
//  pElemRef = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_tsRect){120,60,50,10},
//    (char*)"",0,E_FONT_TXT);
  static char gui_flow_str[8] = "85%";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_FLOW_TXT_DISP,E_PG_MAIN,(gslc_tsRect){320,180,50,10},gui_flow_str,sizeof(gui_flow_str),FLOW_PERCENT_FONT_TXT);
  gslc_ElemSetCol(&m_gui,pElemRef,outlineColor,GSLC_COL_GRAY,GSLC_COL_BLACK);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,textColor);
  //gslc_ElemSetFillEn(&m_gui,pElemRef,false);  // This slows down the program a ton
  gslc_ElemSetTxtAlign(&m_gui,pElemRef,GSLC_ALIGN_MID_MID);
  m_pFlowDisp = pElemRef; // Save for quick access

  return true;
}

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

void setup()
{
  // Buttons and Inputs
  pinMode(temp_set_down_button, INPUT);
  pinMode(temp_set_up_button, INPUT);
  pinMode(water_on_toggle, INPUT);
  pinMode(flow_control_toggle, INPUT);
  pinMode(flow_up_button, INPUT);
  pinMode(flow_down_button, INPUT);

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
  gslc_SetPageCur(&m_gui,E_PG_MAIN);

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
  int Set_Temp_Value = 80;
  int Set_Flow_Value = 0;
// Initialize input and input state variables
  int Set_Temp_Down_State = 0;
  int Set_Temp_Up_State = 0;
  int Set_Flow_Down_State = 0;
  int Set_Flow_Up_State = 0;
  int Water_On_State = 0;
  int Flow_Control_State = 0;

void loop()
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
      delay(100);
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
      delay(100);
    }
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
      delay(100);
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
      delay(100);
    }
  }
/********************************************************************/

  VL53L0X_RangingMeasurementData_t measure;

  char distMeasureTxt[MAX_STR]; // string for updating proximity distance range
  char flowMeasureTxt[MAX_STR]; // string for updating flow range

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

  // Update distance values from TOF on GUI
  // Limit the numerical sensor value of the ToF sensor from 0 to 1000
  if (ToF_Measurement>1000)
  {
    ToF_Measurement = 1000;
    snprintf(distMeasureTxt,MAX_STR,"%u",ToF_Measurement);
    gslc_ElemSetTxtStr(&m_gui,m_pElemCnt,distMeasureTxt);
  }
  else if (ToF_Measurement<0)
  {
    ToF_Measurement = 0;
    snprintf(distMeasureTxt,MAX_STR,"%u",ToF_Measurement);
    gslc_ElemSetTxtStr(&m_gui,m_pElemCnt,distMeasureTxt);
  }
  else
  {
    snprintf(distMeasureTxt,MAX_STR,"%u",ToF_Measurement);
    gslc_ElemSetTxtStr(&m_gui,m_pElemCnt,distMeasureTxt);
  }
    // do nothing

/********************************************************************/

  // Request and Update Temp Sensor
  if (millis() - tempTimer > tempCheckDelay)
  {
    tempTimer = millis(); // resets timer value
    tempSensor.requestTemperatures();
    Water_Temp_Measurement = (tempSensor.getTempCByIndex(0)*1.8)+32;
    Serial.print("Water Temperature is: ");
    Serial.println(Water_Temp_Measurement);
  }

  // Update elements on active page

  // Updating distance graph in gui
  gslc_ElemXGaugeUpdate(&m_gui,m_pElemProgress,(ToF_Measurement));

  // Updating Flow percentage graph in GUI
  gslc_ElemXGaugeUpdate(&m_gui,m_pElemProgress1,(Set_Flow_Value*2.5));

  // Updating Flow percentage text in GUI
  snprintf(flowMeasureTxt,MAX_STR,"%d%%",Set_Flow_Value);
  gslc_ElemSetTxtStr(&m_gui,m_pFlowDisp,flowMeasureTxt);

  // Setting the checkbox if the ToF sensor measures an object within 300mm
  if (ToF_Measurement<300)
  {
    //gslc_ElemXCheckboxSetState (gslc_tsGui pGui, gslc_tsElemRef pElemRef, bool bChecked)
    gslc_ElemXCheckboxSetState(&m_gui, m_pElemCheckBox1, true);
  }
  else
  {
    gslc_ElemXCheckboxSetState(&m_gui, m_pElemCheckBox1, false);
  }

  // Periodically call GUIslice update function
  gslc_Update(&m_gui);

  // Slow down updates
  delay(100);

}
