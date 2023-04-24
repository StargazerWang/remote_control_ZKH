/******************************************************************************
 * @file     audio_common.h
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

#ifndef AUDIO_COMMON_H_
#define AUDIO_COMMON_H_
//掩码（Mask）：是一种用于标记或表示某些属性或选项的二进制值。在编程中，掩码通常用于按位与（Bitwise AND）操作，以判断一个二进制值中是否包含或缺失了某些特定的属性或选项。掩码还常用于在寄存器或内存中标记某些 bit 是否被设置，或者状态是否被激活。掩码是计算机底层编程非常常见的概念，应用广泛。

//SBC、mSBC 和 ADPCM 三种编码模式的掩码。
#define        TL_AUDIO_MASK_SBC_MODE                           (0x00000001)    //SBC
#define        TL_AUDIO_MASK_MSBC_MODE                          (0x00000002)    //mSBC
#define        TL_AUDIO_MASK_ADPCM_MODE                         (0x00000004)    //ADPCM

//HID、Telink 和 Google 三种 GATT 传输通道的掩码。
#define        TL_AUDIO_MASK_HID_SERVICE_CHANNEL                (0x00000100)    //HID
#define        TL_AUDIO_MASK_GATT_SERVICE_TELINK_CHANNEL        (0x00000200)    //TELINK
#define        TL_AUDIO_MASK_GATT_SERVICE_GOOGLE_CHANNEL        (0x00000400)    //GOOGLE

//从 Dongle 到 STB 的传输通道的掩码。
#define        TL_AUDIO_MASK_DONGLE_TO_STB                      (0x00010000)    

// RCU 和 Dongle 两个项目的掩码。
#define        RCU_PROJECT                                      (0x01000000)
#define        DONGLE_PROJECT                                   (0x02000000)

//表示禁用音频功能
#define        AUDIO_DISABLE                                    0               
//-------------------------------------------------------------------------------------------------------------------------------------
//RCU Audio Mode
//用于在 RCU 项目中配置不同的音频模式。这些宏由多个掩码按位或组成，可以表示不同的编码方式、传输通道和项目类型。
//每一个宏都以 TL_AUDIO_RCU_ 开头，后面跟着音频编码方式和传输通道的缩写，以及可能的项目类型（即 RCU_PROJECT）。其中最后两个宏还包含了一个名为 DONGLE_TO_STB 的传输通道掩码

//--使用 ADPCM 编码，在 Telink GATT 传输通道上进行音频传输--
#define        TL_AUDIO_RCU_ADPCM_GATT_TLEINK                   (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_TELINK_CHANNEL)

//--使用 ADPCM 编码，在 Google GATT 传输通道上进行音频传输--
#define        TL_AUDIO_RCU_ADPCM_GATT_GOOGLE                   (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_GOOGLE_CHANNEL)

//--使用 ADPCM 编码，在 HID 传输通道上进行音频传输--
#define        TL_AUDIO_RCU_ADPCM_HID                           (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

//--使用 SBC 编码，在 HID 传输通道上进行音频传输--
#define        TL_AUDIO_RCU_SBC_HID                             (RCU_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

//--使用 ADPCM 编码，在 HID 和 Dongle 到 STB 传输通道上进行音频传输--
#define        TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB             (RCU_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

//--使用 SBC 编码，在 HID 和 Dongle 到 STB 传输通道上进行音频传输--
#define        TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB               (RCU_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

//--使用 mSBC 编码，在 HID 传输通道上进行音频传输--
#define        TL_AUDIO_RCU_MSBC_HID                            (RCU_PROJECT | TL_AUDIO_MASK_MSBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

//-------------------------------------------------------------------------------------------------------------------------------------
//Dongle Audio Mode
//根据具体需要选择，以实现不同的音频传输方式

//使用ADPCM编码，通过GATT服务发送音频数据到Telink通道
#define        TL_AUDIO_DONGLE_ADPCM_GATT_TELINK                (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_TELINK_CHANNEL)

//使用ADPCM编码，通过GATT服务发送音频数据到Google通道
#define        TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE                (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_GATT_SERVICE_GOOGLE_CHANNEL)

//使用ADPCM编码，通过HID服务发送音频数据
#define        TL_AUDIO_DONGLE_ADPCM_HID                        (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

//使用ADPCM编码，通过HID服务发送音频数据
#define        TL_AUDIO_DONGLE_SBC_HID                          (DONGLE_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)

//使用ADPCM编码，通过HID服务发送音频数据
#define        TL_AUDIO_DONGLE_ADPCM_HID_DONGLE_TO_STB          (DONGLE_PROJECT | TL_AUDIO_MASK_ADPCM_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

//使用SBC编码，通过HID服务将音频数据从Dongle发送到STB设备
#define        TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB            (DONGLE_PROJECT | TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL | TL_AUDIO_MASK_DONGLE_TO_STB)

//使用mSBC编码，通过HID服务发送音频数据
#define        TL_AUDIO_DONGLE_MSBC_HID                         (DONGLE_PROJECT | TL_AUDIO_MASK_MSBC_MODE | TL_AUDIO_MASK_HID_SERVICE_CHANNEL)






#endif /* AUDIO_COMMON_H_ */
