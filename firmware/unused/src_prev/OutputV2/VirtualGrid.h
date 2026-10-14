//
// Created by Primoz Lavric on 28. 12. 25.
//

#pragma once

#include "OutputV2/LEDGrid.h"

namespace OutputV2 {



class VirtualGrid {
public:
  struct Config {
    Range xRange;
    uint16_t xStride;
    Range yRange;
    uint16_t yStride;
  };

 VirtualGrid(LEDGrid& grid, const Config &config) : hwGrid(grid), config(config) {

 }

 void setColor(uint16_t x, uint16_t y, Color color) {
     for (uint16_t i = 0; i < config.xStride; i++) {
       for (uint16_t j = 0; j < config.yStride; j++) {
         uint16_t hwX = config.xRange.begin + x * i;
         uint16_t hwY = config.yRange.begin + y * j;
         hwGrid.setColor(hwX, hwY, color);
       }
     }
 }

private:
 LEDGrid& hwGrid;
 Config config;
};

}