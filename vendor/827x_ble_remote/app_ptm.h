#ifndef __APP_RC_TEST_HANDLER_H
#define __APP_RC_TEST_HANDLER_H

#include "../../common/types.h"

void app_uart_test_init(void);
void app_uart_irq_proc(void);
void app_uart_send_buffer(u8 *p_data,u16 len);
void app_check_enter_ptm(void);
void app_uart_send_voice_data(uint8_t *p_data,uint16_t len);

void ptm_uart_cmd_proc(void);

#if 0
void app_rc_test_read_version_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_read_uid_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_read_mac_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_enter_sleep_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_rf_signel_carrier_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_dtm_mode_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_ctrl_gpio_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_ctrl_ir_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_sensor_data_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_eeprom_read_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_flash_read_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_batt_data_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_batt_vol_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_start_ble_adv_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_stop_ble_adv_handler(uint8_t *p_data,uint16_t len);
void app_rc_test_disconnect_ble_handler(uint8_t *p_data,uint16_t len);
void app_rc_deleted_bound_handler(uint8_t *p_data,uint16_t len);
void app_rc_change_baud_handler(uint8_t *p_data,uint16_t len);
void app_rc_start_record_handler(uint8_t *p_data,uint16_t len);
void app_rc_stop_record_handler(uint8_t *p_data,uint16_t len);
void app_device_reset_handler(uint8_t *p_data,uint16_t len);
#endif




































#endif
