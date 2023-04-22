/******************************************************************************
 * @file     app_ui.c
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

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "app_config.h"

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"
#include "../common/blt_led.h"
#include "../common/blt_soft_timer.h"

#include "rc_ir.h"
#include "battery_check.h"
#include "app_audio.h"
#include "gl_audio.h"
#include "app_flash_write.h"
#include "app_ir.h"
#include "app_ui.h"

#if (MP_TEST_MODE)
#include "app_test.h"
#endif

#include "app_custom.h"

//////////////////// key type ///////////////////////
#define IDLE_KEY                   0
#define CONSUMER_KEY               1
#define KEYBOARD_KEY               2
#define IR_KEY                     3
#define VOICE_KEY                  4

//------------------- telink 827x RCU PCBA key index mapping---------------------------------------------

// config with each PCBA
#if (PCB_VER == PCB_R52)

#define KEY_VOICE           42  //key index of PCB_R52 PCBA
#define KEY_MUTE            37  //key index of PCB_R52 PCBA
#define KEY_SOURCE          7  
#define KEY_BACK            26
#define KEY_OK              41  
#define KEY_UP              32
#define KEY_DOWN            34
#define KEY_RIGHT           33
#define KEY_LEFT            15
#define KEY_HOME            23
#define KEY_SET             4
// #define KEY_AD              47   //AD or info key,
#define KEY_0               5
#define KEY_9               12   //for test del pair
#define KEY_AMBIL           0  //ambilight , use key learn for test
#define KEY_GUIDE           40  // guide or list
#define KEY_OPTIONS         39  //options  
#elif (PCB_VER == PCB_TELINK_827x)
#define KEY_VOICE           5  //key index of PCB_R52 PCBA

#define KEY_MUTE            28 //key index of telink PCBA
#define KEY_SOURCE          6 //key index of telink PCBA
#define KEY_BACK            26 //key index of telink PCBA
#define KEY_OK              16 //key index of telink PCBA
#define KEY_UP              22 //key index of telink PCBA
#define KEY_DOWN            21 //key index of telink PCBA
#define KEY_RIGHT           11 //key index of telink PCBA
#define KEY_LEFT            27 //key index of telink PCBA
#define KEY_AD              29 //AD or info key, use menu for replacment
#define KEY_0               25 //key index of telink PCBA
#define KEY_9               23 //for test del pair
#define KEY_AMBIL           7  //ambilight , use key learn for test

#endif

//-----------------------------------------------------------------------------------------------------
#define     MY_USER_PAIR_IND_ADV_TIME       60000000
#define     OK_KEY_DIRECT_ADV_TIME          80000//500000 //ok// spec 50 ms, 1s for test, 500ms ok
#define     OK_KEY_INDIRECT_ADV_TIME        3000000 // spec 3s,

#define     SRC_KEY_PRESS_TIME              2000//spec 3000ms, test 1000ms



_attribute_data_retention_ u8   key_type;
_attribute_data_retention_ int  key_not_released;
_attribute_data_retention_ u8   ota_is_working = 0;


_attribute_data_retention_ bool   rc_reconn_normal = 1; // 1 normal , 0 fall back


u8  ir_hw_initialed = 0;   //note: can not be retention variable
_attribute_data_retention_  int ir_not_released=0;
//_attribute_data_retention_  u8  user_key_mode;
//_attribute_data_retention_  u8  wait_repair = FALSE;
//_attribute_data_retention_  u8  is_pairing_mode=0;
//_attribute_data_retention_  u8  is_reconn_mode=0;
_attribute_data_retention_  u8  ota_flag = 0;  //0:pause 1:continue

//_attribute_data_retention_ u8 wakeup_key_send_flag=1;     //2: already send wake up packet

//extern u8 comb_key_keyid[7];
extern u32  latest_user_event_tick;
//extern u8   lowbat;
extern int  device_in_connection_state;
//extern u8   flag_schedule_ota;
//extern u8   device_timeout_state;
//extern u8   wakeup_src;
extern u8   power_on_from;

//extern int ir_fallback_send(u8 key_down);
extern void device_led_off(u8 led);
//extern void app_enter_deep_timeset(void);
//extern void app_set_adv_interval_downgrade_indirect(void);
//extern void app_tx_power_wakeup(u8 keyid);
//extern u8  app_custom_is_wakeup_key(u8 keyid);
extern ble_sts_t app_terminate(u8 reason);
//extern void task_before_deep();
extern void advertise_tick_retime(void);
extern void direct_adv(u8 e, u8 *p, int n); // used for R52
extern void indirect_adv(u8 e, u8 *p, int n); // used for R52

extern _attribute_data_retention_ u32 app_active_remote_timer;
extern _attribute_data_retention_ u32 app_audio_transfer_timer;

u8 user_unpair_wait_terminate=FALSE;
u8 user_pair_wait_terminate=FALSE;


u32 ftick_key_source =0; // key src press time
u32 ftick_key_back_home = 0;
u32 ftick_mute_digit0_press_time =0;
u32 ftick_keyok_back_press_time=0;
u32 ftick_keyback_down_press_time=0;

u32 ftick_key_list =0;

_attribute_data_retention_ u32 ftick_user_pair_start =0;
_attribute_data_retention_  u8  is_cleanconn_repairing_mode=0;

// u32 ftick_key_ad =0;
u32 ftick_guide_key = 0;
u32 ftick_options_key = 0;
//u32 ftick_to_send_release =0;
u32 ftick_ir_release_n_indirect_adv=0;
u32 ftick_power_on_key_pressed =0;
u32 ftick_vol_up_pressed =0;
u32 ftick_post_process_voice_key=0;
u32  ftick_post_process_voice_key_reconn =0;
bool spcial_combine_key = 0;

static const u16 vk_consumer_map[] = {
    G00GLE_MKEY_INFO,
    G00GLE_MKEY_SUBTITLE,
    G00GLE_MKEY_RED,
    G00GLE_MKEY_GREEN,
    G00GLE_MKEY_YELLOW,
    G00GLE_MKEY_BLUE,
    G00GLE_MKEY_YOUTUBE,
    G00GLE_MKEY_NETFLIX,
    G00GLE_MKEY_DISNEY,
    G00GLE_MKEY_HBOMAX,
    G00GLE_MKEY_UP,
    G00GLE_MKEY_DN,
    G00GLE_MKEY_LEFT,
    G00GLE_MKEY_RIGHT,
    G00GLE_MKEY_CENTER,
    G00GLE_MKEY_HOME,
    G00GLE_MKEY_BACK,
    G00GLE_MKEY_POWER,
    G00GLE_MKEY_VOL_MUTE,
    G00GLE_MKEY_CHN_UP,
    G00GLE_MKEY_CHN_DN,
    G00GLE_MKEY_GUIDE,
    G00GLE_MKEY_BOOKMARK,
    G00GLE_MKEY_ASSIST,
    G00GLE_MKEY_INPUT,
    G00GLE_MKEY_DASHBOARD,
};


/////////////////////////// led management /////////////////////
#if (BLT_APP_LED_ENABLE)

    enum{
        LED_POWER_ON = 0,
        LED_AUDIO_ON,           //1
        LED_AUDIO_OFF,          //2
        LED_SHINE_SLOW,         //3
        LED_SHINE_FAST,         //4
        LED_SHINE_SUCCESS,      //5
        LED_SHINE_PAIRING,      //6
        LED_SHINE_DISCONNECT,   //7
        LED_SHINE_PTT,
        LED_SHINE_LOWBAT,       //9
        LED_PAIR_ERROR,         //10
        LED_OTA_START,          //11
        LED_OTA_END,            //12
        LED_KEYPRESS,           //13

        LED_SHINE_ERASE_PAIR,   //14

        LED_AMBILIGHT_PRESS,    //15
        LED_AMBILIGHT_RELEASE,  //16
        LED_LONG_ON,            //17
        LED_LONG_OFF,           //18

    };

    const led_cfg_t led_cfg[] = {
            //              1st on                 repeatCount, iskeepon
            {APP_LED_BLUE,   1,   1000,     0,      1,      0,  },    //0 power-on, 1s on
            {APP_LED_BLUE,   1,   100,      0 ,     0,      1,  },    //1 audio on, long on
            {APP_LED_BLUE,   0,    0,       100 ,   0,      1,  },    //2 audio off, long off
            {APP_LED_BLUE,   1,   500,      500 ,   2,      0,  },    //3 1Hz for 2 seconds
            {APP_LED_BLUE,   1,   100,      100 ,   2,      0,  },    //4 5Hz for 1 seconds
            {APP_LED_BLUE,   1,   1000,     0,      3,      0,  },    //5 5Hz for 1 seconds
            {APP_LED_BLUE,   1,   250,      250 ,   2,      0,  },    //6 2Hz
            {APP_LED_BLUE,   1,   200,      200 ,   3,      0,  },    //7 disconnect 3 times
            {APP_LED_BLUE,   1,   500,      500 ,   0,      1,  },    //8 1Hz
            {APP_LED_BLUE,   1,   300,      700 ,   2,      0,  },    //9 (2Hz) LED for 2s
            {APP_LED_BLUE,   1,   50,       50,     4,      0,  },
            {APP_LED_BLUE,   1,   250,      250,    2,      0,  },    //11 OTA
            {APP_LED_BLUE,   1,   0,        1000,   3,      0,  },
            {APP_LED_BLUE,   1,   100,      100,    1,      0,  },    //13 5Hz for 1 seconds
            {APP_LED_BLUE,   1,   300,      700,    3,      0,  },    //14 1Hz for 3s
            {APP_LED_WHITE,   1,   1000,     0,      0,      1,  },    //15 long on
            {APP_LED_WHITE,   1,   1000,     0,      5,      0,  },    //16 on for 5s
            {APP_LED_BLUE,   1,   100,     0,      0xFF,    0,  },    //15 long on
            {APP_LED_BLUE,   0,   0,     100,      0xFF,    0,  },    //16 on for 5s

            //another led
            //{APP_LED_GREEN,   1,   500,      500 ,   0,      0,  },    //8 1Hz

    };

#endif

void app_pairing_error(void)
{
    ftick_user_pair_start=0; // clear pair mode
    device_led_setup(led_cfg[LED_PAIR_ERROR]);
}

//extern u32 keyScanLongPressTick;


#if (REMOTE_IR_ENABLE)
    //ir key
    #define TYPE_IR_SEND               1
    #define TYPE_IR_RELEASE            2

    ///////////////////// key mode //////////////////////
    #define KEY_MODE_BLE               0    //ble key
    #define KEY_MODE_IR                1    //ir  key


    static const u8 kb_map_ble[49] =    KB_MAP_BLE;
    static const u8 kb_map_ir[49] =     KB_MAP_IR;


void ir_dispatch(u8 type, u8 repeat, u8 ircode) {

    if (!ir_hw_initialed) {
        ir_hw_initialed = 1;
        //rc_ir_init_nec();
        rc_ir_init_philips();
    }
    if (type == TYPE_IR_SEND) {
        if (!is_batt_low()) {
            //ir_send_ctrl.is_full_sequence = 0;
            //ir_nec_send(syscode,~(syscode),ircode);
            ir_philips_send(repeat, ircode); // u8 mode , u8 code
        }
    }
    else if (type == TYPE_IR_RELEASE) {
        ir_send_release();
    }
}


#endif //REMOTE_IR_ENABLE

enum {
    WAKEUP_NONE,    //dummy , will not use this
    WAKEUP_PAD,     //from pad wakeup
    WAKEUP_NON_PAD, //others
};
u8 app_wakeup_src(void) {
    u8 flag = analog_read(0x44);
    if (flag & 0x08) return WAKEUP_PAD;  //wakeup for pad 1
    else return WAKEUP_NON_PAD;  //wakeup for not pad, 2
}

//------------------- DUAL OTA ----------------------------------------------------

//#if (BLE_REMOTE_OTA_ENABLE)
#if (GOOGLE_OTA)

#include "app_ota.h"
u8 app_enter_ota_mode(u8 *data) // call in lib
{
    printf("ota start data:");
    array_printf(data,4);

    u8 check_data[4];
    bls_ota_get_random(check_data);

    if(memcmp(check_data,data,4 )){
        u8 report_data[3] ={CLIENT_REPORT_CMD, TYPE_OTA_RESULT, OTA_START_KEY_ERR};
        bls_att_pushNotifyData(OTA_CMD_OUT_DP_H_GL, report_data, 3);
        printf("OTA_START_KEY_ERR\n");

        return 1;
    }
    bls_pm_setSuspendMask (SUSPEND_DISABLE);
    ota_flag = 1;

    //add user code
    printf("ota start red led on\r\n");
    device_led_setup(led_cfg[LED_OTA_START]);
    ota_is_working = 1;
    bls_ota_setTimeout(90 * 1000 * 1000); //set OTA timeout  90 seconds

    return 0;
}

/*******************************************************************
 *** Use the following functions to modify various types of keys ***
 *******************************************************************/
