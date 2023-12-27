

#include "game.h"

//#include "game/worldsavetests.h"


int main() {
    Game game;

    //saveAndLoadWorldTest();

    while(!glfwWindowShouldClose(game.window)) {
        game.runStep();
    }
    
    glfwTerminate();
    glfwDestroyWindow(game.window);
}