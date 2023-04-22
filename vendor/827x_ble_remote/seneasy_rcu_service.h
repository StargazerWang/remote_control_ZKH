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

#ifndef __SMART_RCU_SERVICE_H
#define __SMART_RCU_SERVICE_H

#include "tl_common.h"
#include "application/audio/audio_config.h"
#include "stack/ble/ble.h"
#include "app_audio.h"


int seneasy_rcu_svc_attr_write_cb(void *p);
int seneasy_rcu_svc_attr_read_cb(void *p);
int seneasy_rcu_ccc_update_cb(void *p);

#endif