#if 1 // use custom OTA key

//change authentication plaintext and plaintext key
void app_ota_get_plaintext_key(u8 *plaintext,u8 *cus_auth_key){

    u8 auth_plaintext[16] = {'0','1','2','3','4','5','6','7','8','9','a','b',0x00,0x00,0x00,0x00,};
    u8 auth_key[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};  //OTA key;

    memcpy(plaintext,auth_plaintext,16);
    memcpy(cus_auth_key,auth_key,16);
}

void app_ota_get_fw_decryption_key(u8 *dec_key){

    u8 key[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};
    memcpy(dec_key,key,16);
}

//change ecdsa public key
void app_ota_get_ecdsa_pub_key(u8 *pub_key){
    u8 key[64] = {0x8d,0x82,0x79,0xc1,0xbb,0x22,0xe7,0x99,0xe6,0x97,0x49,0xf6,0xab,0x28,0x44,0x6d,
                  0x9f,0x21,0xce,0x13,0x5b,0x43,0xfa,0x74,0x87,0xe2,0x96,0x49,0xcd,0xc8,0xdd,0xc3,
                  0x92,0x6f,0x6e,0x3d,0x89,0xb0,0x5c,0xa7,0xf9,0xce,0xb7,0x38,0x97,0xa3,0xce,0x9a,
                  0xae,0xaf,0x89,0xbe,0x8d,0x8d,0x03,0x1e,0x6b,0xb9,0x14,0x24,0xda,0x14,0x11,0xbe};
    memcpy(pub_key,key,64);
}
#endif // #if 1 // use custom OTA key

void app_ota_result(u8 result)
{
    printf("google_ota_result result=%x\r\n",result);
    u8 report_data[3];
    report_data[0] = CLIENT_REPORT_CMD;
    report_data[1] = TYPE_OTA_RESULT;
    report_data[2] = (u8)result;
    if(device_in_connection_state)
    {
        u16 i = 0;
        while(bls_att_pushNotifyData(OTA_CMD_OUT_DP_H_GL, report_data, 3) &&  i < 2000){
        sleep_us(100);
        i++;
        }

        i = 0;
        while(app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000){
        sleep_us(100);
        i++;
        }

        i = 0;
        while(blc_ll_getTxFifoNumber() &&  i < 400){
        sleep_us(10000);//printf("waiting TX fifo is empty\n"); //waiting TX fifo is empty
        i++;
        }
    }
    if(result == 0)
    {
        device_led_setup(led_cfg[LED_OTA_END]);
        printf("ota ok , write_ana\r\n");
        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | OTA_FLG);
        printf("analog_read(USED_DEEP_ANA_REG)=%x\r\n",analog_read(USED_DEEP_ANA_REG));
    }
}

extern u16 count_schedule_ota;
void app_schedule_ota(u8 *data)
{
    //array_printf(data,4);
    u16 count = (data[0]<<8) + data[1];

    if(count ==0)
    {
        count_schedule_ota = 90;

    }else if(count >= 1440){    //24 hour
        count_schedule_ota = 1440;
    }else{
        count_schedule_ota = count;
    }
    printf("app_schedule_ota:0x%d\n",count);

    flag_schedule_ota = 0x55;
    analog_write(DEEP_ANA_REG2, 0);
}

int app_ota_timeoutcb(void)
{
    printf("app_ota_timeoutcb\r\n");
    app_ota_terminate(OTA_TIMEOUT);
    return -1;
}

//extern u8  google_ota_start_flag;
void app_ota_timeout(void)
{
    app_pairing_error();
    blt_soft_timer_add(app_ota_timeoutcb, 1000000);
}

void app_ota_status(u8 status)
{
    if(device_in_connection_state == 0) return;
    if(ota_is_working == 0) return;
    ble_sts_t stat;
    if((status == 0) && (ota_flag == 1))
    {
        stat = app_ota_pause();
        ota_flag = 0;
        printf("app_ota_pause stat=%x\r\n",stat);
    }

    if((status == 1) && (ota_flag == 0))
    {
        ble_sts_t stat;
        stat = app_ota_continue();
        ota_flag = 1;
        printf("app_ota_continue stat=%x\r\n",stat);
    }
}

void app_ota_slave_terminate(void)
{
    printf("app_ota_slave_terminate\n");
    u8 report_data[3] = {CLIENT_REPORT_CMD, TYPE_OTA_RESULT, OTA_USER_TERMINATE};

    u16 i = 0;
    while(bls_att_pushNotifyData(OTA_CMD_OUT_DP_H_GL, report_data, 3) &&  i < 2000){
       sleep_us(100);
       i++;
    }
}


#define SCHE_OTA_1MIN    32000*60
#define SCHE_OTA_1HOUR   32000*60*60
void app_schedule_ota_enter_deep(void)
{
    //0: normal OTA, 90:90mins, 2:2hours...
    u8 sch_ota_timer[7]= {0,90,2,3,5,8,13};
    u8 data = analog_read(DEEP_ANA_REG2);
    if(data == 0) // 0: normal OTA,
    {
        if((flag_schedule_ota == 0x55) && (device_timeout_state == 0x55))
        {
            printf("schedule ota first\r\n");
            analog_write(DEEP_ANA_REG2, 1);
            cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,count_schedule_ota*SCHE_OTA_1MIN);
        }
        else
        {
            printf("schedule ota idle to sleep\r\n");
            task_before_deep();
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
        }
    }
    else
    {
        if(app_wakeup_src() == 2) // if from deep?
        {
            data++;
            printf("sch ota timer ++,data=%x\r\n",data);
        }
        else
            printf("sch ota timer maintain\r\n");

        if(data >= 7) // over sch_ota_timer array index
        {
            printf("ota data >= 7 idle to sleep\r\n");
            analog_write(DEEP_ANA_REG2, 0);
            task_before_deep();
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
        }

        analog_write(DEEP_ANA_REG2, data);
        if(data<7)
            printf("sch ota data =%x,time =%x\r\n",data,sch_ota_timer[data]);
        if(data == 1)
            cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,sch_ota_timer[data]*SCHE_OTA_1MIN);
        else
            cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,sch_ota_timer[data]*SCHE_OTA_1HOUR);
    }
}

void app_schedule_ota_send_notify_to_continue_ota(void)
{
    u8 dat[2] = {0};
    u8 report_data[4] = {CLIENT_REPORT_CMD,0x04,0x73,0x6f};
    u8 data = analog_read(DEEP_ANA_REG2);

    dat[0] = 0x73;
    dat[1] = 0x6f;
    if((app_wakeup_src() == 2) && (data))//wake up for timer
    {
        printf("app_schedule_ota_send_notify_to_continue_ota\r\n");
        bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, dat, 2);
        bls_att_pushNotifyData(OTA_CMD_OUT_DP_H_GL, report_data, 4);
        analog_write(DEEP_ANA_REG2, 0);
    }
}

void app_schedule_ota_wakeup_host(void)
{
#if 1
    u8 data = analog_read(DEEP_ANA_REG2);
    u8 flag=0;

    if((app_wakeup_src() == WAKEUP_NON_PAD) && (data) && (power_on_from == POWER_ON_DEEP))
    {
        flag = app_custom_is_enable_wakeup_key();
        if(flag)
        {
            printf("app_schedule_ota_wakeup_host\r\n");
            app_tx_power_wakeup(0xff);
        }
    }
#else
    printf("app_schedule_ota_wakeup_host\r\n");
    app_tx_power_wakeup(0xff);

#endif
}

#else // we have some dependancy with audio.c, so make some dummy here
void app_ota_status(u8 status){}
void app_schedule_ota_wakeup_host(void){}
void app_schedule_ota_send_notify_to_continue_ota(void){}
#endif //GOOGLE_OTA


#if (TELINK_OTA)

void app_enter_ota_mode(void)
{
    ota_is_working = 1;
    #if (BLT_APP_LED_ENABLE)
        device_led_setup(led_cfg[LED_OTA_START]);
    #endif
    bls_ota_setTimeout(150 * 1000 * 1000); //set OTA timeout  150 seconds
    bls_pm_setManualLatency(0);
    // bls_pm_setSuspendMask(SUSPEND_DISABLE);

}
void app_debug_ota_result(int result)
{
    printf("ota result: %d\n",result);
    #if(1 && BLT_APP_LED_ENABLE)  //this is only for debug
        gpio_set_output_en(GPIO_LED, 1);
        if(result == OTA_SUCCESS){  //led for debug: OTA success
          
        }
        else{  //OTA fail

            #if 1 //this is only for debug,  can not use this in application code
               /* irq_disable();
                WATCHDOG_DISABLE;
                while(1){
                    gpio_write(GPIO_LED, 1);
                    sleep_us(200000);
                    gpio_write(GPIO_LED, 0);
                    sleep_us(200000);
                }*/
                printf("ota fail ........\r\n");
               // gpio_write(GPIO_LED, 0);
                device_led_setup(led_cfg[LED_LONG_OFF]);
                ota_is_working=0;
            #endif
        }
        gpio_set_output_en(GPIO_LED, 0);
    #endif
}

