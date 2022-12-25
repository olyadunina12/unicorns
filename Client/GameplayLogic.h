#pragma once
#include "../Connect/Unicorns.h"

#include <functional>

using Command = std::function<void(void)>;

int    userAskQuestion(std::string text, std::vector<std::string> variants);
CardID userSelectCard(PlayerID player, CardPile pile);

void pushGameplayCmd(const Command& cmd);
void startGameplayThread();
void stopGameplayThread();

void tickGameplayLogic();

