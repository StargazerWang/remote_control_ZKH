/******************************************************************************
 * @file     app_config.h
 *
 * @brief    for TLSR chips
 *
 * @author   public@telink-semi.com;
 * @date     Sep. 30, 2010
 *
 * @attention
 *
 *  Copyright (C) 2019-2020 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *****************************************************************************/

#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include  "application/audio/audio_common.h"
#include  "application/usbstd/usbkeycode.h"


/////////////////// FEATURE SELECT /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE                1
#define PM_DEEPSLEEP_RETENTION_ENABLE       1    // will have deep tim 480 ms
#define BLE_REMOTE_SECURITY_ENABLE          1
//#define BLE_REMOTE_OTA_ENABLE             1   // now we have 2 OTA, choose either one
    #define GOOGLE_OTA                      0   // 1: enable
    #define TELINK_OTA                      1   // 1: enable

#define REMOTE_IR_ENABLE                      1
#define BATT_CHECK_ENABLE                     1   //must enable
#define BLE_AUDIO_ENABLE                      1
#define BLT_APP_LED_ENABLE                    1
#define BLT_TEST_SOFT_TIMER_ENABLE            1
#if (BLT_TEST_SOFT_TIMER_ENABLE)
    #define BLT_SOFTWARE_TIMER_ENABLE         1
#endif

#define DEVICE_INFO                           0
#define UNUSED_GPIO_PULLDOWN_ENABLE           0
#define FIRMWARE_CHECK_ENABLE                 0
#define FIRMWARES_SIGNATURE_ENABLE            0
#define AUDIO_TRANS_USE_2M_PHY_ENABLE         0


//---------------------------- PCB --------------------------------------------

#define PCB_TELINK_827x                     1
#define PCB_R52                             2
#define PCB_VER                             PCB_R52//PCB_TELINK_827x//PCB_R52



//---------------------------- project model define --------------------------------------------
#define REMOTE_G10                            1
#define REMOTE_G20                            2
#define REMOTE_R52                            3

//---------------------------- project model define --------------------------------------------
#define ID_PRODUCT                          0x0000  //TBD
#define ID_VENDOR                           0x033D  //TPV

//---------------------------- test and debug mode for development --------------------------------------------
#define EQ_TEST                             0//1//disable ccc

#define MP_TEST_MODE                        0 // off test mode currently, write 0x73000 0x00 to enter test mode, default 0xFF is TEST MODE
// if PTM, will lose UART_PRINT_DEBUG_ENABLE, can be some inconvenience
#define PTM                                 0//TRUE // production test mode, anohter test mode other than MP_TEST_MODE
    #define PTM_FORCE_ENTER                 TRUE // force to enter production test mode, for quick test

#define AMBILIGHT_MODEL                     1  // ambilight model
//set all to 0 when release/MP
#define _KEY_2_ERASE_PAIR_                  0 //:disable


/////////////////// TEST MODE & DUT ////////////////////////////////
#if (MP_TEST_MODE)
#define MODE_TEST                             0xFF
#define MODE_USER                             0x00

#define DUT_INPUT_PORT                        GPIO_PB2
#define PB2_INPUT_ENABLE                      1
#endif
/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG                     DEEP_ANA_REG0 //u8,can save 8 bit info when deep
#define LOW_BATT_FLG                          BIT(0) //if 1: low battery
#define CONN_DEEP_FLG                         BIT(1) //if 1: conn deep, 0: adv deep
//#define IR_MODE_DEEP_FLG                      BIT(2) //if 1: IR mode, 0: BLE mode
#define LOW_BATT_SUSPEND_FLG                  BIT(3) //if 1 : low battery, < 1.8v
#define OTA_FLG                               BIT(4) //if 1: OTA SUCCESS
#define POWER_ON_FLG                          BIT(5)

#if (BATT_CHECK_ENABLE)
#if 1//(__PROJECT_8278_BLE_REMOTE__) use VBAT,8258 need to use a GPIO
    //use VBAT(8278) , then adc measure this VBAT voltage
    #define ADC_INPUT_PCHN                    VBAT    //corresponding  ADC_InputPchTypeDef in adc.h
#else
    //telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
    //use PB7(8258) output high level, then adc measure this high level voltage

    #define GPIO_VBAT_DETECT                  GPIO_PB7
    #define PB7_FUNC                          AS_GPIO
    #define PB7_INPUT_ENABLE                  0
    #define ADC_INPUT_PCHN                    B7P    //corresponding  ADC_InputPchTypeDef in adc.h

#endif
#endif



//---------------------------- APP IR --------------------------------------------
#define APP_IR_ENABLE                         FALSE//TRUE// IR codes from host TV, TODO: modulize this! to much dependency
#define IR_DATA_SECT_0_EXT_ADDR               0x65000
#define IR_DATA_SECT_1_EXT_ADDR               0x66000
#define IR_DATA_SECT_2_EXT_ADDR               0x67000
#define IR_DATA_SECT_3_EXT_ADDR               0x68000

#define IR_DATA_SECT_0_ADDR                   0x6c000
#define IR_DATA_SECT_1_ADDR                   0x6d000
#define IR_DATA_SECT_2_ADDR                   0x6e000
#define IR_DATA_SECT_3_ADDR                   0x6f000