#endif

//------------------- deepback proc ----------------------------------------------------
//This function process ...
void deep_wakeup_proc(void)
{

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
    //if deepsleep wakeup is wakeup by GPIO(key press), we must quickly scan this
    //press, hold this data to the cache, when connection established OK, send to maste

    // #define KEY_OK 31 //forgive me to make it dependent with app_ui.c
    //deepsleep_wakeup_fast_keyscan
    if(analog_read(USED_DEEP_ANA_REG) & POWER_ON_FLG){
        if(kb_scan_key (KB_NUMLOCK_STATUS_POWERON, 1) && kb_event.cnt){
            deepback_key_state = DEEPBACK_KEY_CACHE;
            key_not_released = 1;
            memcpy(&kb_event_cache,&kb_event,sizeof(kb_event));
            printf("key cnt : %d", kb_event.cnt);
            printf("key code : 0x%x\n", kb_event.keycode[0]);

            if ((kb_event.cnt == 1) && (kb_event.keycode[0] == KEY_OK)) {
                printf("deep back OK key , will need to send reconnect if bond\n"); //TODO
            }
        }

        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) & (~CONN_DEEP_FLG));
    }
#endif
}

void deepback_pre_proc(int *det_key)
{
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
    // to handle deepback key cache
    //if(!(*det_key) && deepback_key_state == DEEPBACK_KEY_CACHE && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && clock_time_exceed(bls_ll_getConnectionCreateTime(), 25000)){
    if(!(*det_key) && deepback_key_state == DEEPBACK_KEY_CACHE ){

        memcpy(&kb_event,&kb_event_cache,sizeof(kb_event));
        *det_key = 1;
        printf("deep back key:0x%x\n",kb_event_cache.keycode[0]);
        if(key_not_released || kb_event_cache.keycode[0] == VOICE){  //no need manual release
            deepback_key_state = DEEPBACK_KEY_IDLE;
        }
        else{  //need manual release
            deepback_key_tick = clock_time();
            deepback_key_state = DEEPBACK_KEY_WAIT_RELEASE;
            printf("wait release\n");

        }
    }
#endif
}

void deepback_post_proc(void)
{
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
    //manual key release
    if(deepback_key_state == DEEPBACK_KEY_WAIT_RELEASE && clock_time_exceed(deepback_key_tick,150000)){
        key_not_released = 0;
        u8         key_buf[2] = {0};
        key_buf[1] = 0;
        key_buf[2] = 0;
        bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 2); //release
        deepback_key_state = DEEPBACK_KEY_IDLE;
    }
#endif
}
#if BLE_AUDIO_ENABLE

#include "application/audio/gl_audio.h"


//typedef enum{
//    APP_ACCESSIBILITY_SHORTCUT_NONE=0,
//    APP_ACCESSIBILITY_SHORTCUT_PAIRING,
//    APP_ACCESSIBILITY_SHORTCUT_FACTORYRESET,
//    APP_ACCESSIBILITY_SHORTCUT_BUGREPORT,
//    APP_ACCESSIBILITY_SHORTCUT_SINGLEKEY
//}APP_Accessibility_Shortcut_Mode;


_attribute_data_retention_     u16 SpecialKey = 0;


int app_special_key_delay_timer(void)
{
    printf("app_special_key_delay_timer\r\n");
    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&SpecialKey, 2);
    return -1;
}


#if (STUCK_KEY_PROCESS_ENABLE)
     extern u32 stuckkey_keypresstimer;
#endif

extern void app_adv_indirect(void);
//u8 app_repairing(void) // used for terminate
//{
//    if(wait_repair)
//    {
//         printf("app_repairing\r\n");
//         app_adv_indirect(); // start re pair
//         wait_repair = FALSE;
//         notify_get_rsp_en = 0;
//         return 1;
//    }
//    return 0;
//}

#define  KEY_CACHE_TIMEOUT   8000000
#define  KEY_CACHE_MAX       22

_attribute_data_retention_ u8  app_key_store_count=0;
_attribute_data_retention_ u8  app_key_store_buffer[KEY_CACHE_MAX]={0};
#if 0 //no more used
u8 app_keycache(u8 keyindex)
{
    u8 i;

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    if(bond_number == 0) return 0;
    if(device_in_connection_state == 0)
    {
        if(app_key_store_count < KEY_CACHE_MAX)
        {
            for(i=0;i<app_key_store_count;i++)
            {
                if(app_key_store_buffer[i] == keyindex)
                {
                    i = 0x55;
                    break;
                }
            }
            if(i != 0x55) app_key_store_buffer[app_key_store_count++] = keyindex;
        }
        else
        {
            app_key_store_count = KEY_CACHE_MAX;
            for(i=0; i<KEY_CACHE_MAX-1; i++)
            {
                app_key_store_buffer[i] = app_key_store_buffer[i+1];
            }
            app_key_store_buffer[i] = keyindex;
        }
        printf("app_keycache =  ");
        for(i=0;i<app_key_store_count;i++)
            printf(" %x",app_key_store_buffer[i]);
        return 0;
    }
    else if(device_in_connection_state && app_key_store_count != 0)
    {
        if(app_key_store_count < KEY_CACHE_MAX)
        {
            app_key_store_buffer[app_key_store_count++] = keyindex;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

int app_cachekey_send_timer(void)
{
    u8 i;
    u8 key0;
    u8 key_value;
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u8 consumer_key[2]={0};

    if(device_in_connection_state == 0) return 0;
    if(app_key_store_count == 0) return -1;
    printf("store_count=%x\r\n",app_key_store_count);

    key0 = app_key_store_buffer[0];
    printf("send cache key key=%x\r\n",key0);
    {
#if (APP_IR_ENABLE)
        programming_key_set(0);
#endif
        key_value = kb_map_ble[key0];
        if(key_value >= CR_MEDIA_KEY_INDEX )
        {
            if(key_value == CR_DASHBOARD)
            {
                consumer_key[0] = key_setting_or_noti[1];
            }
            else if(key_value == CR_GUIDE)
            {
                consumer_key[0] = key_guide_or_livetv[1];
            }
            else if(key_value == CR_SUBTITLE)
            {
                consumer_key[0] = key_subtitle_or_teletext[1];
            }
            else
                consumer_key[0] = vk_consumer_map[key_value-CR_MEDIA_KEY_INDEX];
            consumer_key[1] = 0;
            printf("consume key=%x \r\n",consumer_key[0]);     //down
            bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
            consumer_key[0] = 0;                             //release
            consumer_key[1] = 0;
            bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);

        }
        else
        {
            printf("KEYBOARD_KEY_key_value=%x \r\n",key_value);
            key_buf[2] = key_value;                       //down
            bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);

            key_buf[2] = 0;                               //release
            bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
        }
    }

    for(i=1; i<app_key_store_count; i++)
    {
        app_key_store_buffer[i-1] = app_key_store_buffer[i];
    }
    app_key_store_count --;
    if(app_key_store_count == 0)
        return -1;
    else
        return 0;
}

#if 0 // not used
void app_key_send_store(void)
{
    u8 i;
    u8 key0;
    u8 key_value;
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u16 consumer_key[2]={0};

    if(app_key_store_count == 0) return;
    if(device_in_connection_state == 0) return;
    printf("store_count=%x\r\n",app_key_store_count);

    key0 = app_key_store_buffer[0];
    printf("send cache key key=%x\r\n",key0);
    /*
    if(ir_key_is_suppress(key0) == 0)
    {
        printf("is suppress\r\n");
        if(ir_fallback_send_key_code(key0,1))
        {
            ir_fallback_send(0);   //release
        }
    }
    else
    */
    {
        programming_key_set(0);
        key_value = kb_map_ble[key0];
        if(key_value >= CR_MEDIA_KEY_INDEX )
        {
            if(key_value == CR_DASHBOARD)
            {
                consumer_key[0] = key_setting_or_noti[1];
            }
            else if(key_value == CR_GUIDE)
            {
                consumer_key[0] = key_guide_or_livetv[1];
            }
            else if(key_value == CR_SUBTITLE)
            {
                consumer_key[0] = key_subtitle_or_teletext[1];
            }
            else
                consumer_key[0] = vk_consumer_map[key_value-CR_MEDIA_KEY_INDEX];
            consumer_key[1] = 0;
            printf("consume key=%x \r\n",consumer_key[0]);   //down
            bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            consumer_key[0] = 0;                             //release
            consumer_key[1] = 0;
            bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);

        }
        else
        {
            printf("KEYBOARD_KEY_key_value=%x \r\n",key_value);
            key_buf[2] = key_value;                       //down
            bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);

            key_buf[2] = 0;                               //release
            bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
        }
    }

    for(i=1; i<app_key_store_count; i++)
    {
        app_key_store_buffer[i-1] = app_key_store_buffer[i];
    }
    app_key_store_count --;
}
#endif

const u8 navi_key_index[4] = {1,4,15,18}; //left,right,up,down keyid in g10


u8 app_is_navi_key(u8 data0,u8 data1)
{
    u8 flag[2]={5,5};

    if(data0 == data1) return 0;
    for(u8 i=0;i<4;i++)
    {
        if(data0 == navi_key_index[i])
            flag[0] = i;
        if(data1 == navi_key_index[i])
            flag[1] = i;
    }
    if((flag[0]!=5) && (flag[1]!=5))
        return 1;
    return 0;
}

void app_release_hid_key(void)
{
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u8 consumer_key[2]={0};
    u16 i=0;

    if(device_in_connection_state)
    {
        while(bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2) && i<2000)
        {
            sleep_us(100);
            i++;
        }
        i = 0;
        while(bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8) && i<2000)
        {
            sleep_us(100);
            i++;
        }
        i = 0;
        while(app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000)
        {
            sleep_us(100);
            i++;
        }
        i =0;
        while(blc_ll_getTxFifoNumber() && i<400)
        {
            sleep_us(1000);
            i++;
        }
        printf("app_release_hid_key\r\n");
    }

}


#define FACTORY_RESET_TIMEOUT 100000

_attribute_data_retention_     u8 factory_reset_flag = 0;

#if (MP_TEST_MODE)
_attribute_data_retention_     u8 factory_reset_timer_count = 0;
_attribute_data_retention_     u8 factory_reset_temi_conn_flag = 0;

