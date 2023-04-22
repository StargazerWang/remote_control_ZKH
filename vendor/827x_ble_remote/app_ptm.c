#include "app_ptm.h"
#include "stack/ble/ble.h"
#include "vendor/common/blt_common.h"
#include "battery_check.h"
#include "app_fifo_byte.h"

//#include "vendor/common/tl_audio.h"

//extern const u8 my_FWRevtrs [];

//u8 app_rc_test_audio_start = 0;

#define UART_RX_FIFO_SIZE (32) // only 2^x

#define TEST_CMD_MAX_RCV_LEN (32)
//#define TEST_CMD_MAX_ACK_LEN (64)

#define TEST_START_TX (0x011E2003)


app_fifo_byte_t cmd_rx_fifo;
static uint8_t cmd_rx_fifo_buffer[UART_RX_FIFO_SIZE];
static uint8_t test_cmd_rcv_buffer[TEST_CMD_MAX_RCV_LEN];
//uint8_t test_cmd_ack_buffer[TEST_CMD_MAX_ACK_LEN];
//uint8_t rc_test_mode_status = TEST_MODE_NOT_RUN;          //for exit state
bool ptm_enabled = FALSE;

//extern app_audio_send_index and app_audio_transfer_timer here
#if 0
static void _ctrl_google_voice(bool is_start)
{
    if (is_start)
    {
        google_voice_codec_used = CODEC_USED_16K;
        google_voice_packet_length = VOICE_V0P4_ADPCM_PACKET_LEN;
        google_voice_pcm_sample_packet = VOICE_V0P4_ADPCM_UNIT_SIZE;

        app_audio_send_index = 0;

        google_mic_enable();

        app_audio_transfer_timer = 0;
    }
    else
    {
        ui_enable_mic(0);
        //TODO : move led
        _ctrl_led(_ON, _ON);
    }
}
#endif




void enable_ptm(void)
{
    if (!ptm_enabled)
    {
        ptm_enabled = TRUE;
        app_fifo_byte_init(&cmd_rx_fifo, cmd_rx_fifo_buffer, UART_RX_FIFO_SIZE);
        app_uart_test_init();
        //device_led_setup(led_cfg[4]);  //LED_SHINE_FAST: 4
        extern void app_led_pairing(void);
        app_led_pairing();
        // send uart to indicate PTM start...
    }
}

//when power on , check pin to enter test mode,
// if test mode , check uart rx and run test handler, else not enter test mode
void app_check_enter_ptm(void)
{
    //pull down and input, to avoid current leak

    #define PTM_PIN GPIO_PA6 // spec GPIO_PA6, test PB7
	// PTM pin PA6
    gpio_setup_up_down_resistor(PTM_PIN, PM_PIN_PULLDOWN_100K);
    gpio_set_func(PTM_PIN,AS_GPIO);
    gpio_set_input_en(PTM_PIN, 1);
    WaitUs(10);    //wait stable

	if(gpio_read(PTM_PIN))
	{
	    WaitMs(50);
        if(gpio_read(PTM_PIN))
        {
            enable_ptm();
        }
	}

}



void app_uart_test_init(void)
{
    uart_gpio_set(UART_TX_PB1, UART_RX_PB0);// uart tx/rx pin set
	uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

    //baud rate: 115200
    uart_init_baudrate(115200,CLOCK_SYS_CLOCK_HZ,PARITY_NONE, STOP_BIT_ONE); // ok, CLOCK_SYS_CLOCK_HZ

	uart_dma_enable(0, 0);
	irq_disable_type(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 0);
	uart_irq_enable(1,0);   //uart RX irq enable
	uart_ndma_irq_triglevel(1,0);   //set the trig level. 1 indicate one byte will occur interrupt
	irq_enable();
}


//volatile unsigned char uart_rx_flag=0;
//volatile unsigned int  uart_ndmairq_cnt=0;
volatile unsigned char uart_ndmairq_index=0;