#define IR_DATA_SECT_MAX_NUM                  12   //Each sector can be written 3 times ,4*3
#define IR_DATA_SECT_NUM                      4
#define IR_DATA_NEED_ERASE_NUM                6


//---------------------------- APP flash write --------------------------------------------
#define IR_KEY_EVENT_NOTIFY_SECT_ADDR         0x69000
#define WAKEUP_KEYINDEX_DATA_SECT_ADDR        0x6a000
#define CCC_DATA_AREA                         0x70000
#define MAC_DATA_SECT_ADDR                    0x71000
#define APP_CUSTOM_ADDR                       0x78000

//------------------------ LED CONFIG (RCU board) ----------------------------------------
#if (BLT_APP_LED_ENABLE)
#if (PCB_VER == PCB_TELINK_827x)
    //roll back led later as well
    #define LED_ON_LEVAL                      1             //gpio output high voltage to turn on led
    #define GPIO_LED                          GPIO_PC6// GPIO_PC6 PCBv1.1 , GPIO_PD6 G10/20
    #define PC6_FUNC                          AS_GPIO

    //#define GPIO_LED2                         GPIO_PD5
    //#define PD5_FUNC                          AS_GPIO
    //#define PD5_OUTPUT_ENABLE                 1
    //#define PD5_INPUT_ENABLE                  0
#elif (PCB_VER == PCB_R52)
    #define LED_ON_LEVAL                      0             //gpio output high voltage to turn on led
    #define GPIO_LED                          GPIO_PB5
    #define PB5_FUNC                          AS_GPIO
    #define GPIO_LED2                         GPIO_PB4
    #define PB4_FUNC                          AS_GPIO
#endif
#endif//#if (BLT_APP_LED_ENABLE)



////////////////////////// AUDIO CONFIG (RCU board) /////////////////////////////
#if (BLE_AUDIO_ENABLE)
    #define BLE_DMIC_ENABLE                   0  //0: Amic   1: Dmic
    #define IIR_FILTER_ENABLE                 0

//Dmic
    #if BLE_DMIC_ENABLE
        #define GPIO_DMIC_BIAS                GPIO_PB6
        #define GPIO_DMIC_DI                  GPIO_PA0
        #define GPIO_DMIC_CK                  GPIO_PA1
    #else
    #if (PCB_VER == PCB_R52) // R52 and telink use same pin
        #define GPIO_AMIC_BIAS                GPIO_PB6  //GPIO_PC0// need check ,v1.0 PC4, V1.1 PC0  V1.2 PB6 
        #define GPIO_AMIC_C1                  GPIO_PC1
    #endif
   #endif

    /* RCU Audio MODE:
     * TL_AUDIO_RCU_ADPCM_GATT_TLEINK
     * TL_AUDIO_RCU_ADPCM_GATT_GOOGLE
     * TL_AUDIO_RCU_ADPCM_HID
     * TL_AUDIO_RCU_SBC_HID                        //need config 32k retention
     * TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB
     * TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB          //need config 32k retention
     * TL_AUDIO_RCU_MSBC_HID                       //need config 32k retention
     */
    #define TL_AUDIO_MODE                     TL_AUDIO_RCU_ADPCM_GATT_GOOGLE
#endif

//PB3 IRout 100K pulldown when  IR not working,  when IR begin, disable this 100K pulldown
//#define PULL_WAKEUP_SRC_PB3                   PM_PIN_PULLDOWN_100K

//////////////////////////// KEYSCAN/MIC  GPIO //////////////////////////////////
#define MATRIX_ROW_PULL                       PM_PIN_PULLUP_1M//PM_PIN_PULLDOWN_100K//PM_PIN_PULLUP_1M
#define MATRIX_COL_PULL                       PM_PIN_PULLDOWN_100K//PM_PIN_PULLUP_10K//PM_PIN_PULLDOWN_100K

#define KB_LINE_HIGH_VALID                    1   //dirve pin output 0 when keyscan, scanpin read 0 is valid
#define DEEPBACK_FAST_KEYSCAN_ENABLE          0   //proc fast scan when deepsleep back trigged by key press, in case key loss
#define LONG_PRESS_KEY_POWER_OPTIMIZE         1   //lower power when pressing key without release

//stuck key
#define STUCK_KEY_PROCESS_ENABLE              1
#define STUCK_KEY_ENTERDEEP_TIME              30// spec 30s

//repeat key
#define KB_REPEAT_KEY_ENABLE                  0
#define KB_REPEAT_KEY_INTERVAL_MS             200
#define KB_REPEAT_KEY_NUM                     1
#define KB_MAP_REPEAT                         {VK_1, }


#define CR_VOL_UP                             0x80
#define CR_VOL_DN                             0x81


#define CR_MEDIA_KEY_INDEX                    CR_INFO// 0xe6

//G20
#define CR_INFO                               0xe6
#define CR_SUBTITLE                           0xe7
#define CR_RED                                0xe8
#define CR_GREEN                              0xe9
#define CR_YELLOW                             0xea
#define CR_BLUE                               0xeb

