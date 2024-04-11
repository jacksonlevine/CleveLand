#include "acks.h"


std::unordered_map<std::string, std::unordered_set<int>> BUREAU;
std::unordered_map<std::string, Archive> ARCHIVE;
std::mutex BUREAU_LOCK;

void reportMessage(UUID id, std::vector<char>& data, size_t size, int seenBy, int seq) {
    BUREAU_LOCK.lock();
    std::string goose = boost::uuids::to_string(id);
    if(BUREAU.find(goose) == BUREAU.end()) {
        BUREAU.insert_or_assign(goose, std::unordered_set<int>());
        BUREAU.at(goose).insert(seenBy);
        ARCHIVE.insert_or_assign(goose, Archive{std::vector<char>(size), });
        ARCHIVE.at(goose).archive.resize(size);
        std::copy(data.begin(), data.begin() + size, ARCHIVE.at(goose).archive.begin());
    } else {
        BUREAU.at(goose).insert(seenBy);
    }
    BUREAU_LOCK.unlock();
}

void reportSeenMessage(UUID id, int seenBy) {
    BUREAU_LOCK.lock();
    std::string goose = boost::uuids::to_string(id);

    if(BUREAU.find(goose) != BUREAU.end()) {
        BUREAU.at(goose).insert(seenBy);
    }
    BUREAU_LOCK.unlock();
}

void getRecap(UUID id, std::vector<char>::iterator out) {
    BUREAU_LOCK.lock();
    std::string goose = boost::uuids::to_string(id);
    if(ARCHIVE.find(goose) != ARCHIVE.end()) {
        std::copy(ARCHIVE.at(goose).archive.begin(), ARCHIVE.at(goose).archive.end(), out);
    }
    BUREAU_LOCK.unlock();
}

bool getBySeq(int seq, std::vector<char>::iterator out) {
    BUREAU_LOCK.lock();

    Archive *a;
    bool found = false;
    for(auto &[key, val] : ARCHIVE) {
        if(val.seq == seq) {
            a = &val;
            found = true;
            break;
        }
    }
    if( found ) {
        std::copy(a->archive.begin(), a->archive.end(), out);
        BUREAU_LOCK.unlock();
        return true;
    } else {
        std::cout << "We don't have a seq number " << std::to_string(seq) << "\n";

        BUREAU_LOCK.unlock();
        return false;
    }
    

    BUREAU_LOCK.unlock();
}

bool hasSeen(UUID id, int person) {
    BUREAU_LOCK.lock();
    std::string goose = boost::uuids::to_string(id);
    if(BUREAU.find(goose) != BUREAU.end()) {
        if(BUREAU.at(goose).find(person) != BUREAU.at(goose).end()) {
            BUREAU_LOCK.unlock();
            return true;
        }
    }
    BUREAU_LOCK.unlock();
    return false;
}

bool hasSeen(std::string goose, int person) {
    BUREAU_LOCK.lock();
    if(BUREAU.find(goose) != BUREAU.end()) {
        if(BUREAU.at(goose).find(person) != BUREAU.at(goose).end()) {
            BUREAU_LOCK.unlock();
            return true;
        }
    }
    BUREAU_LOCK.unlock();
    return false;
}

bool resolveMessage(UUID id) {
    BUREAU_LOCK.lock();
    std::string goose = boost::uuids::to_string(id);
    if(BUREAU.find(goose) != BUREAU.end()) {
        BUREAU.erase(goose);
    }
    if(ARCHIVE.find(goose) != ARCHIVE.end()) {
        ARCHIVE.erase(goose);
    }
    BUREAU_LOCK.unlock();
}