/******************************************************************************
 * @file     version.h
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

#ifndef VERSION_H_
#define VERSION_H_

#define xstr(s) str(s)
#define str(s) #s

#define FW_VERSION                  0xaabb0128//FW:vAA.bb  SW:v1.28 used in cstartup


#define FW_VERSION_STR      xstr(VERSION)
//FW:vAA.bb SW:v1.11, A – with Ambilight key AA – Major revision , bb – Minor revision – Odd (Official sw), Even (OTA test sw)
// X for O or H
// #ifndef FW_VERSION_STR 
// #define FW_VERSION_STR              "R52GES_T202FFv.01.03"  // 02 OTA, R52GAAH_T102FFv.00.01 or R52GAH_T102FFv.00.01
// #endif
#pragma message(FW_VERSION_STR)

#define DATE                        "20210910"      // printf out later




#endif /* VERSION_H_ */
