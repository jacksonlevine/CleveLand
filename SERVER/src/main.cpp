#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include "voxelworld.h"
#include <chrono>
#include <atomic>
#include <GLFW/glfw3.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <memory>
#include <functional>
// #include "acks.h"
#include <glm/glm.hpp>
#include <stdlib.h>
#include <time.h>
#include "signwords.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using UUID = boost::uuids::uuid;

#include "uuid.h"

const char* worldPath = "world";

void * sendTimeFunc;







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
    MobUpdateBatch,
    SignUpdate,
    RequestSignsString,
    SignsString
};

enum StringMessageType {
    SignPlace,
    Check
};

struct StringMsg {
    StringMessageType type;
    int x;
    int y;
    int z;
    char word[256];
    uint32_t id;
};




enum MobType {
    Player,
    Normal
};


struct MobComponent {
    MobType type;
    uint32_t id;
    glm::vec3 lpos;
    glm::vec3 pos;
    float rot;
    uint8_t health;
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

uint32_t currentMobId = 0;
std::vector<entt::entity> MOBLIST;



VoxelWorld world;

void newMob(MobType type, glm::vec3 pos) {
    entt::entity e = world.registry.create();

    world.registry.emplace<MobComponent>(e, MobComponent {
        type,
        currentMobId++,
        pos,
        pos,
        0.0f,
        100
    });
    MOBLIST.push_back(e);
}   


struct Message {
    UUID goose;
    MessageType type;
    float x;
    float y;
    float z;
    uint32_t info;
    float rot;
};

struct BlockChange {
    int x;
    int y;
    int z;
    uint32_t block;
};
void timeOfDayThreadFunction();

void printHex(const std::string& str) {
    for (char c : str) {
        std::printf("%02x ", static_cast<unsigned char>(c));
    }
    std::cout << std::endl;
}
StringMsg createStringMessage(StringMessageType type, int x, int y, int z, std::string word, uint32_t id) {
    StringMsg msg;
    msg.x = x;
    msg.y = y;
    msg.z = z;
    msg.id = id;

    if(word.size() < sizeof(char) * 250) {
            std::copy(word.begin(), word.end(), msg.word);
            msg.word[word.size()] = '\0';
    } else {
            std::copy(word.begin(), word.begin() + sizeof(char) * 250, msg.word);
            msg.word[250] = '\0';
    }

    return msg;
    
}


struct Client {
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    int id;
    float x;
    float y;
    float z;
    std::chrono::steady_clock::time_point last_active;
    std::string name;

