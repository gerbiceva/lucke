#pragma once
#include <memory>
#include <span>
#include <type_traits>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Core
{
    struct RGB
    {
        union
        {
            struct
            {
                uint8_t r, g, b;
            };
            uint8_t raw[3];
        };
    };

    // Drives a WS2815 strip over SPI. The pixel buffer is borrowed — the
    // caller (LEDBase) owns it and must keep it alive for this object's
    // lifetime. Everything else (SPI bus/device, DMA buffer, mutex) is owned
    // here and released in the destructor or when init() fails.
    class WS2815
    {
        static const uint16_t timing_bits[16];
        static const uint8_t reset_delay;

        struct HeapCapsFree
        {
            void operator()(void *p) const { heap_caps_free(p); }
        };

        struct SemaphoreDelete
        {
            void operator()(SemaphoreHandle_t s) const { vSemaphoreDelete(s); }
        };

        static constexpr spi_host_device_t c_host = SPI2_HOST;

        std::unique_ptr<std::remove_pointer_t<SemaphoreHandle_t>, SemaphoreDelete> spiMutex;
        std::unique_ptr<uint16_t[], HeapCapsFree> dma_buffer;
        size_t dma_buf_size = 0;

        spi_device_handle_t spi = nullptr;
        bool busInitialized = false;

        std::span<RGB> pixels;

        esp_err_t initSPI(int pin);
        esp_err_t update();
        void release();

    public:
        WS2815() = default;
        ~WS2815();

        WS2815(const WS2815 &) = delete;
        WS2815 &operator=(const WS2815 &) = delete;

        // buffer: num_leds * 3 bytes of RGB, owned by the caller.
        void init(int pin, std::span<uint8_t> buffer);

        std::span<RGB> getPixels();
        RGB &operator[](int index);
        uint8_t *getRaw() const;

        void set(RGB color, int i);
        void fill_all(RGB color);

        void show();
    };

}
