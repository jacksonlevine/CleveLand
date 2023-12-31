#include "raycast.h"

RayCastResult rayCast(int chunkwidth, glm::vec3 origin, glm::vec3 dir, std::function<bool(BlockCoord)> predicate, bool includeSideChunks) {
    RayCastResult result;
    static int maximumLength = 10;

    glm::vec3 head(origin);
    for(int d = 0; d < maximumLength*4; d++) {
        BlockCoord here(std::round(head.x), std::round(head.y), std::round(head.z));
        if(predicate(here)) {
            result.blockHit = here;
            result.hit = true;
            ChunkCoord chunkHere(
                std::floor(static_cast<float>(here.x)/chunkwidth),
                std::floor(static_cast<float>(here.z)/chunkwidth)
            );
            result.chunksToRebuild.push_back(chunkHere);
            std::cout << "Chunk: " << "\n" <<
            "   Position: " << chunkHere.x << ", " << chunkHere.z << "\n";
            if(includeSideChunks) {
                result.chunksToRebuild.insert(result.chunksToRebuild.end(), {
                    chunkHere + ChunkCoord(1,0),
                    chunkHere + ChunkCoord(-1,0),
                    chunkHere + ChunkCoord(0,1),
                    chunkHere + ChunkCoord(0,-1)
                });
            }
            break;
        }
        head += dir/4.0f;
    }

    return result;
}