/******************************************************************************
 * @file     app.c
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

#include "application/keyboard/keyboard.h"

#include "vendor/common/blt_led.h"
#include "vendor/common/blt_soft_timer.h"
#include "vendor/common/blt_common.h"

#include "app_ui.h"
#include "battery_check.h"
#include "rc_ir.h"
#include "app_audio.h"
#include "app_flash_write.h"
#include "application/audio/gl_audio.h"
#include "app_ota.h"
#include "app_ir.h"
#include "app_custom.h"
#include "app_config.h"
#include "app_flash_write.h"

#include "../../application/audio/gl_audio.h"


#if (MP_TEST_MODE)
#include "app_test.h"
#endif


#if (__PROJECT_8258_BLE_REMOTE__ || __PROJECT_8278_BLE_REMOTE__)


#define     ADV_INTERNAL_SWITCH_TIME            30  //30s
//#define   ADV_IDLE_ENTER_DEEP_TIME            60  //spec 60s
#define     PAIR_ADV_ENTER_DEEP_TIME            60  //spec 60s
#define     RECONN_ADV_ENTER_DEEP_TIME          3   //spec 60s

#define     ADV_RECONN_ENTER_DEEP_COUNT         1
#define     ADV_IDLE_ENTER_DEEP_COUNT           5
#define     CONN_IDLE_ENTER_DEEP_TIME           60   //60  //6s for test , spec 60s,
#define     IR_IDLE_ENTER_DEEP_TIME             5   //120s for development, spec not defined but better set to 2s





#define     MY_APP_ADV_CHANNEL                  BLT_ENABLE_ADV_ALL
#define     MY_ADV_INTERVAL_MIN                 ADV_INTERVAL_10MS
#define     MY_ADV_INTERVAL_MAX                 ADV_INTERVAL_20MS

#if (MCU_CORE_TYPE == MCU_CORE_8278)
    #define        MY_RF_POWER_INDEX            RF_POWER_P3p50dBm
#else
    #define        MY_RF_POWER_INDEX            RF_POWER_P3p01dBm
#endif

#define        BLE_DEVICE_ADDRESS_TYPE          BLE_DEVICE_ADDRESS_PUBLIC

_attribute_data_retention_    own_addr_type_t   app_own_address_type = OWN_ADDRESS_PUBLIC;


//#define RX_FIFO_SIZE              96                    //-24//64
//#define RX_FIFO_SIZE              160                    //-24//64
#define RX_FIFO_SIZE                160                    //-24//64
#define RX_FIFO_NUM                 8

//#define TX_FIFO_SIZE              88                    //-12//40
#define TX_FIFO_SIZE                148                    //-12//40
#define TX_FIFO_NUM                 16

//#define MTU_SIZE_SETTING          72
#define MTU_SIZE_SETTING            180
//#define MTU_SIZE_SETTING          210

#if 0
    MYFIFO_INIT(blt_rxfifo, RX_FIFO_SIZE, RX_FIFO_NUM);
#else
_attribute_data_retention_  u8  blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_  my_fifo_t   blt_rxfifo = {
                                                RX_FIFO_SIZE,
                                                RX_FIFO_NUM,
                                                0,
                                                0,
                                                blt_rxfifo_b,};
#endif


#if 0
    MYFIFO_INIT(blt_txfifo, TX_FIFO_SIZE, TX_FIFO_NUM);
#else
_attribute_data_retention_  u8  blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
_attribute_data_retention_  my_fifo_t   blt_txfifo = {
                                                    TX_FIFO_SIZE,
                                                    TX_FIFO_NUM,
                                                    0,
                                                    0,
                                                    blt_txfifo_b,};
#endif


//u8 flag_mtu=0;
_attribute_data_retention_    u8    flag_dle=0;
_attribute_data_retention_    int   device_in_connection_state=0;

_attribute_data_retention_    u32   advertise_begin_tick;
//_attribute_data_retention_    bool  advertise_downgrade_flag=TRUE;

_attribute_data_retention_    u8    sendTerminate_before_enterDeep = 0;

_attribute_data_retention_    u32   latest_user_event_tick;

_attribute_data_retention_    u32   lowBattDet_tick   = 0;

_attribute_data_retention_    u32   mtuExchange_check_tick=0;
_attribute_data_retention_    u32   mtuExchange_check_timer=12000000;

_attribute_data_retention_    u32   ir_flash_erase_tick=0;

_attribute_data_retention_    u32   delay3s_to_sleep_tick=0;
_attribute_data_retention_    u32   adv_time_to_deep=0;


_attribute_data_retention_    u16   enterdeep_time_count = 0;
_attribute_data_retention_    u16   enterdeep_time = 0;
//_attribute_data_retention_    bool   service_change = FALSE;

_attribute_data_retention_    u8    device_timeout_state=0;
_attribute_data_retention_    u8    flag_schedule_ota=0;
_attribute_data_retention_    u16   count_schedule_ota=0;

_attribute_data_retention_    u8    app_slave_terminate=0;
_attribute_data_retention_    u8    app_is_set_mtusize=0;
//_attribute_data_retention_    u8    prepareWrite_b[300];
_attribute_data_retention_    bool led_paring_flg =FALSE;
adv_type_t adv_type = ADV_TYPE_CONNECTABLE_UNDIRECTED;
u8  mac_public[6];


//extern u8 is_pairing_mode;
//extern u8 is_reconn_mode;
//extern ble_sts_t    blc_att_requestMtuSizeExchange (u16 connHandle, u16 mtu_size);
extern void is_flash_info_full(void);
extern void battery_refresh(void);
//extern void device_led_off(u8 led);

extern u8 my_devNamelen;
extern u8 my_devName[20];
extern u8 init_ir_key_event_notify(void);
extern  _attribute_data_retention_  u32 ftick_user_pair_start;
extern _attribute_data_retention_  u8  is_cleanconn_repairing_mode;
extern _attribute_data_retention_ u32 ftick_voice_reconn_send_code ;
#if (APP_IR_ENABLE)
extern u8 ir_flash_erase_flag;
//extern void ir_flash_set_flag(u8 flag);
#endif

#if (UART_PRINT_ENABLE)
_attribute_data_retention_    u8 printf_uart_flag=0;

void app_set_printf_uart_flag(unsigned char data)
{
    printf_uart_flag = data;
}
#endif

void set_app_adv_type(adv_type_t type){
    adv_type = type; // for debug printf only
}
void app_enter_deep_timeset(void)
{
    enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
}

void advertise_tick_retime(void)
{
    advertise_begin_tick = clock_time();
   //advertise_downgrade_flag = TRUE; // we dont need downgrade adv
    enterdeep_time_count = 0;
}

//////////////////////////////////////////////////////////////////////////////
//     Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8    tbl_advData_part2[] = {
     0x02, 0x01, 0x05,                          // BLE limited discoverable mode and BR/EDR not supported
     0x03, 0x19, 0x80, 0x01,                    // 384, Generic Remote Control, Generic category
     0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,        // incomplete list of service class UUIDs (0x1812, 0x180F)
     //0x03, 0x02, 0x12, 0x18,         // incomplete list of service class UUIDs (0x1812)

     //0x13, 0xFF,0x3D,0x03,0x00,xx,xx,xx,xx,xx,xx,0x00,0x00,0x00,0x00
     //'P','H','L','R','C'

};

const u8    tbl_advData_TPV[] = {
     0x02, 0x01, 0x05,                          // BLE limited discoverable mode and BR/EDR not supported
     0x03, 0x19, 0x80, 0x01,                    // 384, Generic Remote Control, Generic category
     0x03, 0x02, 0x12, 0x18,                    // incomplete list of service class UUIDs (0x1812-Human Interface Device)
     0x13, 0xFF, 0x3D, 0x03,                    // Manufacturer Specific
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0x48, 0x4C, 0x52, 0x43
     //0x03, 0x02, 0x12, 0x18,         // incomplete list of service class UUIDs (0x1812)

     //0x13, 0xFF,0x3D,0x03,0x00,xx,xx,xx,xx,xx,xx,0x00,0x00,0x00,0x00
     //'P','H','L','R','C'

};

const u8 tbl_scanRsp[] = {
    0x06,0x09,'P','H','L','R','C'
};

#define while_1_debug()    {while (1) {printf("h"); }}

void app_set_advdata_rspdata(void)
{
    bls_ll_setAdvData( (u8 *)tbl_advData_TPV, sizeof(tbl_advData_TPV));
    bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
}

extern bool  rc_reconn_normal;
void indirect_adv_end(u8 e, u8 *p, int n){
    rc_reconn_normal = false;
}


void indirect_adv(u8 e, u8 *p, int n)
{
    printf("indirect_adv1\r\n");
    app_set_advdata_rspdata();
    rc_reconn_normal = FALSE; // fall back, when connected  set to TRUE

    u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
            ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type, 0, NULL,
            MY_APP_ADV_CHANNEL, ADV_FP_NONE);
    if (status != BLE_SUCCESS) { while_1_debug(); }  //debug: adv setting err
    set_app_adv_type(ADV_TYPE_CONNECTABLE_UNDIRECTED);
    //bls_ll_setAdvDuration(n*1000 , 1); //3s
    bls_ll_setAdvDuration(3*1000*1000 , 1); //fix 3s here, beacuse direct_adv time out call to here as well

    bls_app_registerEventCallback(BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &indirect_adv_end); // end recursive call
    if( !is_batt_low() ) {
        adv_time_to_deep = RECONN_ADV_ENTER_DEEP_TIME;
        bls_ll_setAdvEnable(1);
        printf("indirect_adv2\r\n");
        //TODO bug: 1 time ok key does not actually trigger indirect?
    }
}

void direct_adv(u8 e, u8 *p, int n) // used for R52
{
    printf("direct_adv1\n");
    bls_ll_setAdvEnable(0);  //adv enable
    //is_reconn_mode = 0x55; // remove later,for led only
    app_set_advdata_rspdata();
    smp_param_save_t  bondInfo;
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    if(bond_number)   //at least 1 bonding device exist
    {
       //app_set_adv_interval_downgrade_direct(); //when adv time out call direct 30~35MS
       bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

       u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                        bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                        MY_APP_ADV_CHANNEL,
                                        ADV_FP_NONE);
       if(status != BLE_SUCCESS) { while_1_debug(); }  //debug: adv setting err
       set_app_adv_type(ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY);

       bls_ll_setAdvDuration(n, 1);
       //bls_ll_setAdvDuration(1*1000*1000, 1);
       //bls_ll_setAdvDuration(50*1000, 1);

       //    bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &indirect_adv); // set mode to fall back in indirect_adv
       bls_app_registerEventCallback(BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &indirect_adv_end);
       if( !is_batt_low() ) {
           printf("direct_adv2\n");
           adv_time_to_deep = RECONN_ADV_ENTER_DEEP_TIME;
           bls_ll_setAdvEnable(1);
       }
    }

}


#if (UART_PRINT_ENABLE)
void app_uart_tx(void)
{
    gpio_setup_up_down_resistor(UART_TX_PB1, PM_PIN_PULLUP_10K);  //must, for stability and prevent from current leakage
    gpio_set_func(UART_TX_PB1,AS_UART); // set tx pin
    gpio_set_input_en(UART_TX_PB1, 1);  //experiment shows that tx_pin should open input en(confirmed by qiuwei)
    uart_init_baudrate(115200,24000000,PARITY_NONE, STOP_BIT_ONE); //baud rate: 115200
    uart_ndma_clear_tx_index();
    //uart_dma_enable(0, 0);
    //irq_disable_type(FLD_IRQ_DMA_EN);
    //dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 0);
    //uart_irq_enable(0,0);   //uart RX irq enable
    //uart_ndma_irq_triglevel(1,0);    //set the trig level. 1 indicate one byte will occur interrupt
}
#endif

void app_adv_interval_downgrade_direct(void)
{
    smp_param_save_t  bondInfo;
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    //direct adv
    if(bond_number)   //at least 1 bonding device exist
    {
       printf("app_adv_internal_downgrade_direct\r\n");
       bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

       u8 status = bls_ll_setAdvParam( ADV_INTERVAL_30MS, ADV_INTERVAL_35MS,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                        bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                        MY_APP_ADV_CHANNEL,
                                        ADV_FP_NONE);
       if(status != BLE_SUCCESS) { while_1_debug(); }  //debug: adv setting err
       set_app_adv_type(ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY);

    }
}

void app_adv_interval_downgrade_indirect(void)
{
    printf("app_adv_internal_downgrade_indirect\r\n");
    bls_ll_setAdvParam( ADV_INTERVAL_30MS, ADV_INTERVAL_35MS,
                        ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
                        0,  NULL,
                        MY_APP_ADV_CHANNEL,
                        ADV_FP_NONE);
    set_app_adv_type(ADV_TYPE_CONNECTABLE_UNDIRECTED);

}

typedef void (*app_adv_interval_downgradeCb_t)(void);

_attribute_data_retention_ app_adv_interval_downgradeCb_t     app_adv_interval_downgradeCb = NULL;
void app_adv_interval_downgrade_register(app_adv_interval_downgradeCb_t cb)
{
    app_adv_interval_downgradeCb = cb;
}


void app_set_adv_interval_downgrade_indirect(void)
{
    app_adv_interval_downgrade_register(app_adv_interval_downgrade_indirect);
}

void app_set_adv_interval_downgrade_direct(void)
{
    app_adv_interval_downgrade_register(app_adv_interval_downgrade_direct);
}

void app_adv_indirect(void)
{
    printf("app_adv_indirec\n");
    app_set_advdata_rspdata();

    bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
        ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
        0,    NULL,
        MY_APP_ADV_CHANNEL,
        ADV_FP_NONE);
    set_app_adv_type(ADV_TYPE_CONNECTABLE_UNDIRECTED);

    if( !is_batt_low() ) {
        adv_time_to_deep = PAIR_ADV_ENTER_DEEP_TIME;
        bls_ll_setAdvEnable(1);
        printf("app_adv_indirec start adv\n");

        //app_set_adv_interval_downgrade_indirect(); // when adv time out call indirect
    }
}



void app_adv_direct(void)
{
    printf("app_adv_direct\r\n");
    //is_reconn_mode = 0x55;
    smp_param_save_t  bondInfo;
    bls_ll_setAdvEnable(0);  //adv enable
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    if(bond_number)   //at least 1 bonding device exist
    {
       app_set_adv_interval_downgrade_direct(); //when adv time out call direct 30~35MS
       bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

       u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                        bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                        MY_APP_ADV_CHANNEL,
                                        ADV_FP_NONE);
       if(status != BLE_SUCCESS) { while_1_debug(); }  //debug: adv setting err
       set_app_adv_type(ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY);

    }
    else
    {
        app_set_adv_interval_downgrade_indirect(); //when adv time out call indirect
    }
    if( !is_batt_low() ) { bls_ll_setAdvEnable(1); }

}

u8 effective_rx_dle = 0;
void    task_datalengthexchange (u8 e, u8 *p, int n)
{
    flag_dle |= 1;
    printf("task_datalengthexchange\n");
    ll_data_extension_t *data_p = (ll_data_extension_t *)p;
    effective_rx_dle = data_p->connEffectiveMaxRxOctets;
    //printf("connEffective:rx:%d __ tx: %d\n",data_p->connEffectiveMaxRxOctets,data_p->connEffectiveMaxTxOctets);

    set_audio_frame_size(data_p->connMaxTxOctets);
    //set_audio_frame_size(23);
    printf("ftick_voice_reconn_send_code =[%d]key_not_released=[%d]\r\n",ftick_voice_reconn_send_code,key_not_released);
    if (key_not_released == 0 )
    {
        u8 consumer_key[2]={0};
        if (ftick_voice_reconn_send_code) {
            ftick_voice_reconn_send_code = 0;
            reconn_send_voice_ble();
        }
    }
}

//void app_service_change(void)
//{
//    u8 service_data[4]={BATT_PS_H,0,0xff,0};
//    if(service_change)
//    {
//        printf("app_service_change\r\n");
//        bls_att_pushIndicateData(GenericAttribute_ServiceChanged_DP_H,service_data,sizeof(service_data));
//        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) & (~OTA_FLG));
//        service_change = FALSE;
//    }
//}

void mtu_exchange_loop(void)
{
    if(mtuExchange_check_tick && clock_time_exceed(mtuExchange_check_tick, mtuExchange_check_timer ))
    {
        mtuExchange_check_tick = 0;
        app_schedule_ota_send_notify_to_continue_ota();
        if((flag_dle&0x10) == 0)
        {
            printf("1 MTU_SIZE %d\r\n", MTU_SIZE_SETTING);
            blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
        }
        if((flag_dle&0x10) == 0x10)
        {
            if(app_mtu_size < MTU_SIZE_SETTING)
            {
                printf("2 MTU_SIZE %d\r\n", MTU_SIZE_SETTING);
                blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
            }
        }
        if((flag_dle&0x01) == 0)
        {
            printf("blc_ll_exchangeDataLength\r\n");
            blc_ll_exchangeDataLength(LL_LENGTH_REQ,(TX_FIFO_SIZE-18));
            flag_dle |= 0x01;
        }
        if((flag_dle&0x80) == 0)
        {
            printf("bls_l2cap_requestConnParamUpdate 8, 8, 99, 400\r\n");
            bls_l2cap_requestConnParamUpdate (8, 8, 99, 400);
            flag_dle |= 0x80;

            extern u32 ftick_voice_reconn_send_code;
            if (ftick_voice_reconn_send_code) {
                ftick_voice_reconn_send_code = 0;
                send_voice_ble();
            }

            extern u32 ftick_ok_reconn_send_code;
            if (ftick_ok_reconn_send_code) {
                ftick_ok_reconn_send_code = 0;
                send_ok_ble();
            }


        }
        //app_service_change();
    }
}

int app_enable_adv_timeout(void)
{
     printf("ble_ll_reject time out to indirect\r\n");
     //is_pairing_mode = 0x55;
    ir_dispatch(1, 0, 183); // src key ir code 183(0xB7) to trigger TV to pair page
    bls_ll_setAdvDuration(20000000, 1);
    printf("666666666666666666ble_ll_reject time out to indirect led_paring_flg=[%d]\r\n",led_paring_flg);
    app_adv_indirect(); // for ll_reject_ind
    ftick_user_pair_start = clock_time()|TRUE; // to avoid re-enter pair mode
    enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
    if(google_voice_ctl){
        google_voice_ctl = 0;
        get_cap_received = false;
    }
    if(ui_mic_enable){
        ui_enable_mic (0);
    }
    return -1;
}

ble_sts_t app_terminate(u8 reason)
{
    app_slave_terminate = 0x55;
    ble_sts_t status = bls_ll_terminateConnection(reason);
    return status;
}

int app_terminate_timeout(void)
{
    ble_sts_t status = app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
    printf("app_terminate_timeout status=%x\r\n",status);
    return -1;
}

u8 ll_reject_to_adv=0;
void ble_ll_reject(u8 e,u8 *p, int n) //*p is terminate reason
{
    latest_user_event_tick = clock_time();

    printf("ble_ll_reject\r\n");
    if (device_in_connection_state == 0) {
        blt_soft_timer_delete(app_terminate_timeout);
        //blt_soft_timer_add(app_enable_adv_timeout, 1000000);
        ll_reject_to_adv = TRUE;
        ble_sts_t status = app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
        printf("status=%x\r\n", status);
        if (google_voice_ctl & FLAG_GOOGLE_CAPS_RESP)
            google_voice_ctl &= ~FLAG_GOOGLE_CAPS_RESP;
    }
}

#if (GOOGLE_OTA)
int app_ota_terminate_timeoutcb(void)
{
    printf("app_ota_terminate_timeoutcb\r\n");
    app_ota_terminate(OTA_USER_TERMINATE);
    return -1;
}
#else
int app_ota_terminate_timeoutcb(void){
    //do nothing
    return 0;
}
#endif

int app_terminate_to_direct_timeoutcb(void)
{
    printf("app_terminate_to_direct_timeoutcb\r\n");
    app_adv_direct();

    return -1;
}

void app_ui_unpair_user_wait_terminate(){
    extern u8 user_unpair_wait_terminate;
    extern u8 user_pair_wait_terminate;
    extern u32 ftick_ir_release_n_indirect_adv;

    if(user_pair_wait_terminate){
        printf("erase pair info33333333333333\n");
        bls_smp_eraseAllParingInformation();
        user_pair_wait_terminate = FALSE;
        ftick_ir_release_n_indirect_adv = clock_time()|TRUE; // TODO: test only 1 ir frame out, may time out and send 2
    }

    if(user_unpair_wait_terminate){
        printf("erase pair info 44444444444444444\n");
        bls_smp_eraseAllParingInformation();
        user_unpair_wait_terminate = FALSE;
    }

    //TODO : if again indirect adv will trigger host to reconnect
}


void ble_remote_terminate(u8 e,u8 *p, int n) //*p is terminate reason
{

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    printf("ble_remote_terminate  bond_number=%x,*p=%x\r\n",bond_number,*p);

    //app_led_disconnect();

    latest_user_event_tick = clock_time();


    //if(app_repairing() == 0)// don't need to repair
    {
        if(ota_is_working)
        {
            app_pairing_error();
            blt_soft_timer_add(app_ota_terminate_timeoutcb, 1000000);
            app_led_ota_off();
            ota_is_working=0;
            return;
        }
        if(*p == HCI_ERR_CONN_TIMEOUT){
            printf("HCI_ERR_CONN_TIMEOUT\r\n");
            device_timeout_state=0x55;
            ftick_user_pair_start=0;
            //is_pairing_mode = 0;
            //is_reconn_mode = 0;
            if((flag_schedule_ota == 0x55) || (bond_number == 0))
            {
                printf("wwwwwwwwwwwwwwwwwwwwwis_cleanconn_repairing_mode=[%d]\r\n",is_cleanconn_repairing_mode);
                if (is_cleanconn_repairing_mode ==1)
                {
                    latest_user_event_tick = clock_time();
                    blt_soft_timer_add(app_enable_adv_timeout, 1000000); // 4s or 1s?
                    printf("into HCI_ERR_CONN_TIMEOUT is_cleanconn_repairing_mode =1\r\n");
                }
                else
                {
                    enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                    bls_ll_setAdvEnable(0);
                }
            }
            else
            {
                enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;
                printf("ddddddddddddddddddddddddddddd is_cleanconn_repairing_mode=[%d]\r\n",is_cleanconn_repairing_mode);
                if (is_cleanconn_repairing_mode ==1)
                {
                    latest_user_event_tick = clock_time();
                    blt_soft_timer_add(app_enable_adv_timeout, 1000000); // 4s or 1s?
                    printf("into HCI_ERR_CONN_TIMEOUT is_cleanconn_repairing_mode =2\r\n");
                }
                
                //app_adv_direct();
            }
        }
        else if(*p == HCI_ERR_REMOTE_USER_TERM_CONN){  //0x13

            printf("remote user terminate\r\n");
            app_ui_unpair_user_wait_terminate();   //handle user press key to unpair

            device_timeout_state=0x55;             //indicate OTA tiem out to deep

            //if(is_pairing_mode)    is_pairing_mode = 0;
            //if(is_reconn_mode)     is_reconn_mode = 0;

            bls_ll_setAdvEnable(0);
            if(app_slave_terminate == 0x55) // 0x55 means a slave terminate
            {
                enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                app_slave_terminate = 0;
                printf("slave terminate\r\n");
                if(device_in_connection_state == 0)
                {
                    if (is_cleanconn_repairing_mode ==1)
                    {
                        enterdeep_time = 20;
                        latest_user_event_tick = clock_time();
                        blt_soft_timer_add(app_enable_adv_timeout, 1000000); // 4s or 1s?
                    }
                    else
                    {
                        enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                    }
                }
            }
            else
            {
                printf("master terminate is_cleanconn_repairing_mode=[%d],ll_reject_to_adv=[%d]\r\n",is_cleanconn_repairing_mode,ll_reject_to_adv);
                if(device_in_connection_state == 0)
                {
                    if (is_cleanconn_repairing_mode ==1)
                    {
                        enterdeep_time = 20;
                        latest_user_event_tick = clock_time();
                        blt_soft_timer_add(app_enable_adv_timeout, 1000000); // 4s or 1s?
                    }
                    else
                    {
                        enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                    }
                }
                else
                 {
                     if(flag_schedule_ota == 0x55)
                     {
                         enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                         bls_ll_setAdvEnable(0);
                     }
                     else
                     {
                         bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
                         if(bond_number)
                         {
                              enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;
                         }
                         else
                             enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;

                         //when user terminate , no more adv to reconn
                         //blt_soft_timer_add(app_terminate_to_direct_timeoutcb, 600000);
                     }
                 }
             }

             if(ll_reject_to_adv){
                 //app_enable_adv_timeout();
                 latest_user_event_tick = clock_time();

                 blt_soft_timer_add(app_enable_adv_timeout, 1000000); // 4s or 1s?

                 ll_reject_to_adv = FALSE;
             }

        }
        else if(*p == HCI_ERR_CONN_TERM_MIC_FAILURE){
            //is_pairing_mode = 0;
            //is_reconn_mode = 0;
            if (is_cleanconn_repairing_mode ==0)
            {
                app_pairing_error();
            }
            enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
            bls_ll_setAdvEnable(0);
            printf("HCI_ERR_CONN_TERM_MIC_FAILURE\r\n");
            bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
            if(bond_number)
            {
                enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;
            }
            else
                enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
            app_adv_direct();
        }        
        else if(*p == HCI_ERR_CONN_FAILED_TO_ESTABLISH){
            //(*p == HCI_ERR_CONN_TIMEOUT){
            printf("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhHCI_ERR_CONN_FAILED_TO_ESTABLISH bond_number=[%d]  is_cleanconn_repairing_mode=[%d] ftick_user_pair_start=[%d]\r\n",bond_number,is_cleanconn_repairing_mode,ftick_user_pair_start);
            ftick_user_pair_start=0;
            if (is_cleanconn_repairing_mode ==1)
            {
                latest_user_event_tick = clock_time();
                blt_soft_timer_add(app_enable_adv_timeout, 1000000); // 4s or 1s?
                printf("into HCI_ERR_CONN_TIMEOUT is_cleanconn_repairing_mode =1\r\n");
            }
            else
            {
                enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                bls_ll_setAdvEnable(0);
            }
        }
        google_voice_ctl = 0;
        get_cap_received = 0;
    }

    device_in_connection_state = 0;
    mtuExchange_check_tick = 0;
    flag_dle = 0;

#if (BLE_REMOTE_PM_ENABLE)
     //user has push terminate pkt to ble TX buffer before deepsleep
    if(sendTerminate_before_enterDeep == 1){
        sendTerminate_before_enterDeep = 2;
    }
#endif

#if (BLE_AUDIO_ENABLE)
    if(ui_mic_enable){
        ui_enable_mic (0);
    }
#endif

    advertise_tick_retime();
}


_attribute_ram_code_ void    user_set_rf_power (u8 e, u8 *p, int n)
{
#if (MP_TEST_MODE)
    if (gpio_read(DUT_INPUT_PORT) && test_get_mode() == MODE_TEST)
        rf_set_power_level_index(RF_POWER_P7p37dBm);
    else
#endif
    rf_set_power_level_index (MY_RF_POWER_INDEX);
}


#define PEER_MAC_FLASH_ERASE_TIMEOUT 600000

int app_peer_mac_flash_erase_timeoutcb(void)
{
    printf("app_peer_mac_flash_erase_timeoutcb\r\n");

    if(bls_ll_requestConnBrxEventDisable() > 120)
    {
        printf("app_peer_mac_flash_erase \r\n");
        bls_ll_disableConnBrxEvent();
        flash_erase_sector(MAC_DATA_SECT_ADDR);
        bls_ll_restoreConnBrxEvent();
        printf("app_peer_mac_flash_erase succ\r\n");
        write_peer_mac_info(peer_mac);
        return -1;
    }

    return 0;
}

void app_peer_mac_flash_erase(void)
{
    blt_soft_timer_add(app_peer_mac_flash_erase_timeoutcb, PEER_MAC_FLASH_ERASE_TIMEOUT);
}

#if (APP_IR_ENABLE)
void app_save_peer_mac_info(u8 *mac)
{

    if((peer_mac[0] == 0) && (peer_mac[1] == 0))
    {
        memcpy(peer_mac,mac,6);
        write_peer_mac_info(peer_mac);
    }
    else
    {
        printf("peer_mac =\r\n");
        for(u8 i=0;i<6;i++)
            printf(" %x",mac[i]);
        if(memcmp(mac,peer_mac,6))
        {
            printf("new mac\r\n");
            memcpy(peer_mac,mac,6);
            ir_table_init();
            write_ir_key_event_notify(0);
            flag_ccc_data = 0;
            atv_char_ctl_ccc = 0;
            atv_char_rx_ccc = 0;
            ir_flash_erase_flag = 0x55;
            write_ccc_info(&flag_ccc_data);
            if(is_peer_mac_flash_info_full() == 0)
               write_peer_mac_info(peer_mac);
            else
                  app_peer_mac_flash_erase();
        }
    }
}

void app_cmp_mac(void)
{
    smp_param_save_t  bondInfo;

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();
    if(bond_number)
    {
        printf("app_cmp_mac\r\n");
        bls_smp_param_loadByIndex(0, &bondInfo);
        app_save_peer_mac_info(bondInfo.peer_addr);
    }
}
#endif

extern u32 tick_led_pairing_after_connect;

int app_host_event_callback (u32 h, u8 *para, int n)
{

    u8 event = h & 0xFF;

    switch(event)
    {
        case GAP_EVT_SMP_PARING_BEAGIN:
        {
            printf("----- Pairing begin -----\n");
            led_paring_flg = TRUE ;
        }
        break;

        case GAP_EVT_SMP_PARING_SUCCESS:
        {
            gap_smp_paringSuccessEvt_t* p = (gap_smp_paringSuccessEvt_t*)para;
            printf("(%s-%d): Pairing success:bond flg = %d, result = 0x%x \n",__FUNCTION__, __LINE__, p->bonding, p->bonding_result);            
            WaitMs(2);
            if(p->bonding_result){
                app_led_connect();
                #if (APP_IR_ENABLE)
                    app_cmp_mac();
                    printf("save smp key succ\n");
                #endif
                WaitMs(2);
                led_paring_flg=FALSE;
            }
            else{
                printf("save smp key failed\n");
                led_paring_flg=FALSE;
            }
        }
        break;

        case GAP_EVT_SMP_PARING_FAIL:
        {
            gap_smp_paringFailEvt_t* p = (gap_smp_paringFailEvt_t*)para;
            printf("----- Pairing failed:rsn:0x%x -----\n", p->reason);
            bls_ll_setAdvEnable(0);   //disable adv
            if (is_cleanconn_repairing_mode ==0)
            {
                app_pairing_error();
            }
            if(ota_is_working)
	        {
                app_led_ota_off();
                ota_is_working=0;
            }
            led_paring_flg=FALSE;
        }
        break;

        case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
        {
            gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;

            printf("GAP_EVT_SMP_CONN_ENCRYPTION_DONE \n");

            blt_soft_timer_delete(app_terminate_timeout);

            if(p->re_connect == SMP_STANDARD_PAIR){  //first paring
                printf("first pairing\r\n");

                app_is_set_mtusize = 0;
                //mtuExchange_check_timer = 12000000;// if receive IR from host
                mtuExchange_check_timer = 3000000;   // R52 don't need so long, make it quick for 1st time RC power on voice search

#if (APP_IR_ENABLE)
                if(ir_flash_erase_flag == 0x55)
                {
                    ir_flash_set_flag(0);
                    ir_flash_erase_flag = 0;
                }
#endif
            }
            else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect
                if(app_mtu_size < MTU_SIZE_SETTING)
                    blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
                // app_led_connect();
                mtuExchange_check_timer = 250000; //check later if vocie search start slow
                //mtuExchange_check_timer = 3000000; //check later if vocie search start slow
                app_is_set_mtusize = 1;
                printf("auto connect \n");
            }

            device_in_connection_state = 1;
            //is_pairing_mode = 0;
            //is_reconn_mode = 0;

            mtuExchange_check_tick = clock_time() | 1;
            notify_get_rsp_tick = clock_time()|1;



            battery_refresh();

            blt_soft_timer_add(app_cachekey_send_timer,30000);//TODO: remove this?

        }
        break;

        case GAP_EVT_ATT_EXCHANGE_MTU:
        {
           // flag_mtu = 1;
            blt_soft_timer_delete(app_terminate_timeout);
            gap_gatt_mtuSizeExchangeEvt_t *pEvt = (gap_gatt_mtuSizeExchangeEvt_t *)para;
            printf("MTU Peer MTU(%d)/Effect ATT MTU(%d).\n", pEvt->peer_MTU, pEvt->effective_MTU);
            app_mtu_size = U16_LO(pEvt->effective_MTU);
            flag_dle |= 0x10;
            /*
            if(!flag_dle){
                blc_ll_exchangeDataLength(LL_LENGTH_REQ,pEvt->effective_MTU);
            }
            */
        }
        break;


        default:
        break;
    }

    return 0;
}

