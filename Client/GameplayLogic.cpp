#include "GameplayLogic.h"

#include <queue>
#include <functional>
#include <mutex>

#include <SFML/System/Sleep.hpp>
#include <imgui.h>

// read by gameplayLogicThread, written by main thread
bool gShouldExit = false;
std::mutex gCommandsLock;
std::queue<Command> gCommandQueue;

std::thread gameplayLogicThread;

// read by main, written by gameplayLogicThread
using Tickable = std::function<bool(void)>;

std::mutex            gTickablesMutex;
std::vector<Tickable> gTickables;

static void gameplayLogic()
{
    while (!gShouldExit)
    {
        if (gCommandQueue.empty())
        {
            sf::sleep(sf::seconds(.3f));
            continue;
        }
        Command cmd;
        {
            std::lock_guard autoLock(gCommandsLock);
            cmd = gCommandQueue.front();
            gCommandQueue.pop();
        }
        cmd();
    }
}

static void executeOnMain(const Tickable& f)
{
    std::atomic<bool> done = false;
    {
        std::lock_guard autolock(gTickablesMutex);
        int index = gTickables.size();
        gTickables.push_back([&done, &f](){
            bool result = f();
			if (!result)
			{
				done = true;
            }
            return result;
        });
    }
    while (done == false)
    {
        sf::sleep(sf::seconds(0.33f));
    }
}

int    userAskQuestion(std::string text, std::vector<std::string> variants)
{
    int result = -1;
    executeOnMain([&text, &variants, &result]() -> bool {
        ImGui::SetNextWindowFocus();
        ImGui::SetNextWindowPos(ImVec2(1920/2, 1080/2), ImGuiCond_Always);
        ImGui::SetNextItemWidth(1920);
        ImGui::Begin("Prompt", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
        ImGui::Text(text.c_str());
        for (int i = 0; i < variants.size(); ++i)
        {
            if (i != 0)
                ImGui::SameLine();
            if (ImGui::Button(variants[i].c_str(), ImVec2(200, 100)))
            {
                result = i;
            }
        }
        ImGui::End();
        return result == -1;
    });
    return result;
}

CardID userSelectCard(PlayerID player, CardPile pile)
{
    return CardID{ (uint8_t) -1};
}

void pushGameplayCmd(const Command& cmd)
{
    std::lock_guard autoLock(gCommandsLock);
    gCommandQueue.push(cmd);
}

void startGameplayThread()
{
    gameplayLogicThread = std::thread(&gameplayLogic);
}

void stopGameplayThread()
{
    gShouldExit = true;

    if (gameplayLogicThread.joinable())
        gameplayLogicThread.join();
}

void tickGameplayLogic()
{
    std::lock_guard autolock(gTickablesMutex);
    for (auto It = gTickables.begin(); It != gTickables.end();)
    {
        bool result = (*It)();
        if (result)
            ++It;
        else
            It = gTickables.erase(It);
    }
}