    Client() : socket(nullptr), id(0), x(0.0f), y(0.0f), z(0.0f),
    last_active(std::chrono::steady_clock::now()), name(std::string("Brewhaha")) {}
};

int expectedNextSeq(int current) {
    return (current + 1) % 500;
}

struct NameMessage {
    UUID goose;
    int id;
    char data[59];
    int length;
};


Message createMessage(MessageType type, float x, float y, float z, uint32_t info, float r = 0.0f) {
    Message msg;
    boost::uuids::random_generator generator;
    msg.goose = generator();
    msg.type = type;
    msg.x = x;
    msg.y = y;
    msg.z = z;
    msg.info = info;
    msg.rot = r;
    return msg;
}


std::unordered_map<int, Client> CLIENTS;



std::string clientListToString(int excludedID) {
    std::ostringstream outputStream;

    for(auto &[key, val] : CLIENTS) {
        if(key != excludedID) {
            outputStream << key << " " << val.x << " " << val.y << " " << val.z << " " << val.name << "\n";
        }
    }

    return outputStream.str();
}

std::atomic<int> SEQUENCE_NUM = 2;

std::vector<BlockChange> BLOCK_CHANGE_QUEUE;

std::mutex COMMUNICATION_LOCK; 
std::mutex CLIENTS_LOCK;


std::string getMessageTypeString(Message& m) {
    switch(m.type) {
        case MessageType::PlayerMove:
            return std::string("Player move");
        case MessageType::BlockSet:
            return std::string("Block set");
        case MessageType::RequestWorldString:
            return std::string("Request World String");
        case MessageType::WorldString:
            return std::string("World String");
        case MessageType::RequestPlayerList:
            return std::string("Req player list");
        case MessageType::PlayerList:
            return std::string("Player list");
        case MessageType::Heartbeat:
            return std::string("Heartbeat");
        case MessageType::Disconnect:
            return std::string("Disconnect");
        case MessageType::TimeUpdate:
            return std::string("TimeUpdate");
        case MessageType::TellYouYourID:
            return std::string("TellYouYourID");
        case MessageType::MobUpdate:
            return std::string("MobUpdate");
        case MessageType::MobUpdateBatch:
            return std::string("MobUpdateBatch");
        case MessageType::SignUpdate:
            return std::string("SignUpdate");
        case MessageType::RequestSignsString:
            return std::string("RequestSignsString");
        case MessageType::SignsString:
            return std::string("SignsString");
    }
}



std::chrono::steady_clock::time_point lastTimeCheckedForOldUsers = std::chrono::steady_clock::now();

class TCPServer {
public:
    TCPServer(io_context* io_context, unsigned short port)
        : acceptor_(*io_context, tcp::endpoint(tcp::v4(), port)), io_context(io_context)
         {
        
        // Log the endpoints
        std::cout << "Main acceptor listening on: " << acceptor_.local_endpoint() << std::endl;

        start_accept();
    }

private:
    io_context* io_context;
    void start_accept() {
        // Create a new socket for the incoming connection

        auto socket = std::make_shared<tcp::socket>(*io_context);

        // Asynchronously accept a new connection
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
            if (!error) {
                // Successfully accepted a new connection, handle it
                std::cout << "Client socket is a go\n"; 
                handle_connection(socket);
                
            }

            // Continue accepting new connections
            start_accept();
        });
    }




    void handle_connection2(std::shared_ptr<tcp::socket> socket) {
        std::thread([socket]() {
            try {
                // Simple echo logic
                char data[1024];
                boost::system::error_code error;
                size_t length = socket->read_some(buffer(data), error);
                if (!error) {
                    std::cout << "Received: " << std::string(data, length) << "\n";
                    write(*socket, buffer(data, length));
                }
            } catch (const std::exception& e) {
                std::cerr << "Error in connection: " << e.what() << "\n";
            }
        }).detach();
    }










    

    void handle_connection(std::shared_ptr<tcp::socket> socket) {

        Client client;
        client.socket = socket;
        std::cout << "Client socket\n"; 
        client.id = currentID;
        currentID++;

        {
            std::lock_guard<std::mutex> lock(CLIENTS_LOCK);
            CLIENTS[client.id] = std::move(client);

            Message tellId = createMessage(MessageType::TellYouYourID, 0, 0, 0, client.id);
            boost::asio::write(*socket, boost::asio::buffer(&tellId, sizeof(Message)));

        }

        // Create a new thread to handle the connection
        std::thread([this, id = client.id]() mutable {
            try {
                bool run = true;
                while(run) {

                    Message message;


                    
                    std::shared_ptr<tcp::socket> client_socket;
                    {
                        std::lock_guard<std::mutex> lock(CLIENTS_LOCK);
                        client_socket = CLIENTS[id].socket;
                    }



                    boost::asio::read(*client_socket, boost::asio::buffer(&message, sizeof(Message)));
                    
                    auto now = std::chrono::steady_clock::now();
                        
                        std::cout << "Received: " << getMessageTypeString(message) << " from " << std::to_string(id) << std::endl;


                        {
                            std::lock_guard<std::mutex> lock(CLIENTS_LOCK);
                            if(CLIENTS.find(id) != CLIENTS.end()) {
                                CLIENTS.at(id).last_active = now;
                            }
                        }
                        
                        bool playerMove = false;
                        glm::vec3 newPlayerPosition(0,0,0);

                       
                        if(message.type == MessageType::PlayerMove) {
                            playerMove = true;
                            newPlayerPosition = glm::vec3(
                                message.x,
                                message.y,
                                message.z
                            );
                        }
    


                        if(message.type == MessageType::RequestWorldString) {



                            Message m = createMessage(MessageType::WorldString, 0, 0, 0, worldString.size());

                            boost::asio::write(*client_socket, boost::asio::buffer(&m, sizeof(Message)));


                            std::cout << "Sending world string of size " << std::to_string(worldString.size())
                            << "\n";
                            // std::cout << "World string: \n" <<
                            // worldString << "\n";
                            boost::asio::write(*client_socket, boost::asio::buffer(worldString));
                        }

                        if(message.type == MessageType::SignUpdate) {

                               std::cout << "Expecting " << std::to_string(message.info) << " bytes\n";
                                std::vector<char> ws_buffer(static_cast<size_t>(message.info) + 1, '\0');

                                boost::asio::read(*client_socket, boost::asio::buffer(ws_buffer, message.info));

                                std::string receivedString(ws_buffer.data(), message.info);
                                
                                std::cout << "Sign string: \n" <<
                                    receivedString << "\n";

                                BlockCoord coord(message.x, message.y, message.z);
                                signWords.insert_or_assign(coord, receivedString);
                                saveSignWordsToFile();

                                {

                                    std::lock_guard<std::mutex> lock(CLIENTS_LOCK);

                                    for (auto&[key, value] : CLIENTS) {
                                        
                                        boost::asio::write(*(value.socket), boost::asio::buffer(&message, sizeof(Message)));

                                        boost::asio::write(*(value.socket), boost::asio::buffer(receivedString));    
                                        
                                    }
                                    
                                }
                                
                        }


                        if(message.type == MessageType::RequestPlayerList) {


                            
                            std::string playerListString = clientListToString(id);

                            Message m = createMessage(MessageType::PlayerList, 0, 0, 0, playerListString.size());

                            boost::asio::write(*client_socket, boost::asio::buffer(&m, sizeof(Message)));

                            std::cout << "Sending client string of size " << std::to_string(playerListString.size())
                            << "\n";
                            std::cout << "Client string: \n" <<
                            playerListString << "\n";
                            
                            boost::asio::write(*client_socket, boost::asio::buffer(playerListString));
                            
                        }

                        
                        
                        if(message.type == MessageType::BlockSet) {
                            world.setBlock(BlockCoord{
                                static_cast<int>(message.x),
                                static_cast<int>(message.y),
                                static_cast<int>(message.z)}, 
                                static_cast<uint32_t>(message.info));
                            world._saveWorldToFile(worldPath);
                        }

                        if(message.type == MessageType::RequestSignsString) {

                            Message m = createMessage(MessageType::SignsString, 0, 0, 0, signWordsString.size());

                            boost::asio::write(*client_socket, boost::asio::buffer(&m, sizeof(Message)));

                            std::cout << "Sending sign string of size " << std::to_string(signWordsString.size())
                            << "\n";
                            std::cout << "Signs string: \n" <<
                            signWordsString << "\n";
                            
                            boost::asio::write(*client_socket, boost::asio::buffer(signWordsString));

                        }
                        
                        


                        
                        
                        if(message.type != MessageType::RequestWorldString && message.type != MessageType::RequestPlayerList && 
                        message.type != MessageType::Heartbeat   && message.type != MessageType::Disconnect && message.type  != MessageType::RequestSignsString && message.type  != MessageType::SignUpdate) {
                            if(message.type == MessageType::PlayerMove) {
                                //If its a player move, set the info field to the id
                                message.info = currentID;
                            }

                            std::vector<int> keysToRemove;
                            // Forward the message to all other clients
                            CLIENTS_LOCK.lock();
                            
                            for (auto&[key, value] : CLIENTS) {
                                if (id != key || MessageType::BlockSet == message.type) {
                                    auto inactive_duration = std::chrono::duration_cast<std::chrono::seconds>(now - value.last_active);
                                    if (inactive_duration.count() > 50) {
                                        keysToRemove.push_back(key);
                                    } else {
                                        boost::asio::write(*(value.socket), boost::asio::buffer(&message, sizeof(Message)));
                                    }
                                }
                            }

                            for(int& key : keysToRemove) {
                                CLIENTS.erase(key);
                                for (auto&[k, value] : CLIENTS) {
                                    //Tell the clients this guy's gone
                                    Message m = createMessage(MessageType::Disconnect, 0, 0, 0, key);
                                    boost::asio::write(*(value.socket), boost::asio::buffer(&m, sizeof(Message)));
                                }
                            }
                            CLIENTS_LOCK.unlock();
                        } else if (message.type == MessageType::Heartbeat) {

                            timeOfDayThreadFunction();



                            std::vector<int> keysToRemove;
                            CLIENTS_LOCK.lock();


                            Client& cli = CLIENTS.at(id);

                            bool moved = false;
                            if(cli.x != message.x || cli.y != message.y || cli.z != message.z) {
                                cli.x = message.x;
                                cli.y = message.y;
                                cli.z = message.z;
                                moved = true;
                            }

                            if(moved) {
                                for (const auto&[key, value] : CLIENTS) {
                                    if(key != id) {
                                        Message m = createMessage(MessageType::PlayerMove, cli.x, cli.y, cli.z, id, message.rot);
                                        boost::asio::write(*(value.socket), boost::asio::buffer(&m, sizeof(Message)));
                                        auto inactive_duration = std::chrono::duration_cast<std::chrono::seconds>(now - value.last_active);
                                        if (inactive_duration.count() > 50) {
                                            keysToRemove.push_back(key);
                                        }
                                    }
                                    
                                }
                            }    
                                

                            for(int& key : keysToRemove) {
                                CLIENTS.erase(key);
                                for (auto&[k, value] : CLIENTS) {
                                    //Tell the clients this guy's gone
                                    Message m = createMessage(MessageType::Disconnect, 0, 0, 0, key);
                                    boost::asio::write(*(value.socket), boost::asio::buffer(&m, sizeof(Message)));
                                }
                            }
                            CLIENTS_LOCK.unlock();
                        }

                }
            } catch (const std::exception &e) {
                std::cerr << "Error in connection thread: " << e.what() << "\n";
            }

            {
                std::lock_guard<std::mutex> lock(CLIENTS_LOCK);
                CLIENTS.erase(id);
            }

        }).detach();
    }

    inline static int currentID = 0;
    tcp::acceptor acceptor_;
};

