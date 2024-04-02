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
#include <future>
#include "util/username.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using UUID = boost::uuids::uuid;

using boost::asio::ip::udp;

extern bool rcvtpromisesat;

enum MessageType {
    PlayerMove,
    BlockSet,
    RequestWorldString,
    WorldString,
    RequestPlayerList,
    PlayerList,
    Heartbeat,
    Disconnect,
    TimeUpdate,
    Acknowledge,
    RequestSeq,
};

struct Message {
    UUID goose;
    MessageType type;
    float x;
    float y;
    float z;
    uint32_t info;
    int sequence;
};

extern int INCOMING_SEQUENCE_NUMBER;
extern int OUTGOING_SEQUENCE_NUMBER;
extern std::unordered_map<int, std::vector<char>> BACKBURNER;

int NEXT_INCOME_SEQ();
int NEXT_OUTGO_SEQ();

Message createMessage(MessageType type, float x, float y, float z, uint32_t info);

Message createMessageWithID(UUID goose, MessageType type, float x, float y, float z, uint32_t info);

struct OtherPlayer {
    int id;
    float x;
    float y;
    float z;
    std::string name;
};

struct NameMessage {
    UUID goose;
    int id;
    char data[59];
    int length;
};

NameMessage createNameMessage(int id, std::string name, size_t length);


void clientStringToPlayerList(std::vector<OtherPlayer> &out, std::string in);

extern std::vector<OtherPlayer> PLAYERS;
extern std::promise<void> receive_thread_promise;

std::string getMessageTypeString(Message& m);

class UDPClient {
public:
    UDPClient(boost::asio::io_context& io_context, VoxelWorld *voxworld, std::function<void(float)> *gameTimeSet);

    void send(const Message& message);

    void receive();

    void start();
    void stop();

    void connect();
    void disconnect();
    void processMessage(Message* message, udp::endpoint& sender_endpoint);

    void acknowledgeMessage(UUID goose);

    inline static std::atomic<bool> shouldRunReceiveLoop = false;
    inline static std::atomic<bool> shouldRunSendLoop = false;

    inline static std::atomic<bool> receivedWorld = false;

    udp::socket socket_;

private:
    boost::asio::io_context& io_context_;
    
    udp::endpoint server_endpoint_;
    std::thread receive_thread;
    std::thread send_thread;
    VoxelWorld* voxelWorld;
    std::function<void(float)>* setGameTime;
};

#endif