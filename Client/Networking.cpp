#include "Networking.h"
#include "../Connect/Unicorns.h"

#include <SFML/Network.hpp>

sf::TcpSocket serverConnection;

void connectToServerEntry()
{
    sf::Packet handshake;
    handshake << PacketType::Handshake;
    handshake << HANDSHAKE_MAGIC_STRING;

    sf::UdpSocket socket;
    socket.setBlocking(false);

    sf::TcpListener listener;
    listener.setBlocking(false);

    for (int i = 0; i < 100; ++i)
    {
        socket.bind(4242);
        socket.send(handshake, sf::IpAddress::Broadcast, 4242);
        sf::sleep(sf::seconds(0.5f));

        sf::Socket::Status status = listener.listen(4243);
        for (int i = 0; i < 100; ++i)
        {
            sf::sleep(sf::seconds(0.1f));
            if (status == sf::Socket::Done
                && (status = listener.accept(serverConnection)) == sf::Socket::Done)
            {
                break;
            }
        }
        if (status == sf::Socket::Done)
        {
            break;
        }
    }
}