std::mutex TIME_LOCK;

class TimeOfDay {
public:
    float time;
    TimeOfDay(float t, std::string p, TCPServer* serv) : time(t), path(p), servoMax(serv) {}
    void saveToFile() {
        std::ofstream file(path, std::ios::trunc);
        if(file.is_open()) {
            file << std::to_string(time) << "\n";
            file.close();
        }
    }
    void loadFromFile() {
        std::ifstream file(path);
        std::string line;
        if(file.is_open()) {
            while(std::getline(file, line)) {
                std::istringstream linestream(line);
                std::string word;
                int index = 0;
                while (linestream >> word) {
                    if(index == 0) {
                        time = std::stof(word);
                    }
                }
            }
            file.close();
        }
    }
    float getTime() {
        TIME_LOCK.lock();
        float t = time;
        TIME_LOCK.unlock();
        return t;
        
    }
    void setTime(float val) {
        TIME_LOCK.lock();
        time = std::fmod(val, dayLength);
        TIME_LOCK.unlock();
    }
    void sendTimeOut() {
        TIME_LOCK.lock();
        float t = time;
        TIME_LOCK.unlock();

        CLIENTS_LOCK.lock();
        for(auto &[key, val] : CLIENTS) {

            Message m = createMessage(MessageType::TimeUpdate, t, 0, 0, 0);

            boost::asio::write(*(val.socket), boost::asio::buffer(&m, sizeof(Message)));
        }
        CLIENTS_LOCK.unlock();
    }
private:
    std::string path;
    inline static float dayLength = 900.0f;
    TCPServer* servoMax;
};

