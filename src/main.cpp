

#define DEV

#include "game.h"
#include "util/username.h"

//#include "game/worldsavetests.h"


int main() {
    std::cout << "Please enter your username: ";
    std::getline(std::cin, USERNAME);

    Game game;

    //saveAndLoadWorldTest();

    while(!glfwWindowShouldClose(game.window)) {
        game.runStep();
    }
    
    glfwTerminate();
    glfwDestroyWindow(game.window);
}