/*
 * Copyright 2026 Jan-Willem Smaal <usenet@gispen.org>
 * @author Jan-Willem Smaal <usenet@gispen.org>
 * File: main.c
 * Created: May 5, 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

/* Use Mode 3: CPOL=1, CPHA=1 */
#define SPI_OP (SPI_WORD_SET(8) | SPI_TRANSFER_MSB | \
		SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_OP_MODE_SLAVE)

//static const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
static const struct device *spi_dev = DEVICE_DT_GET(DT_ALIAS(spi));
static struct spi_config spi_cfg = {
	.operation = SPI_OP,
	.frequency = 0,
	.slave = 0,
};

/* LED Configuration */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
	uint8_t tx_data[] = {0xCA, 0xFE, 0xBA, 0xBE};
	uint8_t rx_data[4];
	struct spi_buf tx_buf = {.buf = tx_data, .len = sizeof(tx_data)};
	struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
	struct spi_buf rx_buf = {.buf = rx_data, .len = sizeof(rx_data)};
	struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};

	printk("\n*** SPI Slave (MCXW72) Starting with LED Feedback ***\n");

	if (!device_is_ready(spi_dev)) {
		printk("Error: SPI slave device not ready\n");
		return 0;
	}

	if (!gpio_is_ready_dt(&led)) {
		printk("Error: LED GPIO device not ready\n");
		return 0;
	}

	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	while (1) {
		memset(rx_data, 0, sizeof(rx_data));
		
		/* Wait for Master to send data */
		int err = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
		
		if (err >= 0) {
			/* Flash LED on success */
			gpio_pin_set_dt(&led, 1);
			k_sleep(K_MSEC(50)); 
			gpio_pin_set_dt(&led, 0);

			printk("RX: %02x %02x %02x %02x\n",
			       rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
		}
	}

	return 0;
}
