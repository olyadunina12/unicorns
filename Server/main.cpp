#include <SFML/Network.hpp>
#include "../Connect/Unicorns.h"

#include <string>
#include <string_view>
#include <iostream>
#include <thread>

std::atomic<bool> gExiting = false;

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

		std::string_view magic = MAGIC_STRING;
		std::string magicString;
		if ((handshake >> magicString) && magicString == magic)
		{
			std::cout << ("New client connected!!!!") << std::endl;
			std::cout << remoteAddress.toString() << ":" << port << std::endl;
			std::cout << ("------------------------") << std::endl;
		}
	}
}

int main(void)
{
	std::thread clientListeningThread(listenForClients);

	std::string input;
	while (true)
	{
		std::cin >> input;
		if (!input.empty())
		{
			gExiting = true;
			break;
		}
		sf::sleep(sf::seconds(1.f));
	}

	clientListeningThread.join();
}