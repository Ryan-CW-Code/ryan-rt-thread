/*
 * Copyright (c) 2006-2024 RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */

#include <rtthread.h>
#include "utest.h"
#include <rtdevice.h>
#include <stdlib.h>

#define DBG_LVL DBG_LOG

#define TC_UART_DEVICE_NAME "uart2"

#ifdef UTEST_SERIAL_TC

#define TC_UART_SEND_TIMES 100

static struct rt_serial_device *serial;

static rt_err_t uart_find(void)
{
    serial = (struct rt_serial_device *)rt_device_find(TC_UART_DEVICE_NAME);

    if (serial == RT_NULL)
    {
        LOG_E("find %s device failed!\n", TC_UART_DEVICE_NAME);
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t test_item(rt_uint8_t *ch, rt_uint32_t size)
{
    rt_device_write(&serial->parent, 0, ch, size);
    rt_thread_mdelay(size / 11);
    if (1 != rt_device_read(&serial->parent, 0, ch, 1))
    {
        return -RT_ERROR;
    }

    rt_device_control(&serial->parent, RT_SERIAL_CTRL_RX_FLUSH, RT_NULL);
    if (0 != rt_device_read(&serial->parent, 0, ch, 1))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_bool_t uart_api()
{
    rt_err_t result = RT_EOK;

    result = uart_find();
    if (result != RT_EOK)
    {
        return RT_FALSE;
    }

    /* Reinitialize */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate               = BAUD_RATE_115200;
    config.rx_bufsz                = BSP_UART2_RX_BUFSIZE;
    config.tx_bufsz                = BSP_UART2_TX_BUFSIZE;
    rt_device_control(&serial->parent, RT_DEVICE_CTRL_CONFIG, &config);

    result = rt_device_open(&serial->parent, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);
    if (result != RT_EOK)
    {
        LOG_E("Open uart device failed.");
        return RT_FALSE;
    }

    rt_uint8_t *ch;
    rt_uint32_t i;
    ch = (rt_uint8_t *)rt_malloc(sizeof(rt_uint8_t) * (BSP_UART2_TX_BUFSIZE * 5 + 1));

    for (i = 0; i < TC_UART_SEND_TIMES; i++)
    {
        if (RT_EOK != test_item(ch, BSP_UART2_RX_BUFSIZE + BSP_UART2_RX_BUFSIZE * (rand() % 5)))
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (RT_EOK != test_item(ch, rand() % (BSP_UART2_RX_BUFSIZE * 5)))
        {
            result = -RT_ERROR;
            goto __exit;
        }
    }

__exit:
    rt_free(ch);
    rt_device_close(&serial->parent);
    return result == RT_EOK ? RT_TRUE : RT_FALSE;
}

static void tc_uart_api(void)
{
    uassert_true(uart_api() == RT_TRUE);
}

static rt_err_t utest_tc_init(void)
{
    LOG_I("UART TEST: Please connect Tx and Rx directly for self testing.");
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    rt_device_t uart_dev = rt_device_find(TC_UART_DEVICE_NAME);
    while (rt_device_close(uart_dev) != -RT_ERROR);
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(tc_uart_api);
}

UTEST_TC_EXPORT(testcase, "testcases.drivers.uart_flush_rx", utest_tc_init, utest_tc_cleanup, 30);

#endif /* TC_UART_USING_TC */
