#include <SFML/Network.hpp>
#include "../Connect/Unicorns.h"

#include "Comms.h"

#include <string>
#include <string_view>
#include <iostream>
#include <list>
#include <memory>
#include "../Connect/RPC.h"

enum class ClientState : sf::Uint8
{
    Connected,
    Ready,
    Playing,
    CurrentPlayer,
};

struct ClientConnection
{
    std::string name;
    std::unique_ptr<sf::TcpSocket> socket;
    ClientState state;
};

std::vector<ClientConnection> gClientConnections;

sf::UdpSocket gUdpSocket;

void checkForClients()
{
    for (int i = 0; i < 10; i++)
    {
        sf::IpAddress remoteAddress;
        unsigned short port = 0;

        sf::Packet handshake;
        sf::Socket::Status status = gUdpSocket.receive(handshake, remoteAddress, port);

        if (status != sf::Socket::Done)
        {
            sf::sleep(sf::seconds(.1f));
            return;
        }

        static std::string_view magic = HANDSHAKE_MAGIC_STRING;
        std::string magicString;
        PacketType type;
        std::string clientName;

        if (handshake >> type >> magicString >> clientName && 
            type == PacketType::Handshake &&
            magicString == magic)
        {
            Print("New client trying to connect!!!!");
            Print(remoteAddress.toString() + ":" + std::to_string(port));
            Print("--------------------------------");

            std::unique_ptr<sf::TcpSocket> socket(new sf::TcpSocket());
            if (socket->connect(remoteAddress, 4243) == sf::Socket::Done)
            {
                Print("SUCESS");
                gClientConnections.push_back(
                    ClientConnection{
                        std::move(clientName),
                        std::move(socket),
                        ClientState::Connected
                    }
                );
            }
            else
            {
                Print("FAIL");
            }
        }
    }
}

enum class ServerState
{
    Starting,
    Pregame,
    Ingame,
    Exiting,
};

ServerState gState;

void tick(void)
{
    sf::Packet message;
    PacketType type = PacketType::Invalid;
    ClientConnection* client = nullptr;

    sf::SocketSelector selector;
    for (auto& It : gClientConnections)
    {
        selector.add(*It.socket);
    }
    if (selector.wait(sf::milliseconds(100)))
    {
        for (auto& It : gClientConnections)
        {
            if (selector.isReady(*It.socket))
            {
                sf::Socket::Status result = It.socket->receive(message);
                if (result == sf::Socket::Done && message >> type)
                {
                    client = &It;
                    break;
                }
            }
        }
    }
    if (type == PacketType::RPC)
    {
        receiveRPC(message);
    }

    switch (gState)
    {
        case ServerState::Starting:
        {
            checkForClients();
            break; 
        }
        case ServerState::Pregame:
        {
            Print("Change state to Ingame");
            gState = ServerState::Ingame;
            break; 
        }
        case ServerState::Ingame:
        {
            break; 
        }
        default:
        {

        }
    }
}

void playCard(PlayerID p, CardID c)
{
    std::cout << p.Value << " " << c.Value;
}

void exitServer()
{
    gState = ServerState::Exiting;
}

int main(void)
{
    gUdpSocket.bind(4242);
    gUdpSocket.setBlocking(false);

    REGISTER_RPC(playCard);
    REGISTER_RPC(exitServer);

    while (gState != ServerState::Exiting)
    {
        tick();
    }
    Print("Server closing");
}