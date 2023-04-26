/******************************************************************************
 * @file     audio_config.h
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


#include  "application/audio/audio_common.h"
#include  "tl_common.h"
#include  "../../vendor/827x_ble_remote/app_config.h"           //在其173行

#ifndef        TL_AUDIO_MODE    //音频模式
#define        TL_AUDIO_MODE                                    AUDIO_DISABLE//禁用音频，在"application/audio/audio_common.h"-49行
#endif

//***下面宏定义先判断 TL_AUDIO_MODE 匹配到了 RCU 还是 DONGLE ，然后再判断是上述两者下的哪种模式***
//*****以下对应application/audio/audio_common中的模式定义******
//-------------------------------RCU-----------------------------------
#if (TL_AUDIO_MODE & RCU_PROJECT)    //#if (TL_AUDIO_MODE & RCU_PROJECT) -- 检查宏TL_AUDIO_MODE的值是否设置了RCU_PROJECT标志.&-当TL_AUDIO_MODE中包含RCU_PROJECT标志，则说明对应二进制位为1，否则为0

    //使用 ADPCM 编码，在 Telink GATT 传输通道上进行音频传输模式    
    #if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)       
        //说明：下面三个数据对应pdf中的6.2.2-数据压缩，以下都为相同的
        #define ADPCM_PACKET_LEN                  128           //ADPCM包的长度
        #define TL_MIC_ADPCM_UNIT_SIZE            248           //TL_MIC采用的ADPCM单元大小。(UNIT-在计算机科学中，“unit”常用于描述某个组件或模块的最小可操作单元，例如“ADPCM单元大小”表示ADPCM编码过程中的最小可操作数据块大小。)
        #define TL_MIC_BUFFER_SIZE                992           //TL_MIC音频缓存区的大小

    //使用 ADPCM 编码，在 Google GATT 传输通道上进行音频传输    
    #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
        #define ADPCM_PACKET_LEN                  136           //(128+6+2) byte
        #define TL_MIC_ADPCM_UNIT_SIZE            256           //sample
        #define TL_MIC_BUFFER_SIZE                2048          //byte

        #define VOICE_V0P4_ADPCM_PACKET_LEN       ADPCM_PACKET_LEN    //byte    //将语音V0P4 ADPCM包的长度设为ADPCM包的长度
        #define VOICE_V0P4_ADPCM_UNIT_SIZE        TL_MIC_ADPCM_UNIT_SIZE        //将语音V0P4 ADPCM单元大小设为TL_MIC采用的ADPCM单元大小

        #define VOICE_V1P0_ADPCM_PACKET_LEN       128           //byte          //语音V1P0 ADPCM包的长度
        #define VOICE_V1P0_ADPCM_UNIT_SIZE        240           //sample        //语音V1P0 ADPCM单元大小

    //使用 ADPCM 编码，在 HID 传输通道上进行音频传输
    #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB)
        #define ADPCM_PACKET_LEN                  120
        #define TL_MIC_ADPCM_UNIT_SIZE            240
        #define TL_MIC_BUFFER_SIZE                960

    //使用 SBC 编码，在 HID 传输通道上进行音频传输    
    #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID)
        #define ADPCM_PACKET_LEN                  120
        #define TL_MIC_ADPCM_UNIT_SIZE            240
        #define TL_MIC_BUFFER_SIZE                960

    //使用 ADPCM 编码，在 HID 和 Dongle 到 STB 传输通道上进行音频传输   
    #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)
        #define ADPCM_PACKET_LEN                  20
        #define TL_MIC_ADPCM_UNIT_SIZE            80
        #define TL_MIC_BUFFER_SIZE                480
    
    //使用 SBC 编码，在 HID 和 Dongle 到 STB 传输通道上进行音频传输
    #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID)
        #define ADPCM_PACKET_LEN                  20
        #define MIC_SHORT_DEC_SIZE                80
        #define TL_MIC_BUFFER_SIZE                320
    
    //使用 mSBC 编码，在 HID 传输通道上进行音频传输
    #elif (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID)
        #define ADPCM_PACKET_LEN                  57
        #define MIC_SHORT_DEC_SIZE                120
        #define TL_MIC_BUFFER_SIZE                480
    #else

    #endif
//----------------------------Dongle------------------------------------
#elif (TL_AUDIO_MODE & DONGLE_PROJECT)                    

    //使用ADPCM编码，通过GATT服务发送音频数据到Telink通道
    #if (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_TELINK)
        #define MIC_ADPCM_FRAME_SIZE              128
        #define MIC_SHORT_DEC_SIZE                248

    //使用ADPCM编码，通过GATT服务发送音频数据到Google通道
    #elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)
        #define MIC_ADPCM_FRAME_SIZE              136         //128+6+2
        #define MIC_ADPCM_FRAME_SIZE              256

    //使用ADPCM编码，通过HID服务将音频数据从Dongle发送到STB设备
    #elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID_DONGLE_TO_STB)
        #define MIC_ADPCM_FRAME_SIZE              120
        #define MIC_SHORT_DEC_SIZE                240

    //使用ADPCM编码，通过HID服务发送音频数据   
    #elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID)
        #define MIC_ADPCM_FRAME_SIZE              120
        #define MIC_SHORT_DEC_SIZE                240
    
    //使用SBC编码，通过HID服务将音频数据从Dongle发送到STB设备
    #elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB)
        #define MIC_ADPCM_FRAME_SIZE              20
        #define MIC_SHORT_DEC_SIZE                80

    //使用SBC编码，通过HID服务发送音频数据
    #elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID)
        #define MIC_ADPCM_FRAME_SIZE              20
        #define MIC_SHORT_DEC_SIZE                80

    //使用MSBC编码，通过HID服务发送音频数据
    #elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_MSBC_HID)
        #define MIC_ADPCM_FRAME_SIZE              57
        #define MIC_SHORT_DEC_SIZE                120
    #else

    #endif
    //由模式是否为 TL_AUDIO_DONGLE_SBC_HID , TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB , TL_AUDIO_DONGLE_MSBC_HID，这三个来改变码值
    #if ((TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB) || (TL_AUDIO_MODE == TL_AUDIO_DONGLE_MSBC_HID))
        #define MIC_OPEN_FROM_RCU                 0x31999999    //从遥控单元 (RCU) 打开麦克风
        #define MIC_OPEN_TO_STB                   0x32999999    //将麦克风打开并连接到机顶盒 (STB) 
        #define MIC_CLOSE_FROM_RCU                0x34999999    //从遥控单元 (RCU) 关闭麦克风
        #define MIC_CLOSE_TO_STB                  0x35999999    //将麦克风关闭并断开与机顶盒 (STB) 的连接
    #else
        #define MIC_OPEN_FROM_RCU                 0x21999999    //从遥控单元 (RCU) 打开麦克风
        #define MIC_OPEN_TO_STB                   0x22999999    //将麦克风打开并连接到机顶盒 (STB) 
        #define MIC_CLOSE_FROM_RCU                0x24999999    //从遥控单元 (RCU) 关闭麦克风
        #define MIC_CLOSE_TO_STB                  0x25999999    //将麦克风关闭并断开与机顶盒 (STB) 的连接
    #endif
#else

#endif