//G10
#define CR_YOUTUBE                            0xec
#define CR_NETFLIX                            0xed
#define CR_DISNEY                             0xee
#define CR_HBOMAX                             0xef
#define CR_UP                                 0xf0
#define CR_DN                                 0xf1
#define CR_LEFT                               0xf2
#define CR_RIGHT                              0xf3
#define CR_CENTER                             0xf4
#define CR_HOME                               0xf5
#define CR_BACK                               0xf6
#define CR_POWER                              0xf7
#define CR_VOL_MUTE                           0xf8
#define CR_CHN_UP                             0xf9
#define CR_CHN_DN                             0xfa
#define CR_GUIDE                              0xfb
#define CR_BOOKMARK                           0xfc
#define CR_ASSIST                             0xfd
#define CR_INPUT                              0xfe
#define CR_DASHBOARD                          0xff



//special key
#define VOICE                                 0xc0
#define KEY_MODE_SWITCH                       0xc1
#define PHY_TEST                              0xc2
#define VOICE_PTT                             0xc3
#define VOICE_HTT                             0xc4
#define VOICE_ON_REQ                          0xc5
#define AES_TEST                              0xc6

//--------------NEC IR-----------------------------------------------------
#define IR_VK_0                               0x0A
#define IR_VK_1                               0x01
#define IR_VK_2                               0x02
#define IR_VK_3                               0x03
#define IR_VK_4                               0x04
#define IR_VK_5                               0x05
#define IR_VK_6                               0x06
#define IR_VK_7                               0x07
#define IR_VK_8                               0x08
#define IR_VK_9                               0x09

//#define IR_UP                                 0x15
//#define IR_DN                                 0x16
//#define IR_LEFT                               0x17
//#define IR_RIGHT                              0x18
#define IR_CENTER                             0x19

//#define IR_POWER                              0x21
//#define IR_HOME                               0x47
//#define IR_BACK                               0x48
//#define IR_VOL_UP                             0x23
//#define IR_VOL_DN                             0x24
#define IR_CHN_UP                             0x33
#define IR_CHN_DN                             0x34
#define IR_ASSIST                             0x46
//#define IR_YOUTUBE                            0x64
//#define IR_NETFLIX                            0x63
#define IR_DISNEY                             0x67
#define IR_HBOMAX                             0x68

#define IR_BOOKMARK                           0x74
#define IR_INPUT                              0x60
//#define IR_MUTE                               0x25
#define IR_DASHBOARD                          0X0F

#define T_VK_CH_UP                            0xd0
#define T_VK_CH_DN                            0xd1

//G20
//#define IR_INFO                               0x29
#define IR_SUBTITLE                           0x58
//#define IR_RED                                0x4b
//#define IR_GREEN                              0x4a
//#define IR_YELLOW                             0x49
//#define IR_BLUE                               0x4c

//--------------PHILIPS IR-----------------------------------------------
#define IR_POWER                            12
#define IR_MENU                             191
#define IR_AMAZON                           186
#define IR_PRIME_VIDEO                      186

#define IR_GOOGLE_PLAY                      137
#define IR_NETFLIX                          118
#define IR_YOUTUBE                          121
#define IR_INFO                             15
#define IR_VOICE                            184
#define IR_SOURCE                           56 // spec send ir 56 only
//#define IR_SOURCE_183                       183

#define IR_UP                               88
#define IR_LEFT                             90
#define IR_OK                               92
#define IR_RIGHT                            91
#define IR_DN                               89

#define IR_BACK                             10
#define IR_HOME                             84

//#define IR_GUIDE_210                        210
#define IR_GUIDE_204                        204

#define IR_REWIND                           43
#define IR_PLAY                             44
#define IR_FF                               40
#define IR_STOP                             49
#define IR_PAUSE                            48
#define IR_REC                              55

#define IR_RED                              109
#define IR_GREEN                            110
#define IR_YELLOW                           111
#define IR_BLUE                             112

#define IR_VOL_UP                           16
#define IR_EXIT                            159
#define IR_CH_UP                            32
#define IR_VOL_DN                           17
#define IR_MUTE                             13
#define IR_CH_DN                            33


#define IR_DIGIT_1                          1
#define IR_DIGIT_2                          2
#define IR_DIGIT_3                          3
#define IR_DIGIT_4                          4
#define IR_DIGIT_5                          5
#define IR_DIGIT_6                          6
#define IR_DIGIT_7                          7
#define IR_DIGIT_8                          8
#define IR_DIGIT_9                          9
#define IR_DOT                              217
#define IR_DIGIT_0                          0
#define IR_CC                               70

#define IR_AMBLIGHT                         143
#define IR_OPTION                           64
#define IR_QUICK_MENU                       87
#define IR_SMART_TV                         190
#define IR_RAKUTEN_TV                       34
#define IR_TEXT                             60
#define IR_GUIDE                            204
#define IR_TV                               159

#define IR_LIST                             210
#define IR_SET                              191
#define IR_APPS                             128


#if (PCB_VER == PCB_R52)


