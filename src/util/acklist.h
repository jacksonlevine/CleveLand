// #ifndef ACKLIST_H

// #define ACKLIST_H

// #include <boost/uuid/uuid.hpp>
// #include <boost/uuid/uuid_generators.hpp>
// #include <boost/uuid/uuid_io.hpp>
// #include <algorithm>
// #include <vector>
// #include <unordered_map>
// #include <mutex>

// using UUID = boost::uuids::uuid;

// extern std::mutex ACK_LIST_LOCK;
// extern std::mutex ACK_ARCHIVE_LOCK;
// extern std::unordered_map<UUID, std::vector<int>> ACK_LIST;
// extern std::unordered_map<UUID, std::vector<char>> ACK_ARCHIVE;

// class Acknowledger {
// public:
//     void wantAcknowledgementForThis(UUID id);
// };

// #endif