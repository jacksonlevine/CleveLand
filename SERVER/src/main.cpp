#include <boost/asio.hpp>
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
#include "acks.h"

using UUID = boost::uuids::uuid;
using boost::asio::ip::udp;

const char* worldPath = "world";

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
    RequestSeq
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

struct BlockChange {
    int x;
    int y;
    int z;
    uint32_t block;
};

void printHex(const std::string& str) {
    for (char c : str) {
        std::printf("%02x ", static_cast<unsigned char>(c));
    }
    std::cout << std::endl;
}


struct Client {
    udp::endpoint endpoint;
    int id;
    float x;
    float y;
    float z;
    std::chrono::steady_clock::time_point last_active;
    std::string name;
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


Message createMessage(MessageType type, float x, float y, float z, uint32_t info, int seq) {
    Message msg;
    boost::uuids::random_generator generator;
    msg.goose = generator();
    msg.type = type;
    msg.x = x;
    msg.y = y;
    msg.z = z;
    msg.info = info;
    msg.sequence = seq;
    return msg;
}

NameMessage createNameMessage(int id, std::string name, size_t length) {
    NameMessage msg = {0};
    int index = 0;
    for(char &c : name) {
        msg.data[index] = c;
        index++;
    }
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
        case MessageType::Acknowledge:
            return std::string("Acknowledge");
        case MessageType::RequestSeq:
            return std::string("RequestSeq");
    }
}



VoxelWorld world;

std::chrono::steady_clock::time_point lastTimeCheckedForOldUsers = std::chrono::steady_clock::now();