void task_connect (u8 e, u8 *p, int n)
{
    printf("task_connect\n");
    app_mtu_size = 23;
    bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(1000); //1000
    enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;

    latest_user_event_tick = clock_time();
    #if (GOOGLE_OTA)
    blt_soft_timer_add(app_terminate_timeout, 2000000);//for google OTA?
    bls_ota_set_random(p+12);
    #endif
    rc_reconn_normal = TRUE; // normal mode

    tick_led_pairing_after_connect = clock_time() | 1;
}

void task_conn_update_req (u8 e, u8 *p, int n)
{
    printf("task_conn_update_req\r\n");
}

void task_conn_update_done (u8 e, u8 *p, int n)
{
    printf("task_conn_update_done\n");
    latest_user_event_tick = clock_time();
}

int app_conn_param_update_response(u8 id, u16  result)
{
    if(result == CONN_PARAM_UPDATE_ACCEPT){
       // printf("CONN_PARAM_UPDATE_ACCEPT\n");
    }
    else if(result == CONN_PARAM_UPDATE_REJECT){
        //printf("CONN_PARAM_UPDATE_REJECT\n");
    }

    return 0;
}

#if (AUDIO_TRANS_USE_2M_PHY_ENABLE)

{
    u8 event = h & 0xFF;
    if(event==GAP_EVT_ATT_EXCHANGE_MTU)
    {
        blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_2M,      PHY_PREFER_2M,    CODED_PHY_PREFER_NONE);
    }
    return 0;
}
void     app_phy_update_complete_event(u8 e,u8 *p, int n)
{
}
#endif