int app_factory_reset_timer(void)
{
    u8 bond_number;
    u16 i=0;
    if(device_in_connection_state)
    {
        printf("app_factory_reset_timer_google_voice_ctl=%x\r\n",google_voice_ctl);
        if(((google_voice_ctl & FLAG_AUDIO_CLOSE) == 0) || (ota_is_working))
        {
            if(ota_is_working)
            {
#if (GOOGLE_OTA)
                app_ota_slave_terminate();
                device_led_off(APP_LED_RED);
#endif
            }
            if(factory_reset_temi_conn_flag == 0)
            {
                i = 0;
                while(app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000){
                sleep_us(100);
                i++;
                }
            }
            else
            {
                if(blc_ll_getTxFifoNumber())
                {
                    factory_reset_temi_conn_flag++;
                    if(factory_reset_temi_conn_flag < 20)
                    {
                        return 0;
                    }
                }
            }
        }
        else
        {
            factory_reset_timer_count++;
            if(factory_reset_timer_count < 20)
            {
                return 0;
            }
        }
    }

    ir_flash_factory();
    flash_erase_sector(MAC_DATA_SECT_ADDR);
    bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    printf("bond_number2=%x\r\n",bond_number);
    if(bond_number)
    {
        printf("delete bond info\r\n");
        bls_smp_param_deleteByIndex(0);
    }
    factory_reset_flag = 0;
    factory_reset_temi_conn_flag = 0;
    factory_reset_timer_count = 0;
    google_voice_ctl = 0;
    ota_is_working = 0;
    return -1;
}
#endif

void app_mic_led(u8 en)
{
    if(audio_start_reason == REASON_PTT)
    {
        if(en)
        {
            device_led_setup(led_cfg[LED_AUDIO_ON]);
        }
        else
        {
            if(factory_reset_flag == 0)
            {
                device_led_setup(led_cfg[en ? 1 : 2]);
            }
        }
    }
    else
    {
        if(en)
        {
            device_led_setup(led_cfg[en ? 1 : 2]);
        }
        else
        {
            if(factory_reset_flag == 0)
            {
                device_led_setup(led_cfg[en ? 1 : 2]);
            }
        }
    }
}


 _attribute_data_retention_ u32 tick_led_pairing_after_connect = 0;
void app_led_batt_low(void)
{
    device_led_setup(led_cfg[LED_SHINE_LOWBAT]);
}
void app_led_connect(void)
{   
    tick_led_pairing_after_connect = 0;
    device_led_setup(led_cfg[LED_SHINE_SUCCESS]);
}

void app_led_pairing(void)
{
    device_led_setup(led_cfg[LED_SHINE_PAIRING]);
}
void app_led_long_on(void)
{
    device_led_setup(led_cfg[LED_LONG_ON]);
}

void app_led_ota_off(void)
{
    device_led_setup(led_cfg[LED_LONG_OFF]);
}

void app_debug_adv(){
    extern adv_type_t adv_type;
    if (adv_type== ADV_TYPE_CONNECTABLE_UNDIRECTED){
        printf("INDIRECT ");
    }
    else if (adv_type== ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY){
        printf("DIRECT ");
    }
    else{
        printf("adv %d ", adv_type);
    }
}

void app_led_proc2(){

    static u32 adv_led_tick=0;
    static u32 ftick_led_batt=0;

    if(clock_time_exceed(adv_led_tick, 1000*1000))
    {
        adv_led_tick = clock_time() | 1;

        //1 source key 长按,先启动LED指示(此时并未启动广播,按键松开时启动)
        if( ftick_key_source && clock_time_exceed(ftick_key_source, SRC_KEY_PRESS_TIME*1000)) {
            app_led_pairing();
        }

        if( ftick_key_back_home && clock_time_exceed(ftick_key_back_home, SRC_KEY_PRESS_TIME*1000)  ) {
            app_led_pairing();
        }
        //2 adv 长按Source键启动广播时亮灯,其他广播不亮灯
        if (ftick_user_pair_start) {
            if (blc_ll_getCurrentState() == BLS_LINK_STATE_ADV) {
                app_debug_adv();
                app_led_pairing();
                tick_led_pairing_after_connect = 0;
            } else {
                if (tick_led_pairing_after_connect && !clock_time_exceed(tick_led_pairing_after_connect, 10000*1000)) {
                    app_led_pairing();
                }
            }

        }

        //3 ota
        if(ota_is_working){
            device_led_setup(led_cfg[LED_OTA_START]);
        }
    }

    if(clock_time_exceed(ftick_led_batt, 2000*1000))
    {
        ftick_led_batt = clock_time() | 1;
        if( is_batt_low()){
            //Fast blink (2Hz) LED for 2s to indicate battery low
            app_led_batt_low();
        }
    }
}


//------------------- ui post proc--------------------------------------------------------------------

_attribute_data_retention_ bool send_ble_batt_poor = FALSE;
_attribute_data_retention_ bool send_ir_batt_poor = FALSE;
#define BLE_KEY_BATT_POOR 0x71 // spec 0x71

int send_low_power_ir_code(void)
{
    //single ir 000.195
    if (!ir_send_ctrl.is_sending){ 
        printf("send batt IR 195\n");
        ir_dispatch(TYPE_IR_SEND, FALSE, 195); // IR_KEY_BATT_POOR 195
        return -1;
    }
    else return 0;
}

void ui_post_process(){
    //if send batt poor IR ...
    if(send_ir_batt_poor){
        if (!ir_send_ctrl.is_sending){ 
            send_ir_batt_poor = FALSE;
            blt_soft_timer_add(&send_low_power_ir_code, 150*1000); 
        }
    }

    //if send batt poor BLE.., if release sent?
    if(send_ble_batt_poor){
        u8 key_buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

        key_buf[2] = BLE_KEY_BATT_POOR;
        bls_att_pushNotifyData(HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf,8);
        key_buf[2] = 0;
        bls_att_pushNotifyData(HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf,8);
        send_ble_batt_poor = FALSE;
    }


}

//------------------- key process--------------------------------------------------------------------
void app_erase_pair(){ //erase when connected, watch out ll_reject_ind
    //Press and hold the Back key (>3s) while inserting batteries will also erase pair
    //move codes here
    printf("app_erase_pair device_in_connection_state=[%d]\n",device_in_connection_state);
    device_led_setup(led_cfg[LED_SHINE_ERASE_PAIR]); //shine erase pair
    if(device_in_connection_state){
        printf("disconnect and unpair\n");
        app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
        user_unpair_wait_terminate = TRUE; // wait terminate then delete bonding
    }
    else{
        u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
        if(bond_number)   //at least 1 bonding device exist
        {
            printf("erase pair info 11111111\n");
            bls_smp_eraseAllParingInformation();
        }
        else{
            printf("no pair info to erase\n");
        }
    }
}

_attribute_data_retention_ u32 ftick_ok_reconn_send_code =0;
_attribute_data_retention_ u32 ftick_voice_reconn_send_code =0;


void send_ok_ble(){
    u8 consumer_key[2] = { 0 };

    printf("send ok ble and release\n\n\n");
    key_type = CONSUMER_KEY;
    //consumer_key[0] = CR_CENTER; //key ok
    consumer_key[0] = 0x0041;//vk_consumer_map[CR_CENTER - CR_MEDIA_KEY_INDEX]; //0x0041 for OK
    consumer_key[1] = 0;

    printf("consume key ok =%x \r\n", consumer_key[0]);
    bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, (u8 *) &consumer_key, 2);

    consumer_key[0] = 0;                             //release
    consumer_key[1] = 0;
    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);

    google_voice_dpad_select();
}

void send_voice_ble(){
#if (BLE_AUDIO_ENABLE)
    if (google_voice_ctl & FLAG_NOTIFY_AUDIO_DATA) return;
    printf("send_voice_ble get_cap_received=[%d]\n",get_cap_received);
    if (!get_cap_received) {
        ftick_post_process_voice_key = clock_time() | TRUE;
        printf(" need to wait CAP to post start voice\n");
        return ;
    }
    key_type = VOICE_KEY;
    WaitMs(200); //receive caps wait 20 ms  start voice
    google_voice_start(); // voice key to tv , not vocie start
    active_mic_open(); // just for debug mic open time out, no special function
#endif

}

void reconn_send_voice_ble(){
#if (BLE_AUDIO_ENABLE)
    if (google_voice_ctl & FLAG_NOTIFY_AUDIO_DATA) return;
    printf("reconn_send_voice_ble get_cap_received=[%d]\n",get_cap_received);
    if (!get_cap_received) {
        ftick_post_process_voice_key_reconn = clock_time() | TRUE;
        printf(" need to wait CAP to post start voice\n");
        return ;
    }
    key_type = VOICE_KEY;
    WaitMs(200); //receive caps wait 20 ms  start voice
    google_voice_start(); // voice key to tv , not vocie start
    active_mic_open(); // just for debug mic open time out, no special function
#endif

}

