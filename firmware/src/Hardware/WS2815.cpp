#include "WS2815.h"

#include <Arduino.h>
#include <string.h>
#include <algorithm>

namespace Core
{

    const uint16_t WS2815::timing_bits[16] =
        {
            0x1111, 0x7111, 0x1711, 0x7711,
            0x1171, 0x7171, 0x1771, 0x7771,
            0x1117, 0x7117, 0x1717, 0x7717,
            0x1177, 0x7177, 0x1777, 0x7777};

    const uint8_t WS2815::reset_delay = 64;

    WS2815::~WS2815()
    {
        release();
    }

    void WS2815::init(int pin, std::span<uint8_t> buffer)
    {
        release();
        pixels = {reinterpret_cast<RGB *>(buffer.data()), buffer.size() / sizeof(RGB)};

        esp_err_t err = initSPI(pin);
        if (err != ESP_OK)
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(err);
            release();
        }
    }

    esp_err_t WS2815::initSPI(int pin)
    {
        // Serializes show() across tasks (fixture_update / wifi_animation both
        // drive the same SPI device); concurrent spi_device_transmit() trips
        // the "ret_trans == trans_desc" assert in spi_master.c.
        spiMutex.reset(xSemaphoreCreateMutex());
        if (!spiMutex)
        {
            return ESP_ERR_NO_MEM;
        }

        spi_device_interface_config_t devcfg{
            .command_bits = 0,
            .address_bits = 0,
            .mode = 0,                 // SPI mode 0
            .clock_speed_hz = 3200000, // Clock out at 3.2 MHz
            .spics_io_num = -1,        // CS pin
            .flags = SPI_DEVICE_TXBIT_LSBFIRST,
            .queue_size = 1,
        };

        spi_bus_config_t buscfg{
            .miso_io_num = -1,
            .sclk_io_num = -1,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
        };

        dma_buf_size = (pixels.size() * 6 + reset_delay * 2) * sizeof(uint16_t);

        buscfg.mosi_io_num = pin;
        buscfg.max_transfer_sz = dma_buf_size;

        // The bus (and its MOSI pin) belongs to SPI2_HOST, so only one WS2815
        // can own it. A second instance fails here with ESP_ERR_INVALID_STATE.
        esp_err_t err = spi_bus_initialize(c_host, &buscfg, SPI_DMA_CH_AUTO);
        if (err != ESP_OK)
        {
            return err;
        }
        busInitialized = true;

        err = spi_bus_add_device(c_host, &devcfg, &spi);
        if (err != ESP_OK)
        {
            spi = nullptr;
            return err;
        }

        // Critical to be DMA memory.
        dma_buffer.reset(static_cast<uint16_t *>(heap_caps_malloc(dma_buf_size, MALLOC_CAP_DMA)));
        if (!dma_buffer)
        {
            return ESP_ERR_NO_MEM;
        }
        return ESP_OK;
    }

    // Releases everything this instance acquired, in reverse order. Safe to
    // call on a partially initialized or already released object.
    void WS2815::release()
    {
        dma_buffer.reset();
        dma_buf_size = 0;

        if (spi)
        {
            spi_bus_remove_device(spi);
            spi = nullptr;
        }

        if (busInitialized)
        {
            spi_bus_free(c_host);
            busInitialized = false;
        }

        spiMutex.reset();
    }

    esp_err_t WS2815::update()
    {
        esp_err_t err;
        int n = 0;

        memset(dma_buffer.get(), 0, dma_buf_size);

        // ---- PRE RESET (CRITICAL) ----
        for (int i = 0; i < reset_delay; i++)
        {
            dma_buffer[n++] = 0;
        }

        // ---- LED DATA ----
        for (const RGB &px : pixels)
        {
            uint8_t r = px.r;
            uint8_t g = px.g;
            uint8_t b = px.b;

            // Red
            dma_buffer[n++] = timing_bits[(r >> 4) & 0x0F];
            dma_buffer[n++] = timing_bits[r & 0x0F];

            // Green
            dma_buffer[n++] = timing_bits[(g >> 4) & 0x0F];
            dma_buffer[n++] = timing_bits[g & 0x0F];

            // Blue
            dma_buffer[n++] = timing_bits[(b >> 4) & 0x0F];
            dma_buffer[n++] = timing_bits[b & 0x0F];
        }

        // ---- POST RESET ----
        for (int i = 0; i < reset_delay; i++)
        {
            dma_buffer[n++] = 0;
        }

        spi_transaction_t t{};
        t.length = dma_buf_size * 8;
        t.tx_buffer = dma_buffer.get();

        err = spi_device_transmit(spi, &t);

        return err;
    }

    std::span<RGB> WS2815::getPixels()
    {
        return pixels;
    }

    RGB &WS2815::operator[](int index)
    {
        return pixels[index % pixels.size()];
    }

    uint8_t *WS2815::getRaw() const
    {
        return reinterpret_cast<uint8_t *>(pixels.data());
    }

    void WS2815::set(RGB color, int i)
    {
        pixels[i % pixels.size()] = color;
    }

    void WS2815::fill_all(RGB color)
    {
        std::ranges::fill(pixels, color);
    }

    void WS2815::show()
    {
        // init() failed or was never called — nothing to drive.
        if (!dma_buffer)
        {
            return;
        }

        xSemaphoreTake(spiMutex.get(), portMAX_DELAY);
        ESP_ERROR_CHECK_WITHOUT_ABORT(update());
        xSemaphoreGive(spiMutex.get());
    }

}
