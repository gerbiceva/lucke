#include "SacnHandler.h"

namespace Handler
{
    std::map<uint8_t, uint8_t> Sacn::activeReceivers;
    std::map<uint8_t, Receiver*> Sacn::sacnMap;
    std::map<uint8_t, std::array<uint8_t, 512U>> Sacn::bufferMap;
    WiFiUDP Sacn::udp;

    void Sacn::incrementActive(uint8_t universe)
	{
		if(activeReceivers.find(universe) == activeReceivers.end())
		{
			activeReceivers[universe] = 1U;
		}
		else 
		{
			activeReceivers[universe]++;
		}
	}

	void Sacn::decrementActive(uint8_t universe)
	{
		if(activeReceivers[universe] == 0)
		{
			delete sacnMap[universe];
		}
        else
        {
    		activeReceivers[universe]--;
        }
	}

    uint8_t* Sacn::buffer(uint8_t universe)
    {
        if(sacnMap.find(universe) == sacnMap.end())
		{
            Receiver* ret = new Receiver(udp);
            // ret->callbackDMX([](){ Controller::get().newPacket(); });
            // ret->callbackSource([](){ Controller::get().printNewret(); });
            // ret->callbackFramerate([](){ Controller::get().updateFramerate(); });
            // ret->callbackSeqDiff([](){ Controller::get().seqDiff(); });
            // ret->callbackTimeout([](){});
            ret->begin(universe);
            sacnMap[universe] = ret;
        }

		incrementActive(universe);
        return &bufferMap[universe][0];
    }

	// static Receiver* operator[](uint8_t universe)
	// {
	// 	return recv(universe);
	// }

	void Sacn::update()
	{
		for(auto& pair : sacnMap)
		{
			pair.second->dmx(&bufferMap[pair.first][0]);
			pair.second->update();
		}
	}
}