#define     KB_MAP_NORMAL   {\
                {0,     1,      2,      3,      4,      5}, \
                {6,     7,      8,      9,      10,     11}, \
                {12,    13,     14,     15,     16,     17}, \
                {18,    19,     20,     21,     22,     23}, \
                {24,    25,     26,     27,     28,     29}, \
                {30,    31,     32,     33,     34,     35}, \
                {36,    37,     38,     39,     40,     41}, \
                {42,    43,     44,     45,     46,     47}}

                // {0,        6,      12,     18,    24,    30,    36,     42 }, 
                // {1,        7,      13,     19,    25,    31,    37,     43 }, 
                // {2,        8,      14,     20,    26,    32,    38,     44 }, 
                // {3,        9,      15,     21,    27,    33,    39,     45 }, 
                // {4,        10,     16,     22,    28,    34,    40,     46 }, 
                // {5,        11,     17,     23,    29,    25,    41,     47 }}

#define     KB_MAP_IR   {\
                IR_AMBLIGHT,    IR_POWER,     IR_RED,       IR_GREEN,       IR_OPTION,  IR_DIGIT_0, \
                IR_DIGIT_6,     IR_QUICK_MENU,IR_AMAZON,    IR_SMART_TV,    IR_PAUSE,   IR_YELLOW,  \
                IR_DIGIT_9,     IR_DIGIT_8,   IR_DIGIT_7,   IR_LEFT,        IR_NETFLIX, IR_RAKUTEN_TV, \
                IR_PLAY,        IR_BLUE,      IR_TEXT,      IR_DIGIT_5,     IR_DIGIT_4, IR_GUIDE, \
                IR_VOICE,       IR_SOURCE,    IR_BACK,      IR_REC,         IR_DIGIT_1, IR_DIGIT_2, \
                IR_DIGIT_3,     IR_REWIND,    IR_UP,        IR_RIGHT,       IR_DN,      IR_FF, \
                IR_CH_DN,       IR_MUTE,      IR_VOL_DN,    IR_STOP,        IR_TV,      IR_OK,\
                IR_HOME,        IR_CH_UP,     IR_VOL_UP,    0,              0,          0}

#define     KB_MAP_BLE  {\
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE, \
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE, \
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE, \
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE, \
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE, \
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE, \
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE, \
                VK_NONE,        VK_NONE,      VK_NONE,      VK_NONE,        VK_NONE,    VK_NONE}

//////////////////// KEY CONFIG (RCU board) ///////////////////////////
//#define  KB_DRIVE_PINS  {GPIO_PD2, GPIO_PA0, GPIO_PA1, GPIO_PA2, GPIO_PA5, GPIO_PA6, GPIO_PB2}
//#define  KB_SCAN_PINS   {GPIO_PD4, GPIO_PD5, GPIO_PD6, GPIO_PD7, GPIO_PD3, GPIO_PB3, GPIO_PB7}

// watch out driver pin to wake up deep
#define  KB_DRIVE_PINS  {GPIO_PD0, GPIO_PD1, GPIO_PD2, GPIO_PD3, GPIO_PD4, GPIO_PD5}
#define  KB_SCAN_PINS   {GPIO_PC4, GPIO_PC5, GPIO_PA2, GPIO_PA3, GPIO_PA4, GPIO_PA5, GPIO_PA6, GPIO_PB2}

//drive pin as gpio
#define PD0_FUNC                AS_GPIO
#define PD1_FUNC                AS_GPIO
#define PD2_FUNC                AS_GPIO
#define PD3_FUNC                AS_GPIO
#define PD4_FUNC                AS_GPIO
#define PD5_FUNC                AS_GPIO

//drive pin need pullup
#define PULL_WAKEUP_SRC_PD0     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD1     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD2     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD3     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD4     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD5     MATRIX_ROW_PULL

//drive pin open input to read gpio wakeup level
#define PD0_INPUT_ENABLE        1
#define PD1_INPUT_ENABLE        1
#define PD2_INPUT_ENABLE        1
#define PD3_INPUT_ENABLE        1
#define PD4_INPUT_ENABLE        1
#define PD5_INPUT_ENABLE        1

//scan pin as gpio
#define PC4_FUNC                AS_GPIO
#define PC5_FUNC                AS_GPIO
#define PA2_FUNC                AS_GPIO
#define PA3_FUNC                AS_GPIO
#define PA4_FUNC                AS_GPIO
#define PA5_FUNC                AS_GPIO
#define PA6_FUNC                AS_GPIO
#define PB2_FUNC                AS_GPIO

//scan  pin need 10K pullup
#define PULL_WAKEUP_SRC_PC4     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PC5     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA2     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA3     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA4     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA5     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA6     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PB2     MATRIX_COL_PULL

//scan pin open input to read gpio level
#define PC4_INPUT_ENABLE        1
#define PC5_INPUT_ENABLE        1
#define PA2_INPUT_ENABLE        1
#define PA3_INPUT_ENABLE        1
#define PA4_INPUT_ENABLE        1
#define PA5_INPUT_ENABLE        1
#define PA6_INPUT_ENABLE        1
#define PB2_INPUT_ENABLE        1

#elif (PCB_VER == PCB_TELINK_827x)

#define     KB_MAP_NORMAL   {\
                {0,         1,              2,          3,          4,      }, \
                {5,         6,              7,          8,          9       }, \
                {10,        11,             12,         13,         14,     }, \
                {15,        16,             17,         18,         19,     }, \
                {20,        21,             22,         23,         24,     }, \
                {25,        26,             27,         28,         29,     }, }


