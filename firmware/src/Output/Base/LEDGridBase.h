// #pragma once
// #include <cstdint>
// #include <cstdio>
// #include <vector>
// #include "Base.h"
// #include "Utils/Logger.h"
// #include "Utils/JsonUtils.h"
//
// namespace Output
// {
//     struct GColor
//     {
//         uint8_t r, g, b;
//     };
//
//     struct GRange
//     {
//         uint16_t begin, end;
//         uint16_t size() const { return end - begin; };
//     };
//
//     // Backend-agnostic grid: holds the cell<->buffer mapping and all the
//     // source->cell logic. Subclasses only allocate a strip's buffer, bind it
//     // to hardware (FastLED / WS2815) and flush it in postupdate().
//     class LEDGridBase : public Base
//     {
//     protected:
//         struct View
//         {
//             GRange x, y;
//             uint16_t cellW, cellH;
//             uint16_t vW() const { return x.size() / cellW; }
//             uint16_t vH() const { return y.size() / cellH; }
//         };
//
//         uint16_t m_w, m_h;
//         const char *m_kind;
//         std::vector<uint8_t *> m_buffers; // one per strip
//         GColor **m_cells;                 // m_w*m_h pointers into m_buffers
//         std::vector<View> m_views;
//
//         uint16_t idx(uint16_t x, uint16_t y) const { return x * m_h + y; }
//
//         // Maps an already-bound strip buffer onto the grid cells. Called by the
//         // backend-specific addStrip() once the buffer exists.
//         void registerStrip(uint8_t *buf, GRange xr, GRange yr,
//                            uint16_t cellW, uint16_t cellH, bool horizontal)
//         {
//             GColor *px = reinterpret_cast<GColor *>(buf);
//             uint16_t local = 0;
//             if (horizontal)
//             {
//                 for (uint16_t y = yr.begin; y < yr.end; y++)
//                     for (uint16_t x = xr.begin; x < xr.end; x++)
//                         m_cells[idx(x, y)] = &px[local++];
//             }
//             else
//             {
//                 for (uint16_t x = xr.begin; x < xr.end; x++)
//                     for (uint16_t y = yr.begin; y < yr.end; y++)
//                         m_cells[idx(x, y)] = &px[local++];
//             }
//
//             m_buffers.push_back(buf);
//             m_views.push_back({xr, yr, cellW, cellH});
//         }
//
//     public:
//         LEDGridBase(uint16_t w, uint16_t h, const char *kind)
//             : Base(0), m_w(w), m_h(h), m_kind(kind)
//         {
//             m_cells = new GColor *[w * h]();
//         }
//
//         // source bytes consumed = sum over views of (virtual_w * virtual_h * 3)
//         uint16_t footprint() const override
//         {
//             uint16_t f = 0;
//             for (auto &v : m_views)
//                 f += v.vW() * v.vH() * 3;
//             return f;
//         }
//
//         // Footprint under the given subgrid preset (width/height = block counts),
//         // without mutating the live cell sizes. Mirrors setPreset()'s math.
//         uint16_t footprintFor(const JsonDocument &setting) const override
//         {
//             uint16_t vW = m_w, vH = m_h;
//             Utils::Json::updateElement<uint16_t>(setting, "width", vW);
//             Utils::Json::updateElement<uint16_t>(setting, "height", vH);
//
//             if (vW == 0 || vH == 0 || m_w % vW != 0 || m_h % vH != 0)
//                 return footprint();
//
//             uint16_t cellW = m_w / vW;
//             uint16_t cellH = m_h / vH;
//             uint16_t f = 0;
//             for (auto &v : m_views)
//                 f += (v.x.size() / cellW) * (v.y.size() / cellH) * 3;
//             return f;
//         }
//
//
//         void update(std::span<const uint8_t> src) override
//         {
//             size_t i = 0;
//             for (auto &v : m_views)
//                 for (uint16_t vy = 0; vy < v.vH(); vy++)
//                     for (uint16_t vx = 0; vx < v.vW(); vx++)
//                     {
//                         GColor c{src[i], src[i + 1], src[i + 2]};
//                         i += 3;
//                         uint16_t bx = v.x.begin + vx * v.cellW;
//                         uint16_t by = v.y.begin + vy * v.cellH;
//                         for (uint16_t dx = 0; dx < v.cellW; dx++)
//                             for (uint16_t dy = 0; dy < v.cellH; dy++)
//                                 if (auto *cell = m_cells[idx(bx + dx, by + dy)])
//                                     *cell = c;
//                     }
//         }
//
//         // Preset carries the virtual resolution of the whole grid (block count).
//         //   "width"/"height" = number of subgrid cells across / down
//         //   e.g. 8x24 grid:  1x3 -> 8x8 cells,  2x6 -> 4x4 cells,  8x24 -> 1x1 cells
//         void setPreset(const JsonDocument &doc) override
//         {
//             uint16_t vW = m_w, vH = m_h; // default: full resolution
//             Utils::Json::updateElement<uint16_t>(doc, "width", vW);
//             Utils::Json::updateElement<uint16_t>(doc, "height", vH);
//
//             if (vW == 0 || vH == 0 || m_w % vW != 0 || m_h % vH != 0)
//             {
//                 Utils::Logger::printf("[GRID] invalid subgrid %ux%u for %ux%u grid\n",
//                                       vW, vH, m_w, m_h);
//                 return;
//             }
//
//             uint16_t cellW = m_w / vW;
//             uint16_t cellH = m_h / vH;
//             for (auto &v : m_views)
//             {
//                 v.cellW = cellW;
//                 v.cellH = cellH;
//             }
//         }
//
//         const char *name() const override { return "Grid"; }
//
//         void describe(JsonObject &desc) override
//         {
//             Base::describe(desc);
//             desc["width"] = m_w;
//             desc["height"] = m_h;
//         }
//
//         void toJson(JsonObject &doc) override
//         {
//             char type[24];
//             snprintf(type, sizeof(type), "%s-Grid", m_kind);
//
//             doc["uuid"] = this->m_uuid;
//             doc["type"] = type;
//             doc["width"] = m_w;
//             doc["height"] = m_h;
//
//             JsonObject desc = doc["description"].to<JsonObject>();
//             describe(desc);
//         }
//     };
// }
