#ifndef UUID_H
#define UUID_H


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

extern boost::uuids::random_generator UUID_GENERATOR;

std::string get_uuid();

#endif