//----------------------------------------------------------------------------

void task_before_deep(){
    // not used pin ouput disable , input enable and pull down, to avoid current leakage,
    //...
    ftick_user_pair_start=0; // clear pair mode
    //or UNUSED_GPIO_PULLDOWN_ENABLE

}






void blt_pm_proc(void)
{
#if(BLE_REMOTE_PM_ENABLE)

#if (PTM)
    extern bool ptm_enabled;
#endif

#if (UART_PRINT_ENABLE)
    if(printf_uart_flag || (reg_uart_buf_cnt & FLD_UART_TX_BUF_CNT))
    {
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
        return;
    }
#endif

#if (STUCK_KEY_PROCESS_ENABLE)
    extern u32 stuckkey_keypresstimer;
    if(key_not_released && stuckkey_keypresstimer && clock_time_exceed(stuckkey_keypresstimer, STUCK_KEY_ENTERDEEP_TIME*1000000))
    {
        printf("stuck key\r\n");
        stuckkey_keypresstimer = 0;

        {
            extern u32 drive_pins[];
            u32 pin[] = KB_DRIVE_PINS;
            for(int i=0; i<(sizeof(pin)/sizeof(*pin));i++)
            {
                extern u8 stuckKeyPress[];
                if(stuckKeyPress[i])
                {
                    printf("stuck key i=%x\r\n",i);
                    cpu_set_gpio_wakeup(drive_pins[i], KB_LINE_HIGH_VALID, 1);//set H active , then if key release , wakeup from deep
                }
                else
                {
                    cpu_set_gpio_wakeup(drive_pins[i], !KB_LINE_HIGH_VALID, 0);
                }
            }
            app_release_hid_key();
            printf("stuckkey to deep\r\n");
            // printf("--------------before enter deep sleep-----------------\n");
            // printf("GPIO A:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x580), analog_read(0x21), analog_read(0x27));
            // printf("GPIO B:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x588), analog_read(0x22), analog_read(0x28));
            // printf("GPIO C:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x590), analog_read(0x23), analog_read(0x29));
            // printf("GPIO D:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x598), analog_read(0x24), analog_read(0x2A));
            // printf("afe_0x26=0x%02X\n", analog_read(0x26));
            // printf("afe_0x6E=0x%02X\n", analog_read(0x6E));
            // printf("------------------------------------------------------\n");           
            task_before_deep();
            //cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K,PM_WAKEUP_PAD,0);
            cpu_sleep_wakeup(DEEPSLEEP_MODE,PM_WAKEUP_PAD,0);
            printf("stuckkey to deep fail\r\n");
        }
    }
#endif

    //printf("2 ");
    if(device_led_busy())
    {
        //printf("pm led ");
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
        return;
    }
#if (APP_IR_ENABLE)
    if(ir_flash_busy())
    {
        //printf("flash busy\n");
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
        return;
    }
#endif
    if((ui_mic_enable || google_voice_ctl || ota_is_working || led_paring_flg))
    {
        //printf("pm mic or voice ctl or ota ");
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
    }
#if(REMOTE_IR_ENABLE)
    else if (ir_send_ctrl.is_sending) {
        //printf("pm ir sending ");
        bls_pm_setSuspendMask(SUSPEND_DISABLE);
    }
#endif

#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)
    else if( blc_phy_isPhyTestEnable() )
    {
        bls_pm_setSuspendMask(SUSPEND_DISABLE); //phy test can not enter suspend
    }
#endif

#if (MP_TEST_MODE)
    else if (test_get_mode() == MODE_TEST)
    {
        if (test_get_state() == TEST_STAT_LED)
            cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_PAD, 0);
        else if (test_get_state() == TEST_STAT_CURRENT)
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
        else bls_pm_setSuspendMask(SUSPEND_DISABLE);
    }
