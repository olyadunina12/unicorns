#include "Networking.h"
#include "../Connect/Serialization.h"
#include "../Connect/Unicorns.h"
#include "../Connect/RPC.h"

#include <SFML/Network.hpp>
#include <cassert>

sf::TcpSocket serverConnection;
bool isConnected;

bool isConnectedToServer()
{
    return isConnected;
}

void connectToServerEntry(std::string Name)
{
    sf::Packet handshake;
    handshake << PacketType::Handshake;
    handshake << HANDSHAKE_MAGIC_STRING;
    handshake << Name;

    sf::UdpSocket socket;
    socket.setBlocking(false);
    socket.bind(4242);

    sf::TcpListener listener;
    listener.setBlocking(false);

    for (int i = 0; i < 100; ++i)
    {
        socket.send(handshake, sf::IpAddress::Broadcast, 4242);
        sf::sleep(sf::seconds(0.5f));

        sf::Socket::Status status = listener.listen(4243);
        for (int i = 0; i < 100; ++i)
        {
            sf::sleep(sf::seconds(0.1f));
            if (status == sf::Socket::Done
                && (status = listener.accept(serverConnection)) == sf::Socket::Done)
            {
                serverConnection.setBlocking(false);
                break;
            }
        }
        if (status == sf::Socket::Done)
        {
            isConnected = true;
            break;
        }
    }
}

void sendPacket(sf::Packet packet)
{
    serverConnection.send(packet);
}

void tickNetwork()
{
    sf::Packet message;
    PacketType type = PacketType::Invalid;

    sf::Socket::Status result = serverConnection.receive(message);
    if (result != sf::Socket::Done || (message >> type) == false)
    {
        return;
    }
    assert(type == PacketType::RPC);
    receiveRPC(message);
}