u8 D_uart_rx =0;
void app_uart_irq_proc(void)
{
    static unsigned char uart_ndma_irqsrc;
	//static unsigned char uart_ndmairq_index = 0;
	uart_ndma_irqsrc = uart_ndmairq_get();  ///get the status of uart irq.
	if (uart_ndma_irqsrc){
	    //debug
	    D_uart_rx = read_reg8(0x90 + uart_ndmairq_index);
		app_fifo_byte_put(&cmd_rx_fifo, D_uart_rx);

		//or
		//app_fifo_byte_put(&cmd_rx_fifo, read_reg8(0x90 + uart_ndmairq_index));

		uart_ndmairq_index++;
		uart_ndmairq_index &= 0x03;
	}
	else{
	}

}






//------------ sample test items list below for future cusmization ref only----------------------------------------------------

void app_uart_send_buffer(u8* p_data,u16 len)
{
    uint16_t i;

    for(i=0;i<len;i++)
    {
        uart_ndma_send_byte(p_data[i]);
    }

}

void app_start_mic_for_test(u8 en)
{
	//AMIC Bias output
	extern void ui_enable_mic(int en);
	if(en){  //audio on
		///////////////////// AUDIO initialization///////////////////
		//buffer_mic set must before audio_init !!!
        ui_enable_mic(1);
        audio_amic_init(AUDIO_16K);
	}
	else{  //audio off
	    //8271
		//audio_codec_and_pga_disable();	//power off codec and pga
		//amic_gpio_reset();

		//8251
        ui_enable_mic(0);
	}
	#if ((!BLE_DMIC_ENABLE) && (BATT_CHECK_ENABLE))
		battery_set_detect_enable(!en);
	#endif

}

void app_handler_voice_data(void)
{

#define TL_MIC_BUFFER_SIZE                2048    //byte
#if 1
    extern s16        buffer_mic[];
	static u16	buffer_mic_rptr = 0;
	static u16   dn_sample_cnt = 0;

	//u16 index;
	u16 mic_wptr = get_mic_wr_ptr();
	u16 l = (mic_wptr >= buffer_mic_rptr) ? (mic_wptr - buffer_mic_rptr) : 0xffff;
	u16 *p_data = (u16*)(buffer_mic + buffer_mic_rptr);

	if(l >= 64)
	{
	    #if (1) // send raw data only for test
		    //app_uart_send_buffer((u8*)p_data,64*2);


	        //test by 4 bytes
	        if(((dn_sample_cnt++) % 8) == 0)
	            app_uart_send_buffer((u8*)p_data,1);

	    #else   // send header cmd and crc, need to know how pc side tool decode it , or use prior project PTM codes
		    app_uart_send_voice_data((u8* )p_data,64 * 2);
		#endif

		buffer_mic_rptr = buffer_mic_rptr + 64;

		if(buffer_mic_rptr >= (TL_MIC_BUFFER_SIZE>>1))
		{
			buffer_mic_rptr = 0;
		}
	}
#endif

}


u8 D_cmd =0;
void ptm_uart_cmd_proc(void)
{
    uint8_t byte;
    //uint16_t check_sum;

    //uint8_t fifo_is_empt = 0;

    static uint16_t index = 0;
    while (!app_fifo_byte_get(&cmd_rx_fifo, &byte))
    {
        // will reimplement this...or use method of 8251

        test_cmd_rcv_buffer[index++] = byte;
        D_cmd = test_cmd_rcv_buffer[0];

        //app_uart_send_buffer((u8*)&D_cmd,1);

        if (index == 7){ //TEST_START_TX
            u32 cmd =0;
            cmd = (test_cmd_rcv_buffer[0]) | (test_cmd_rcv_buffer[1]<8) | (test_cmd_rcv_buffer[1]<16) | (test_cmd_rcv_buffer[1]<24) ;
            if (cmd == TEST_START_TX)
            {
                //process cmd
                //if cmd ok, index=0;
                index = 0;//reset
            }
        }

        if (index == 6){
        }

        if (index == 5){
        }


        if(D_cmd == 0x01){ // 01 to start voice test
            // don't use google voice
            //blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
            //_ctrl_google_voice(TRUE);

            //use pcm directly
            //u8 start = 0x03;
            //app_uart_send_buffer((u8*)&start,1);

            app_start_mic_for_test(TRUE);
            index = 0;//reset
        }

        if(D_cmd == 0x02){ // 02 to stop voice test
            app_start_mic_for_test(FALSE);
            index = 0;//reset
        }


    }
}

