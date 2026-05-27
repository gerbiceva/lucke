#pragma once
#include <cstdint>

namespace Interface
{
    
class InputSource
{
public:
    virtual void begin(uint8_t universe) = 0;
    virtual void stop() = 0;
    virtual void poll(uint8_t* buff) {};
};
}