#endif

#if (PTM)
    else if (ptm_enabled)
    {
        bls_pm_setSuspendMask(SUSPEND_DISABLE);
    }

#endif
    else
    {
        #if (PM_DEEPSLEEP_RETENTION_ENABLE)
            bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
        #else
            bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
        #endif

        int user_task_flg = ota_is_working || scan_pin_need || key_not_released || ui_mic_enable || led_paring_flg;

        if(user_task_flg){
            //printf("pm user task\n");
            bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);

            #if (LONG_PRESS_KEY_POWER_OPTIMIZE)
                extern int key_matrix_same_as_last_cnt;
                if(!ota_is_working && key_matrix_same_as_last_cnt > 5){  //key matrix stable can optize
                    bls_pm_setManualLatency(2);
                }
                else{
                    bls_pm_setManualLatency(0);  //latency off: 0
                }
            #else
                bls_pm_setManualLatency(0);
            #endif
        }


    #if 1 //deepsleep
        if(sendTerminate_before_enterDeep == 1){ //sending Terminate and wait for ack before enter deepsleep
            if(user_task_flg){  //detect key Press again,  can not enter deep now
                sendTerminate_before_enterDeep = 0;
                if( !is_batt_low() ) { bls_ll_setAdvEnable(1); }
            }
        }
        else if(sendTerminate_before_enterDeep == 2){  //Terminate OK
            analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | CONN_DEEP_FLG);
            printf("deep conn idle or user terminate\n");
            sendTerminate_before_enterDeep = 0;
            task_before_deep();
            //cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
            cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_PAD, 0);
        }

        if( !blc_ll_isControllerEventPending() ){  //no controller event pending
            #if 0
            //don't need downgrade adv
            if( blc_ll_getCurrentState() == BLS_LINK_STATE_ADV && !sendTerminate_before_enterDeep && \
                clock_time_exceed(advertise_begin_tick , ADV_INTERNAL_SWITCH_TIME * 1000000))
            {
                if(advertise_downgrade_flag)
                {
                    advertise_downgrade_flag = FALSE;//every time call to direct indirect, get 1 chance to call diwngrade cb
                    if(app_adv_interval_downgradeCb)    app_adv_interval_downgradeCb();
                }
            }

            //enterdeep_time_count as 5 or 1, but we don't this
            if( blc_ll_getCurrentState() == BLS_LINK_STATE_ADV && !sendTerminate_before_enterDeep && \
                clock_time_exceed(advertise_begin_tick , ADV_IDLE_ENTER_DEEP_TIME * 1000000))
            {
                enterdeep_time_count++;
                printf("enterdeep_time_count=%x\r\n",enterdeep_time_count);
                if(enterdeep_time_count == enterdeep_time)
                {
                    if(enterdeep_time >1)
                        printf("adv 10 mins\r\n");
                    else
                        printf("adv 2 mins\r\n");
                    delay3s_to_sleep_tick = clock_time()|1; // for shine led...
                    //device_led_off(APP_LED_WHITE);
                    app_pairing_error();
                }
                else
                {
                    advertise_begin_tick = clock_time();
                }
            }
            #endif
            //set adv to deep as 60 or 3s
            //ADV_IDLE_ENTER_DEEP_TIME = 60 or 3s
            if ((blc_ll_getCurrentState() == BLS_LINK_STATE_ADV) &&
                 clock_time_exceed(advertise_begin_tick, adv_time_to_deep * 1000000)) {
                 printf("deep adv idle\n");
                task_before_deep();
                //cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
                cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_PAD, 0);
            }

            if(blc_ll_getCurrentState() == BLS_LINK_STATE_ADV && delay3s_to_sleep_tick && clock_time_exceed(delay3s_to_sleep_tick,3000*1000))
            {
                printf("adv timeout\r\n");
#if (GOOGLE_OTA)
                app_schedule_ota_enter_deep();
#else
                task_before_deep();
                //cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
                cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_PAD, 0);