TimeOfDay *timeOfDay;



TCPServer* servo;



std::atomic<bool> runTimeFunc = true;

float lastFrame = 0.0f;
float deltaDayTime = 0.0f;

void updateDayTime() {
    double currentFrame = glfwGetTime();
    deltaDayTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

float lastFrame2 = 0.0f;
float deltaTickTime = 0.0f;

void updateTickTime() {
    double currentFrame2 = glfwGetTime();
    deltaTickTime = currentFrame2 - lastFrame2;
    lastFrame2 = currentFrame2;
}



float timeTickTimer = 0.0f;
float timeTickInterval = 10.0f;

float mobTickTimer = 0.0f;
float mobTickInterval = 0.25f;


void timeOfDayThreadFunction() {

    updateTickTime();
    if(timeTickTimer > timeTickInterval) {
        float time = timeOfDay->getTime();
        updateDayTime();
        time += deltaDayTime;
        std::cout << "Updated time to " << std::to_string(time) << '\n';
        timeOfDay->setTime(time);
        timeOfDay->sendTimeOut();
        timeTickTimer = 0.0f;
    } else {
        timeTickTimer += deltaTickTime;
    }
    if(mobTickTimer > mobTickInterval) {
        std::vector<MobMsg> queue;
        auto mobView = world.registry.view<MobComponent>();
        for(const entt::entity e : mobView) {
            MobComponent &m = world.registry.get<MobComponent>(e);
            if(static_cast<float>(rand()) / RAND_MAX > 0.5f) {
                float changedX =  (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) -0.5f);
                float changedY = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)-0.5f);
                float changedZ = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)-0.5f);
                float changedRot = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) -0.5f;
                glm::vec3 change(changedX, changedY, changedZ);
                m.lpos = m.pos;
                m.pos += change;
                m.rot += changedRot;

                MobMsg mm;
                mm.flags = 0;
                mm.health = m.health;
                mm.id = m.id;
                mm.pos = m.pos;
                mm.rot = m.rot;
                mm.type = m.type;
                queue.push_back(mm);
            }
        }

        while(queue.size() > 0) {
            if(queue.size() >= MOBBATCHSIZE) {
                MobMsgBatch mmb = MobMsgBatch{
                    {
                        queue[queue.size()-1],
                        queue[queue.size()-2],
                        queue[queue.size()-3],
                        queue[queue.size()-4],
                        queue[queue.size()-5],
                        queue[queue.size()-6],
                        queue[queue.size()-7],
                        queue[queue.size()-8],
                    }
                };
                for(int i = 0; i < MOBBATCHSIZE; i++) {
                    queue.pop_back();
                }
                CLIENTS_LOCK.lock();
                for(auto &[key, val] : CLIENTS) {
                    Message m = createMessage(MessageType::MobUpdateBatch, 0, 0, 0, 0);
                    boost::asio::write(*(val.socket), boost::asio::buffer(&m, sizeof(Message)));

                    boost::asio::write(*(val.socket), boost::asio::buffer(&mmb, sizeof(MobMsgBatch)));
                }
                CLIENTS_LOCK.unlock();
            } else {
                MobMsg mm = queue[queue.size()-1];
                

                CLIENTS_LOCK.lock();
                for(auto &[key, val] : CLIENTS) {
                    Message m = createMessage(MessageType::MobUpdate, 0, 0, 0, 0);
                    boost::asio::write(*(val.socket), boost::asio::buffer(&m, sizeof(Message)));

                    boost::asio::write(*(val.socket), boost::asio::buffer(&mm, sizeof(MobMsg)));
                }

                queue.pop_back();
                CLIENTS_LOCK.unlock();
            }
        }

        mobTickTimer = 0.0f;
    } else {
        //auto mobView = world.registry.view<MobComponent>();

        mobTickTimer += deltaTickTime;
    }
    // while(runTimeFunc.load()) {
        
    //     std::this_thread::sleep_for(std::chrono::seconds(5));
        
        
    // }
}


int main() {

    glfwInit();

    world.loadOrCreateSaveGame(worldPath);
    loadSignWordsFromFile();

    newMob(MobType::Normal, glm::vec3(0,60,0));
    newMob(MobType::Normal, glm::vec3(8,50,0));
    newMob(MobType::Normal, glm::vec3(12,53,10));
    newMob(MobType::Normal, glm::vec3(0,50,0));
    newMob(MobType::Normal, glm::vec3(0,40,0));
    newMob(MobType::Normal, glm::vec3(53,60,5));
    newMob(MobType::Normal, glm::vec3(10,50,10));
    newMob(MobType::Normal, glm::vec3(6,50,20));
    newMob(MobType::Normal, glm::vec3(9,45,9));
    newMob(MobType::Normal, glm::vec3(10,60,15));

    try {
        io_context io_context;
        TCPServer server(&io_context, 6969);

        servo = &server;
        TimeOfDay tod(0.0f, std::string("time"), &server);
        timeOfDay = &tod;

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}