extern _attribute_data_retention_    u8    sendTerminate_before_enterDeep;
void ui_key_post_process()
{
    if (ftick_vol_up_pressed && clock_time_exceed(ftick_vol_up_pressed, 3*1000*1000)) {
        ftick_vol_up_pressed = 0;
        bls_ll_setAdvEnable(0);
        device_led_setup(led_cfg[LED_LONG_ON]);
        // 进入单载波模式
        rf_emi_single_tone(RF_POWER_P3p33dBm, 42);
        while (1) {
            if(kb_scan_key (0, 1)){
               //printf("kb_scan_key 222222222\n");
               break;
            }
        }
        WaitMs(7000);
        device_led_setup(led_cfg[LED_LONG_OFF]);
        rf_emi_stop();
        cpu_sleep_wakeup(DEEPSLEEP_MODE,PM_WAKEUP_PAD,0);
    }

    // 上电是按住back键,清除配对信息
    if(ftick_power_on_key_pressed && clock_time_exceed(ftick_power_on_key_pressed, 3*1000*1000) ){ // spec 3s
        ftick_power_on_key_pressed =0;
        //go clear bonding
        app_erase_pair();
    }

    if(ftick_key_back_home && clock_time_exceed(ftick_key_back_home, SRC_KEY_PRESS_TIME*1000) ){ //wait 1000ms get_cap
        ftick_key_back_home =0;//time out
        process_key_back_home_released();
      /*  if(!device_in_connection_state ){    // 如果设备没处于连接状态,清除配对信息并启动新的配对广播
                if(blc_ll_getCurrentState() != BLS_LINK_STATE_ADV){
                    printf("erase pair info ftick_key_back_home is press\n");
                    bls_smp_eraseAllParingInformation();
                    WaitMs(20); //wait flash IO
                    // single frame, no need release
                    ir_dispatch(TYPE_IR_SEND, 0, 183); // src key ir code 183(0xB7) to trigger TV to pair page
                    printf("back home key to send ir code 183\n");
                    bls_ll_setAdvDuration(MY_USER_PAIR_IND_ADV_TIME, 1);
                    app_adv_indirect(); // key src to start pair
                    ftick_user_pair_start = clock_time()|TRUE; // to avoid re-enter pair mode
                }
                else printf("back home key to adving\n");
            }
            else{
                printf("back home key to terminate\n");
                app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
                user_pair_wait_terminate = TRUE; // wait terminate then set ftick_ir_release_n_indirect_adv to start
                ir_dispatch(TYPE_IR_SEND, 0, 183); // src key ir code 183(0xB7) to trigger TV to pair page
                printf("back home key to send ir code 183\n");
            }
            */

    }

    if(ftick_keyok_back_press_time && clock_time_exceed(ftick_keyok_back_press_time, 3*1000*1000) ){ // down + ok 键 按3s
        ftick_keyok_back_press_time = 0;
        process_keyback_ok_googldebug();
    }

    if(ftick_keyback_down_press_time && clock_time_exceed(ftick_keyback_down_press_time, 7*1000*1000) ){ // down + back 键 按7s
        ftick_keyback_down_press_time = 0;
        process_keyback_down_googletalkback();
    }


    if(ftick_mute_digit0_press_time && clock_time_exceed(ftick_mute_digit0_press_time, 3*1000*1000) ){ // spec 3s
        ftick_mute_digit0_press_time = 0;
        app_erase_pair();
    }

    if(ftick_user_pair_start && clock_time_exceed(ftick_user_pair_start, 60*1000*1000) ){ // 60s
        ftick_user_pair_start = 0;  // re enable enter pair mode
        is_cleanconn_repairing_mode=0;
        spcial_combine_key = 0;
        bls_ll_setAdvEnable(0);
    }

    //use OK_KEY_DIRECT_ADV_TIME + 200 ms ?
    if(ftick_ok_reconn_send_code && clock_time_exceed(ftick_ok_reconn_send_code, 200*1000) ){ // check how fast tv reconnect
        ftick_ok_reconn_send_code = 0;  // re enable enter pair mode
        printf("send ok ir single\n");
        sendTerminate_before_enterDeep=0;
        if (ir_send_ctrl.is_sending == IR_SENDING_NONE) {
            ir_dispatch(TYPE_IR_SEND, FALSE, IR_OK); //simplt send a single key , no worry about release
        }
    }



    //or use blt_soft_timer_add(app_send_ir_release, 11000000);
//    if(ftick_to_send_release && clock_time_exceed(ftick_to_send_release, 110*1000) ){ //110ms
//        ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
//        ftick_to_send_release = 0;// src key process done
//    }

    //or use blt_soft_timer_add(app_send_ir_release_n_indirect, 11000000);
    if(ftick_ir_release_n_indirect_adv && clock_time_exceed(ftick_ir_release_n_indirect_adv, 110*1000) ){ //110ms
        ftick_ir_release_n_indirect_adv = 0;// clear

        WaitMs(10); //wait ir release
        bls_ll_setAdvDuration(MY_USER_PAIR_IND_ADV_TIME, 1);
        app_adv_indirect(); // key src to start pair
        ftick_user_pair_start = clock_time()|TRUE; // to avoid re-enter pair mode

    }

    if(ftick_post_process_voice_key && !clock_time_exceed(ftick_post_process_voice_key, 5000*1000) ){ //wait 1000ms get_cap
          if(get_cap_received){
            printf("post start voice\n");
            key_type = VOICE_KEY;
            WaitMs(1000); //receive caps wait 20 ms  start voice
            google_voice_start(); // voice key to tv , not vocie start
            active_mic_open(); // just for debug mic open time out, no special function
            ftick_post_process_voice_key = 0;
        }
    }

    if(ftick_post_process_voice_key_reconn && !clock_time_exceed(ftick_post_process_voice_key_reconn, 5000*1000) ){ //wait 1000ms get_cap
          if(get_cap_received){
            printf("post start voice ftick_post_process_voice_key_reconn\n");
            key_type = VOICE_KEY;
            WaitMs(1000); //receive caps wait 20 ms  start voice
            google_voice_start(); // voice key to tv , not vocie start
            active_mic_open(); // just for debug mic open time out, no special function
            ftick_post_process_voice_key_reconn = 0;
   // if((audio_start_reason == REASON_HTT) && (google_voice_ctl & FLAG_NOTIFY_AUDIO_DATA)){
            printf("key release_audio_start_reason 1:0x%x\n",audio_start_reason);
            google_voice_ctl |= FLAG_AUDIO_CLOSE;
            audio_stop_reason = REASON_RELEASE_HTT;
            audio_start_reason = 0;
            ui_enable_mic(0);
            app_active_remote_timer=0;
            app_audio_transfer_timer = 0;
        // }
        u8 consumer_key[2]={0};
        consumer_key[0] = 0;
        consumer_key[1] = 0;
        bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
        }
    }

    if(ftick_post_process_voice_key && clock_time_exceed(ftick_post_process_voice_key, 5000*1000) ){ //wait 1000ms get_cap
        printf("revice get_caps time out ...\r\n");
        ftick_post_process_voice_key =0;//time out
        ui_enable_mic(0);
        google_voice_ctl |= FLAG_AUDIO_CLOSE;
        audio_stop_reason = REASON_RELEASE_HTT;
        audio_start_reason = 0;
        app_active_remote_timer=0;
        app_audio_transfer_timer = 0;
    }
}




// #define AD_KEY_PRESS_TIME 2000//spec 2000ms
// void process_key_ad(){

//     if( ftick_key_ad && clock_time_exceed(ftick_key_ad, AD_KEY_PRESS_TIME*1000) ) {
//         ir_dispatch(TYPE_IR_SEND, FALSE, 105); // info 105 , single frame
//     }
//     else{
//         ir_dispatch(TYPE_IR_SEND, FALSE, 15); // AD 015
//     }
//     ftick_key_ad = 0; //clear

// }

void process_key_guide_release()
{   
    // GUIDE/LIST键长按,发送LIST(000.204)
    if(ftick_guide_key && clock_time_exceed(ftick_guide_key, 2000*1000)) {
        ir_dispatch(TYPE_IR_SEND, FALSE, 210);
    } 
    // GUIDE/LIST键短按,发送GUIDE(000.204)
    else {
        ir_dispatch(TYPE_IR_SEND, FALSE, 204);
    }
    ftick_guide_key = 0;
}
void process_key_options_release()
{   
    // GUIDE/LIST键长按,发送LIST(000.204)
    if(ftick_options_key && clock_time_exceed(ftick_options_key, 2000*1000)) {
        ir_dispatch(TYPE_IR_SEND, FALSE, 15);
    } 
    // GUIDE/LIST键短按,发送GUIDE(000.204)
    else {
        ir_dispatch(TYPE_IR_SEND, FALSE, 64);
    }
    ftick_options_key = 0;
}

#define SRC_KEY_2_PAIR_IR_CODE 183

void process_key_source_release(){

    if(ftick_key_source) {
        // 短按(<2S)
        if (!clock_time_exceed(ftick_key_source, SRC_KEY_PRESS_TIME*1000)) {
            // 停止配对广播
            if (ftick_user_pair_start){ // pair mode and short press src
                ftick_user_pair_start=0; // clear pair mode
                bls_ll_setAdvEnable(0); // should not come here
                device_led_off(APP_LED_BLUE);
            }
            // 发送Source按键
            printf("src key to send ir code 56\n");
            ir_dispatch(TYPE_IR_SEND, 0, 56); // src key ir code 56
        }
        // 长按(>2S)
        else {
            ftick_voice_reconn_send_code=FALSE;
            if(!device_in_connection_state ){    // 如果设备没处于连接状态,清除配对信息并启动新的配对广播
                if(blc_ll_getCurrentState() != BLS_LINK_STATE_ADV){
                    is_cleanconn_repairing_mode=0;
                    printf("erase pair info 2222222222222\n");
                    bls_smp_eraseAllParingInformation();
                    WaitMs(20); //wait flash IO
                    // single frame, no need release

                    ir_dispatch(TYPE_IR_SEND, 0, SRC_KEY_2_PAIR_IR_CODE); // src key ir code 183(0xB7) to trigger TV to pair page  
                    printf("zfm src key to send ir code 183\n");
                    bls_ll_setAdvDuration(MY_USER_PAIR_IND_ADV_TIME, 1);
                    app_adv_indirect(); // key src to start pair
                    ftick_user_pair_start = clock_time()|TRUE; // to avoid re-enter pair mode
                }
                else printf("src key to adving\n");
            }
            else{
                printf("src key to terminate\n");
                user_pair_wait_terminate = TRUE; // wait terminate then set ftick_ir_release_n_indirect_adv to start
                is_cleanconn_repairing_mode=1;
                app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
                ir_dispatch(TYPE_IR_SEND, 0, SRC_KEY_2_PAIR_IR_CODE); // src key ir code 183(0xB7) to trigger TV to pair page  
                printf("zfm src key to send ir code 183\n");
            }
        }
    }
    ftick_key_source =0;//clear // no matter src key time out or not, clear!
}

void process_keyback_ok_googldebug(){
    u8 key0 = kb_event.keycode[0];
    u8 key1 = kb_event.keycode[1];
    u8 key_value;
    u8 consumer_key[2]={0};
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber(); // get bonded device number
    if (device_in_connection_state)
    {
        printf("ble key 0x00EC\n");
        key_type = CONSUMER_KEY;
        consumer_key[0] = 0xEC;//back + ok key
        consumer_key[1] = 0;
        bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H,(u8 *) &consumer_key, 2);
        if (app_is_navi_key(key0, key1) == 0)
        {
            printf(" stop double key in ir mode\r\n");
            // blt_soft_timer_delete(app_ir_bugreport_key_delay_timer);
            printf("ir_send_ctrl.is_sending =%x\r\n", ir_send_ctrl.is_sending);
            if (ir_send_ctrl.is_sending)
            {
                key_type = IR_KEY;
                printf("INTO  ir_send_release \r\n");
                ir_send_release();
                ir_not_released = 0;
            }
        }
        // send release later release come
        // key_buf[2] = 0;
        // bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
    }
    else
    {
        if (ir_send_ctrl.is_sending)
        {
            ir_dispatch(TYPE_IR_RELEASE, 0, 0); // release
            WaitMs(20);
        }
        key_type = IR_KEY;
        printf("send ir repat=[0] ir=157 ir0x=9D\r\n");
        ir_dispatch(TYPE_IR_SEND, FALSE, 157);
        WaitMs(20);
        key_type = IR_KEY;
        ir_send_release();
        ir_not_released = 0;
    }
}