#endif
            }

            // connect time out to deep
            if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && !user_task_flg && \
                    clock_time_exceed(latest_user_event_tick, CONN_IDLE_ENTER_DEEP_TIME * 1000000) )
            {
                bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
                bls_ll_setAdvEnable(0);   //disable adv
                ftick_user_pair_start=0; // clear pair mode
                sendTerminate_before_enterDeep = 0;
                printf("no action to terminate connection\n");
            }


            //IR idle to deep
            if ( (blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) && (blc_ll_getCurrentState() != BLS_LINK_STATE_ADV) &&  !user_task_flg ){
                //IR mode. IR idle to time out
                if (clock_time_exceed(latest_user_event_tick, IR_IDLE_ENTER_DEEP_TIME * 1000000)) { //when pairing, need to add up at least 4s
                    printf("ir idle to deep\n");

                    // extern u32 drive_pins[];
                    // u32 pin[] = KB_DRIVE_PINS;                    
                    // for(int i=0; i<(sizeof(pin)/sizeof(*pin));i++)
                    // {
                    //     printf("key i=%x\r\n",i);
                    //     cpu_set_gpio_wakeup(drive_pins[i], KB_LINE_HIGH_VALID, 1);//set H active , then if key release , wakeup from deep

                    // }  
                    // u8 gpio[4] = {0, 0, 0, 0};
                    // gpio_read_all(gpio);
                    // printf("--------------before enter deep sleep-----------------\n");
                    // printf("GPIO A:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x580), analog_read(0x21), analog_read(0x27));
                    // printf("GPIO B:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x588), analog_read(0x22), analog_read(0x28));
                    // printf("GPIO C:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x590), analog_read(0x23), analog_read(0x29));
                    // printf("GPIO D:input=0x%02X, Polarity=0x%02X, ENABLE=0x%2X\n", REG_ADDR8(0x598), analog_read(0x24), analog_read(0x2A));
                    // printf("afe_0x26=0x%02X\n", analog_read(0x26));
                    // printf("afe_0x6E=0x%02X\n", analog_read(0x6E));
                    // printf("------------------------------------------------------\n");

                    // sleep_us(10000*1000);
                    // while(1);
                    // cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deep;
                    // printf("deep IR idle\n");
#if (GOOGLE_OTA)
                    app_schedule_ota_enter_deep();
#else
                    task_before_deep();
                    //cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
                    cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_PAD, 0);
#endif

                }
            }

            //don't need this
