#ifndef ACKS_H

#define ACKS_H

#include <vector>
#include <unordered_map>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <string>

using UUID = boost::uuids::uuid;

struct Archive {
    std::vector<char> archive;
    int seq;
};

extern std::unordered_map<std::string, std::unordered_set<int>> BUREAU;
extern std::unordered_map<std::string, Archive> ARCHIVE;

void reportMessage(UUID id, std::vector<char>& data, size_t size, int seenBy);
void reportSeenMessage(UUID id, int seenBy);
void getRecap(UUID id, std::vector<char>::iterator out);
bool hasSeen(UUID id, int person);
bool hasSeen(std::string id, int person);
bool resolveMessage(UUID id);
bool getBySeq(int seq, std::vector<char>::iterator out);

extern std::mutex BUREAU_LOCK;

#endif