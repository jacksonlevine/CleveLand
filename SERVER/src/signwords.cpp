#include "signwords.h"


std::unordered_map<BlockCoord, std::string, IntTupHash> signWords;
std::string signWordsString;

void saveSignWordsToFile(){
    std::ofstream outf("signwords", std::ios::trunc);
    std::ostringstream out;
    for(auto &[key, val] : signWords) {
        out << std::to_string(key.x) << " " << std::to_string(key.y) << " " << std::to_string(key.z) << " " << val << "\n";
    }
    outf << out.str();
    signWordsString = out.str();
    outf.close();
}

void loadSignWordsFromFile(){
    signWords.clear();
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
            std::cout << "Loaded sign: " << std::to_string(coord.x) << " " << std::to_string(coord.y) << " " << std::to_string(coord.z) << " " << signWord << "\n";
            signWords.insert_or_assign(coord, signWord);
            
            lineIndex++;
        }
        saveSignWordsToFile();
        file.close();

}