#define     KB_MAP_BLE  {\
                VK_NONE,    VK_NONE,        CR_POWER,   VK_NONE,    CR_HOME,    \
                IR_VOICE,   VK_NONE,        VK_NONE,    CR_VOL_UP,  CR_VOL_DN,  \
                VK_2,       CR_RIGHT,       VK_NONE,    VK_3,       VK_1,       \
                VK_5,       CR_CENTER,      VK_NONE,    VK_6,       VK_4,       \
                VK_8,       CR_DN,          CR_UP,      VK_9,       VK_7,       \
                VK_0,       CR_BACK,        CR_LEFT,    VK_NONE,    VK_NONE,    }


#define     KB_MAP_IR   {\
                VK_NONE,        IR_POWER,   IR_POWER,   VK_NONE,        IR_HOME,    \
                VK_NONE,        IR_SOURCE,  VK_NONE,    VK_NONE,        VK_NONE,    \
                IR_DIGIT_2,     IR_RIGHT,   VK_NONE,    IR_DIGIT_3,     IR_DIGIT_1,\
                IR_DIGIT_5,     IR_OK,      VK_NONE,    IR_DIGIT_6,     IR_DIGIT_4,\
                IR_DIGIT_8,     IR_DN,      IR_UP,      IR_DIGIT_9,     IR_DIGIT_7,\
                IR_DIGIT_0,     IR_BACK,    IR_LEFT,    IR_MUTE,        VK_NONE,     }

//////////////////// KEY CONFIG (RCU board) ///////////////////////////
#define  KB_DRIVE_PINS  {GPIO_PD5, GPIO_PD2, GPIO_PD4, GPIO_PD6, GPIO_PD7}          // last pin 'GPIO_PD7' abnormal
#define  KB_SCAN_PINS   {GPIO_PC5, GPIO_PA0, GPIO_PB2, GPIO_PA4, GPIO_PA3, GPIO_PD3}// second pin 'GPIO_PA0' abnormal


//drive pin as gpio
#define PD5_FUNC                AS_GPIO
#define PD2_FUNC                AS_GPIO
#define PD4_FUNC                AS_GPIO
#define PD6_FUNC                AS_GPIO
#define PD7_FUNC                AS_GPIO

//drive pin need 100K pulldown
#define PULL_WAKEUP_SRC_PD5     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD2     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD4     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD6     MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD7     MATRIX_ROW_PULL

//drive pin open input to read gpio wakeup level
#define PD5_INPUT_ENABLE        1
#define PD2_INPUT_ENABLE        1
#define PD4_INPUT_ENABLE        1
#define PD6_INPUT_ENABLE        1
#define PD7_INPUT_ENABLE        1

//scan pin as gpio
#define PC5_FUNC                AS_GPIO
#define PA0_FUNC                AS_GPIO
#define PB2_FUNC                AS_GPIO
#define PA4_FUNC                AS_GPIO
#define PA3_FUNC                AS_GPIO
#define PD3_FUNC                AS_GPIO

//scan  pin need 10K pullup
#define PULL_WAKEUP_SRC_PC5     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA0     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PB2     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA4     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA3     MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PD3     MATRIX_COL_PULL

//scan pin open input to read gpio level
#define PC5_INPUT_ENABLE        1
#define PA0_INPUT_ENABLE        1
#define PB2_INPUT_ENABLE        1
#define PA4_INPUT_ENABLE        1
#define PA3_INPUT_ENABLE        1
#define PD3_INPUT_ENABLE        1



#endif//#if (PCB_VER == PCB_TELINK_827x)



#define        KB_MAP_NUM        KB_MAP_NORMAL
#define        KB_MAP_FN         KB_MAP_NORMAL

/////////////////// Clock  /////////////////////////////////
#if (TL_AUDIO_MODE & (TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_MSBC_MODE))
#define CLOCK_SYS_CLOCK_HZ                    32000000
#else
#define CLOCK_SYS_CLOCK_HZ                    24000000
#endif
enum{
    CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,
    CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
    CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
};



/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE                0
#define WATCHDOG_INIT_TIMEOUT                 500  //ms





