#include "WS2815.h"

#include <Arduino.h>
#include <string.h>

namespace Core
{

const uint16_t WS2815::timing_bits[16] =
{
    0x1111, 0x7111, 0x1711, 0x7711,
    0x1171, 0x7171, 0x1771, 0x7771,
    0x1117, 0x7117, 0x1717, 0x7717,
    0x1177, 0x7177, 0x1777, 0x7777
};

const uint8_t WS2815::reset_delay = 30;

WS2815::WS2815(uint16_t num_of_leds, int pin)
    : n_leds(num_of_leds)
{
    pixels = new CRGB[n_leds];
    ESP_ERROR_CHECK_WITHOUT_ABORT(initSPI(pin));
}

void WS2815::init(uint16_t num_of_leds, int pin)
{
    n_leds = num_of_leds;
    pixels = new CRGB[n_leds];
    ESP_ERROR_CHECK_WITHOUT_ABORT(initSPI(pin));
}

esp_err_t WS2815::initSPI(int pin)
{
    spi_settings = spi_settings_t{
        .host = SPI2_HOST,
        .dma_chan = SPI_DMA_CH_AUTO,
        .devcfg =
            {
                .command_bits = 0,
                .address_bits = 0,
                .mode = 0,                           // SPI mode 0
                .clock_speed_hz = 3200000,           // Clock out at 3.2 MHz
                .spics_io_num = -1,                  // CS pin
                .flags = SPI_DEVICE_TXBIT_LSBFIRST,
                .queue_size = 1,
            },
        .buscfg =
            {
                .miso_io_num = -1,
                .sclk_io_num = -1,
                .quadwp_io_num = -1,
                .quadhd_io_num = -1,
            },
    };

    dma_buf_size = (n_leds * 6 + reset_delay * 2) * sizeof(uint16_t);

    spi_settings.buscfg.mosi_io_num = pin;
    spi_settings.buscfg.max_transfer_sz = dma_buf_size;
    esp_err_t err = spi_bus_initialize(spi_settings.host, &spi_settings.buscfg,
                            spi_settings.dma_chan);
    if (err != ESP_OK) {
        delete[] pixels;
        return err;
    }
    err = spi_bus_add_device(spi_settings.host, &spi_settings.devcfg,
                            &spi_settings.spi);
    if (err != ESP_OK) {
        delete[] pixels;
        return err;
    }
    // Critical to be DMA memory.
    dma_buffer = (uint16_t*)(heap_caps_malloc(dma_buf_size, MALLOC_CAP_DMA));
    if (dma_buffer == NULL) {
        delete[] pixels;
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t WS2815::update()
{
    esp_err_t err;
    int n = 0;

    memset(dma_buffer, 0, dma_buf_size);

    // ---- PRE RESET (CRITICAL) ----
    for(int i = 0; i < reset_delay; i++)
    {
        dma_buffer[n++] = 0;
    }

    // ---- LED DATA ----
    for (int i = 0; i < n_leds; i++)
    {
        uint8_t r = pixels[i].r;
        uint8_t g = pixels[i].g;
        uint8_t b = pixels[i].b;

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
    t.tx_buffer = dma_buffer;
    
    err = spi_device_transmit(spi_settings.spi, &t);
    
    return err;
}

CRGB* WS2815::getPixels()
{
    return pixels;
}

CRGB& WS2815::operator[](int index)
{
    return pixels[index % n_leds];
}

uint8_t* WS2815::getRaw() const
{
    return reinterpret_cast<uint8_t *>(pixels);
}

void WS2815::set(CRGB color, int i)
{
    pixels[i % n_leds] = color;
}

void WS2815::fill_all(CRGB color) {
    for (int i = 0; i < n_leds; i++) 
    {
        pixels[i] = color;
    }
}

void WS2815::show()
{
    // taskENTER_CRITICAL(&myMux);
    // // noInterrupts();
    // ESP_ERROR_CHECK_WITHOUT_ABORT(update());
    // taskEXIT_CRITICAL(&myMux);
    // interrupts();
    // xSemaphoreTake(spiMutex, portMAX_DELAY);

    ESP_ERROR_CHECK_WITHOUT_ABORT(update());

    // xSemaphoreGive(spiMutex);
}

}