//  7S 触发 googletalkback
void process_keyback_down_googletalkback(){
    u8 key0 = kb_event.keycode[0];
    u8 key1 = kb_event.keycode[1];
    u8 key_value;
    u8 consumer_key[2] = {0};
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber(); // get bonded device number
    if (device_in_connection_state)
    {
        printf("ble key 0x00ED\n");
        key_type = CONSUMER_KEY;
        consumer_key[0] = 0xED;//back + down key
        consumer_key[1] = 0;
        bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H,(u8 *) &consumer_key, 2);
        if (app_is_navi_key(key0, key1) == 0)
        {
            printf(" stop double key in ir mode\r\n");
            // blt_soft_timer_delete(app_ir_bugreport_key_delay_timer);
            printf("ir_send_ctrl.is_sending =%x\r\n", ir_send_ctrl.is_sending);
            if (ir_send_ctrl.is_sending)
            {
                key_type = IR_KEY;
                printf("INTO  ir_send_release \r\n");
                ir_send_release();
                ir_not_released = 0;
            }
        }
        // send release later release come
        // key_buf[2] = 0;
        // bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
    }
    else
    {
        if (ir_send_ctrl.is_sending)
        {
            ir_dispatch(TYPE_IR_RELEASE, 0, 0); // release
            WaitMs(20);
        }
        key_type = IR_KEY;
        // printf("send ir repat=[0] ir=114 ir0x=72\r\n");
        ir_dispatch(TYPE_IR_SEND, FALSE, 114);
        WaitMs(20);
        key_type = IR_KEY;
        ir_send_release();
        ir_not_released = 0;
    }
}

void process_key_list_release(){
    if(ftick_key_list) {
        // 短按(<2S)
        if (!clock_time_exceed(ftick_key_list, SRC_KEY_PRESS_TIME*1000)) {
            // 发送list 红外码 210
            printf("src key to send ir code 210\n");
            ir_dispatch(TYPE_IR_SEND, true, 210); // src key ir code 210
            WaitMs(20);
            key_type = IR_KEY;
            ir_send_release();
            ir_not_released = 0;
        }else {
            if (ir_send_ctrl.is_sending)
            {
                ir_dispatch(TYPE_IR_RELEASE, 0, 0); // release
                WaitMs(20);
            }
            key_type = IR_KEY;
            printf("src key to send ir code 15\n");
            ir_dispatch(TYPE_IR_SEND, 0, 15); // src key ir code 183(0xB7) to trigger TV to pair page 
            WaitMs(20);
            key_type = IR_KEY;
            ir_send_release();
            ir_not_released = 0;
         }
    }
    ftick_key_list =0;//clear // no matter src key time out or not, clear!
}

void process_batt_poor(){ //set flag to delay send
#define KCOUNT_BASE 10 // spec 10
    _attribute_data_retention_ static u16 key_count=0;
    if(is_batt_poor()){
        key_count++;
        if (key_count == 10) {
            send_ir_batt_poor = TRUE;
        }

        if (key_count == 20) {
            if (device_in_connection_state) {
                send_ble_batt_poor = TRUE;
            } else {
                send_ir_batt_poor = TRUE;
            }
        }

        if (key_count >= 20) key_count = 0;
    }

    if(is_batt_good()){
        key_count=0;
    }

}

void process_ok_key(){
 //1 connected, send ok search
    if (device_in_connection_state) {

#if 0
        //send IR
        ir_dispatch(TYPE_IR_SEND, FALSE, IR_OK); //connected also send IR
#else
        //or ble
        u8 consumer_key[2]={0};
        key_type = CONSUMER_KEY;
        consumer_key[0] = 0x41;//ok key
        consumer_key[1] = 0;

        printf("ok key consume key=%x \r\n", consumer_key[0]);
        bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H,(u8 *) &consumer_key, 2);
#endif
#if (BLE_AUDIO_ENABLE)
        // printf("OK_KEY search\n");
        // google_voice_dpad_select();
        //active_mic_open(); // just for debug mic open time out, no special function
#endif


    }
   
    //2  bond , not connected, send adv
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    //if(bond_number && (blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) && (blc_ll_getCurrentState() != BLS_LINK_STATE_ADV) ){
    if(bond_number && (blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) ){
        printf("process_ok_key rc_reconn_normal=[%d]",rc_reconn_normal);
        if (rc_reconn_normal) {
            // 先发送回连广播
            ftick_user_pair_start=0;
            direct_adv(0, 0, 3*1000*1000);
            printf("ftick_ok_reconn_send_code enabled \n");
            // 一定时间内未回连成功再发送红外键值
            ftick_ok_reconn_send_code = TRUE| clock_time(); //send ir single if not connected in 50ms
        }
        else { //fall back
            ftick_user_pair_start=0;
            direct_adv(0,0,3*1000*1000);//3s
            // 直接发送红外键值
            if (ir_send_ctrl.is_sending == IR_SENDING_NONE) {
                key_not_released = 1;
                key_type = IR_KEY;
                ir_dispatch(TYPE_IR_SEND, TRUE, IR_OK);
                ir_not_released = 1;
            }
        }
    }

    //3 not bond, send IR , and ok search
    if(((bond_number == 0 ) || (blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) || !device_in_connection_state) && ftick_ok_reconn_send_code == 0) {
        if (ir_send_ctrl.is_sending == IR_SENDING_NONE) {
            key_not_released = 1;
            key_type = IR_KEY;
            ir_dispatch(TYPE_IR_SEND, TRUE, IR_OK);
            ir_not_released = 1;
        }
    }   
    ftick_voice_reconn_send_code=false;
}




void process_voice_key(){  
    //1  connected , send search
    if (device_in_connection_state) {
#if (BLE_AUDIO_ENABLE)
        if (google_voice_ctl & FLAG_NOTIFY_AUDIO_DATA) return;
        printf("VOICE_KEY get_cap_received=[%dd]\n",get_cap_received);
        if (!get_cap_received) {
           // WaitMs(2000); //receive caps wait 20 ms  start voice
            ftick_post_process_voice_key = clock_time() | TRUE;
            printf(" need to wait CAP to post start voice\n");
            return ;
        }
        WaitMs(20); //receive caps wait 20 ms  start voice
        key_type = VOICE_KEY;
        google_voice_start(); // voice key to tv , not vocie start
        active_mic_open(); // just for debug mic open time out, no special function
#endif

    }
    //2  bond , not connected , send adv
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    if(bond_number && (blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) && (blc_ll_getCurrentState() != BLS_LINK_STATE_ADV) ){
        if (rc_reconn_normal) {
            // dir 50ms, time out set rc_reconn_normal = FALSE;
            ftick_user_pair_start=0;
            direct_adv(0,0, 3*1000*1000); //at least
            ftick_voice_reconn_send_code = TRUE|clock_time();
        }
        else { //fall back
            //ind 3s --> if connected --> normal
            ftick_user_pair_start=0;
            direct_adv(0,0,3*1000*1000);//3s
            ftick_voice_reconn_send_code = TRUE|clock_time();
         }
    }
    //3 not bond , send ir
    if(bond_number ==0 ){
        key_type = IR_KEY;
        ir_dispatch(TYPE_IR_SEND, TRUE, IR_VOICE);
        ir_not_released = 1;
    }
}

//#if VOICE_DIRECT_TEST.....
void voice_direct_test(){
    extern u8 app_audio_send_index;
    extern void google_mic_enable(void);
    extern u32 app_audio_transfer_timer;

    if (!ui_mic_enable) {
        google_voice_codec_used = CODEC_USED_16K;
        google_voice_packet_length = VOICE_V0P4_ADPCM_PACKET_LEN;
        google_voice_pcm_sample_packet = VOICE_V0P4_ADPCM_UNIT_SIZE;
        app_audio_send_index = 0;
        google_mic_enable();
        app_audio_transfer_timer = 0;
    }
    else{
        ui_enable_mic(0);
    }
}


