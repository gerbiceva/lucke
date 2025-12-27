//
// Created by Primoz Lavric on 27. 12. 25.
//

#pragma once

#include <cstdint>
#include "OutputV2/LEDStrip.h"

namespace OutputV2 {

struct StrideRange {
  uint16_t offset;
  uint16_t size;

  uint8_t stride;

  uint16_t strideSize() const {
    return size * stride;
  }
};

struct LEDStripMapping {
  LEDStripMapping(LEDStrip *strip, const StrideRange &rangeX,
                  const StrideRange &rangeY)
      : strip(strip), rangeX(rangeX), rangeY(rangeY) {
   assert(strip->count() == rangeX.strideSize() * rangeY.strideSize());
  }

  void setColor(const uint16_t xIdx, const uint16_t yIdx, const Color color) {
    const uint16_t xRawOffset = (xIdx - rangeX.offset) * rangeX.stride;
    const uint16_t yRawOffset = (yIdx - rangeY.offset) * rangeY.stride;

    for (uint16_t i = 0; i < rangeX.stride; i++) {
      for (uint16_t j = 0; j < rangeY.stride; j++) {
        const uint16_t linIdx = (xRawOffset + i) * rangeY.strideSize() + yRawOffset + j;

        (*strip)[linIdx] = color;
      }
    }
  }

  bool contains(const uint16_t xIdx, const uint16_t yIdx) const {
    const bool xInside = xIdx >= rangeX.offset && (xIdx - rangeX.offset) < rangeX.size;
    const bool yInside = yIdx >= rangeY.offset && (yIdx - rangeY.offset) < rangeY.size;

    return xInside && yInside;
  }

  LEDStrip * strip;
  StrideRange rangeX;
  StrideRange rangeY;
};

class LEDGrid {
 public:
    LEDGrid(const uint16_t xSize, const uint16_t ySize) : m_xSize(xSize), m_ySize(ySize) {
    }

  void addLEDStrip(const LEDStripMapping& mapping) {
      assert(mapping.rangeX.offset + mapping.rangeX.size <= m_xSize);
      assert(mapping.rangeY.offset + mapping.rangeY.size <= m_ySize);

    m_mappings.emplace_back(mapping);
  }

  bool isComplete() const {
      for (uint16_t x = 0; x < m_xSize; x++) {
        for (uint16_t y = 0; y < m_ySize; y++) {
          bool covered = false;

          for (const auto& mapping : m_mappings) {
            if (mapping.contains(x, y)) {
              covered = true;
              break;
            }
          }

          if (!covered) {
            return false;
          }
        }
      }

      return true;
  }

  void setColor(const uint16_t xIdx, const uint16_t yIdx, const Color color) {
      for (auto& m_mapping : m_mappings) {
        if (!m_mapping.contains(xIdx, yIdx)) {
          continue;
        }

        m_mapping.setColor(xIdx, yIdx, color);
      }
  }

 private:
  uint16_t toGridIdx(const uint16_t x, const uint16_t y) const {
   return x * m_ySize + y;
  }

  uint16_t m_xSize;
  uint16_t m_ySize;

  std::vector<LEDStripMapping> m_mappings;
};

}