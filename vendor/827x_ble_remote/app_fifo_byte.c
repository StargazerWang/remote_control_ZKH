#include "app_fifo_byte.h"

/*
    使用注意试项：
        1.fifo size 必须是2的整次幂
        2.该fifo代码针对的是 对32位操作数的读写操作是原子操作，即写入或者读取 32 位操作数只需要一条指令，不能被中断分割
        3.如果不能确认32位操作数是否是原子操作，则在main 函数进程中调用fifo相关的代码必须开启临界区。

*/

#define NRF_SUCCESS 0
#define NRF_ERROR_NO_MEM 1
#define NRF_ERROR_NULL 2
#define NRF_ERROR_INVALID_LENGTH 3
#define NRF_ERROR_NOT_FOUND 4

#define MIN(a, b) (((a) > (b)) ? b : a)





static inline uint32_t fifo_length(app_fifo_byte_t *p_fifo)
{
    uint32_t tmp = p_fifo->read_pos;
    return p_fifo->write_pos - tmp;
}

#define FIFO_LENGTH() fifo_length(p_fifo) /**< Macro for calculating the FIFO length. */

/**@brief Put one byte to the FIFO. */
static inline void fifo_put(app_fifo_byte_t *p_fifo, uint8_t byte)
{
    p_fifo->p_buf[p_fifo->write_pos & p_fifo->buf_size_mask] = byte;
    p_fifo->write_pos++;
}

/**@brief Look at one byte in the FIFO. */
static inline void fifo_peek(app_fifo_byte_t *p_fifo, uint16_t index, uint8_t *p_byte)
{
    *p_byte = p_fifo->p_buf[(p_fifo->read_pos + index) & p_fifo->buf_size_mask];
}

/**@brief Get one byte from the FIFO. */
static inline void fifo_get(app_fifo_byte_t *p_fifo, uint8_t *p_byte)
{
    fifo_peek(p_fifo, 0, p_byte);
    p_fifo->read_pos++;
}

uint32_t app_fifo_byte_init(app_fifo_byte_t *p_fifo, uint8_t *p_buf, uint16_t buf_size)
{

    p_fifo->p_buf = p_buf;
    p_fifo->buf_size_mask = buf_size - 1;
    p_fifo->read_pos = 0;
    p_fifo->write_pos = 0;

    return NRF_SUCCESS;
}

//强行替换FIFO 中最后一个元素
uint32_t app_fifo_byte_replace_end_data(app_fifo_byte_t *p_fifo, uint8_t byte)
{
    p_fifo->p_buf[(p_fifo->write_pos - 1) & p_fifo->buf_size_mask] = byte;
    return NRF_SUCCESS;
}

uint32_t app_fifo_byte_put(app_fifo_byte_t *p_fifo, uint8_t byte)
{
    if (FIFO_LENGTH() <= p_fifo->buf_size_mask)
    {
        fifo_put(p_fifo, byte);
        return NRF_SUCCESS;
    }

    return NRF_ERROR_NO_MEM;
}

uint32_t app_fifo_byte_get(app_fifo_byte_t *p_fifo, uint8_t *p_byte)
{
    if (FIFO_LENGTH() != 0)
    {
        fifo_get(p_fifo, p_byte);
        return NRF_SUCCESS;
    }

    return NRF_ERROR_NOT_FOUND;
}

uint32_t app_fifo_byte_peek(app_fifo_byte_t *p_fifo, uint16_t index, uint8_t *p_byte)
{
    if (FIFO_LENGTH() > index)
    {
        fifo_peek(p_fifo, index, p_byte);
        return NRF_SUCCESS;
    }

    return NRF_ERROR_NOT_FOUND;
}

uint32_t app_fifo_byte_flush(app_fifo_byte_t *p_fifo)
{
    p_fifo->read_pos = p_fifo->write_pos;
    return NRF_SUCCESS;
}

uint32_t app_fifo_byte_read(app_fifo_byte_t *p_fifo, uint8_t *p_byte_array, uint32_t *p_size)
{

    const uint32_t byte_count = fifo_length(p_fifo);
    const uint32_t requested_len = (*p_size);
    uint32_t index = 0;
    uint32_t read_size = MIN(requested_len, byte_count);

    (*p_size) = byte_count;

    // Check if the FIFO is empty.
    if (byte_count == 0)
    {
        return NRF_ERROR_NOT_FOUND;
    }

    // Check if application has requested only the size.
    if (p_byte_array == NULL)
    {
        return NRF_SUCCESS;
    }

    // Fetch bytes from the FIFO.
    while (index < read_size)
    {
        fifo_get(p_fifo, &p_byte_array[index++]);
    }

    (*p_size) = read_size;

    return NRF_SUCCESS;
}

uint32_t app_fifo_byte_write(app_fifo_byte_t *p_fifo, uint8_t const *p_byte_array, uint32_t *p_size)
{

    const uint32_t available_count = p_fifo->buf_size_mask - fifo_length(p_fifo) + 1;
    const uint32_t requested_len = (*p_size);
    uint32_t index = 0;
    uint32_t write_size = MIN(requested_len, available_count);

    (*p_size) = available_count;

    // Check if the FIFO is FULL.
    if (available_count == 0)
    {
        return NRF_ERROR_NO_MEM;
    }

    // Check if application has requested only the size.
    if (p_byte_array == NULL)
    {
        return NRF_SUCCESS;
    }

    //Fetch bytes from the FIFO.
    while (index < write_size)
    {
        fifo_put(p_fifo, p_byte_array[index++]);
    }

    (*p_size) = write_size;

    return NRF_SUCCESS;
}