bool is_ambilight_pressed = FALSE;
void process_1_key(){
    u8 key0 = kb_event.keycode[0];
    //u8 key1 = kb_event.keycode[1];
    u8 key_value;
    printf(" zfm into process_1_key key0 =[%d] spcial_combine_key=[%d]\r\n",key0,spcial_combine_key);
    //return;
    if(spcial_combine_key == 1)
    {
        if (ir_send_ctrl.is_sending)
        { 
            key_type = IR_KEY;
            printf("INTO  ir_send_release \r\n");
            ir_send_release();
            ir_not_released = 0;
        }
        return;
    }
    WaitMs(40);
    //app_keypress_led(0);
    advertise_tick_retime(); // recount advertise_begin_tick

#if (STUCK_KEY_PROCESS_ENABLE)
    stuckkey_keypresstimer = clock_time() | 1;
#endif

#if (PCB_VER == PCB_TELINK_827x)
    if (key0 == 28) {// MUTE to del bonding
        printf("del bonding when use 827x PCBA\n");
        bls_smp_eraseAllParingInformation();
        return;
    }
#endif

    // if SRC key , set timer to count , return
    // when release , check src key timer , if timer over 3s, send ADV, else send IR and send release after 1 frame 110MS
    if ((key0 == KEY_SOURCE) ){
        printf("KEY_SOURCE\n");
        key_type = IR_KEY;  // source补发hid键
        ftick_key_source = clock_time()|TRUE; // call to process_key_source()
        return;
    } 
    else {
        //if advertising, 任意其他按键将停止广播
        if ((blc_ll_getCurrentState() == BLS_LINK_STATE_ADV) ) {
            printf("stop adv\n");
            bls_ll_setAdvEnable(0);
            device_led_off(APP_LED_BLUE);
        }
        ftick_user_pair_start = 0; //enable to again enter pair        
    }

    /**
     * @brief 语音键单独处理
     * 
     */
    if (key0 == KEY_VOICE) {
        process_voice_key();

        //#if (VOICE_DIRECT_TEST)
        //voice_direct_test();
        return;
    }
/*
    if ((key0 == KEY_LIST) ){
        printf("KEY_LIST\n");
        key_type = IR_KEY;  
        ftick_key_list = clock_time()|TRUE; // 
        //process_key_list();
        return;
    } 
*/
    /**
     * @brief OK键单独处理
     * 
     */
    if (key0 == KEY_OK) {
        process_ok_key();
        return;
    }

    //active_mic_open(); // just for debug mic open time out, no special function
    key_type = IR_KEY; // to process IR release

#if (APP_IR_ENABLE)
    if (ir_key_is_suppress(key0) == 0) {
        printf("is suppress\r\n");
        //app_keypress_led(1);
        if (ir_fallback_send_key_code(key0, 1)) {
            if (!ir_not_released) ir_not_released = 1;
            return;
        }
    }
#endif
    // watch out fall through logic of programming_key_set and app_keycache here
    //if (app_keycache(key0) == 1) {return;}
#if (APP_IR_ENABLE)
    programming_key_set(0);
#endif

#if (AMBILIGHT_MODEL)
    if(key0 == KEY_AMBIL){
        printf("KEY_AMBIL\n");
        device_led_setup(led_cfg[LED_AMBILIGHT_PRESS]);
        is_ambilight_pressed = TRUE;
    }
#endif
    // GUIDE/LIST
    // 1. 小于2.0s（短按），在释放键时，RC传输（0xCC）命令（仅仅发送一帧代码）
    // 2. 大于2.0s（长按），在释放键时，RC传输（0xD2）命令（仅仅发送一帧代码）
    else if (key0 == KEY_GUIDE){
        //over 2s or less 2s
        ftick_guide_key = clock_time() | TRUE;
        return;
    }
    else if (key0 == KEY_OPTIONS){
        //over 2s or less 2s
        ftick_options_key = clock_time() | TRUE;
        return;
    }
    else{ // key other than src
        ;
    }

    // -----------send ble key....------------------------------
#if 0
    if (device_in_connection_state) {
        key_value = kb_map_ble[key0];
        printf("ble key_value 0x%x\n",key_value);
//        if (key_value >= CR_MEDIA_KEY_INDEX) {
//            key_type = CONSUMER_KEY;
//            consumer_key[0] = vk_consumer_map[key_value - CR_MEDIA_KEY_INDEX];//actually no need to remap...
//            consumer_key[1] = 0;
//
//            printf("consume key=%x \r\n", consumer_key[0]);
//            bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, (u8 *) &consumer_key, 4);
//            if (consumer_key[0] == G00GLE_MKEY_CENTER) {
//                google_voice_dpad_select();
//            }
//        }
//        else {
//            printf("KEYBOARD_KEY_key_value=%x \r\n",key_value);
//            key_type = KEYBOARD_KEY;
//            key_buf[2] = key_value;
//            bls_att_pushNotifyData(HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf,8);
//        }

    }
#endif
    // DPAD 按键
    // 1. BLE Advertising and Reconnection to TV. 
    // 2. Maximum BLE advertising time - 3s. RC goes to sleep mode if not connected to TV.
    if (key0 == KEY_UP || key0 == KEY_DOWN || key0 == KEY_LEFT || key0 == KEY_RIGHT) {
        //2  bond , not connected, send adv
        u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
        if(bond_number && (blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) ){
            //ind 3s --> if connected --> normal
            //indirect_adv(0,0,3000);//3s
            ftick_user_pair_start=0;
            direct_adv(0, 0, 3*1000*1000);
            sendTerminate_before_enterDeep=1;
        }       
    }

    if (1) { //send ir always
        //------------send IR--------------------------------
        key_value = kb_map_ir[key0];
        key_type = IR_KEY; // for key release proc
        printf("ir_not_released =[%d]  \n",ir_not_released);
        if (!ir_not_released) { // for key release process
            printf(" send ir:0x%x \n",key_value);
            ir_dispatch(TYPE_IR_SEND, TRUE, key_value);
            ir_not_released = 1;
        }
    }

}

void process_key_back_home_released(){
    u16 consumer_key[2] = { 0 };
    // 短按(<3S)
    if (!clock_time_exceed(ftick_key_back_home, 2000*1000)) {
        if (ir_send_ctrl.is_sending)
        {
            ir_dispatch(TYPE_IR_RELEASE, 0, 0); // release
            ir_not_released = 1;
        }
    }
    else {
        if (ir_send_ctrl.is_sending)
        {
            ir_dispatch(TYPE_IR_RELEASE, 0, 0); // release
            ir_not_released = 0;
        }
        WaitMs(200);
        ftick_voice_reconn_send_code=false;
        if(!device_in_connection_state ){    // 如果设备没处于连接状态,清除配对信息并启动新的配对广播
            if(blc_ll_getCurrentState() != BLS_LINK_STATE_ADV){
                printf("erase pair info ftick_key_back_home is press\n");
                bls_smp_eraseAllParingInformation();
                WaitMs(20); //wait flash IO
                // single frame, no need release
                ir_dispatch(TYPE_IR_SEND, 0, 183); // src key ir code 183(0xB7) to trigger TV to pair page
                is_cleanconn_repairing_mode=0;
                printf("back home key to send ir code 183\n");
                bls_ll_setAdvDuration(MY_USER_PAIR_IND_ADV_TIME, 1);
                app_adv_indirect(); // key src to start pair
                ftick_user_pair_start = clock_time()|TRUE; // to avoid re-enter pair mode
            }
            else printf("back home key to adving\n");
        }
        else{
            printf("back home key to terminate\n");
            app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
            user_pair_wait_terminate = TRUE; // wait terminate then set ftick_ir_release_n_indirect_adv to start
            ir_dispatch(TYPE_IR_SEND, 0, 183); // src key ir code 183(0xB7) to trigger TV to pair page
            is_cleanconn_repairing_mode=1;
            printf("back home key to send ir code 183\n");
        }
        WaitMs(20);
        key_type = IR_KEY;
        ir_send_release();
        ir_not_released = 1;
    }
    ftick_key_back_home =0;//clear // no matter src key time out or not, clear!
}


int stop_send_source_ir_code(void)  //发三帧红外的时间后停止发送
{
    if(ir_send_ctrl.is_sending)
    {
        ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
        printf("stop_send_source_ir_code\n");
    }  
    
    return -1; 
}
void process_2_keys(){

    u8 key0 = kb_event.keycode[0];
    u8 key1 = kb_event.keycode[1];
    u8 key_value;
    u8 key_buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    u16 consumer_key[2] = { 0 };

    //printf("process 2 keys\n");
    printf("intp process_2_keys ir_not_released=[%d] key0 %d, key1 %d\n", ir_not_released,key0, key1);

    advertise_tick_retime(); // recount advertise_begin_tick

    //if(wakeup_key_send_flag == 1) {wakeup_key_send_flag = 2;} // make set tx power run only once
   
    WaitMs(40);
    //if user have start pair...no worry..

#if (STUCK_KEY_PROCESS_ENABLE)
    stuckkey_keypresstimer = clock_time() | 1;
#endif

    if(( (key0 == KEY_BACK) && (key1 == KEY_HOME) ) || ( (key0 == KEY_HOME) && (key1 == KEY_BACK) )){
        spcial_combine_key = 1;
        key_value = kb_map_ir[key0];
        key_type = IR_KEY;  // source补发hid键
        ftick_key_back_home = clock_time()|TRUE; // call to process_key_source()
        printf("key_hom_back ir_not_released =[%d] send ir:0x%x senf ir:[%d] \n",ir_not_released, key_value,key_value);
        if (!ir_not_released) { 
            ir_dispatch(TYPE_IR_SEND, TRUE, key_value);
            ir_not_released = 1;
        }
    } 
    else if ( ( (key0 == KEY_MUTE) && (key1 == KEY_0) ) || ( (key0 == KEY_0) && (key1 == KEY_MUTE) ) )  {//mute and 0 to erase pair
        ftick_mute_digit0_press_time = clock_time()|TRUE;
        blt_soft_timer_add(&stop_send_source_ir_code, 245*1000);  //发3帧红外码后停止   245mS    
        key_type = IR_KEY;
        ir_send_release();
        ir_not_released = 0; 
    }
    else if ( ( (key0 == KEY_OK) && (key1 == KEY_BACK) ) || ( (key0 == KEY_BACK) && (key1 == KEY_OK) ) )  {//back and OK  长按“back+down”键3S
        ftick_keyok_back_press_time = clock_time()|TRUE;
        spcial_combine_key = 1;
        key_type = IR_KEY;
        ir_send_release();
        ir_not_released = 0;    
    }
    else if ( ( (key0 == KEY_DOWN) && (key1 == KEY_BACK) ) || ( (key0 == KEY_BACK) && (key1 == KEY_DOWN) ) )  {//back and down 长按“back+down”键7s，在3s的时间，关闭背光
        ftick_keyback_down_press_time = clock_time()|TRUE;
        spcial_combine_key = 1;
        key_type = IR_KEY;
        ir_send_release();
        ir_not_released = 0;    
    }
    else
    {
        if(device_in_connection_state == 0)
        {
            if(app_is_navi_key(key0,key1) == 0)
            {
                printf(" stop double key in ir mode\r\n");
                //blt_soft_timer_delete(app_ir_bugreport_key_delay_timer);
                printf("ir_send_ctrl.is_sending =%x\r\n",ir_send_ctrl.is_sending);
                if(ir_send_ctrl.is_sending)
                {
                    key_type = IR_KEY;
                    printf("INTO  ir_send_release \r\n");
                    ir_send_release();
                    ir_not_released=0;
                }
            }
            else
            {
                printf("double key is navi in ir mode\r\n");
                if(ir_send_ctrl.is_sending == IR_SENDING_NONE)
                {
//                    if(key0 == comb_key_keyid[4])              //setting
//                        key_value = key_setting_or_noti[0];
//                    else if(key0 == comb_key_keyid[5])         //guide
//                        key_value = key_guide_or_livetv[0];
//                    else if(key0 == comb_key_keyid[6])         //subtitle
//                        key_value = key_subtitle_or_teletext[0];
//                    else
                        key_value = kb_map_ir[key0]; // just send IR

                    key_type = IR_KEY;
                    ir_dispatch(TYPE_IR_SEND, FALSE, key_value);
                    if(ir_not_released == 0) ir_not_released = 1;
                }
            }
        }
        else
        {
            if(app_is_navi_key(key0,key1) == 0)
            {
                printf(" stop double key in rf mode\r\n");
                if(key_type == CONSUMER_KEY)
                {
                    consumer_key[0] = 0;
                    consumer_key[1] = 0;
                    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
                }
                if(key_type == KEYBOARD_KEY)
                {
                    key_buf[2] = 0;
                    bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
                }
                if(ir_send_ctrl.is_sending)
                {
                    key_type = IR_KEY;
                    printf("INTO  ir_send_release \r\n");
                    ir_send_release();
                    ir_not_released=0;
                }
            }
            else
            {
                printf("double key is navi in rf mode,key0=%x,key1=%x\r\n",key0,key1);
                u16 comb_key[2];
                comb_key[0] = vk_consumer_map[kb_map_ble[key0]-CR_MEDIA_KEY_INDEX];
                comb_key[1] = vk_consumer_map[kb_map_ble[key1]-CR_MEDIA_KEY_INDEX];
                printf("comb_key[0]=%02x,comb_key[1]=[%02x] comb_key[2]=[%02x] comb_key[3]=[%02x]\r\n",U16_LO(comb_key[0]),U16_HI(comb_key[0]),U16_LO(comb_key[1]),U16_HI(comb_key[1]));
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&comb_key, 4);
            }
        }
    }
}

