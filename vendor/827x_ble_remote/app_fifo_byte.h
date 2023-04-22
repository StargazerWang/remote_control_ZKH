#ifndef APP_FIFO_BYTE_H__
#define APP_FIFO_BYTE_H__

#include "../../common/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

//#define uint8_t u8
//#define uint16_t u16
//#define uint32_t u32

    typedef struct
    {
        uint8_t *p_buf;              /**< Pointer to FIFO buffer memory.                      */
        uint16_t buf_size_mask;      /**< Read/write index mask. Also used for size checking. */
        volatile uint32_t read_pos;  /**< Next read position in the FIFO buffer.              */
        volatile uint32_t write_pos; /**< Next write position in the FIFO buffer.             */
    } app_fifo_byte_t;

    uint32_t app_fifo_byte_init(app_fifo_byte_t *p_fifo, uint8_t *p_buf, uint16_t buf_size);

    uint32_t app_fifo_byte_put(app_fifo_byte_t *p_fifo, uint8_t byte);

    uint32_t app_fifo_byte_get(app_fifo_byte_t *p_fifo, uint8_t *p_byte);

    uint32_t app_fifo_byte_peek(app_fifo_byte_t *p_fifo, uint16_t index, uint8_t *p_byte);

    uint32_t app_fifo_byte_flush(app_fifo_byte_t *p_fifo);

    uint32_t app_fifo_byte_replace_end_data(app_fifo_byte_t *p_fifo, uint8_t byte);

    uint32_t app_fifo_byte_read(app_fifo_byte_t *p_fifo, uint8_t *p_byte_array, uint32_t *p_size);

    uint32_t app_fifo_byte_write(app_fifo_byte_t *p_fifo, uint8_t const *p_byte_array, uint32_t *p_size);

#ifdef __cplusplus
}
#endif

#endif // APP_FIFO_H__

/** @} */
