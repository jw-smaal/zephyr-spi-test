/*
 * Copyright 2026 Jan-Willem Smaal <usenet@gispen.org>
 * @author Jan-Willem Smaal <usenet@gispen.org>
 * File: main.c
 * Created: May 5, 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>

/* Use Mode 3: CPOL=1, CPHA=1 */
#define SPI_OP (SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA)

static const struct spi_dt_spec slave0 = SPI_DT_SPEC_GET(DT_NODELABEL(spi_slave_0), SPI_OP);
static const struct spi_dt_spec slave1 = SPI_DT_SPEC_GET(DT_NODELABEL(spi_slave_1), SPI_OP);

int main(void)
{
	uint8_t tx_data0[] = {0xDE, 0xAD, 0xBE, 0xEF};
	uint8_t tx_data1[] = {0xCA, 0xFE, 0xCA, 0xFE};
	uint8_t rx_data[4];
	
	struct spi_buf rx_buf = {.buf = rx_data, .len = sizeof(rx_data)};
	struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

	printk("\n*** SPI Master (MCXW71) Starting - Unique Dual Payloads ***\n");
	k_sleep(K_MSEC(5000)); /* Give Slaves time to start */

	if (!spi_is_ready_dt(&slave0) || !spi_is_ready_dt(&slave1)) {
		printk("Error: SPI devices not ready\n");
		return 0;
	}

	while (1) {
		/* Talk to Slave 0 (DEADBEEF) */
		struct spi_buf tx_buf0 = {.buf = tx_data0, .len = sizeof(tx_data0)};
		struct spi_buf_set tx_bufs0 = {.buffers = &tx_buf0, .count = 1};
		
		memset(rx_data, 0, sizeof(rx_data));
		printk("Master -> Slave 0 (A0): Sending DEADBEEF | ");
		if (spi_transceive_dt(&slave0, &tx_bufs0, &rx_bufs) == 0) {
			printk("RX: %02x %02x %02x %02x\n",
			       rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
		} else {
			printk("FAILED\n");
		}

		k_sleep(K_MSEC(500));

		/* Talk to Slave 1 (CAFECAFE) */
		struct spi_buf tx_buf1 = {.buf = tx_data1, .len = sizeof(tx_data1)};
		struct spi_buf_set tx_bufs1 = {.buffers = &tx_buf1, .count = 1};
		
		memset(rx_data, 0, sizeof(rx_data));
		printk("Master -> Slave 1 (A1): Sending CAFECAFE | ");
		if (spi_transceive_dt(&slave1, &tx_bufs1, &rx_bufs) == 0) {
			printk("RX: %02x %02x %02x %02x\n",
			       rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
		} else {
			printk("FAILED\n");
		}

		k_sleep(K_MSEC(1000));
	}

	return 0;
}
