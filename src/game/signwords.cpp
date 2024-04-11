#include "signwords.h"


std::unordered_map<BlockCoord, std::string, IntTupHash> signWords;

std::string SIGN_BUFFER;

void saveSignWordsToFile(){
    std::ofstream outf("signwords", std::ios::trunc);

    for(auto &[key, val] : signWords) {
        outf << std::to_string(key.x) << " " << std::to_string(key.y) << " " << std::to_string(key.z) << " " << val << "\n";
    }

    outf.close();
}

void loadSignWordsFromFile(){
    signWords.clear();
    if(std::filesystem::exists("signwords")) {
        std::ifstream file("signwords");
        std::string line;
        int lineIndex = 0;
        while(std::getline(file, line)) {
            std::istringstream linestream(line);

            std::string word;
            int localIndex = 0;
            BlockCoord coord;
            std::string signWord;
            while(linestream >> word) {
                if(localIndex == 0) {
                    coord.x = std::stoi(word);
                }
                if(localIndex == 1) {
                    coord.y = std::stoi(word);
                }
                if(localIndex == 2) {
                    coord.z = std::stoi(word);
                }
                if(localIndex > 2) {

                    do {
                        signWord += word + " ";

                    } while(linestream >> word);

                    
                }
                
                localIndex++;
            }
            signWords.insert_or_assign(coord, signWord);
            
            lineIndex++;
        }
    } else {
        saveSignWordsToFile();
    }
}




void loadSignWordsFromString(std::string& signString){
    signWords.clear();
    if(std::filesystem::exists("signwords")) {
        std::istringstream file(signString);
        std::string line;
        int lineIndex = 0;
        while(std::getline(file, line)) {
            std::istringstream linestream(line);

            std::string word;
            int localIndex = 0;
            BlockCoord coord;
            std::string signWord;
            while(linestream >> word) {
                if(localIndex == 0) {
                    coord.x = std::stoi(word);
                }
                if(localIndex == 1) {
                    coord.y = std::stoi(word);
                }
                if(localIndex == 2) {
                    coord.z = std::stoi(word);
                }
                if(localIndex > 2) {

                    do {
                        signWord += word + " ";

                    } while(linestream >> word);

                    
                }
                
                localIndex++;
            }
            signWords.insert_or_assign(coord, signWord);
            
            lineIndex++;
        }
    } else {
        saveSignWordsToFile();
    }
}