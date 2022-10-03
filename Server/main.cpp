#include <SFML/Network.hpp>
#include "../Connect/Unicorns.h"

#include "Comms.h"

#include <string>
#include <string_view>
#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <memory>

std::atomic<bool> gExiting = false;

std::mutex gClientsLock;
std::list<std::unique_ptr<sf::TcpSocket>> gClients;

void listenForClients()
{
	sf::UdpSocket socket;
	socket.bind(4242);
	socket.setBlocking(false);

	while (!gExiting)
	{
		sf::IpAddress remoteAddress;
		unsigned short port = 0;

		sf::Packet handshake;
		sf::Socket::Status status = socket.receive(handshake, remoteAddress, port);

		if (status != sf::Socket::Status::Done)
		{
			sf::sleep(sf::seconds(1.f));
			continue;
		}

		static std::string_view magic = HANDSHAKE_MAGIC_STRING;
		std::string magicString;
		PacketType type;

		if ((handshake >> type) &&
			type == PacketType::Handshake &&
			(handshake >> magicString) &&
			magicString == magic)
		{
			Print("New client trying to connect!!!!");
			Print(remoteAddress.toString() + ":" + std::to_string(port));
			Print("--------------------------------");
			
			std::unique_ptr<sf::TcpSocket> socket(new sf::TcpSocket());
			if (socket->connect(remoteAddress, 4243) == sf::Socket::Done)
			{
				Print("SUCESS");
				gClientsLock.lock();
				gClients.push_back(std::move(socket));
				gClientsLock.unlock();
			}
			else
			{
				Print("FAIL");
			}
		}
	}
}

int main(void)
{
	std::thread clientListeningThread(listenForClients);

	while (true)
	{
		std::string input;
		if (ReadInput(input) && input.find("quit") != input.npos)
		{
			gExiting = true;
			break;
		}
		sf::sleep(sf::seconds(1.f));
	}

	clientListeningThread.join();
}