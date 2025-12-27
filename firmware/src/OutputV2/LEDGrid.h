//
// Created by Primoz Lavric on 27. 12. 25.
//

#pragma once

#include <cstdint>
#include "OutputV2/LEDStrip.h"

namespace OutputV2 {

struct Range {
  uint16_t begin;
  uint16_t end;

  bool contains(const uint16_t idx) const {
    return idx >= begin && idx < end;
  }

  uint16_t size() const {
    return end - begin;
  }
};

struct LEDStripHWMapping {
  LEDStripHWMapping(LEDStrip* strip, Range xRange, Range yRange) : strip(strip), xRange(xRange), yRange(yRange) {}

  bool contains(const uint16_t xIdx, const uint16_t yIdx) const {
    return xRange.contains(xIdx) && yRange.contains(yIdx);
  }

  void setColor(const uint16_t xIdx, const uint16_t yIdx,
                const Color color) const {
    uint16_t idx = (xIdx - xRange.begin) * yRange.size() + yIdx - yRange.begin;
    (*strip)[idx] = color;
  }

  LEDStrip* strip;
  Range xRange;
  Range yRange;
};

class LEDGrid {
 public:
  LEDGrid(const uint16_t xSize, const uint16_t ySize) : m_xSize(xSize), m_ySize(ySize) {
    m_linGrid = new Color*[xSize * ySize];
  }

  void addLEDStrip(LEDStrip& strip, const Range xRange, const Range yRange) {
    uint16_t idx = 0;
    for (uint16_t x = xRange.begin; x < xRange.end; x++) {
      for (uint16_t y = yRange.begin; y < yRange.end; y++) {
        idx++;
        assert(m_linGrid[idx] == nullptr);
        m_linGrid[idx] = &strip[idx];
      }
    }
  }

  bool isComplete() const {
      for (uint16_t x = 0; x < m_xSize; x++) {
        for (uint16_t y = 0; y < m_ySize; y++) {
          if (m_linGrid[toGridIdx(x, y)] == nullptr) {
            return false;
          }
        }
      }

      return true;
  }

  void setColor(const uint16_t x, const uint16_t y, const Color color) {
    uint16_t gridIdx = toGridIdx(x, y);
    *m_linGrid[gridIdx] = color;
  }

 private:
  uint16_t toGridIdx(const uint16_t x, const uint16_t y) const {
   return x * m_ySize + y;
  }

  uint16_t m_xSize;
  uint16_t m_ySize;

  Color** m_linGrid;
};

}