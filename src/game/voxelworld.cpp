#include "voxelworld.h"

void VoxelWorld::saveWorldToFile(const char *path) {
    std::ofstream outputFile(path, std::ios::trunc);
    if(outputFile.is_open()) {
        outputFile << seed << '\n';
        for(auto &[chunkcoord, map] : userDataMap) {
            outputFile << chunkcoord.x << " " << chunkcoord.z << "\n";
            for(auto &[blockcoord, blockid] : map) {
                outputFile << "% " << blockcoord.x << " " << blockcoord.y << " " << blockcoord.z << " " <<
                blockid << "\n";
            }
        }
        outputFile.close();
    } else {
        throw std::exception("Failed to save world.");
    }
}

void VoxelWorld::loadWorldFromFile(const char *path) {
    userDataMap.clear();
    std::ifstream file(path);
    if(file.is_open()) {
        std::string line;
        ChunkCoord currentChunkCoord;
        int lineNumber = 0;
        while(std::getline(file, line)) {
            if(lineNumber == 0) {
                seed = static_cast<unsigned int>(std::stoi(line));
            } else {
                std::istringstream linestream(line);
                if(line.front() != '%') {
                    std::string word;
                    int localIndex = 0;
                    ChunkCoord thisChunkCoord;
                    while(linestream >> word) {
                        if(localIndex == 0) {
                            thisChunkCoord.x = std::stoi(word);
                        }
                        if(localIndex == 1) {
                            thisChunkCoord.z = std::stoi(word);
                        }
                        localIndex++;
                    }
                    currentChunkCoord = thisChunkCoord;
                    std::unordered_map<BlockCoord, unsigned int, IntTupHash> thisBlockMap;
                    userDataMap.insert_or_assign(currentChunkCoord, thisBlockMap);
                } else {
                    std::string word;
                    int localIndex = 0;
                    BlockCoord thisBlockCoord;
                    unsigned int thisID;
                    while(linestream >> word) {
                        if(localIndex == 1) {
                            thisBlockCoord.x = std::stoi(word);
                        }
                        if(localIndex == 2) {
                            thisBlockCoord.y = std::stoi(word);
                        }
                        if(localIndex == 3) {
                            thisBlockCoord.z = std::stoi(word);
                        }
                        if(localIndex == 4) {
                            thisID = static_cast<unsigned int>(std::stoi(word));
                        }
                        localIndex++;
                    }
                    userDataMap.at(currentChunkCoord).insert_or_assign(thisBlockCoord, thisID);
                }
            }
            lineNumber++;
        }
    } else {
        throw std::exception("Could not open world file.");
    }
}