//            if((is_pairing_mode == 0) && (is_reconn_mode == 0) && (device_in_connection_state == 0) &&  !user_task_flg )
//            {
//                 u8 time=0;
//                 if(enterdeep_time == ADV_RECONN_ENTER_DEEP_COUNT)
//                        time = ADV_IDLE_ENTER_DEEP_TIME;
//                 else
//                        time = 30; //3;
//                 if(clock_time_exceed(advertise_begin_tick ,  time* 1000000))
//                 {
//                     printf("schedule_ota_enter_deep\r\n");
//                     app_schedule_ota_enter_deep();
//                 }
//            }

        }
    #endif
    }

#endif  //END of  BLE_REMOTE_PM_ENABLE
}



void  ble_remote_set_sleep_wakeup (u8 e, u8 *p, int n)
{
    if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * CLOCK_16M_SYS_TIMER_CLK_1MS){  //suspend time > 30ms.add gpio wakeup
        bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
    }
}

_attribute_ram_code_ void user_init_battery_power_check(void)
{
    /*****************************************************************************************
     Note: battery check must do before any flash write/erase operation, cause flash write/erase
           under a low or unstable power supply will lead to error flash operation

           Some module initialization may involve flash write/erase, include: OTA initialization,
                SMP initialization, ..
                So these initialization must be done after  battery check
    *****************************************************************************************/
    #if (BATT_CHECK_ENABLE)  //battery check must do before OTA relative operation
        #if(VBAT_LEAKAGE_PROTECT_EN)
        do{
            u8 analog_deep = analog_read(USED_DEEP_ANA_REG);
            // u16 vbat_deep_thres = VBAT_DEEP_THRES_MV;
            // u16 vbat_suspend_thres = VBAT_SUSPEND_THRES_MV;
            // if(analog_deep & LOW_BATT_FLG){
            //     if(analog_deep & LOW_BATT_SUSPEND_FLG){//<1.8v
            //         vbat_deep_thres += 200;
            //         vbat_suspend_thres += 100;
            //     }
            //     else{//1.8--2.0v
            //         vbat_deep_thres += 200;
            //     }
            // }
            app_battery_power_check(VBAT_DEEP_THRES_MV,VBAT_SUSPEND_THRES_MV);
            #if (MODULE_WATCHDOG_ENABLE)
            wd_clear(); //clear watch dog
            #endif
            if(analog_deep & LOW_BATT_SUSPEND_FLG){
                sleep_us(100000);
            }
        }while(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_SUSPEND_FLG);
        #else
            if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
                app_battery_power_check(VBAT_DEEP_THRES_MV + 200);
            }
            else{
                app_battery_power_check(VBAT_DEEP_THRES_MV);
            }
        #endif

    #endif
}



_attribute_data_retention_ u8 power_on_from=POWER_ON_NORMAL;          //1: the rcu previous state is power down 2: the rcu previous state is power down


typedef struct{
    u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

    u8    type;               //RA(1)_TA(1)_RFU(2)_TYPE(4)
    u8    rf_len;             //LEN(6)_RFU(2)
    u8    advA[6];            //address
    u8    data[31];           //0-31 byte
}rf_packet_adv_ind_module_t;