//--------------------------------------------------------------------------------

#if 0
void app_uart_send_voice_data(uint8_t *p_data,uint16_t len)
{
    uint16_t i;

    if((5+len+2) > TEST_CMD_MAX_ACK_LEN)
    {
        while(1);
    }

    rc_test_set_ack_status_code(0x02);
    rc_test_set_ack_data_len(len);

    u8 *p_ack_data = rc_test_get_ack_data_pointer();

    for(i=0;i<len;i++)
    {
        p_ack_data[i] = p_data[i];
    }
    rc_test_send_ack(REQ_START_RECORD_CMD);
}
void app_rc_test_read_version_handler(uint8_t *p_data,uint16_t len)
{
    uint8_t *p_ack_data,i;
    rc_test_set_ack_status_code(0x00);
    if(rc_test_set_ack_data_len(strlen((char *)my_FWRevtrs)))
    {
        while(1);
    }

    p_ack_data = rc_test_get_ack_data_pointer();

    for(i=0;i<strlen((char *)my_FWRevtrs);i++)
    {
        *p_ack_data++ = my_FWRevtrs[i];
    }

}

//extern int u_sprintf(char *out, const char *format, ...);
extern  int flash_read_mid_uid_with_check( unsigned int *flash_mid ,unsigned char *flash_uid);

void app_rc_test_read_uid_handler(uint8_t *p_data,uint16_t len)
{

    uint8_t *p_ack_data;
    u32 mid;
    u8 buff[16] = {0};
    rc_test_set_ack_status_code(0x00);

    if(rc_test_set_ack_data_len(32))
     {
        //这里出错直接进入while
        while(1);
    }

    p_ack_data = rc_test_get_ack_data_pointer();

    //watch out api flash_read_mid_uid_with_check, memory over lap
    if(!flash_read_mid_uid_with_check(&mid,buff))
    {
        while(1);
    }

    for(u8 i = 0;i<16;i++)
    {
    	u_sprintf(p_ack_data + i*2,"%02X",buff[i]); // warning here
    }

}

void app_rc_test_read_mac_handler(uint8_t *p_data,uint16_t len)
{
    uint8_t *p_ack_data;
    u8 buff[16];

    rc_test_set_ack_status_code(0x00);
    if(rc_test_set_ack_data_len(12))
     {
        //这里出错直接进入while
        while(1);
    }

    p_ack_data = rc_test_get_ack_data_pointer();

	//flash_read_page(flash_sector_mac_address, 6, buff);
    flash_read_page(CFG_ADR_MAC, 6, buff);



    for(u8 i = 0;i<6;i++)
    {
    	u_sprintf(p_ack_data + i*2,"%02X",buff[i]);
    }
}


void app_rc_test_enter_sleep_handler(uint8_t *p_data,uint16_t len)
{

	rc_test_set_ack_status_code(0x00);
	rc_test_set_ack_data_len(00);
	rc_test_send_ack(ENTER_SLEEP_CMD);
    WaitMs(10);
	bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);

    //进入睡眠
    cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep

}


//extern void app_emi_init(void);
void app_emi_init(void)
{
    write_reg32(0x408,0x29417671 );//access code  0xf8118ac9

//    unsigned char  mode=1;//1
//    unsigned char  power_level = 0;
//    unsigned char  chn = 2;//2
//    unsigned char  cmd_now=1;//1
//    unsigned char  run=1;
//    unsigned char  tx_cnt=0;
//    unsigned char  hop=0;

    write_reg8(0x40004,0);

//    write_reg8(0x40005,0);//tx_cnt
//    write_reg8(0x40006,1);//run
//    write_reg8(0x40007,1);//cmd
//    write_reg8(0x40008,0);//power
//    write_reg8(0x40009,2);//chn
//    write_reg8(0x4000a,1);//mode
//    write_reg8(0x4000b,0);//hop

    write_reg32(0x4000c,0);
}

