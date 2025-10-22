#include "Fixture2D.h"

Fixture2D::Fixture2D(std::string name, std::string type, uint8_t width, uint8_t height)
    : Fixture(name, type, width * height), 
    m_width(width), m_height(height),
    m_column_width(width), m_row_height(height)
{
    setPreset(selectedPreset);
}

void Fixture2D::update() 
{

    for(uint8_t y = 0; y < m_num_rows; y++) {
		for(uint8_t x = 0; x < m_num_columns; x++) {
			const auto& indexes = getGridIndexes(x,y);
			int dmxIndex = address + (x * k_numPxls) + (y * k_numPxls) * m_num_columns;
			// printf("dmxIndex = %d\n", dmxIndex);
			for(auto index : indexes) {
				for (uint16_t k = 0; k < k_numPxls; k++) {
					// LOGF("led[%d] = dmx [%d]\n", (index * lamp->numPxls + k), (dmxIndex + k));
					m_ledBuffer[index * k_numPxls + k] = m_dmxBuffer[dmxIndex + k]; 
				}
			}
		}
	}
}

void Fixture2D::setPreset(uint8_t newPreset)
{
    selectedPreset = newPreset;
    m_storage.putUChar("preset_index", selectedPreset);
    m_column_width = m_presets[newPreset % m_presets.size()].width;
    m_row_height = m_presets[newPreset % m_presets.size()].height;
    m_num_columns = (m_width / m_column_width);
    m_num_rows = (m_height / m_row_height);
    m_grid_hash.clear();
}

std::vector<uint32_t>& Fixture2D::getGridIndexes(uint8_t x, uint8_t y)
{
    uint32_t hnum = x * 7741 + y * 7757;
		
    if(m_grid_hash.find(hnum) != m_grid_hash.end()) 
    {
        return m_grid_hash[hnum];
    }

    for(uint32_t yi = y * m_row_height; yi < (y + 1) * m_row_height; yi++) 
    {
        for(uint32_t xi = x * m_column_width; xi < (x + 1) * m_column_width; xi++) 
        {
            m_grid_hash[hnum].push_back(xi + yi * m_width);
        }
    }

    return m_grid_hash[hnum];
}

JsonDocument Fixture2D::presetsToJson() const
{
    JsonDocument doc;
    doc["presets"] = JsonDocument();
	JsonArray jsonArray = doc["presets"].to<JsonArray>();
	for (auto& preset : m_presets) {
		jsonArray.add(preset.toJson());
	}
    
    return doc;
}
