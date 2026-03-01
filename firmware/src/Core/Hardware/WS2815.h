#pragma once
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "CRGB.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


namespace Core
{

// template<uint16_t TLedNum>
class WS2815
{
    static const uint16_t timing_bits[16];
    static const uint8_t reset_delay;

    struct spi_settings_t {
        spi_host_device_t host;
        spi_device_handle_t spi;
        int dma_chan;
        spi_device_interface_config_t devcfg;
        spi_bus_config_t buscfg;
    };

    portMUX_TYPE myMux = portMUX_INITIALIZER_UNLOCKED;
    SemaphoreHandle_t spiMutex;

    spi_settings_t spi_settings;
    uint16_t *dma_buffer;
    CRGB *pixels;
    int n_leds, dma_buf_size;
    
    esp_err_t initSPI(int pin);
    esp_err_t update();
    
public:
    WS2815() = default;
    WS2815(uint16_t num_of_leds, int pin);
    void init(uint16_t num_of_leds, int pin);

    CRGB* getPixels();
    CRGB& operator[](int index);
    uint8_t* getRaw() const;

    void set(CRGB color, int i);
    void fill_all(CRGB color);

    void show();
};

}