#include "hud.h"


Hud::Hud() {
    glGenBuffers(1, &vbo);
}

void Hud::rebuildDisplayData() {
    float chImageWidth = 25;

    GUITextureFace cht(2, 0);

    glm::vec2 chLowerLeft(-chImageWidth/windowWidth, -chImageWidth/windowHeight);
    float relHeight = chImageWidth/(windowHeight/2);
    float relWidth = chImageWidth/(windowWidth/2);


    displayData = {
        chLowerLeft.x, chLowerLeft.y,                    cht.bl.x, cht.bl.y,   -1.0f,
        chLowerLeft.x, chLowerLeft.y+relHeight,          cht.tl.x, cht.tl.y,   -1.0f,
        chLowerLeft.x+relWidth, chLowerLeft.y+relHeight, cht.tr.x, cht.tr.y,   -1.0f,

        chLowerLeft.x+relWidth, chLowerLeft.y+relHeight, cht.tr.x, cht.tr.y,   -1.0f,
        chLowerLeft.x+relWidth, chLowerLeft.y,           cht.br.x, cht.br.y,   -1.0f,
        chLowerLeft.x, chLowerLeft.y,                    cht.bl.x, cht.bl.y,   -1.0f
    };

}