///////////////////////////////////// ATT  HANDLER define ///////////////////////////////////////
typedef enum
{
    ATT_H_START = 0,


    //// Gap ////
    /**********************************************************************************************/
    GenericAccess_PS_H,                     //UUID: 2800,     VALUE: uuid 1800
    GenericAccess_DeviceName_CD_H,          //UUID: 2803,     VALUE:              Prop: Read | Notify
    GenericAccess_DeviceName_DP_H,          //UUID: 2A00,     VALUE: device name
    GenericAccess_Appearance_CD_H,          //UUID: 2803,     VALUE:              Prop: Read
    GenericAccess_Appearance_DP_H,          //UUID: 2A01,     VALUE: appearance
    CONN_PARAM_CD_H,                        //UUID: 2803,     VALUE:              Prop: Read
    CONN_PARAM_DP_H,                        //UUID: 2A04,     VALUE: connParameter


    //// gatt ////
    /**********************************************************************************************/
    GenericAttribute_PS_H,                  //UUID: 2800,     VALUE: uuid 1801
    GenericAttribute_ServiceChanged_CD_H,   //UUID: 2803,     VALUE:              Prop: Indicate
    GenericAttribute_ServiceChanged_DP_H,   //UUID: 2A05,     VALUE: service change
    GenericAttribute_ServiceChanged_CCB_H,  //UUID: 2902,     VALUE: serviceChangeCCC


    //// device information ////
    /**********************************************************************************************/
    DeviceInformation_PS_H,                 //UUID: 2800,     VALUE: uuid 180A
    DeviceInformation_pnpID_CD_H,           //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_pnpID_DP_H,           //UUID: 2A50,     VALUE: PnPtrs

    DeviceInformation_firmwareRev_CD_H,     //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_firmwareRev_DP_H,     //UUID: 2A50,     VALUE: PnPtrs

    DeviceInformation_softwareRev_CD_H,     //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_softwareRev_DP_H,     //UUID: 2A50,     VALUE: PnPtrs

    DeviceInformation_hardwareRev_CD_H,     //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_hardwareRev_DP_H,     //UUID: 2A50,     VALUE: PnPtrs

    //// HID ////
    /**********************************************************************************************/
    HID_PS_H,                               //UUID: 2800,     VALUE: uuid 1812

    //include
    HID_INCLUDE_H,                          //UUID: 2802,     VALUE: include

    //protocol
    HID_PROTOCOL_MODE_CD_H,                 //UUID: 2803,     VALUE:              Prop: read | write_without_rsp
    HID_PROTOCOL_MODE_DP_H,                 //UUID: 2A4E,     VALUE: protocolMode

    //boot keyboard input report
    HID_BOOT_KB_REPORT_INPUT_CD_H,          //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_BOOT_KB_REPORT_INPUT_DP_H,          //UUID: 2A22,     VALUE: bootKeyInReport
    HID_BOOT_KB_REPORT_INPUT_CCB_H,         //UUID: 2902,     VALUE: bootKeyInReportCCC

    //boot keyboard output report
    HID_BOOT_KB_REPORT_OUTPUT_CD_H,         //UUID: 2803,     VALUE:              Prop: Read | write| write_without_rsp
    HID_BOOT_KB_REPORT_OUTPUT_DP_H,         //UUID: 2A32,     VALUE: bootKeyOutReport

    //consume report in
    HID_CONSUME_REPORT_INPUT_CD_H,          //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_CONSUME_REPORT_INPUT_DP_H,          //UUID: 2A4D,     VALUE: reportConsumerIn
    HID_CONSUME_REPORT_INPUT_CCB_H,         //UUID: 2902,     VALUE: reportConsumerInCCC
    HID_CONSUME_REPORT_INPUT_REF_H,         //UUID: 2908      VALUE: REPORT_ID_CONSUMER, TYPE_INPUT

    //keyboard report in
    HID_NORMAL_KB_REPORT_INPUT_CD_H,        //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_NORMAL_KB_REPORT_INPUT_DP_H,        //UUID: 2A4D,     VALUE: reportKeyIn
    HID_NORMAL_KB_REPORT_INPUT_CCB_H,       //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_NORMAL_KB_REPORT_INPUT_REF_H,       //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT

    //keyboard report out
    HID_NORMAL_KB_REPORT_OUTPUT_CD_H,       //UUID: 2803,     VALUE:              Prop: Read | write| write_without_rsp
    HID_NORMAL_KB_REPORT_OUTPUT_DP_H,       //UUID: 2A4D,     VALUE: reportKeyOut
    HID_NORMAL_KB_REPORT_OUTPUT_REF_H,      //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_OUTPUT

    // report map
    HID_REPORT_MAP_CD_H,                    //UUID: 2803,     VALUE:              Prop: Read
    HID_REPORT_MAP_DP_H,                    //UUID: 2A4B,     VALUE: reportKeyIn
    HID_REPORT_MAP_EXT_REF_H,               //UUID: 2907      VALUE: extService

    //hid information
    HID_INFORMATION_CD_H,                   //UUID: 2803,     VALUE:              Prop: read
    HID_INFORMATION_DP_H,                   //UUID: 2A4A      VALUE: hidInformation

    //control point
    HID_CONTROL_POINT_CD_H,                 //UUID: 2803,     VALUE:              Prop: write_without_rsp
    HID_CONTROL_POINT_DP_H,                 //UUID: 2A4C      VALUE: controlPoint
#if (BLE_AUDIO_ENABLE)
    #if (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL)
    //audio report in 1
    HID_AUDIO_REPORT_INPUT_FIRST_CD_H,      //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_AUDIO_REPORT_INPUT_FIRST_DP_H,      //UUID: 2A4D,     VALUE: reportKeyIn
    HID_AUDIO_REPORT_INPUT_FIRST_CCB_H,     //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_AUDIO_REPORT_INPUT_FIRST_REF_H,     //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT
    //audio report in 2
    HID_AUDIO_REPORT_INPUT_SECND_CD_H,      //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_AUDIO_REPORT_INPUT_SECND_DP_H,      //UUID: 2A4D,     VALUE: reportKeyIn
    HID_AUDIO_REPORT_INPUT_SECND_CCB_H,     //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_AUDIO_REPORT_INPUT_SECND_REF_H,     //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT
    //audio report in 3
    HID_AUDIO_REPORT_INPUT_THIRD_CD_H,      //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_AUDIO_REPORT_INPUT_THIRD_DP_H,      //UUID: 2A4D,     VALUE: reportKeyIn
    HID_AUDIO_REPORT_INPUT_THIRD_CCB_H,     //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_AUDIO_REPORT_INPUT_THIRD_REF_H,     //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT
    #endif
#endif
    //// battery service ////
    /**********************************************************************************************/
    BATT_PS_H,                              //UUID: 2800,     VALUE: uuid 180f
    BATT_LEVEL_INPUT_CD_H,                  //UUID: 2803,     VALUE:              Prop: Read | Notify
    BATT_LEVEL_INPUT_DP_H,                  //UUID: 2A19      VALUE: batVal
    BATT_LEVEL_INPUT_CCB_H,                 //UUID: 2902,     VALUE: batValCCC


    //// Ota ////
    /**********************************************************************************************/
#if (GOOGLE_OTA||TELINK_OTA)
    OTA_PS_H,                               //UUID: 2800,     VALUE: telink ota service uuid
#endif

#if (GOOGLE_OTA)
    OTA_CMD_OUT_CD_H_GL,                       //UUID: 2803,     VALUE:              Prop: read | write_without_rsp
    OTA_CMD_OUT_DP_H_GL,                       //UUID: telink ota uuid,  VALUE: otaData
    OTA_CMD_OUT_CCC_H,
#endif
#if (TELINK_OTA)
    OTA_CMD_OUT_CD_H_TL,                       //UUID: 2803,     VALUE:              Prop: read | write_without_rsp
    OTA_CMD_OUT_DP_H_TL,                       //UUID: telink ota uuid,  VALUE: otaData
#endif
#if (GOOGLE_OTA||TELINK_OTA)
    OTA_CMD_OUT_DESC_H,                     //UUID: 2901,     VALUE: otaName
#endif


#if (BLE_AUDIO_ENABLE)
    #if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
    /********************************************************************************************
    * GOOGLE AUDIO service 9:
    ********************************************************************************************/
    AUDIO_GOOGLE_PS_H ,
    AUDIO_GOOGLE_TX_CD_H,
    AUDIO_GOOGLE_TX_DP_H ,
    AUDIO_GOOGLE_RX_CD_H ,
    AUDIO_GOOGLE_RX_DP_H,
    AUDIO_GOOGLE_RX_CCC_H,
    AUDIO_GOOGLE_CTL_CD_H,
    AUDIO_GOOGLE_CTL_DP_H,
    AUDIO_GOOGLE_CTL_CCC_H,

    #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)
    //// Audio ////
    /**********************************************************************************************/
    AUDIO_PS_H,                             //UUID: 2800,     VALUE: telink audio service uuid

    //mic
    AUDIO_MIC_INPUT_CD_H,                   //UUID: 2803,     VALUE:              Prop: Read | Notify
    AUDIO_MIC_INPUT_DP_H,                   //UUID: telink mic uuid,  VALUE: micData
    AUDIO_MIC_INPUT_CCB_H,                  //UUID: 2A19      VALUE: micDataCCC
    AUDIO_MIC_INPUT_DESC_H,                 //UUID: 2901,     VALUE: micName

    //speaker
    AUDIO_SPEAKER_OUT_CD_H,                 //UUID: 2803,     VALUE:              Prop: write_without_rsp
    AUDIO_SPEAKER_OUT_DP_H,                 //UUID: telink speaker uuid,  VALUE: speakerData
    AUDIO_SPEAKEROUT_DESC_H,                //UUID: 2901,     VALUE: speakerName
    #endif
#endif

#if (APP_IR_ENABLE)
    //ir
    IR_PS_H,                                //UUID: 2800,     VALUE: telink audio service uuid
    IR_PROG_OUT_CONTROL_CD_H,               //UUID: 2800,     VALUE: telink audio service uui
    IR_PROG_OUT_CONTROL_H,
    IR_KEY_ID_CD_H,
    IR_KEY_ID_H,
    IR_CODE_CD_H,
    IR_CODE_H,
    IR_SUPPRESS_CD_H,
    IR_SUPPRESS_H,
    IR_KEY_EVENT_CD_H,
    IR_KEY_EVENT_H,
    IR_KEY_EVENT_CCC_H,
#endif


#if (MP_TEST_MODE)
    //// Test Mode ////
    /**********************************************************************************************/
    TEST_MODE_PS_H,                         //UUID: 2800,     VALUE: test mode service uuid
    TEST_MODE_CD_H,                         //UUID: 2803,     VALUE: Prop: read | write | write_without_rsp | notify
    TEST_MODE_DP_H,                         //UUID: test mode uuid,  VALUE: testData
    TEST_MODE_CCB_H,                        //UUID: 2902,     VALUE: testDataCCC
#endif


    SENEASY_RCU_PS_H,
    SENEASY_RCU_RD_CD_H,
    SENEASY_RCU_RD_DP_H,
    SENEASY_RCU_WR_CD_H,
    SENEASY_RCU_WR_DP_H,

    ATT_END_H,

}ATT_HANDLE;



