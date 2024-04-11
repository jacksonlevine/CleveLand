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
#include <mutex>
#include <memory>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "chat/chatstuff.h"
#include "mobtype.h"

using UUID = boost::uuids::uuid;

using boost::asio::ip::tcp;

extern bool rcvtpromisesat;

extern std::mutex WRITE_MUTEX;

extern std::string TYPED_IN_SERVER_IP;

extern uint32_t MY_ID;


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
    TellYouYourID,
    MobUpdate,
    MobUpdateBatch
};


struct MobComponent {
    MobType type;
    uint32_t id;
    glm::vec3 lpos;
    glm::vec3 pos;
    float rot;
    float lrot;
    uint8_t health;
    float timePosted;
};




struct MobMsg {
    uint32_t id;
    glm::vec3 pos;
    float rot;
    uint8_t flags;
    MobType type;
    uint8_t health;
};

#define MOBBATCHSIZE 8

struct MobMsgBatch {
    MobMsg msgs[MOBBATCHSIZE];
};


extern std::mutex MOBS_MUTEX;
extern std::unordered_map<int, MobComponent> MOBS;




struct Message {
    UUID goose;
    MessageType type;
    float x;
    float y;
    float z;
    uint32_t info;
    float rot;
};


Message createMessage(MessageType type, float x, float y, float z, uint32_t info, float r = 0.0f);

struct OtherPlayer {
    int id;
    float x;
    float y;
    float z;
    float lx;
    float ly;
    float lz;
    float t;
    float rot;
    float lrot;
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
extern std::atomic<bool> PLAYERSCHANGED;

extern std::atomic<bool> MOBSCHANGED;


extern std::promise<void> receive_thread_promise;

std::string getMessageTypeString(Message& m);

class Camera3D;

class TCPClient {
public:
    TCPClient(boost::asio::io_context& io_context, VoxelWorld *voxworld, std::function<void(float)> *gameTimeSet, glm::vec3 *cameraPos, std::atomic<float> * camRot);

    void send(const Message& message);

    void receive();

    void start();
    void stop();

    void connect();
    void disconnect();
    void processMessage(Message* message);


    inline static std::atomic<bool> shouldRunReceiveLoop = false;
    inline static std::atomic<bool> shouldRunSendLoop = false;

    inline static std::atomic<bool> receivedWorld = false;

    tcp::socket socket_;

private:
    boost::asio::io_context& io_context_;
    
    std::thread receive_thread;
    std::thread send_thread;
    VoxelWorld* voxelWorld;
    std::function<void(float)>* setGameTime;
    glm::vec3 *cameraPos;
    std::atomic<float> *camRot;
};

#endif