#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <atomic>
#include "game/voxelworld.h"

using boost::asio::ip::udp;

enum MessageType {
    PlayerMove,
    BlockSet,
    RequestWorldString,
    WorldString,
    RequestPlayerList,
    PlayerList,
};

struct Message {
    MessageType type;
    float x;
    float y;
    float z;
    uint32_t info;
};

struct OtherPlayer {
    int id;
    float x;
    float y;
    float z;
};

void clientStringToPlayerList(std::vector<OtherPlayer> &out, std::string in);

extern std::vector<OtherPlayer> PLAYERS;

std::string getMessageTypeString(Message& m);

class UDPClient {
public:
    UDPClient(boost::asio::io_context& io_context, VoxelWorld *voxworld);

    void send(const Message& message);

    void receive();

    void start();
    void stop();

    void connect();
    void disconnect();

    inline static std::atomic<bool> shouldRunReceiveLoop = false;

    inline static std::atomic<bool> receivedWorld = false;

private:
    boost::asio::io_context& io_context_;
    udp::socket socket_;
    udp::endpoint server_endpoint_;
    std::thread receive_thread;
    VoxelWorld* voxelWorld;
};

#endif