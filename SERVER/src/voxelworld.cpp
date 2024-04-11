#include "voxelworld.h"

// "PLAYER FILE" IS USED HERE AS A WORLD TIME KEEPER

std::string worldString;

void VoxelWorld::setBlock(BlockCoord coord, uint32_t block) {
    ChunkCoord cc(
        std::floor(static_cast<float>(coord.x) / chunkWidth),
        std::floor(static_cast<float>(coord.z) / chunkWidth)
    );
    auto chunkIt = userDataMap.find(cc);
    if(chunkIt == userDataMap.end()) {
        userDataMap.insert_or_assign(cc, std::unordered_map<BlockCoord, uint32_t, IntTupHash>());
    }
    userDataMap.at(cc).insert_or_assign(coord, block);
}

void VoxelWorld::saveGame(const char* path) {
    _saveWorldToFile(path);
    std::string playerInfoPath = std::string(path) + "/player.save";
    std::ofstream playerFile(playerInfoPath, std::ios::trunc);
    if(playerFile.is_open()) {

        playerFile << timeOfDay << "\n";
    } else {
        std::cerr << "Couldn't open player file when saving. \n";
    }
    playerFile.close();
}

bool VoxelWorld::saveExists(const char* path) {
    std::string worldPath(path);
    worldPath += "/world.save";
    return std::filesystem::exists(worldPath);
}

void VoxelWorld::_saveWorldToFile(const char *path) {
    std::string worldPath(path);
    worldPath += "/world.save";

    std::filesystem::create_directories(std::filesystem::path(worldPath).parent_path());

    std::ostringstream exportStream;

    std::ofstream outputFile(worldPath, std::ios::trunc);
    if(outputFile.is_open()) {
        exportStream << "version " << worldGenVersion << " " << seed << '\n';
        for(auto &[chunkcoord, map] : userDataMap) {
            exportStream << chunkcoord.x << " " << chunkcoord.z << "\n";
            for(auto &[blockcoord, blockid] : map) {
                exportStream << "% " << blockcoord.x << " " << blockcoord.y << " " << blockcoord.z << " " <<
                blockid << "\n";
            }
        }
        outputFile << exportStream.str();
        outputFile.close();

        worldString = exportStream.str();
    } else {
        throw std::exception("Failed to save world.");
    }
}

void VoxelWorld::_loadWorldFromFile(const char *path) {
    userDataMap.clear();
    std::string worldPath(path);
    worldPath += "/world.save";
    std::ifstream file(worldPath);
    if(file.is_open()) {
        // Read the entire file into the worldString
        std::stringstream buffer;
        buffer << file.rdbuf();
        worldString = buffer.str();
        file.close();

        std::istringstream fileStream(worldString);

        std::string line;
        ChunkCoord currentChunkCoord;
        int lineNumber = 0;
        while(std::getline(fileStream, line)) {
            if(lineNumber == 0) {
                std::istringstream linestream(line);
                std::vector<std::string> words;
                std::string word;

                while(linestream >> word) {
                    words.push_back(word);
                }

                if(words.at(0) == "version") {
                    worldGenVersion = std::stoi(words.at(1));
                    
                    seed = static_cast<unsigned int>(std::stoi(words.at(2)));

                } else {

                    seed = static_cast<unsigned int>(std::stoi(words.at(0)));
                }

                std::cout << "Worldgen version is " << worldGenVersion << "\n";

                
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
                    std::unordered_map<BlockCoord, uint32_t, IntTupHash> thisBlockMap;
                    userDataMap.insert_or_assign(currentChunkCoord, thisBlockMap);
                } else {
                    std::string word;
                    int localIndex = 0;
                    BlockCoord thisBlockCoord;
                    uint32_t thisID;
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
                            thisID = static_cast<uint32_t>(std::stoi(word));
                        }
                        localIndex++;
                    }
                    userDataMap.at(currentChunkCoord).insert_or_assign(thisBlockCoord, thisID);
                }
            }
            lineNumber++;
        }
        file.close();
    } else {
        throw std::exception("Could not open world file.");
    }
}


void VoxelWorld::loadOrCreateSaveGame(const char* path) {
    if(saveExists(path)) {
        _loadWorldFromFile(path);
    } else {
        std::cout << "No world file, generating a new one!\n"; 
        seed = time(NULL);
        _saveWorldToFile(path);
    }
    
    std::string playerInfoPath = std::string(path) + "/player.save";
    if(!std::filesystem::exists(playerInfoPath)) {
        std::ofstream playerFile(playerInfoPath, std::ios::trunc);
        if(playerFile.is_open()) {
            playerFile << timeOfDay << "\n";
        } else {
            std::cerr << "Couldn't open player file when initializing. \n";
        }
        playerFile.close();
    }

    std::ifstream playerFile(playerInfoPath);
    if(playerFile.is_open()) {
        std::string line;
        int lineIndex = 0;
        while(std::getline(playerFile, line)) {
            std::istringstream linestream(line);
            std::string word;
            int localIndex = 0;
            while(linestream >> word) {
                if(lineIndex == 0) {
                    timeOfDay = std::stof(word);
                }
                localIndex++;
            }
            lineIndex++;
        }
    playerFile.close();

    } else {
        std::cerr << "Couldn't open player file when loading. \n";
    }
    

}