class UDPServer {
public:
    UDPServer(boost::asio::io_context& io_context, unsigned short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)) {
        start_receive();
    }

    void start_receive() {
        static std::vector<char> recv_buffer(65000);
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer), remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                        auto now = std::chrono::steady_clock::now();
                      //Find the client in client map if we have it
                      int thisPlayersID = 0;
                      try {
                        CLIENTS_LOCK.lock();
                        auto it = std::find_if(CLIENTS.begin(), CLIENTS.end(), [this](const auto& pair) {
                            return pair.second.endpoint == this->remote_endpoint_;
                        });


                        

                        if (it != CLIENTS.end()) {

                            thisPlayersID = it->second.id;
                        } else {
                            // Add the sender to the map of clients with a dummy position or a real position
                            // If we have it
                            
                                CLIENTS.insert_or_assign(currentID, Client{
                                    remote_endpoint_,
                                    currentID,
                                    0,
                                    0,
                                    0,
                                    now,
                                    std::string("")
                                });

                            thisPlayersID = currentID;
                            //IMPORTANT: INCREMENT THE ID
                            currentID++;
                        }
                        CLIENTS_LOCK.unlock();
                      } catch (std::exception& e) {
                        std::cout << e.what() << "\n";
                      }
                        


                    if(bytes_recvd == sizeof(Message)) {

                        

                        std::memcpy(&recv_message_, recv_buffer.data(), sizeof(Message));
                        
                        
                        std::cout << "Received: " << getMessageTypeString(recv_message_) << " from " << remote_endpoint_ << std::endl;
                        
                        bool playerMove = false;
                        glm::vec3 newPlayerPosition(0,0,0);

                       
                        if(recv_message_.type == MessageType::PlayerMove) {
                            playerMove = true;
                            newPlayerPosition = glm::vec3(
                                recv_message_.x,
                                recv_message_.y,
                                recv_message_.z
                            );
                        }

                         //Find the client in client map if we have it
                        CLIENTS_LOCK.lock();
                        auto it = std::find_if(CLIENTS.begin(), CLIENTS.end(), [this](const auto& pair) {
                            return pair.second.endpoint == this->remote_endpoint_;
                        });



                        if (it != CLIENTS.end()) {
                            //Client is already in list. Update their position if we have new pos
                            if(playerMove == true) {
                                it->second.x = newPlayerPosition.x;
                                it->second.y = newPlayerPosition.y;
                                it->second.z = newPlayerPosition.z;
                                it->second.last_active = now;
                            }
                            thisPlayersID = it->second.id;
                        } else {
                            // Add the sender to the map of clients with a dummy position or a real position
                            // If we have it
                            if(playerMove == true) {
                                CLIENTS.insert_or_assign(currentID, Client{
                                    remote_endpoint_,
                                    currentID,
                                    newPlayerPosition.x,
                                    newPlayerPosition.y,
                                    newPlayerPosition.z,
                                    now,
                                    std::string("")
                                });
                            } else {
                                CLIENTS.insert_or_assign(currentID, Client{
                                    remote_endpoint_,
                                    currentID,
                                    0,
                                    0,
                                    0,
                                    now,
                                    std::string("")
                                });
                            }
                            thisPlayersID = currentID;
                            //IMPORTANT: INCREMENT THE ID
                            currentID++;
                        }
                        CLIENTS_LOCK.unlock();

                        


                        if(recv_message_.type == MessageType::RequestWorldString) {

                            COMMUNICATION_LOCK.lock();

                            Message m = createMessage(MessageType::WorldString, 0, 0, 0, worldString.size(), 0);


                            socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), remote_endpoint_);

                            std::cout << "Sending world string of size " << std::to_string(worldString.size())
                            << "\n";
                            // std::cout << "World string: \n" <<
                            // worldString << "\n";
                            socket_.send_to(boost::asio::buffer(worldString), remote_endpoint_);
                            COMMUNICATION_LOCK.unlock();
                        }


                        if(recv_message_.type == MessageType::RequestPlayerList) {


                            COMMUNICATION_LOCK.lock();
                            
                            std::string playerListString = clientListToString(thisPlayersID);

                            Message m = createMessage(MessageType::PlayerList, 0, 0, 0, playerListString.size(),  SEQUENCE_NUM.load());
                            SEQUENCE_NUM.store(expectedNextSeq( SEQUENCE_NUM.load()));

                            socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), remote_endpoint_);

                            std::cout << "Sending client string of size " << std::to_string(playerListString.size())
                            << "\n";
                            std::cout << "Client string: \n" <<
                            playerListString << "\n";

                            socket_.send_to(boost::asio::buffer(playerListString), remote_endpoint_);
                            COMMUNICATION_LOCK.unlock();
                        }

                        if(recv_message_.type == MessageType::Acknowledge) {
                            reportSeenMessage(recv_message_.goose, thisPlayersID);
                            CLIENTS_LOCK.lock();
                            bool allIn = true;
                            for(auto &[key, val] : CLIENTS)
                            {
                                if(!hasSeen(recv_message_.goose, key)) {
                                    allIn = false;
                                }
                            }
                            if(allIn) {
                                resolveMessage(recv_message_.goose);
                            }                            
                            CLIENTS_LOCK.unlock();
                        }
                        
                        
                        if(recv_message_.type == MessageType::BlockSet) {
                            world.setBlock(BlockCoord{
                                static_cast<int>(recv_message_.x),
                                static_cast<int>(recv_message_.y),
                                static_cast<int>(recv_message_.z)}, 
                                static_cast<uint32_t>(recv_message_.info));
                            world._saveWorldToFile(worldPath);
                        }
                        

                        if(recv_message_.type == MessageType::RequestSeq) {
                            int seq = recv_message_.info;

                            std::vector<char> data;
                            data.resize(sizeof(Message));
                            if(getBySeq(seq, data.begin())) {
                                socket_.send_to(boost::asio::buffer(data, sizeof(Message)), remote_endpoint_);
                            }


                        }

                        
                        
                        if(recv_message_.type != MessageType::RequestWorldString && recv_message_.type != MessageType::RequestPlayerList && 
                        recv_message_.type != MessageType::Heartbeat && recv_message_.type != MessageType::Acknowledge  && recv_message_.type != MessageType::Disconnect && recv_message_.type != MessageType::RequestSeq) {
                            if(recv_message_.type == MessageType::PlayerMove) {
                                //If its a player move, set the info field to the id
                                recv_message_.info = currentID;
                            }

                                reportMessage(recv_message_.goose, recv_buffer, sizeof(Message), thisPlayersID);

                                


                            std::vector<int> keysToRemove;
                            // Forward the message to all other clients
                            CLIENTS_LOCK.lock();
                            
                            for (auto&[key, value] : CLIENTS) {
                                if (value.endpoint != remote_endpoint_) {
                                    auto inactive_duration = std::chrono::duration_cast<std::chrono::seconds>(now - value.last_active);
                                    if (inactive_duration.count() > 30) {
                                        keysToRemove.push_back(key);
                                    } else {
                                        recv_message_.sequence = SEQUENCE_NUM.load();
                                        SEQUENCE_NUM.store(expectedNextSeq(SEQUENCE_NUM.load()));
                                        socket_.send_to(boost::asio::buffer(&recv_message_, sizeof(Message)), value.endpoint);
                                    }
                                }
                            }

                            for(int& key : keysToRemove) {
                                CLIENTS.erase(key);
                                for (auto&[k, value] : CLIENTS) {
                                    //Tell the clients this guy's gone
                                    Message m = createMessage(MessageType::Disconnect, 0, 0, 0, key, SEQUENCE_NUM.load());
                                    SEQUENCE_NUM.store(expectedNextSeq(SEQUENCE_NUM.load()));
                                    socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), value.endpoint);
                                }
                            }
                            CLIENTS_LOCK.unlock();
                        } else if (recv_message_.type == MessageType::Heartbeat) {
                            std::vector<int> keysToRemove;
                            CLIENTS_LOCK.lock();
                            for (const auto&[key, value] : CLIENTS) {
                                auto inactive_duration = std::chrono::duration_cast<std::chrono::seconds>(now - value.last_active);
                                if (inactive_duration.count() > 50) {
                                    keysToRemove.push_back(key);
                                }
                            }
                            for(int& key : keysToRemove) {
                                CLIENTS.erase(key);
                                for (auto&[k, value] : CLIENTS) {
                                    //Tell the clients this guy's gone
                                    Message m = createMessage(MessageType::Disconnect, 0, 0, 0, key,  SEQUENCE_NUM.load());
                                     SEQUENCE_NUM.store(expectedNextSeq( SEQUENCE_NUM.load()));
                                    socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), value.endpoint);
                                }
                            }
                            CLIENTS_LOCK.unlock();
                        }


                        CLIENTS_LOCK.lock();
                        for( auto&[key, val] : CLIENTS) {
                            
                            for( auto&[k, v] : BUREAU) {
                                if(!hasSeen(k, key)) {
                                    std::cout << "This guy " << val.name << " didn't get " << k << ", sending now.\n";
                                    std::vector<char> buf(sizeof(Message));
                                    buf.resize(sizeof(Message));
                                    BUREAU_LOCK.lock();
                                    std::copy(ARCHIVE.at(k).archive.begin(), ARCHIVE.at(k).archive.end(), buf.begin());
                                    socket_.send_to(boost::asio::buffer(buf, sizeof(Message)), val.endpoint);
                                    BUREAU_LOCK.unlock();
                                }
                            }
                            
                        }
                        CLIENTS_LOCK.unlock();
                    } 
                    else 
                    if (bytes_recvd == sizeof(NameMessage)) {
                        std::cout << "Size of name message \n";
                        NameMessage *recv_name = reinterpret_cast<NameMessage*>(recv_buffer.data());
                        std::string name(recv_name->data, recv_name->length);
                        std::cout << "Received name " << name << "\n";
                        CLIENTS_LOCK.lock();
                        CLIENTS.at(thisPlayersID).name = name;

                        NameMessage m = createNameMessage(thisPlayersID, name, name.size());
                        m.goose = recv_name->goose;

                        for (const auto&[k, value] : CLIENTS) {
                            //Tell the clients this guy's name
                            if(value.endpoint != remote_endpoint_) {
                                socket_.send_to(boost::asio::buffer(&m, sizeof(NameMessage)), value.endpoint);
                            }
                            
                        }
                        CLIENTS_LOCK.unlock();
                    }

                    
                }
                start_receive();
            });
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    Message recv_message_;
    inline static int currentID = 0;
};