//extern void emicarrieronly(RF_ModeTypeDef rf_mode,RF_PowerTypeDef pwr,signed char rf_chn);
void emicarrieronly(RF_ModeTypeDef rf_mode,RF_PowerTypeDef pwr,signed char rf_chn)
{
    rf_emi_single_tone(pwr,rf_chn);
#if 0
    while( ((read_reg8(0x40006)) == run ) &&  ((read_reg8(0x40007)) == cmd_now ) \
            && ((read_reg8(0x40008)) == power_level ) &&  ((read_reg8(0x40009)) == chn ) \
            && ((read_reg8(0x4000a)) == mode ));
#endif
    //will stop emi
    //rf_emi_stop();
}

void app_rc_test_rf_signel_carrier_handler(uint8_t *p_data,uint16_t len)
{
    //static bool emi_on =0;
    uint8_t rf_ch = *p_data;

    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);
    //rc_test_send_ack(ENTER_RF_SINGLE_CARRIER_CMD);

    rf_drv_init(RF_MODE_BLE_1M);
    app_emi_init();
    //这里需要执行一次复位操作才可以，不然运行将会失败。
    emicarrieronly(RF_MODE_BLE_1M, RF_POWER_P3p94dBm, rf_ch); // , why RF_POWER_P3p50dBm??
}

void app_rc_test_dtm_mode_handler(uint8_t *p_data,uint16_t len)
{
    //#warning "这里最后再设置"
}


//u8 D_gpio_pin=0;
//u8 D_gpio_polarity=0;

#define GPIO_INDEX_LED 1  // IO index , use 1 as LED

void app_rc_test_ctrl_gpio_handler(uint8_t *p_data,uint16_t len)
{
    //len == 2
    //GPIO_LED PC3
    //gpio_pin =  *p_data;
    //D_gpio_polarity = *(p_data+1);
    u8 gpio_pin = *p_data;
    u8 gpio_polarity = *(p_data+1);

    if(gpio_pin == GPIO_INDEX_LED){
        //set gpio
        gpio_set_output_en(GPIO_LED, 1);  //output enable
        gpio_write(GPIO_LED, gpio_polarity);
        sleep_us(1000000);
    }

//    gpio_write(GPIO_LED, 1);
//    sleep_us(500000);
//    gpio_write(GPIO_LED, 0);
//    sleep_us(500000);
//    gpio_write(GPIO_LED, 1);
//    sleep_us(500000);
//    gpio_write(GPIO_LED, 0);
//    sleep_us(500000);

}

void app_rc_test_ctrl_ir_handler(uint8_t *p_data,uint16_t len)
{

    extern void ir_dispatch2(u8 type, u8 syscode ,u16 ircode);

    //u8 ir_polarity = *p_data; // send data or release


    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);

    //ir_dispatch2(1, 0x84, IR_POWER);

    if(*p_data)
    {
        //app_ir_send(TV_POWER,1);
        ir_dispatch2(1, 0x84, IR_POWER);
    }
    else
    {
        //app_ir_send(TV_NC,0);
        //release
        //#define TYPE_IR_RELEASE 2
        ir_dispatch2(2, 0, 0);  //release. //

    }
}

void app_rc_test_sensor_data_handler(uint8_t *p_data,uint16_t len)
{
    uint8_t *p_ack_data;
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(1);
    p_ack_data = rc_test_get_ack_data_pointer();
    *p_ack_data = 0x00;

}


void app_rc_test_eeprom_read_handler(uint8_t *p_data,uint16_t len)
{
    uint8_t *p_ack_data;
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(1);
    p_ack_data = rc_test_get_ack_data_pointer();
    *p_ack_data = 0x00;

}

//这里用来读取FLASH 中IR MODE 配置信息
void app_rc_test_flash_read_handler(uint8_t *p_data,uint16_t len)
{
    //电池电量百分比获取
    //extern   u8  current_IR_mode;
    //TODO read flash to fill the IR mode
    //u8  current_IR_mode =0; //RF391A no IR mode

    uint8_t* p_ack_data;
    rc_test_set_ack_status_code(0x00);
    if(rc_test_set_ack_data_len(1))
    {
        //这里出错直接进入while
        while(1);
    }
    p_ack_data = rc_test_get_ack_data_pointer();

    *p_ack_data = 0x00; // RF391A no IR mode ,current_IR_mode;

}