//u32 fitck_ambilight =FALSE;
void process_key_release(){
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u8 consumer_key[2]={0};
    spcial_combine_key = 0;

    //printf("key release\r\n");
      printf("into process_key_release \r\n");
    advertise_tick_retime(); // recount advertise_begin_tick
    //ftick_voice_reconn_send_code=FALSE;
    if((audio_start_reason == REASON_HTT) && (google_voice_ctl & FLAG_NOTIFY_AUDIO_DATA)){
        printf("key release_audio_start_reason:0x%x\n",audio_start_reason);
        google_voice_ctl |= FLAG_AUDIO_CLOSE;
        audio_stop_reason = REASON_RELEASE_HTT;
        audio_start_reason = 0;
        ui_enable_mic(0);
        app_active_remote_timer=0;
        app_audio_transfer_timer = 0;
    }
    key_not_released = 0;
    //keyScanLongPressTick = 0;
    //LongPressTick = 0;

    ftick_power_on_key_pressed = 0;// clear power on pressed back key

    #if (STUCK_KEY_PROCESS_ENABLE)
    stuckkey_keypresstimer = 0;
    #endif
#if (APP_IR_ENABLE)
    if(is_programming_key_send() == 1)
    {
       printf("key up\r\n");
       ir_fallback_send(0);
    }
    programming_key_set(0);
#endif

    if(ftick_key_source){
        // process_key_source();
        process_key_source_release();
    }
/*
    if(ftick_key_list){
        // process_key_source();
        process_key_list_release();
    }
*/
    if(ftick_options_key){
        process_key_options_release();
    }

    if(ftick_guide_key) {
        process_key_guide_release();
    }
    
   
    // if(ftick_key_ad){
    //     process_key_ad();
    //     return;
    // }

    if(is_ambilight_pressed) {
        printf("ambilight on 5s\n");
        device_led_off(APP_LED_WHITE);
        device_led_setup(led_cfg[LED_AMBILIGHT_RELEASE]);

        //fitck_ambilight = clock_time()|TRUE;
        is_ambilight_pressed = FALSE;
    } else {
        device_led_off(APP_LED_WHITE);
    }

    if(key_type == CONSUMER_KEY)
    {
        consumer_key[0] = 0;
        consumer_key[1] = 0;
        bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
        if(ir_send_ctrl.is_sending)
        {
            ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
        }
    }
    else if(key_type == KEYBOARD_KEY)
    {
        key_buf[2] = 0;
        bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
        if(ir_send_ctrl.is_sending)
        {
            ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
        }
    }

#if (REMOTE_IR_ENABLE)
    else if(key_type == IR_KEY)
    {
        printf("keyreleased  ir_not_released=[%d]\r\n",ir_not_released);
        if(ir_not_released){
            ir_not_released = 0;
            //ir_not_released = ir_not_released -1;
            if(ir_send_ctrl.is_sending)
            {
                ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
            }
#if (APP_IR_ENABLE)
            ir_repeat_delay_release_time();
#endif
        }
    }
#endif

}

void key_change_proc(void)
{
    latest_user_event_tick = clock_time();  //record latest key change time
    
    ftick_power_on_key_pressed=0;
    ftick_keyok_back_press_time=0;
    ftick_mute_digit0_press_time=0;
    ftick_keyback_down_press_time=0;
    ftick_vol_up_pressed = 0;
    ftick_key_back_home = 0;
   // ftick_key_list =0;
    key_not_released = 1;
    if (kb_event.cnt == 1)      {process_1_key();}
    else if(kb_event.cnt == 2)  {process_2_keys();}
    else                        {
        process_batt_poor(); // count key
        process_key_release();
    }
}
//--------------------------------------------------------------------------------

#if (MP_TEST_MODE)
void app_factory_reset(void)
{
    u8 audio_open=0;

    printf("Factory Reset Mode ongoning\r\n");
    //batt low check
    //if(lowbat == 0x55) { return; }

    //global vari
    factory_reset_flag = 0x55;
    //is_reconn_mode = 0;
    //is_pairing_mode = 0;

    flag_ccc_data = 0;
    atv_char_ctl_ccc = 0;
    atv_char_rx_ccc = 0;
    write_ccc_info(&flag_ccc_data);
    //led
    device_led_setup(led_cfg[LED_SHINE_SUCCESS]);

    //ir
    write_ir_key_event_notify(0);
    ir_table_init();
    analog_write(DEEP_ANA_REG2, 0);
    bls_ll_setAdvEnable(0);
    if(device_in_connection_state)
    {
        if(ui_mic_enable)
        {
            audio_open = 1;
            extern u8 app_audio_key_stop(u8 reason);
            app_audio_key_stop(REASON_OTHERS);
        }
    }
    app_enter_deep_timeset();
    if(audio_open)
        blt_soft_timer_add(app_factory_reset_timer, FACTORY_RESET_TIMEOUT);
    else
        app_factory_reset_timer();

}
#endif

#define GPIO_WAKEUP_KEYPROC_CNT                3


_attribute_data_retention_    static int gpioWakeup_keyProc_cnt = 0;
_attribute_data_retention_    static u32 keyScanTick = 0;

void proc_keyboard (u8 e, u8 *p, int n)
{
    //when key press gpio wakeup suspend, proc keyscan at least GPIO_WAKEUP_KEYPROC_CNT times
    //regardless of 8000 us interval
    if(e == BLT_EV_FLAG_GPIO_EARLY_WAKEUP){
        gpioWakeup_keyProc_cnt = GPIO_WAKEUP_KEYPROC_CNT;
    }
    else if(gpioWakeup_keyProc_cnt){
        gpioWakeup_keyProc_cnt --;
    }


    if(gpioWakeup_keyProc_cnt || clock_time_exceed(keyScanTick, 8000)){
        keyScanTick = clock_time();
    }
    else{
        return;
    }

    kb_event.keycode[0] = 0;
    int det_key = kb_scan_key (0, 1);


#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
    if(deepback_key_state != DEEPBACK_KEY_IDLE){
        deepback_pre_proc(&det_key);
    }
#endif


    if (det_key){
#if (MP_TEST_MODE)
        if (test_get_mode() == MODE_TEST) test_proc_key();
        else
#endif
        key_change_proc();
    }

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
    if(deepback_key_state != DEEPBACK_KEY_IDLE){
        deepback_post_proc();
    }
#endif

    //app_key_send_store();
}
#endif




void app_ui_init_normal(void)
{
    /////////// keyboard gpio wakeup init ////////
    kb_wake_config();

    bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);

    #if (BLE_AUDIO_ENABLE)
        #if (BLE_DMIC_ENABLE)
            dmic_gpio_reset();
        #else
            amic_gpio_reset();
        #endif
    #endif


#if (BLT_APP_LED_ENABLE)
    device_led_init();  //LED initialization
    device_led_off(APP_LED_BLUE);
    device_led_off(APP_LED_WHITE);

    if((analog_read(USED_DEEP_ANA_REG) & POWER_ON_FLG) == 0)
    {
        // device_led_setup(led_cfg[LED_POWER_ON]);  //only poweron 2021.08.11, no need power on indication
        ;
    }
#endif
    analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | POWER_ON_FLG);// means system already cold powered on



#if (BLT_TEST_SOFT_TIMER_ENABLE)
    blt_soft_timer_init();

    if(
            (power_on_from == POWER_ON_NORMAL) // cold start
#if (MP_TEST_MODE)
            && (test_get_mode() != MODE_TEST)
#endif
      )
    {
        //dont need delay indirect
        //blt_soft_timer_add(app_poweron_timeout, 1000000);
    }
#endif

}



void app_ui_init_deepRetn(void)
{
    /////////// keyboard gpio wakeup init ////////
//    u32 pin[] = KB_DRIVE_PINS;
//    for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
//    {
//        cpu_set_gpio_wakeup (pin[i], Level_High, 1);  //drive pin pad high wakeup deepsleep
//    }
    kb_wake_config();

    #if (BLE_AUDIO_ENABLE)
        #if (BLE_DMIC_ENABLE)
            dmic_gpio_reset();
        #else
            amic_gpio_reset();
        #endif
    #endif

#if (BLT_APP_LED_ENABLE)
    device_led_init();  //LED initialization
#endif

#if (REMOTE_IR_ENABLE)
    //user_key_mode = analog_read(USED_DEEP_ANA_REG) & IR_MODE_DEEP_FLG ? KEY_MODE_IR : KEY_MODE_BLE;
#endif
}



extern u16 atv_char_ctl_ccc;
extern u16 atv_char_rx_ccc;

int att_ccc_control(void* p){
    printf("att_ccc_control\n");
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
    u16 handle = (pw->hh << 8) + pw->hl;

    u16 data = (pw->dat[1] << 8) + pw->dat[0];

    array_printf(pw->dat,(pw->l2cap - 3));

    u8 save_to_flash = 1;

    switch(handle){
        case AUDIO_GOOGLE_RX_CCC_H:
        {
            printf("AUDIO_GOOGLE_RX_CCC_H\n");

            atv_char_rx_ccc = data;

            if(data){
                if(flag_ccc_data & FLAG_GOOGLE_RX_CCC){
                    save_to_flash = 0;
                }else{
                    flag_ccc_data |= FLAG_GOOGLE_RX_CCC;
                }

            }else{
                if(flag_ccc_data & FLAG_GOOGLE_RX_CCC){
                    flag_ccc_data &= ~FLAG_GOOGLE_RX_CCC;
                }else{
                    save_to_flash = 0;
                }
            }
            break;
        }
        case AUDIO_GOOGLE_CTL_CCC_H:
        {
            printf("AUDIO_GOOGLE_CTL_CCC_H\n");

            atv_char_ctl_ccc = data;

            if(data){
                if(flag_ccc_data & FLAG_GOOGLE_CTL_CCC){
                    save_to_flash = 0;
                }else{
                    flag_ccc_data |= FLAG_GOOGLE_CTL_CCC;
                }

            }else{
                if(flag_ccc_data & FLAG_GOOGLE_CTL_CCC){
                    flag_ccc_data &= ~FLAG_GOOGLE_CTL_CCC;
                }else{
                    save_to_flash = 0;
                }
            }
            break;
        }
#if (GOOGLE_OTA)
        case OTA_CMD_OUT_CCC_H:
        {
            printf("OTA_CMD_OUT_CCC_H\n");

            extern u16 my_Otaccc;
            my_Otaccc = data;

            save_to_flash = 0;
            break;
        }
#endif
        default:
        {
            save_to_flash = 0;
            break;
        }
    }

    if(save_to_flash){
        //save ccc data to flash
        write_ccc_info(&flag_ccc_data);
    }

    return 0;
}

u8 app_is_key_released(void)
{
    return key_not_released;
}


