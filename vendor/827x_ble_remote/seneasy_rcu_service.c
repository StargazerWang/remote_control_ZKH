/** 
********************************************************************************************************* 
*               Copyright(c) 2021, Seneasy Corporation. All rights reserved. 
**********************************************************************************************************
 * @ file:  
 * @ brief:  
 * @ details:  
 * @ author: 张凯华
 * @ Create Time: 2021-04-07
 * @ version: V1.0
 *********************************************************************************************************/
#include "tl_common.h"
#include "application/audio/audio_config.h"
#include "stack/ble/ble.h"
#include "app_audio.h"


extern int  device_in_connection_state;
extern u8 user_pair_wait_terminate;
extern void app_erase_pair();

u16 char_desc_smart_rcu_ccc = 0;

int seneasy_rcu_svc_attr_write_cb(void *p) 
{
    u8 sendBuff[2] = {0};
    rf_packet_att_data_t *pw = p;
    if ((pw->l2cap - 3) == 1 && pw->dat[0] == 0xF5) {
       // app_erase_pair();
        
        u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
        if(bond_number)   //at least 1 bonding device exist
        {
            printf("erase pair info 6666666\n");
            bls_smp_eraseAllParingInformation();
        }
        sendBuff[0] = 0x01;
        sendBuff[1] = 0x00;
        bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, sendBuff, 2);
        WaitMs(625);
        sendBuff[0] = 0x00;
        sendBuff[1] = 0x00;
        bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, sendBuff, 2);
        WaitMs(25);
        cpu_sleep_wakeup(DEEPSLEEP_MODE,PM_WAKEUP_PAD,0);
    } 
    
    if ((pw->l2cap - 3) == 1 && pw->dat[0] == 0xF4) {
       // app_erase_pair();
        sendBuff[0] = 0x01;
        sendBuff[1] = 0x01;
        bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, sendBuff, 2);
    }
    return 1;
}

int seneasy_rcu_svc_attr_read_cb(void *p) 
{
    return 1;
}

int seneasy_rcu_ccc_update_cb(void *p) 
{
    return 1;
}