#define DEBUG_GPIO_ENABLE                     0

#if(DEBUG_GPIO_ENABLE)
    //define debug GPIO here according to your hardware

    #define GPIO_CHN0                         GPIO_PB4
    #define GPIO_CHN1                         GPIO_PB5
    #define GPIO_CHN2                         GPIO_PB6
    #define GPIO_CHN3                         //GPIO_PC2  // PC2/PC3 may used for external crystal input
    #define GPIO_CHN4                         //GPIO_PC3  // PC2/PC3 may used for external crystal input
    #define GPIO_CHN5                         GPIO_PB0
    #define GPIO_CHN6                         GPIO_PB1


    #define PB4_OUTPUT_ENABLE                 1
    #define PB5_OUTPUT_ENABLE                 1
    #define PB6_OUTPUT_ENABLE                 1
    //#define PC2_OUTPUT_ENABLE               1
    //#define PC3_OUTPUT_ENABLE               1
    #define PB0_OUTPUT_ENABLE                 1
    #define PB1_OUTPUT_ENABLE                 1




    #define DBG_CHN0_LOW                      gpio_write(GPIO_CHN0, 0)
    #define DBG_CHN0_HIGH                     gpio_write(GPIO_CHN0, 1)
    #define DBG_CHN0_TOGGLE                   gpio_toggle(GPIO_CHN0)
    #define DBG_CHN1_LOW                      gpio_write(GPIO_CHN1, 0)
    #define DBG_CHN1_HIGH                     gpio_write(GPIO_CHN1, 1)
    #define DBG_CHN1_TOGGLE                   gpio_toggle(GPIO_CHN1)
    #define DBG_CHN2_LOW                      gpio_write(GPIO_CHN2, 0)
    #define DBG_CHN2_HIGH                     gpio_write(GPIO_CHN2, 1)
    #define DBG_CHN2_TOGGLE                   gpio_toggle(GPIO_CHN2)
    #define DBG_CHN3_LOW                      //gpio_write(GPIO_CHN3, 0)
    #define DBG_CHN3_HIGH                     //gpio_write(GPIO_CHN3, 1)
    #define DBG_CHN3_TOGGLE                   //gpio_toggle(GPIO_CHN3)
    #define DBG_CHN4_LOW                      //gpio_write(GPIO_CHN4, 0)
    #define DBG_CHN4_HIGH                     //gpio_write(GPIO_CHN4, 1)
    #define DBG_CHN4_TOGGLE                   //gpio_toggle(GPIO_CHN4)
    #define DBG_CHN5_LOW                      gpio_write(GPIO_CHN5, 0)
    #define DBG_CHN5_HIGH                     gpio_write(GPIO_CHN5, 1)
    #define DBG_CHN5_TOGGLE                   gpio_toggle(GPIO_CHN5)
    #define DBG_CHN6_LOW                      gpio_write(GPIO_CHN6, 0)
    #define DBG_CHN6_HIGH                     gpio_write(GPIO_CHN6, 1)
    #define DBG_CHN6_TOGGLE                   gpio_toggle(GPIO_CHN6)
