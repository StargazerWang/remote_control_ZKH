/******************************************************************************
 * @file     adpcm.c
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
#include    "tl_common.h"       //包含常用数据类型、宏定义和函数声明
#include    "drivers.h"         //包含与硬件驱动相关的函数声明
#include    "audio_config.h"    //包含音频配置参数
#include    "adpcm.h"           //包含 ADPCM 压缩算法的函数声明
#include    "gl_audio.h"        //包含音频播放相关的函数声明


#if     (TL_AUDIO_MODE & TL_AUDIO_MASK_ADPCM_MODE)                 //Adpcm mode//对应audio_common.h的34行


static const signed char idxtbl[] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};//索引表（index table）
static const unsigned short steptbl[] = {                       //步长表（step table）
     7,  8,  9,  10,  11,  12,  13,  14,  16,  17,
     19,  21,  23,  25,  28,  31,  34,  37,  41,  45,
     50,  55,  60,  66,  73,  80,  88,  97,  107, 118,
     130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
     337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
     876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
     2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
     5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
     15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767   };


#define     NUM_OF_ORIG_SAMPLE                2         //原始样本数量

#if  (TL_AUDIO_MODE & RCU_PROJECT)                            //RCU

#if  (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)        //RCU,GATT TELINK
/////////////////////////////////////////////////////////////////////////////////
//    256-samples split into 2
/////////////////////////////////////////////////////////////////////////////////

/// @brief 将麦克风采集的音频转换为ADPCM格式并进行拆分,该函数将输入的采样数据进行 ADPCM 压缩，生成一个 ADPCM 数据包并存储到指定的缓冲区中
/// @param ps       输入的采样数据缓冲区
/// @param len      采样数据缓冲区的长度
/// @param pds      ADPCM 数据缓冲区
/// @param start    标志是否开始新的数据块
void mic_to_adpcm_split (signed short *ps, int len, signed short *pds, int start)
{
    int i, j;
    unsigned short code = 0;        //无符号的 16 位整型变量，用来存储 ADPCM 编码后的音频数据
    unsigned short code16 = 0;      //无符号的 16 位整型变量，用来存储 ADPCM 编码后的音频数据
    static int predict_idx = 0;     //一个静态的整型变量，用来存储预测器中的索引值
    code = 0;
    static signed short *pd;        //一个静态的有符号短整型指针，指向音频数据的位置
    static int predict;             //一个有符号的整型变量，用来存储当前采样点的预测值
    //上面代码中的变量和指针是用来实现将 PCM 格式的音频数据转换为 ADPCM 格式编码的功能。其中，ADPCM 格式编码是通过将采样点的差值压缩成相应的编码格式来实现的。而压缩算法的实现需要使用到预测器和索引值，因此这段代码中也定义了相关的变量来实现预测和索引的功能。
    
    //byte2,byte1: predict;  byte3: predict_idx; byte4:adpcm data len
    if (start)      //如果start为真就开始
    {
        pd = pds;           //pd设为pds(函数输入值) ,pds 是一个指向缓存区的指针变量
        *pd++ = predict;    //将当前的预测值 predict 存储到 pds(pd) 指向的地址中，并且将指针 pd 向前移动一位以便于存储下一个采样点的值
        * (((signed char *)pds) + 2)= predict_idx;              //将 predict_idx 存储在 pds 的第三个字节位置上。predict_idx 存储的是 ADPCM 编码器中使用的索引值，用于计算当前采样点的预测值
        * (((unsigned char *)pds) + 3)= (ADPCM_PACKET_LEN - 4); //在 pds 的第四个字节位置上存储了一个常量值 ADPCM_PACKET_LEN - 4，表示 ADPCM 数据包的长度减去头部占用的 4 个字节。这个常量值是用来在解码时确定数据包的长度。然后将指针 pd 向前移动一位以便于存储下一个采样点的值
        pd++;               //将指针 pd 向前移动一位以便于存储下一个采样点的值
    }
    //上述代码的功能是在程序开始时将预测值、索引值和数据包长度信息存储到缓存区中，对应byte2,byte1: predict;  byte3: predict_idx; byte4:adpcm data len
       
    //byte5- byte128: 124 byte(62 sample) adpcm data
    for (i=0; i<len; i++) {

        s16 di = ps[i];     //将输入数组“ps”中的当前样本赋值给变量“di”。s16是一个有符号的16位整数类型，它表示范围为-32768到32767的整数。这里的“di”将作为编码算法中计算差异的基础
        int step = steptbl[predict_idx];    //从预测器索引表“steptbl”中获取步长大小，并将其分配给变量“step”。步长大小是ADPCM算法中用于计算差异的一个参数
        int diff = di - predict;            //计算当前样本“di”与预测样本“predict”之间的差异，并将结果分配给变量“diff”。这个差异将被用于后续的编码过程中
        
        //这部分代码是根据差异的正负号来设置编码的初始值。如果差异为非负数，则将编码“code”设置为0，否则将其设置为8
        if (diff >=0 ) { //如果差异>=0，则表示当前样本的值大于等于预测值，因此可以使用较少的比特数进行编码。在这种情况下，用0来表示编码的差异
            code = 0;
        }
        else {          //如果差异为负数，则表示当前样本的值小于预测值，因此需要使用更多的比特数才能准确地编码差异。在这种情况下，用8来表示编码的差异，并将差异的值取反以便后面的编码过程
            diff = -diff;
            code = 8;
        }
     
        int diffq = step >> 3;//将变量“step”右移三位（即除以8），并将结果赋值给变量“diffq”。这里的目的是计算步长的1/8，用于后续的编码过程.在ADPCM算法中，步长大小根据先前样本的解码值动态调整。对于当前样本，需要将步长大小分为8个部分之一，并将其与差异进行比较以确定编码方式。因此，将步长大小除以8可以有效地减小编码过程中的计算复杂度。

        for (j=4; j>0; j=j>>1) {        //循环中的变量“j”初始化为4，每次迭代都将其右移一位（即除以2），直到j等于0为止。这是因为ADPCM算法使用4位半字节对样本进行编码
            if( diff >= step) {         //在每次迭代中，将差异“diff”与当前的步长大小“step”进行比较。
                diff = diff - step;     //如果差异大于等于步长，则将步长从差异中减去，
                diffq = diffq + step;   //并将步长的1/8累加到变量“diffq”上，以便记录编码后的差异大小
                code = code + j;        //通过将当前迭代的“j”添加到编码变量“code”上，可以构建可变长度编码。每个迭代将添加一个半字节的编码，最终形成包含4个半字节的完整编码字
            }
            step = step >> 1;           //变量“step”右移一位，即将当前步长大小除以2,在ADPCM算法中，步长大小根据先前样本的解码值动态调整。对于当前样本，需要将步长大小分为8个部分之一，并将其与差异进行比较以确定编码方式。因此，在编码过程中，需要不断地将步长大小除以2，以便得到合适的步长大小，从而使编码更加精确。每次将步长大小除以2后，可以使用新的步长大小来计算下一个半字节的编码
        }

        code16 = (code16 >> 4) | (code << 12);//这行代码将当前的半字节编码“code”左移12位，并将其存储在变量“code16”的高字节中。然后，将变量“code16”右移4位，以清除已经处理过的前一个半字节编码。最后，使用按位或（|）运算符将变量“code”与右移后的低字节组合起来，得到完整的16位编码字。这个编码字包含了前面处理过的两个半字节编码。这个操作是将两个8位的半字节组成一个16位的字，其中高8位由之前处理过的半字节的4位编码组成，低8位由当前处理的半字节的4位编码组成。这个操作是为了方便存储压缩后的数据，因为一个16位的字可以更有效地存储和传输数据
        
        //这部分代码是在每处理完4个样本后，将存储了四个半字节编码的一个16位编码字存储到输出数组“pd”中。
        if ( (i&3) == 3) {//检查当前处理的样本数量是否是4的倍数（i & 3 的结果就是 i % 4）。如果是，则表示已经处理了4个样本，需要将存储了四个半字节编码的编码字存储到输出数组中。       
            *pd++ = code16;//使用指针“pd++”来递增输出数组的地址，以便存储下一个编码字，同时返回之前存储的编码字。
        }

        //基于编码的差异值更新预测器“predict”的值。
        if(code >= 8) { //如果当前的半字节编码“code”大于等于8，则表示预测的样本值比实际样本小，需要从预测值“predict”中减去解码差异值“diffq”，以获得更准确的预测值
            predict = predict - diffq;
        }
        else {          //如果当前半字节编码“code”小于8，则表示预测的样本值比实际样本大，需要将解码差异值“diffq”加到预测值“predict”上，以获得更准确的预测值。
            predict = predict + diffq;
        }

        //这部分代码用于限制更新后的预测器“predict”的值在16位PCM值的范围内。16位PCM值的取值范围为-32768到32767
        if (predict > 32767) {      //如果更新后的预测器“predict”的值大于32767，则将其截断为最大值32767；
            predict = 32767;
        }
        else if (predict < -32768) {//如果更新后的预测器“predict”的值小于-32768，则将其截断为最小值-32768
            predict = -32768;
        }

        //用于根据编码的差异更新预测器索引值“predict_idx”
        //这样做的目的是确保预测器索引值始终在预定义的有效范围内，以避免超出索引表的范围而导致错误的解码结果。通过这些步骤，该算法可以自适应地调整预测器和索引，以便更好地压缩音频数据并实现更高质量的解码结果
        predict_idx = predict_idx + idxtbl[code];//预测器索引表“idxtbl”包含了每个半字节编码对应的索引调整值。在这里，根据当前的半字节编码“code”，从表中获取相应的索引调整值，并将其添加到预测器索引值“predict_idx”上
        if(predict_idx < 0) {       //如果更新后的预测器索引值“predict_idx”小于0，则将其截断为最小值0；
            predict_idx = 0;
        }
        else if(predict_idx > 88) {//如果更新后的预测器索引值“predict_idx”大于88，则将其截断为最大值88
            predict_idx = 88;
        }
    }
}

#elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)        //RCU,GATT GOOGLE

#define STREAM_ID    0x00

//static int serial_id = 0;

u16 adpcm_sequence_num = 0;
int adpcm_predict      = 0;
int adpcm_predict_idx  = 1;
//int adpcm_predict_idx  = 0;

_attribute_ram_code_ void mic_to_adpcm_split (signed short *ps, int len, signed short *pds, int start)
{
    int i, j;
    unsigned short code=0;
    unsigned short code16=0;
    static signed short *pd;
//    static int predict_idx = 1;
    code = 0;
//    static int predict;

    pd = pds;

    if (start == 10){    //google voice v1.0
        *pd++ = ((adpcm_sequence_num>>8)&0x00ff)|((adpcm_sequence_num<<8)&0xff00);
        *pd++ = (google_voice_codec_used&0x00ff)|((adpcm_predict&0xff00));
        *pd++ = ((adpcm_predict)&0x00ff)|((adpcm_predict_idx<<8)&0xff00);
    }else{                //google voice v0.4
        //printf("ver 0.4_len:%d\n",len);
        //Seq# 2bytes; Id: 1bytes; Prev.pred: 2bytes; index: 1bytes
        *pd++ = ((adpcm_sequence_num>>8)&0x00ff)|((adpcm_sequence_num<<8)&0xff00);
        *pd++ = (STREAM_ID)|((adpcm_predict&0xff00));
        *pd++ = ((adpcm_predict)&0x00ff)|((adpcm_predict_idx<<8)&0xff00);
    }
    adpcm_sequence_num ++;


    for(i=0 ; i<len; i++){//unit sample

        s16 di;
        if(google_voice_codec_used == CODEC_USED_16K){
            di = ps[i];
        }else{
            di = ps[i*2];
        }

        int step = steptbl[adpcm_predict_idx];
        int diff = di - adpcm_predict;

        if(diff >= 0){
            code = 0;
        }else{
            diff = -diff;
            code = 8;
        }

        int diffq = step >> 3;

        for(j=4; j>0; j=j>>1){
            if(diff >= step){
                diff = diff - step;
                diffq = diffq + step;
                code = code + j;
            }
            step = step >> 1;
        }

        code16 = (code16 >> 4) | (code << 12);
        if((i&3) == 3){
            code16 = ((code16&0x0f)<<4)|((code16&0xf0)>>4) | ((code16&0x0f00)<<4)|((code16&0xf000)>>4);
            *pd++ = code16;
        }

        if(code >= 8) {
            adpcm_predict = adpcm_predict - diffq;
        }else {
            adpcm_predict = adpcm_predict + diffq;
        }

        if(adpcm_predict > 32767){
            adpcm_predict = 32767;
            //printf("adpcm_predict = 32768\n");
        }else if (adpcm_predict < -32768) {
            adpcm_predict = -32768;
            //printf("adpcm_predict = -32768\n");
        }

//        adpcm_predict_idx = adpcm_predict_idx + idxtbl[code];

        adpcm_predict_idx = adpcm_predict_idx + idxtbl[code&15];

        if(adpcm_predict_idx < 0) {
            adpcm_predict_idx = 0;
            //printf("adpcm_predict_idx = 0\n");
        }else if(adpcm_predict_idx > 88){
            //printf("adpcm_predict_idx = 88\n");
            adpcm_predict_idx = 88;
        }
    }
}

#elif (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL)        //RCU,HID SERVICE

int predict = 0;
int predict_idx = 0;
/////////////////////////////////////////////////////////////////////////////////
//    256-samples split into 2
/////////////////////////////////////////////////////////////////////////////////
_attribute_ram_code_ void mic_to_adpcm_split (signed short *ps, int len, signed short *pds, int start)
{
    int i, j;
    unsigned short code=0;
    unsigned short code16=0;
    static signed short *pd;
    pd = pds;
    code = 0;

    for (i=0; i<len; i++) {

        s16 di = ps[i];
        int step = steptbl[predict_idx];
        int diff = di - predict;

        if (diff >=0 ) {
            code = 0;
        }
        else {
            diff = -diff;
            code = 8;
        }

        int diffq = step >> 3;

        for (j=4; j>0; j=j>>1) {
            if( diff >= step) {
                diff = diff - step;
                diffq = diffq + step;
                code = code + j;
            }
            step = step >> 1;
        }

        code16 = (code16 >> 4) | (code << 12);
        if ( (i&3) == 3) {
            code16 = ((code16&0x0f)<<4)|((code16&0xf0)>>4) | ((code16&0x0f00)<<4)|((code16&0xf000)>>4);
            *pd++ = code16;
        }

        if(code >= 8) {
            predict = predict - diffq;
        }
        else {
            predict = predict + diffq;
        }

        if (predict > 32767) {
            predict = 32767;
        }
        else if (predict < -32768) {
            predict = -32768;
        }

        predict_idx = predict_idx + idxtbl[code & 15];
        if(predict_idx < 0) {
            predict_idx = 0;
        }
        else if(predict_idx > 88) {
            predict_idx = 88;
        }
    }
}
#else

#endif//end RCU_PROJECT pcm to adpcm

#elif (TL_AUDIO_MODE & DONGLE_PROJECT)                            //Dongle

#if  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_TELINK)        //Dongle,GATT TELINK

void adpcm_to_pcm (signed short *ps, signed short *pd, int len)
{
    int i;

    //byte2,byte1: predict;  byte3: predict_idx; byte4:adpcm data len
    int predict = ps[0];
    int predict_idx = ps[1] & 0xff;
//    int adpcm_len = (ps[1]>>8) & 0xff;

    unsigned char *pcode = (unsigned char *) (ps + NUM_OF_ORIG_SAMPLE);

    unsigned char code;
    code = *pcode ++;

    //byte5- byte128: 124 byte(62 sample) adpcm data
    for (i=0; i<len; i++) {

        if (1) {
            int step = steptbl[predict_idx];

            int diffq = step >> 3;

            if (code & 4) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 2) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 1) {
                diffq = diffq + step;
            }

            if (code & 8) {
                predict = predict - diffq;
            }
            else {
                predict = predict + diffq;
            }

            if (predict > 32767) {
                predict = 32767;
            }
            else if (predict < -32768) {
                predict = -32768;
            }

            predict_idx = predict_idx + idxtbl[code & 15];

            if(predict_idx < 0) {
                predict_idx = 0;
            }
            else if(predict_idx > 88) {
                predict_idx = 88;
            }

            if (i&1) {
                code = *pcode ++;
            }
            else {
                code = code >> 4;
            }
        }

        if (0 && i < NUM_OF_ORIG_SAMPLE) {
            *pd++ = ps[i];
        }
        else {
            *pd++ = predict;
        }
    }
}


#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)        //Dongle,GATT GOOGLE

#define ADPCM_ANDROID_ID    0x00

_attribute_ram_code_ void adpcm_to_pcm (signed short *ps, signed short *pd, int len)
{
    int i;

    int predict = (s16)((ps[1]&0xff00) | (ps[2]&0x00ff));
    int predict_idx = (s8)((ps[2]&0xff00)>>8);
    unsigned char *pcode = (unsigned char *) (ps + 3);
    unsigned char code;
    code = *pcode ++;
    code = ((code>>4)&0x0f)|((code<<4)&0xf0);

    //google: byte7-byte134: 128 byte(62 sample) adpcm data
    //t4h: byte5-byte100: 96 byte(48 sample) adpcm data
    for(i=0; i<len; i++){
        int step = steptbl[predict_idx];
        int diffq = step >> 3;

        if(code & 4){
            diffq = diffq + step;
        }
        step = step >> 1;
        if(code & 2){
            diffq = diffq + step;
        }
        step = step >> 1;
        if(code & 1){
            diffq = diffq + step;
        }

        if(code & 8){
            predict = predict - diffq;
        }else{
            predict = predict + diffq;
        }

        if(predict > 32767){
            predict = 32767;
        }else if(predict < -32768){
            predict = -32768;
        }

        predict_idx = predict_idx + idxtbl[code & 15];

        if(predict_idx < 0){
            predict_idx = 0;
        }else if(predict_idx > 88){
            predict_idx = 88;
        }

        if(i&1){
            code = *pcode ++;
            code = ((code>>4)&0x0f)|((code<<4)&0xf0);  //add by qiuwei for android 8
        }else{
            code = code >> 4;
        }

        if (0 && i < NUM_OF_ORIG_SAMPLE) {
            *pd++ = ps[i];
        }
        else {
            *pd++ = predict;
        }
    }
}

#elif (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL)        //Dongle,HID SERVICE

int predict  = 0;
int predict_idx = 0;

void adpcm_to_pcm (signed short *ps, signed short *pd, int len)
{
    int i;
    unsigned char *pcode = (unsigned char *) ps;
    unsigned char code;
    code = *pcode ++;
    code = ((code>>4)&0x0f)|((code<<4) &0xf0);

    for (i=0; i<len; i++) {

        if (1) {
            int step = steptbl[predict_idx];

            int diffq = step >> 3;

            if (code & 4) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 2) {
                diffq = diffq + step;
            }
            step = step >> 1;
            if (code & 1) {
                diffq = diffq + step;
            }

            if (code & 8) {
                predict = predict - diffq;
            }
            else {
                predict = predict + diffq;
            }

            if (predict > 32767) {
                predict = 32767;
            }
            else if (predict < -32768) {
                predict = -32768;
            }

            predict_idx = predict_idx + idxtbl[code & 15];

            if(predict_idx < 0) {
                predict_idx = 0;
            }
            else if(predict_idx > 88) {
                predict_idx = 88;
            }

            if (i&1) {
                code = *pcode ++;
                code = ((code>>4)&0x0f)|((code<<4) &0xf0);
            }
            else {
                code = code >> 4;
            }
        }

        if (0 && i < NUM_OF_ORIG_SAMPLE) {
            *pd++ = ps[i];
        }
        else {
            *pd++ = predict;
        }
    }
}
#else

#endif//end DONGLE_PROJECT adpcm to pcm

#endif//end RCU_PROJECT OR DONGLE_PROJECT

#endif