void app_rc_test_batt_data_handler(uint8_t *p_data,uint16_t len)
{
    extern u8 my_batVal[1];

    //电池电量百分比获取

    uint8_t *p_ack_data;
    rc_test_set_ack_status_code(0x00);
    if(rc_test_set_ack_data_len(1))
     {
        //这里出错直接进入while
        while(1);
    }
    p_ack_data = rc_test_get_ack_data_pointer();

    *p_ack_data = my_batVal[0];		//直接对应的电池电量值 %

}

void app_rc_test_batt_vol_handler(uint8_t *p_data,uint16_t len)
{
#if (BATT_CHECK_ENABLE)
    //电池电压值获取
    extern u16 batt_vol_mv;
    uint8_t *p_ack_data;

    //app_battery_power_check(VBAT_DEEP_THRES_MV,VBAT_SUSPEND_THRES_MV);    //8271
    app_battery_power_check(VBAT_ALRAM_THRES_MV);                           //8251

    rc_test_set_ack_status_code(0x00);
    if(rc_test_set_ack_data_len(4))
    {
        //这里出错直接进入while
        while(1);
    }
    p_ack_data = rc_test_get_ack_data_pointer();

    u_sprintf(p_ack_data,"%4d",batt_vol_mv);
#endif

}

void app_rc_test_start_ble_adv_handler(uint8_t *p_data,uint16_t len)
{
	extern void app_pair_adv_enable(void);
	extern void indirect_advertise(int adv_duration);
	extern u8 user_pair_wait_terminate;

    //启动配对广播
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);

    //app_pair_adv_enable();
    //indirect_advertise(30*1000*1000);

    //广播结束或者配对成功，则清零标志位
    if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
    {
        //断开连接后就自动开始启动配对广播
        //bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
    }



    bls_ll_setAdvEnable(0);
    //if connected , send to terminate, after terminate, del bond info, and start ADV
    if(BLS_LINK_STATE_CONN == blc_ll_getCurrentState()){
        bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
        user_pair_wait_terminate = TRUE;
    }
    //if not connected , repair --> del bond info, and start indirect
    else{
        bls_smp_eraseAllParingInformation(); // if del old pair info, need to del with a new key pressed, due to it will trigger a direct adv
        indirect_advertise(30*1000*1000);
        //app_pair_adv_enable();//no time out
    }



}

void app_rc_test_stop_ble_adv_handler(uint8_t *p_data,uint16_t len)
{

    //停止广播
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);

    bls_ll_setAdvEnable(0);
}

void app_rc_test_disconnect_ble_handler(uint8_t *p_data,uint16_t len)
{
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);

    if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
    {
        //断开连接后就自动开始启动配对广播
        bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
    }
}


void app_rc_deleted_bound_handler(uint8_t *p_data,uint16_t len)
{
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);

    //删除绑定信息
    bls_smp_eraseAllParingInformation();
}

void app_rc_change_baud_handler(uint8_t *p_data,uint16_t len)
{
    //不需要修改
}

/if a uart command in to start rec
void app_rc_start_record_handler(uint8_t *p_data,uint16_t len)
{
	//extern u8  google_voice_ver;
    //we don't have voice ver . should not need it anymore
    u8  google_voice_ver;

    //启动语音PDM传输
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);
    google_voice_ver = 4;	//强制为0 保证MIC buffer FIFO的总长度为2048
    app_start_mic_for_test(1);
    app_rc_test_audio_start = 1; // debug only

}

void app_rc_stop_record_handler(uint8_t *p_data,uint16_t len)
{
    //停止语音PDM 传输
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);
    app_start_mic_for_test(0);
    app_rc_test_audio_start = 0;
}

void app_device_reset_handler(uint8_t *p_data,uint16_t len)
{
    //复位设备
    rc_test_set_ack_status_code(0x00);
    rc_test_set_ack_data_len(0);
	rc_test_send_ack(SET_DEVICE_RESET_CMD);
    WaitMs(10);

    reg_pwdn_ctrl = FLD_PWDN_CTRL_REBOOT;
}

#endif









































