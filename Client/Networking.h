#pragma once
#include <string>
#include <SFML/Network.hpp>

bool isConnectedToServer();
void connectToServerEntry(std::string Name);
void sendPacket(sf::Packet packet);
void tickNetwork();
