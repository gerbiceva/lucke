// #pragma once
// #include "InputSource.h"
// #include "Utils/SacnParser.h"

// namespace Input
// {
//     class Sacn : public SourceIface
//     {
//         Utils::SacnParser m_sacn;
//         uint8_t m_universe;
//     public:
//         Sacn() = default;
//         // ~Sacn();

//         void begin(uint8_t universe) override;
//         void stop() override;
//         void poll(uint8_t* buff) override;

//         // void toJson(JsonObject& doc) override;
//     };
// }