std::mutex TIME_LOCK;

class TimeOfDay {
public:
    float time;
    TimeOfDay(float t, std::string p, UDPServer* serv) : time(t), path(p), servoMax(serv) {}
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

            Message m = createMessage(MessageType::TimeUpdate, t, 0, 0, 0,  SEQUENCE_NUM.load());
             SEQUENCE_NUM.store(expectedNextSeq( SEQUENCE_NUM.load()));
            servoMax->socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), val.endpoint);
        }
        CLIENTS_LOCK.unlock();
    }
private:
    std::string path;
    inline static float dayLength = 900.0f;
    UDPServer* servoMax;
};

TimeOfDay *timeOfDay;



UDPServer* servo;



std::atomic<bool> runTimeFunc = true;

float lastFrame = 0.0f;
float deltaTime = 0.0f;

void updateTime() {
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}


float timeTickTimer = 0.0f;
void timeOfDayThreadFunction() {

    while(runTimeFunc.load()) {
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        float time = timeOfDay->getTime();
        updateTime();
        time += deltaTime;
        std::cout << "Updated time to " << std::to_string(time) << '\n';
        timeOfDay->setTime(time);
        timeOfDay->sendTimeOut();

        

        std::vector<int> keysToRemove;
        CLIENTS_LOCK.lock();
        for (const auto&[key, value] : CLIENTS) {
            auto inactive_duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - value.last_active);
            if (inactive_duration.count() > 50) {
                keysToRemove.push_back(key);
            }
        }

        for(int& key : keysToRemove) {
            CLIENTS.erase(key);
            for (auto&[k, value] : CLIENTS) {
                //Tell the clients this guy's gone
                Message m = createMessage(MessageType::Disconnect, 0, 0, 0, key,  SEQUENCE_NUM.load());
                     SEQUENCE_NUM.store( expectedNextSeq( SEQUENCE_NUM.load()));
                servo->socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), value.endpoint);
            }
        }
        CLIENTS_LOCK.unlock();

    }
}



int main() {

    

    glfwInit();

    world.loadOrCreateSaveGame(worldPath);

    try {
        boost::asio::io_context io_context;
        UDPServer server(io_context, 12345);
        servo = &server;
        TimeOfDay tod(0.0f, std::string("time"), &server);
        timeOfDay = &tod;
        std::thread timeThread(
            timeOfDayThreadFunction
        );
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}