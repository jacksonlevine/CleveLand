

#define DEV

#define _WIN32_WINNT 0x0601

#include "game.h"
#include "util/username.h"



//#include "game/worldsavetests.h"


int main() {
    //::cout << "Please enter your username: ";
    //std::getline(std::cin, USERNAME);

    USERNAME = "Test name";

    Game game;

    //saveAndLoadWorldTest();

    while(!glfwWindowShouldClose(game.window)) {
        game.runStep();
    }
    
    glfwTerminate();
    glfwDestroyWindow(game.window);
}