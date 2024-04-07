#include "uuid.h"

boost::uuids::random_generator UUID_GENERATOR;


std::string get_uuid() {
    std::string big_uuid_string = boost::uuids::to_string(UUID_GENERATOR());
    return big_uuid_string.substr(0, 8);
}
