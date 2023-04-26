/******************************************************************************
 * @file     adpcm.h
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

#ifndef ADPCM_H_
#define ADPCM_H_


extern u16 adpcm_sequence_num;      //表示 ADPCM 编码的序列号，即数据包的编号。
extern int adpcm_predict;           //int adpcm_predict 和 int adpcm_predict_idx：表示 ADPCM 编码器的状态参数。
extern int adpcm_predict_idx;       //adpcm 的预测标识


void mic_to_adpcm_split (signed short *ps, int len, signed short *pds, int start);  //将从麦克风读取的音频数据进行 ADPCM 压缩，并将结果保存到指定的缓冲区中。
void adpcm_to_pcm       (signed short *ps, signed short *pd, int len);                    //将 ADPCM 压缩过的音频数据解压缩为 PCM 格式，并将结果保存到指定的缓冲区中。

#endif /* ADPCM_H_ */
