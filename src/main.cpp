

#define DEV


#include "game.h"
#include "util/username.h"
#include "util/textloader.h"


//#include "game/worldsavetests.h"


int main() {
    //::cout << "Please enter your username: ";
    //std::getline(std::cin, USERNAME);

    try
    {
        load_text("versionstring.txt", VERSIONSTRING);

        USERNAME = "Test name";


        Game game;

        //saveAndLoadWorldTest();

        while(!glfwWindowShouldClose(game.window)) {
            game.runStep();
        }

        glfwTerminate();
        glfwDestroyWindow(game.window);
    } catch(std::exception e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }

}