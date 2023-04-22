/******************************************************************************
 * @file     app_custom.h
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

#ifndef APP_CUSTOM_H_
#define APP_CUSTOM_H_

#define APP_CUSTOM_WAKEUPKEY_NUM 12

extern u8 device_type;
extern u8 key_setting_or_noti[2];
extern u8 key_guide_or_livetv[2];
extern u16 key_subtitle_or_teletext[2];
extern u8 wakeup_key_pos;
extern const unsigned char Kb_Map_Normal[8][3];
//extern
void app_custom_init(void);
void app_custom_init_R52(void);
extern u8 app_custom_get_drive_len(void);
//extern void app_custom_param_init(u8 device);
extern u8 app_custom_is_enable_wakeup_key(void);
extern u8  app_custom_wakeupkey_packet_index(u8 keyid);

#endif
