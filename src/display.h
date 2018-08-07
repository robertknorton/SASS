/*
Author: Robert Norton
Project: Senior Design Project: Smart Automated Shower System (SASS)
Date Last Updated: 7/7/2018

Using GUISlice version 0.10.0

ARDUINO NOTES:
- GUIslice_config.h must be edited to match the pinout connections
between the Arduino CPU and the display controller (see ADAGFX_PIN_*).
   */

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

// Adafruit Display includes
#include <GUIslice.h>
#include <GUIslice_ex.h>
#include <GUIslice_drv.h>
#include <Adafruit_GFX.h>

// To demonstrate additional fonts, uncomment the following line:
#define USE_EXTRA_FONTS

#ifdef USE_EXTRA_FONTS
  // Note that these files are located within the Adafruit-GFX library folder:
  #include "Fonts/FreeSansBold12pt7b.h"
#endif

// Enumerations for pages, elements, fonts, images
// Pages
enum {DISP_PG_MENU, DISP_PG_MAIN};
// Elements
enum {DISP_ELEM_BG_BOX,
      DISP_ELEM_BACK_BTN,
      //E_ELEM_TXT_COUNT,
      //E_ELEM_PROGRESS,
      DISP_ELEM_FLOW_PROGRESS,
      DISP_ELEM_TEMP_PROGRESS,
      //E_ELEM_CHECK1,
      DISP_ELEM_FLOW_TXT,
      DISP_ELEM_TEMP_TXT
      };
      // Fonts
enum {
        MENU_GENERAL_FONT,
        E_FONT_BTN,
        E_FONT_TXT,
        FLOW_PERCENT_FONT_TXT,
        TEMP_DEGREE_FONT_TXT
        };
// Group
enum {E_GROUP1};

bool        m_bQuit = false;

// Free-running counter for display
unsigned    m_nCount = 0;


// Instantiate the GUI
#define MAX_PAGE                3
#define MAX_FONT                5

// Define the maximum number of elements per page
#define MAX_ELEM_PG_MAIN          16                    // # Elems total
#define MAX_ELEM_PG_MAIN_RAM      MAX_ELEM_PG_MAIN      // # Elems in RAM

gslc_tsGui                  m_gui;
gslc_tsDriver               m_drv;
gslc_tsFont                 m_asFont[MAX_FONT];
gslc_tsPage                 m_asPage[MAX_PAGE];
gslc_tsElem                 m_asPageElem[MAX_ELEM_PG_MAIN_RAM];
gslc_tsElemRef              m_asPageElemRef[MAX_ELEM_PG_MAIN];

gslc_tsXGauge               m_sXGauge,FLOW_Gauge_obj,TEMP_Gauge_obj;
//gslc_tsXCheckbox            m_asXCheck[3];


#define MAX_STR             25

  // Save some element references for quick access
  // gslc_tsElemRef*  m_pElemCnt        = NULL;
  // gslc_tsElemRef*  m_pElemProgress   = NULL;
  // gslc_tsElemRef*  m_pElemCheckBox1  = NULL;
  gslc_tsElemRef*  ref_DISP_ELEM_FLOW_PROGRESS = NULL;
  gslc_tsElemRef*  ref_DISP_ELEM_TEMP_PROGRESS = NULL;
  gslc_tsElemRef*  ref_DISP_ELEM_FLOW_TXT      = NULL;
  gslc_tsElemRef*  ref_DISP_ELEM_TEMP_TXT      = NULL;
  gslc_tsElemRef*  ref_DISP_ELEM_BACK_BTN      = NULL;


// Define debug message function
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

// Button callbacks

// back Button
void Cb_BackBtn()
{
    // Go back to menu page
}

