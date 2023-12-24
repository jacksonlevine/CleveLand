

#include "game.h"




int main() {
    Game game;

    while(!glfwWindowShouldClose(game.window)) {
        game.runStep();
    }

    glfwTerminate();
    glfwDestroyWindow(game.window);
}