#pragma once
#include <map>
#include "sACN.h"

namespace Handler 
{
struct Sacn
{

	static std::map<uint8_t, Receiver*> sacnMap;
	static std::map<uint8_t, std::array<uint8_t, 512U>> bufferMap;
	static WifiUDP udp;

	static Receiver* at(uint8_t uni)
	{
		if(sacnMap.contains(uni))
		{
			return sacnMap[uni];
		}

		Receiver* ret = new Receiver(udp);
		// ret->callbackDMX([](){ Controller::get().newPacket(); });
		// ret->callbackSource([](){ Controller::get().printNewret(); });
		// ret->callbackFramerate([](){ Controller::get().updateFramerate(); });
		// ret->callbackSeqDiff([](){ Controller::get().seqDiff(); });
		// ret->callbackTimeout([](){});
		ret.begin(uni);
		sacnMap[uni] = ret;
		return ret;
	}

    static uint8_t* buffer(uint8_t universe)
    {
        at(universe);
        return &bufferMap[universe][0];
    }

	// static Receiver* operator[](uint8_t universe)
	// {
	// 	return recv(universe);
	// }

	static void update()
	{
		for(auto& pair : sacnMap)
		{
			pair.second->dmx(&bufferMap[0])
			pair.second->update();
		}
	}

};
}
