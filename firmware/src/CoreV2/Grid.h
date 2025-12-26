#pragma once
#include "OutputV2/HardwareLEDStrip.h"
#include <vector>

namespace CoreV2 {

struct Range {
  uint16_t start;
  uint16_t end;

  uint16_t offset(uint16_t other) const {
    return other - start;
  }

  uint16_t count() const {
    return end - start;
  }
};


struct LEDStripMapping {
  OutputV2::HardwareLEDStrip* strip;
  Range range_x;
  uint16_t stride_x = 1;
  Range range_y;
  uint16_t stride_y = 1;
};

struct UniverseUpdate {
 uint8_t id;
 uint8_t* src_buffer;
 Range x;
 Range y;
};

class Grid {
 public:
  struct Config {
    uint16_t x;
    uint16_t y;
  };

  Grid(Config config);
  void addMapping(const LEDStripMapping& mapping) {
    // Calculate the total number of physical LEDs per grid cell
    uint16_t leds_per_cell_x = mapping.stride_x;
    uint16_t leds_per_cell_y = mapping.stride_y;
    
    for (uint16_t i = mapping.range_x.start; i < mapping.range_x.end; i++) {
      for (uint16_t j = mapping.range_y.start; j < mapping.range_y.end; j++) {
        m_grid_updaters[i][j] = [=](uint8_t r, uint8_t g, uint8_t b) {
          uint16_t x_rel = mapping.range_x.offset(i);
          uint16_t y_rel = mapping.range_y.offset(j);
          
          // Set color for all physical LEDs in this grid cell based on stride
          for (uint16_t x_stride = 0; x_stride < leds_per_cell_x; x_stride++) {
            for (uint16_t y_stride = 0; y_stride < leds_per_cell_y; y_stride++) {
              // Calculate the physical LED index on the strip
              // Assumes row-major ordering with stride
              uint16_t idx = (x_rel * leds_per_cell_x + x_stride) * 
                             (mapping.range_y.count() * leds_per_cell_y) + 
                             (y_rel * leds_per_cell_y + y_stride);
              
              mapping.strip->setColor(idx, r, g, b);
            }
          }
        };
      }
    }
  }

  void update(const UniverseUpdate& update) {
    uint16_t current_offset = 0;
    for (uint16_t i = update.x.start; i < update.x.end; i++) {
      for (uint16_t j = update.y.start; j < update.y.end; j++) {
        m_grid_updaters[i][j](
          update.src_buffer[current_offset], 
          update.src_buffer[current_offset + 1], 
          update.src_buffer[current_offset + 2]
        );
        current_offset += 3;
      }
    }
  }


 
 private:
   std::vector<std::vector<std::function<void(uint8_t, uint8_t, uint8_t)>>> m_grid_updaters;
};

}