void app_tx_power_wakeup(u8 keyid)
{
    rf_packet_adv_ind_module_t tx_power_wakeup;
    smp_param_save_t  bondInfo;
    u8 buffer[40];
    u8 index=0,i;
    u8 pos_index[3]={0};

    printf("app_tx_power_wakeup\r\n");
    rf_set_ble_access_code_adv();
    rf_set_ble_crc_adv();
    bls_smp_param_loadByIndex(0, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

    flash_read_page(0x78000+wakeup_key_pos, 40, buffer);

    //google format
    {
        tx_power_wakeup.rf_len = 19;
        tx_power_wakeup.dma_len = tx_power_wakeup.rf_len + 2;
        tx_power_wakeup.type = ADV_TYPE_CONNECTABLE_UNDIRECTED;

        memcpy(&(tx_power_wakeup.advA[0]),mac_public,6);
        tx_power_wakeup.data[0] = 0x0c;
        tx_power_wakeup.data[1] = 0x16;
        tx_power_wakeup.data[2] = 0x36;
        tx_power_wakeup.data[3] = 0xfd;
        tx_power_wakeup.data[4] = 0x01;
        if(keyid == 0xff)
        {
            tx_power_wakeup.data[5] = 0x02;  //schedule ota wakeup packet
        }
        else
        {
            tx_power_wakeup.data[5] = app_custom_wakeupkey_packet_index(keyid);
        }
        if(read_wakeup_keyindex_info(&index) == 0)
        {
            if(keyid == 0xff)
            {
                if(index)
                  tx_power_wakeup.data[6] = index-1;
                else
                  tx_power_wakeup.data[6] = 0;
            }
            else
                tx_power_wakeup.data[6] = index;
        }
        else
            tx_power_wakeup.data[6] = 0;

        memcpy(&(tx_power_wakeup.data[7]), bondInfo.peer_addr, 6);
    }

    for(u8 i=0;i<3;i++)
    {
       rf_set_ble_channel(37+i);

       rf_start_stx (&tx_power_wakeup, 100);

       while(rf_tx_finish()!=0x01);

       rf_tx_finish_clear_flag();
    }

    //custom format
    if(buffer[0] != 0xff)
    {
        tx_power_wakeup.rf_len = buffer[0]+6;
        tx_power_wakeup.dma_len = tx_power_wakeup.rf_len + 2;
        tx_power_wakeup.type = 0;
        memcpy(&(tx_power_wakeup.advA[0]),mac_public,6);
        for(i=0;i<buffer[0];i++)
            tx_power_wakeup.data[i] = buffer[4+i];
        printf("raw packet =\r\n");
        for(i=0;i<buffer[0];i++)
            printf(" %x",tx_power_wakeup.data[i]);
        for(i=0;i<3;i++)
        {
            if(buffer[i])
            {
                pos_index[i] = buffer[i+1];
            }
            else
                pos_index[i] = 0xff;
        }
        printf("\r\npos_index[0]=%x,pos_index[1]=%x,pos_index[2]=%x\r\n",pos_index[0],pos_index[1],pos_index[2]);

        for(i=0;i<buffer[0];i++)
        {
            if(i == pos_index[0])
            {
                if(keyid == 0xff)
                    tx_power_wakeup.data[i] = 2;
                else
                    tx_power_wakeup.data[i] = app_custom_wakeupkey_packet_index(keyid);
            }
            else if(i == pos_index[1])
            {
                 if(read_wakeup_keyindex_info(&index) == 0)
                 {
                    if(keyid == 0xff)
                    {
                        if(index)
                            tx_power_wakeup.data[i] = index-1;
                        else
                            tx_power_wakeup.data[i] = 0;
                    }
                    else
                        tx_power_wakeup.data[i] = index;
                 }
                 else
                     tx_power_wakeup.data[i] = 0;
                 printf("tx_power_wakeup.data[i]=%x\r\n",tx_power_wakeup.data[i]);
            }
            else if(i == pos_index[2])
            {
                 memcpy(&(tx_power_wakeup.data[i]), bondInfo.peer_addr, 6);
                 i += 5;
            }
        }

        printf("wakeup packet =\r\n");
        for(i=0;i<buffer[0];i++)
            printf(" %x",tx_power_wakeup.data[i]);
        for(u8 i=0;i<3;i++)
        {
           rf_set_ble_channel(37+i);

           rf_start_stx (&tx_power_wakeup, 100);

           while(rf_tx_finish()!=0x01);

           rf_tx_finish_clear_flag();
        }

    }

    if(keyid != 0xff)
    {
        index++;
        write_wakeup_keyindex_info(index);
    }
}

void app_delay_set(void)
{
    //printf("app_delay_set\r\n");
    google_get_rsp_delay();
    if((flag_dle & 0x80) == 0)
    {
        mtuExchange_check_tick = clock_time() | 1;
        mtuExchange_check_timer = 500000;
    }
}

void app_set_mtusize(void)
{
    if(app_is_set_mtusize == 0)
    {
        app_is_set_mtusize = 1;
        printf("app_mtu_size=%x\r\n",app_mtu_size);
        if(app_mtu_size < MTU_SIZE_SETTING)
           blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
    }
}

u8 is_mic_enable(void)
{
    return ui_mic_enable;
}

void user_init_normal(void)
{
    //random number generator must be initiated here( in the beginning of user_init_nromal)
    //when deepSleep retention wakeUp, no need initialize again
    random_generator_init();  //this is must

////////////////// BLE stack initialization ////////////////////////////////////

    u8  mac_random_static[6];
    //for 512K Flash, flash_sector_mac_address equals to 0x76000
    //for 1M   Flash, flash_sector_mac_address equals to 0xFF000
    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
    printf("mac_public=[%02X %02X %02X %02X %02X %02X]\r\n",mac_public[5],mac_public[4], mac_public[3], mac_public[2], mac_public[1], mac_public[0]);
    #if(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
        app_own_address_type = OWN_ADDRESS_PUBLIC;
    #elif(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
        app_own_address_type = OWN_ADDRESS_RANDOM;
        blc_ll_setRandomAddr(mac_random_static);
    #endif

    ////// Controller Initialization  //////////
    blc_ll_initBasicMCU();                      //mandatory
    blc_ll_initStandby_module(mac_public);                //mandatory
    blc_ll_initAdvertising_module(mac_public);     //adv module:          mandatory for BLE slave,
    blc_ll_initConnection_module();                //connection module  mandatory for BLE slave/master
    blc_ll_initSlaveRole_module();                //slave module:      mandatory for BLE slave,
    blc_ll_initPowerManagement_module();        //pm module:           optional
    blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);

    #if(AUDIO_TRANS_USE_2M_PHY_ENABLE)
        blc_ll_init2MPhyCodedPhy_feature();            //enable 2M/coded PHY
    #endif

    ////// Host Initialization  //////////
    blc_gap_peripheral_init();    //gap initialization
    extern void my_att_init ();
    my_att_init (); //gatt initialization
    blc_l2cap_register_handler (blc_l2cap_packet_receive);      //l2cap initialization

    //after patch new liblt_8278.a, blc_att_prepareWriteBufferCfg() is not available.
    //if want to use goolge IR , need to set as prepareWrite_b 300
    //default as 256
    //extern ble_sts_t blc_att_prepareWriteBufferCfg (u8 *pBuf, u16 bufSize);
    //u8 status = blc_att_prepareWriteBufferCfg (prepareWrite_b, sizeof(prepareWrite_b));
    //if(status != BLE_SUCCESS) { while_1_debug(); }  //debug: l2cap buffer setting err

    blc_att_setRxMtuSize(MTU_SIZE_SETTING);

    //Smp Initialization may involve flash write/erase(when one sector stores too much information,
    //   is about to exceed the sector threshold, this sector must be erased, and all useful information
    //   should re_stored) , so it must be done after battery check
    #if (BLE_REMOTE_SECURITY_ENABLE)
        blc_smp_peripheral_init();

        //Hid device on android7.0/7.1 or later version
        // New paring: send security_request immediately after connection complete
        // reConnect:  send security_request 1000mS after connection complete. If master start paring or encryption before 1000mS timeout, slave do not send security_request.
        blc_smp_configSecurityRequestSending(SecReq_PEND_SEND, SecReq_PEND_SEND, 8000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
        //blc_smp_configSecurityRequestSending(SecReq_NOT_SEND, SecReq_NOT_SEND, 1000);
    #else
        blc_smp_setSecurityLevel(No_Security);
    #endif

///////////////////// USER application initialization ///////////////////
    app_set_advdata_rspdata();

    if(analog_read(USED_DEEP_ANA_REG) & POWER_ON_FLG) //already powered on once
    {
        //debug only, deep wake up should go here..
        //printf("power on deep\r\n");
        printf("------------------ power on deep -----------------------------\r\n");
        power_on_from = POWER_ON_DEEP;
//        if (kb_scan_key(KB_NUMLOCK_STATUS_POWERON, 1) && kb_event.cnt) {
//            //memcpy(&kb_event_cache, &kb_event, sizeof(kb_event));
//            printf("kb_event.cnt %d, kb_event.keycode %d\n", kb_event.cnt,kb_event.keycode[0]);
//            if ((kb_event.cnt == 1) && (kb_event.keycode[0] == KEY_OK)) {
//                printf("deep back ok key pressed\n");
//            }
//        }

    }
    else
    {
        #define KEY_BACK 26 //forgive me to make it dependent with app_ui.c
        #define KEY_VOL_DOWN    38

        power_on_from = POWER_ON_NORMAL;
        printf("------------------ power on Cold start -----------------------------\r\n");
        if(kb_scan_key (KB_NUMLOCK_STATUS_POWERON, 1) && kb_event.cnt){
             memcpy(&kb_event_cache,&kb_event,sizeof(kb_event));
             printf("kb_event.cnt %d, kb_event.keycode %d\n",kb_event.cnt, kb_event.keycode[0]);
             //power_on_pressed_key = kb_event.keycode[0];
             if( (kb_event.cnt == 1) && (kb_event.keycode[0] == KEY_BACK) ){
                 extern u32 ftick_power_on_key_pressed;
                 ftick_power_on_key_pressed = clock_time()|TRUE;
                 printf("power on back key pressed\n");
             }

             if (kb_event.cnt == 1 && kb_event.keycode[0] == KEY_VOL_DOWN) {
                extern u32 ftick_vol_up_pressed;
                app_led_long_on();
                ftick_vol_up_pressed = clock_time() | TRUE;
                printf("power on VOL- key pressed\n");
                app_adv_indirect(); // 启动广播

             }
             // process in post
        }

    }
    u8 data = analog_read(DEEP_ANA_REG2);
    if(data == 0x0f)
        analog_write(DEEP_ANA_REG2, 0);
    app_schedule_ota_wakeup_host();

    //is_pairing_mode = 0;
    device_in_connection_state = 0;
    //is_reconn_mode = 0;

    ////////////////// config adv packet /////////////////////

    blc_smp_param_setBondingDeviceMaxNumber(1);
#if (MP_TEST_MODE)
    if (test_get_mode() != MODE_TEST)
#endif
    {
        //bls_smp_eraseAllParingInformation();

        u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
        smp_param_save_t  bondInfo;
        if(bond_number)   //at least 1 bonding device exist
        {
           bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )
        }
        printf("init bond_number %d\n",bond_number);


        if(bond_number)   //set direct adv
        {
            //app_set_adv_interval_downgrade_direct();
            printf("peer_addr=%x %x %x %x %x %x\r\n",bondInfo.peer_addr[0],bondInfo.peer_addr[1],bondInfo.peer_addr[2],bondInfo.peer_addr[3],bondInfo.peer_addr[4],bondInfo.peer_addr[5]);
//            if((analog_read(USED_DEEP_ANA_REG) & OTA_FLG) == OTA_FLG) // if deep from OTA, call a service change(no more used)
//            {
//                printf("ana read = ota succ\r\n");
//                service_change = TRUE;
//            }
           /* enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;
            //set direct adv
            u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                            ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                            bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                            MY_APP_ADV_CHANNEL,
                                            ADV_FP_NONE);
            if(status != BLE_SUCCESS) { while_1_debug(); }  //debug: adv setting err
            //it is recommended that direct adv only last for several seconds, then switch to indirect adv
            set_app_adv_type(ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY);


            //recursively call to direct adv
            bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &indirect_adv);

            bls_ll_setAdvDuration(2000000, 1); // 2s
            #if (EQ_TEST)
                bls_ll_setAdvEnable(1);  printf("power on direct adv\r\n");
            #endif
            //adv_time_to_deep = RECONN_ADV_ENTER_DEEP_TIME;
           */
            ftick_user_pair_start=0;
            direct_adv(0, 0, 3*1000*1000);
            printf("ftick_ok_reconn_send_code enabled \n");
        }
        else   //set indirect adv
        {
            app_set_adv_interval_downgrade_indirect();
            enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
            u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                             ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
                                             0,  NULL,
                                             MY_APP_ADV_CHANNEL,
                                             ADV_FP_NONE);
            if(status != BLE_SUCCESS) { while_1_debug(); }  //debug: adv setting err
            set_app_adv_type(ADV_TYPE_CONNECTABLE_UNDIRECTED);

            #if (EQ_TEST)
                bls_ll_setAdvEnable(1);  printf("power on indirect adv\r\n");
            #endif
        }
    }
#if (MP_TEST_MODE)
    else
    {
        printf("test mode\r\n");
        app_set_adv_interval_downgrade_indirect();
        enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
        u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                         ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
                                         0,  NULL,
                                         MY_APP_ADV_CHANNEL,
                                         ADV_FP_NONE);
        if(status != BLE_SUCCESS) { while_1_debug(); }  //debug: adv setting err
        set_app_adv_type(ADV_TYPE_CONNECTABLE_UNDIRECTED);

        printf("inderict adv\r\n");
    }
#endif

    //set rf power index, user must set it after every suspend wakeup, cause relative setting will be reset in suspend
    user_set_rf_power(0, 0, 0);
    bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &user_set_rf_power);

    blc_gap_registerHostEventHandler( app_host_event_callback );
    blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN             |  \
                          GAP_EVT_MASK_SMP_PARING_SUCCESS           |  \
                          GAP_EVT_MASK_SMP_PARING_FAIL                |  \
                          GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
                          GAP_EVT_MASK_ATT_EXCHANGE_MTU
                          );

    //ble event call back
    bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
    bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &ble_remote_terminate);

    bls_app_registerEventCallback (BLT_EV_FLAG_LL_REJECT_IND, &ble_ll_reject);

    bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_REQ, &task_conn_update_req);
    bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_UPDATE, &task_conn_update_done);

    bls_app_registerEventCallback (BLT_EV_FLAG_DATA_LENGTH_EXCHANGE, &task_datalengthexchange);
    blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);
    #if(AUDIO_TRANS_USE_2M_PHY_ENABLE)
        blc_gap_setEventMask(GAP_EVT_MASK_ATT_EXCHANGE_MTU);
        blc_gap_registerHostEventHandler( app_host_event_callback );
        bls_app_registerEventCallback (BLT_EV_FLAG_PHY_UPDATE, &app_phy_update_complete_event);
    #endif

    ///////////////////// Power Management initialization///////////////////
