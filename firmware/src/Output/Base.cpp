#include "Base.h"

namespace Output
{
    uint16_t Base::s_uuid = 0;

    Base::Base(uint16_t buffer_size)
        : m_size(buffer_size)
    {
        m_uuid = s_uuid++;
    }

    uint16_t Base::footprint() const
    {
        return m_size;
    }

    
} // namespace Output