#else
    #define DBG_CHN0_LOW
    #define DBG_CHN0_HIGH
    #define DBG_CHN0_TOGGLE
    #define DBG_CHN1_LOW
    #define DBG_CHN1_HIGH
    #define DBG_CHN1_TOGGLE
    #define DBG_CHN2_LOW
    #define DBG_CHN2_HIGH
    #define DBG_CHN2_TOGGLE
    #define DBG_CHN3_LOW
    #define DBG_CHN3_HIGH
    #define DBG_CHN3_TOGGLE
    #define DBG_CHN4_LOW
    #define DBG_CHN4_HIGH
    #define DBG_CHN4_TOGGLE
    #define DBG_CHN5_LOW
    #define DBG_CHN5_HIGH
    #define DBG_CHN5_TOGGLE
    #define DBG_CHN6_LOW
    #define DBG_CHN6_HIGH
    #define DBG_CHN6_TOGGLE
    #define DBG_CHN7_LOW
    #define DBG_CHN7_HIGH
    #define DBG_CHN7_TOGGLE
#endif  //end of DEBUG_GPIO_ENABLE




#if (MP_TEST_MODE)
#define BLE_PHYTEST_MODE                      PHYTEST_MODE_THROUGH_2_WIRE_UART
#else
#define BLE_PHYTEST_MODE                      PHYTEST_MODE_DISABLE
#endif


#define UART_PRINT_ENABLE                      1   //uart_tx as uart print func

#if (PTM)
    #define UART_PRINT_DEBUG_ENABLE             0   //fixed 0, this ptm need to use uart, TODO: anyway can have another uart out?
#else
    #define UART_PRINT_DEBUG_ENABLE             0   //0 or 1,enable custom uart debug here GPIO simulate uart print func
#endif



#if (UART_PRINT_DEBUG_ENABLE)
    //the baud rate should not bigger than 1M(system timer clock is constant 16M)
    #define PRINT_BAUD_RATE                   115200//1000000
    #define DEBUG_INFO_TX_PIN                 GPIO_PB1
    #define PULL_WAKEUP_SRC_PB1               PM_PIN_PULLUP_10K
    #define PB1_OUTPUT_ENABLE                 1
    #define PB1_DATA_OUT                      1 //must
    #include "application/print/u_printf.h"
#endif




#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