// Create page elements
bool InitOverlays()
{
    gslc_tsElemRef* ref_PLACEHOLDER;
    // Adds a menu page
    gslc_PageAdd(&m_gui,
               DISP_PG_MENU,
               m_asPageElem,
               MAX_ELEM_PG_MAIN_RAM,
               m_asPageElemRef,
               MAX_ELEM_PG_MAIN);
    // Adds a main page for shower
    gslc_PageAdd(&m_gui,
               DISP_PG_MAIN,
               m_asPageElem,
               MAX_ELEM_PG_MAIN_RAM,
               m_asPageElemRef,
               MAX_ELEM_PG_MAIN);

    gslc_FontAdd(&m_gui,
               FLOW_PERCENT_FONT_TXT,
               GSLC_FONTREF_PTR,
               NULL,
               5);

    gslc_FontAdd(&m_gui,
               TEMP_DEGREE_FONT_TXT,
               GSLC_FONTREF_PTR,
               NULL,
               5);

   gslc_FontAdd(&m_gui,
              MENU_GENERAL_FONT,
              GSLC_FONTREF_PTR,
              NULL,
              5);

    #define bgColor           GSLC_COL_GRAY_DK3
    #define textColor         GSLC_COL_GRAY_LT2
    #define outlineColor      GSLC_COL_GRAY_LT2

    // ===============================================================================================
    // MENU page
    // ===============================================================================================


    //   // Create counter
    // //  ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,DISP_PG_MAIN,(gslc_tsRect){120,60,50,10},
    // //    (char*)"",0,E_FONT_TXT);
    //   static char mstr1[8] = "";
    //   ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,E_ELEM_TXT_COUNT,DISP_PG_MAIN,(gslc_tsRect){210,80,50,10},mstr1,sizeof(mstr1),E_FONT_TXT);
    //   gslc_ElemSetCol(&m_gui,ref_PLACEHOLDER,outlineColor,bgColor,GSLC_COL_BLACK);
    //   gslc_ElemSetTxtCol(&m_gui,ref_PLACEHOLDER,textColor);
    //   m_pElemCnt = ref_PLACEHOLDER; // Save for quick access
    //
    //   // Create progress bar (horizontal)
    //   ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,DISP_PG_MAIN,(gslc_tsRect){20,80,50,10},(char*)"Distance(mm):",0,E_FONT_TXT);
    //   gslc_ElemSetCol(&m_gui,ref_PLACEHOLDER,GSLC_COL_ORANGE,bgColor,GSLC_COL_BLACK);
    //   gslc_ElemSetTxtCol(&m_gui,ref_PLACEHOLDER,textColor);
    //   ref_PLACEHOLDER = gslc_ElemXGaugeCreate(&m_gui,E_ELEM_PROGRESS,DISP_PG_MAIN,&m_sXGauge,(gslc_tsRect){100,80,100,10},0,1000,0,GSLC_COL_GREEN,false);
    //   gslc_ElemSetCol(&m_gui,ref_PLACEHOLDER,outlineColor,bgColor,GSLC_COL_BLACK);
    //   m_pElemProgress = ref_PLACEHOLDER; // Save for quick access
    //
    //   // Create checkbox 1
    //   ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,DISP_PG_MAIN,(gslc_tsRect){20,100,20,20},(char*)"Under 300mm:",0,E_FONT_TXT);
    //   gslc_ElemSetCol(&m_gui,ref_PLACEHOLDER,GSLC_COL_ORANGE,bgColor,GSLC_COL_BLACK);
    //   gslc_ElemSetTxtCol(&m_gui,ref_PLACEHOLDER,textColor);
    //   ref_PLACEHOLDER = gslc_ElemXCheckboxCreate(&m_gui,E_ELEM_CHECK1,DISP_PG_MAIN,&m_asXCheck[0],(gslc_tsRect){100,100,20,20},false,GSLCX_CHECKBOX_STYLE_X,GSLC_COL_BLUE_LT2,false);
    //   gslc_ElemSetCol(&m_gui,ref_PLACEHOLDER,outlineColor,bgColor,GSLC_COL_BLACK);
    //   m_pElemCheckBox1 = ref_PLACEHOLDER; // Save for quick access

    // ===============================================================================================
    // SHOWER page
    // ===============================================================================================
    // ref_PLACEHOLDER = gslc_ElemCreateBtnTxt_P(&m_gui,
    //                                           DISP_ELEM_BACK_BTN,
    //                                           DISP_PG_MAIN,
    //                                           45,   //nX
    //                                           80,   //nY
    //                                           30,   //nW
    //                                           30,   //nH
    //                                           'Back',
    //                                           MENU_GENERAL_FONT,
    //                                           GSLC_COL_WHITE,
    //                                           GSLC_COL_BLUE_LT3,
    //                                           GSLC_COL_YELLOW_DK,
    //                                           GSLC_COL_BLUE_LT1,
    //                                           GSLC_COL_YELLOW,
    //                                           GSLC_ALIGN_MID_MID,
    //                                           true,
    //                                           true,
    //                                           NULL,
    //                                           NULL);


    // ===============================================================================================
    // Background flat color
    gslc_SetBkgndColor(&m_gui,bgColor);

    // Create background box
    ref_PLACEHOLDER = gslc_ElemCreateBox(&m_gui,
                                     DISP_ELEM_BG_BOX,
                                     DISP_PG_MAIN,
                                     (gslc_tsRect){0,0,480,320});
    gslc_ElemSetCol(&m_gui,
                ref_PLACEHOLDER,
                GSLC_COL_BLACK,
                bgColor,
                GSLC_COL_BLACK);
    // ===============================================================================================
    // Back Button
    ref_PLACEHOLDER = gslc_ElemCreateBtnTxt(&m_gui,
                                            DISP_ELEM_BACK_BTN,
                                            DISP_PG_MAIN,
                                            (gslc_tsRect){175,240,130,60}, //{x,y,w,h}
                                            "Back",
                                            6,
                                            MENU_GENERAL_FONT,
                                            Cb_BackBtn); // GSLC_CB_TOUCH cbTouch
    gslc_ElemSetTxtAlign(&m_gui,
                         ref_PLACEHOLDER,
                         GSLC_ALIGN_MID_RIGHT);
    gslc_ElemSetCol(&m_gui,
                    ref_PLACEHOLDER,
                    GSLC_COL_BLUE_LT2,
                    GSLC_COL_BLUE_LT3,
                    GSLC_COL_BLUE_LT1);

    ref_DISP_ELEM_BACK_BTN = ref_PLACEHOLDER; // Save for quick access
    // ===============================================================================================
    // Water Flow Progress Bar (vertical)
    // - Demonstration of vertical bar with offset zero-pt showing both positive and negative range
    ref_PLACEHOLDER = gslc_ElemXGaugeCreate(&m_gui,
                                          DISP_ELEM_FLOW_PROGRESS,
                                          DISP_PG_MAIN,
                                          &FLOW_Gauge_obj,
                                          (gslc_tsRect){310,10,160,300},
                                          0,
                                          100,
                                          0,
                                          GSLC_COL_BLUE,
                                          true);

    gslc_ElemSetCol(&m_gui,
                    ref_PLACEHOLDER,
                    outlineColor,
                    GSLC_COL_GRAY,
                    GSLC_COL_BLACK);
    ref_DISP_ELEM_FLOW_PROGRESS = ref_PLACEHOLDER; // Save for quick access
    // ===============================================================================================
    // Water Flow Display Text
    //  ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,DISP_PG_MAIN,(gslc_tsRect){120,60,50,10},
    //    (char*)"",0,E_FONT_TXT);
    static char gui_flow_str[8] = "85%%";
    ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,
                                       DISP_ELEM_FLOW_TXT,
                                       DISP_PG_MAIN,
                                       (gslc_tsRect){365,35,50,10},
                                       gui_flow_str,
                                       sizeof(gui_flow_str),
                                       FLOW_PERCENT_FONT_TXT);
    gslc_ElemSetCol(&m_gui,
                  ref_PLACEHOLDER,
                  outlineColor,
                  GSLC_COL_GRAY,
                  GSLC_COL_BLACK);

    gslc_ElemSetTxtCol(&m_gui,
                     ref_PLACEHOLDER,
                     textColor);
    //gslc_ElemSetFillEn(&m_gui,ref_PLACEHOLDER,false);  // This slows down the program a ton
    gslc_ElemSetTxtAlign(&m_gui,
                       ref_PLACEHOLDER,
                       GSLC_ALIGN_MID_MID);
    ref_DISP_ELEM_FLOW_TXT = ref_PLACEHOLDER; // Save for quick access

    // ===============================================================================================
    // Water Temp Progress Bar (vertical)
    // - Demonstration of vertical bar with offset zero-pt showing both positive and negative range
    ref_PLACEHOLDER = gslc_ElemXGaugeCreate(&m_gui,
                                          DISP_ELEM_TEMP_PROGRESS,
                                          DISP_PG_MAIN,
                                          &TEMP_Gauge_obj,
                                          (gslc_tsRect){10,10,160,300},
                                          0,
                                          103,
                                          0,
                                          GSLC_COL_RED,
                                          true);
    gslc_ElemSetCol(&m_gui,
                  ref_PLACEHOLDER,
                  outlineColor,
                  GSLC_COL_GRAY,
                  GSLC_COL_BLACK);
    ref_DISP_ELEM_TEMP_PROGRESS = ref_PLACEHOLDER; // Save for quick access
    // ===============================================================================================
    // Water Temp Display Text
    //  ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,DISP_PG_MAIN,(gslc_tsRect){120,60,50,10},
    //    (char*)"",0,E_FONT_TXT);
    static char gui_temperature_str[8] = "110";
    ref_PLACEHOLDER = gslc_ElemCreateTxt(&m_gui,
      DISP_ELEM_TEMP_TXT,
      DISP_PG_MAIN,
      (gslc_tsRect){60,35,50,10},
      gui_temperature_str,
      sizeof(gui_temperature_str),
      TEMP_DEGREE_FONT_TXT);

    gslc_ElemSetCol(&m_gui,
                  ref_PLACEHOLDER,
                  outlineColor,
                  GSLC_COL_GRAY,
                  GSLC_COL_BLACK);

    gslc_ElemSetTxtCol(&m_gui,
                     ref_PLACEHOLDER,
                     textColor);
    //gslc_ElemSetFillEn(&m_gui,ref_PLACEHOLDER,false);  // This slows down the program a ton
    gslc_ElemSetTxtAlign(&m_gui,
                       ref_PLACEHOLDER,
                       GSLC_ALIGN_MID_MID);
    ref_DISP_ELEM_TEMP_TXT = ref_PLACEHOLDER; // Save for quick access
    // ===============================================================================================
    return true;
}
