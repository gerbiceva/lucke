#pragma once
#include "Fixture.h"

class Fixture2D : public Fixture 
{
    uint8_t m_width;
    uint8_t m_height;
    uint8_t m_column_width;
    uint8_t m_row_height;
    uint8_t m_num_columns;
    uint8_t m_num_rows;

    std::vector<Preset2D> m_presets;

	std::unordered_map<uint32_t, std::vector<uint32_t>> m_grid_hash;

    const std::vector<uint32_t>& getGridIndexes(uint8_t x, uint8_t y) const;
public:
    Fixture2D(std::string name, std::string type, uint8_t width, uint8_t height);

    void update() override;
    void setPreset(uint8_t newPreset) override;
};