#if(BLE_REMOTE_PM_ENABLE)
    blc_ll_initPowerManagement_module();

    #if (PM_DEEPSLEEP_RETENTION_ENABLE)
        bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
        blc_pm_setDeepsleepRetentionThreshold(50, 30);
        #if (__PROJECT_8278_BLE_REMOTE__)
            blc_pm_setDeepsleepRetentionEarlyWakeupTiming(480);
        #else
            blc_pm_setDeepsleepRetentionEarlyWakeupTiming(400);
        #endif
    #else
        bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
    #endif

    bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &ble_remote_set_sleep_wakeup);
#else
    bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif

#if    (TL_AUDIO_MODE & TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_MSBC_MODE)
    blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
#else

#endif

#if (GOOGLE_OTA)//(BLE_REMOTE_OTA_ENABLE)
    ////////////////// OTA relative ////////////////////////
    bls_ota_clearNewFwDataArea(); //must
    bls_ota_init_handle(OTA_CMD_OUT_DP_H_GL);
#endif
#if (TELINK_OTA)
    bls_ota_clearNewFwDataArea(); //must
    bls_ota_registerStartCmdCb(app_enter_ota_mode);
    bls_ota_registerResultIndicateCb(app_debug_ota_result);  //debug
#endif



    app_ui_init_normal();
    advertise_tick_retime();

    //user data init
    init_ccc_value();
    printf("init_ccc_value data:0x%x\n",flag_ccc_data);
    is_flash_info_full();
    init_peer_mac();
#if (APP_IR_ENABLE)
    app_ir_programming_end_register(app_delay_set);
    ir_param_init(); //init ir which read from TV, do macro disable
    u8 ccc = init_ir_key_event_notify();
    if(ccc != 0xff)
        ir_init_key_event_notify(ccc);
#endif
    //init_wakeup_keyindex();
    extern void init_device_info (void);
    init_device_info();


    //not_used_gpio_pull_down(); // ouput disable , input enable and pull down
}


_attribute_ram_code_ void user_init_deepRetn(void)
{
#if (PM_DEEPSLEEP_RETENTION_ENABLE)
    blc_ll_initBasicMCU();   //mandatory
#if (MP_TEST_MODE)
    if (gpio_read(DUT_INPUT_PORT) && test_get_mode() == MODE_TEST)
        rf_set_power_level_index(RF_POWER_P7p37dBm);
    else
#endif
    rf_set_power_level_index (MY_RF_POWER_INDEX);

    blc_ll_recoverDeepRetention();
    blc_att_setRxMtuSize(MTU_SIZE_SETTING);

    DBG_CHN0_HIGH;    //debug

    irq_enable();

    app_ui_init_deepRetn();
#endif
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
#if (APP_IR_ENABLE)
void ir_flash_erase_loop(void)
{
    if((ir_flash_erase_tick))
    {
        if(ir_flash_erase_timeoutcb() == -1)
        {
            ir_flash_erase_tick = 0;
        }
    }
}
#endif

#if 1
u32 test_toggle_tick=0;
u32 test_toggle_tick2=0;
_attribute_data_retention_ u8 test_toggle_level=0;
//_attribute_data_retention_ u8 test_toggle_count=0;

void test_toggle(void)
{
    if(clock_time_exceed(test_toggle_tick, 1000*1000))
    {
        #if 1
        test_toggle_tick = clock_time() | 1;
        //printf(" test_toggle_count %d\n",test_toggle_count++);
        if(test_toggle_level)
        {
            //device_led_on(0);
            test_toggle_level = 0;
            printf(".");
        }
        else
        {
            //device_led_off(0);
            test_toggle_level = 1;
        }
        #else


        #endif

    }
}


void test_led(){

    if(clock_time_exceed(test_toggle_tick, 1000*1000))
    {
        test_toggle_tick = clock_time() | 1;
        app_led_pairing();
        printf("test led \n");
        //gpio_write(GPIO_PC3,1);

    }

    if(clock_time_exceed(test_toggle_tick2, 1000*1000))
    {
        //test_toggle_tick2 = clock_time() | 1;
        //gpio_write(GPIO_PC3, 0);
    }


#if (BLT_APP_LED_ENABLE)
    device_led_process();
#endif

}
#endif // test function block

void app_debug_connect(){
#if 0
    // hfrom connect count to state BLS_LINK_STATE_CONN
    static u32 connect_time_difference1 =0;
    static u32 connect_time_difference2 =0;
    static u32 connect_time_difference3 =0;
    static u32 connect_time_difference4 =0;

    //conclusion : when reconnect GAP_EVT_SMP_CONN_ENCRYPTION_DONE is the last step
    if((blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) && device_in_connection_state){
        connect_time_difference1 = clock_time() | TRUE;// tick start
    }

    if((blc_ll_getCurrentState() == BLS_LINK_STATE_CONN) && device_in_connection_state && connect_time_difference1){
        printf("conn_diff1 %d \n", clock_time() - connect_time_difference1);
        connect_time_difference1 =0;
    }

    if((blc_ll_getCurrentState() == BLS_LINK_STATE_CONN) && !device_in_connection_state){
        connect_time_difference2 = clock_time() | TRUE;// tick start
    }
    if((blc_ll_getCurrentState() == BLS_LINK_STATE_CONN) && device_in_connection_state && connect_time_difference2){
        printf("conn_diff2 %d \n", clock_time() - connect_time_difference2);
        connect_time_difference2 =0;
    }

    if( device_in_connection_state && !device_in_connection_state2){
        connect_time_difference3 = clock_time() | TRUE;// tick start
    }
    if(device_in_connection_state && connect_time_difference3){
        printf("conn_diff 3 %d \n", clock_time() - connect_time_difference3);
        connect_time_difference3 =0;
    }

    if( !device_in_connection_state && device_in_connection_state2){
        connect_time_difference4 = clock_time() | TRUE;// tick start
    }
    if(device_in_connection_state && connect_time_difference4){
        printf("conn_diff 4 %d \n", clock_time() - connect_time_difference4);
        connect_time_difference4 =0;
    }
#endif

}




void app_ptm_proc_cmd(){
#if (PTM)
    #include "app_ptm.h"
    extern bool ptm_enabled;
    if(ptm_enabled){ // when PTM pin H, set ptm_enabled
        if(ui_mic_enable) {
            extern void app_handler_voice_data(void);
            app_handler_voice_data();
        }
        // cmd proc
        ptm_uart_cmd_proc(); // handle uart rx data, or handle rx fifo, if INT works ok, data ready in  buffer/fifo now....
    }
#endif
}

u32 ftick_loop;
u8 tick_loop;
void main_loop (void)
{
    //test functions
    //test_toggle();
    //test_led(); return;
    //app_debug_connect();

    //--------------------------------------------------------------------------------------

    if(clock_time_exceed(ftick_loop, 1*1000*1000)){
        tick_loop ++;
        ftick_loop = clock_time();
    }

#if (BLT_TEST_SOFT_TIMER_ENABLE)
    blt_soft_timer_process(MAINLOOP_ENTRY);
#endif
    ////////////////////////////////////// BLE entry /////////////////////////////////
    blt_sdk_main_loop();
    mtu_exchange_loop();
#if (APP_IR_ENABLE)
    ir_flash_erase_loop();
    ir_flash_save_loop();
#endif
    ////////////////////////////////////// UI entry /////////////////////////////////
#if (BLE_AUDIO_ENABLE)
    app_audio_task();
#endif
#if (GOOGLE_OTA)
    app_ota_proc();
#endif
    #if (BATT_CHECK_ENABLE)
        // 如果红外有发码,延迟一个周期后再检测电量
        if (ir_send_ctrl.is_sending) {
            lowBattDet_tick = clock_time();
        }
        if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 300000)){
            lowBattDet_tick = clock_time();
            #if(VBAT_LEAKAGE_PROTECT_EN)

            u8 analog_deep = analog_read(USED_DEEP_ANA_REG);
            u16 vbat_deep_thres = VBAT_DEEP_THRES_MV;
            u16 vbat_suspend_thres = VBAT_SUSPEND_THRES_MV;
            if(analog_deep & LOW_BATT_FLG){
                if(analog_deep & LOW_BATT_SUSPEND_FLG){//<1.8v
                    vbat_deep_thres += 200;
                    vbat_suspend_thres += 100;
                }
                else{//1.8--2.0v
                    vbat_deep_thres += 200;
                }
            }
            app_battery_power_check(vbat_deep_thres,vbat_suspend_thres);

            #else
                app_battery_power_check(VBAT_DEEP_THRES_MV);  //2000 mV low battery
            #endif
        }
    #endif
    proc_keyboard (0,0, 0);

    ui_key_post_process();
    ui_post_process();
    app_ptm_proc_cmd();

#if (BLT_APP_LED_ENABLE)
    app_led_proc2();
    device_led_process();
#endif

    blt_pm_proc();



}




#endif  //end of __PROJECT_8267_BLE_REMOTE__ || __PROJECT_8261_BLE_REMOTE__ || __PROJECT_8269_BLE_REMOTE__
