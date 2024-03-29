#include "RPC.h"
#include <cassert>

std::map<std::string, std::function<void (sf::Packet&)>> RPCs;

void bindFunctionToName(const std::string& name, std::function<void (sf::Packet&)> f)
{
    auto& func = RPCs[name];
    assert(!func);
    func = f;
}

void receiveRPC(sf::Packet& packet)
{
    std::string name;
    packet >> name;

#if defined (DEBUG)
    printf("Received RPC: %s", name.c_str());
#endif
    
    auto& func = RPCs[name];
    assert(func);
    func(packet);
}
