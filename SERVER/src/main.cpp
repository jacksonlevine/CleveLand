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
// #include "acks.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using UUID = boost::uuids::uuid;

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
    tcp::socket socket;
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


Message createMessage(MessageType type, float x, float y, float z, uint32_t info) {
    Message msg;
    boost::uuids::random_generator generator;
    msg.goose = generator();
    msg.type = type;
    msg.x = x;
    msg.y = y;
    msg.z = z;
    msg.info = info;
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

class TCPServer {
public:
    TCPServer(io_context& io_context, unsigned short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }

private:
    void start_accept() {
        // Create a new socket for the incoming connection
        tcp::socket socket(acceptor_.get_executor().context());

        // Asynchronously accept a new connection
        acceptor_.async_accept(socket, [this, &socket](const boost::system::error_code& error) {
            if (!error) {
                // Successfully accepted a new connection, handle it
                handle_connection(std::move(socket));
            }

            // Continue accepting new connections
            start_accept();
        });
    }

    void handle_connection(tcp::socket socket) {

        Client client{
            std::move(socket),
            currentID,
            0,
            0,
            0,
            std::chrono::steady_clock::now(),
            std::string("Brewhaha")
        };

        currentID++;

        {
            std::lock_guard<std::mutex> lock(CLIENTS_LOCK);
            CLIENTS[client.id] = std::move(client);
        }

        // Create a new thread to handle the connection
        std::thread([this, id = client.id]() mutable {
            try {
                bool run = true;
                while(run) {

                    Message message;

                    tcp::socket* client_socket;
                    {
                        std::lock_guard<std::mutex> lock(CLIENTS_LOCK);
                        client_socket = &(CLIENTS[id].socket);
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

                            boost::asio::write(client_socket, boost::asio::buffer(&m, sizeof(Message)));


                            std::cout << "Sending world string of size " << std::to_string(worldString.size())
                            << "\n";
                            // std::cout << "World string: \n" <<
                            // worldString << "\n";
                            boost::asio::write(client_socket, boost::asio::buffer(worldString));
                        }


                        if(message.type == MessageType::RequestPlayerList) {


                            
                            std::string playerListString = clientListToString(id);

                            Message m = createMessage(MessageType::PlayerList, 0, 0, 0, playerListString.size());

                            boost::asio::write(client_socket, boost::asio::buffer(&m, sizeof(Message)));

                            std::cout << "Sending client string of size " << std::to_string(playerListString.size())
                            << "\n";
                            std::cout << "Client string: \n" <<
                            playerListString << "\n";
                            
                            boost::asio::write(client_socket, boost::asio::buffer(playerListString));
                            
                        }

                        if(message.type == MessageType::Acknowledge) {
                        
                        }
                        
                        
                        if(message.type == MessageType::BlockSet) {
                            world.setBlock(BlockCoord{
                                static_cast<int>(message.x),
                                static_cast<int>(message.y),
                                static_cast<int>(message.z)}, 
                                static_cast<uint32_t>(message.info));
                            world._saveWorldToFile(worldPath);
                        }
                        

                        if(message.type == MessageType::RequestSeq) {
                        

                        }

                        
                        
                        if(message.type != MessageType::RequestWorldString && message.type != MessageType::RequestPlayerList && 
                        message.type != MessageType::Heartbeat && message.type != MessageType::Acknowledge  && message.type != MessageType::Disconnect && message.type != MessageType::RequestSeq) {
                            if(message.type == MessageType::PlayerMove) {
                                //If its a player move, set the info field to the id
                                message.info = currentID;
                            }

                            std::vector<int> keysToRemove;
                            // Forward the message to all other clients
                            CLIENTS_LOCK.lock();
                            
                            for (auto&[key, value] : CLIENTS) {
                                if (id != key) {
                                    auto inactive_duration = std::chrono::duration_cast<std::chrono::seconds>(now - value.last_active);
                                    if (inactive_duration.count() > 50) {
                                        keysToRemove.push_back(key);
                                    } else {
                                        //DISTRIB!
                                        boost::asio::write(value.socket, boost::asio::buffer(&message, sizeof(Message)));
                                    }
                                }
                            }

                            for(int& key : keysToRemove) {
                                CLIENTS.erase(key);
                                for (auto&[k, value] : CLIENTS) {
                                    //Tell the clients this guy's gone
                                    Message m = createMessage(MessageType::Disconnect, 0, 0, 0, key);
                                    boost::asio::write(value.socket, boost::asio::buffer(&m, sizeof(Message)));
                                }
                            }
                            CLIENTS_LOCK.unlock();
                        } else if (message.type == MessageType::Heartbeat) {
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
                                    Message m = createMessage(MessageType::Disconnect, 0, 0, 0, key);
                                    boost::asio::write(value.socket, boost::asio::buffer(&m, sizeof(Message)));
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



int main() {
    try {
        io_context io_context;
        TCPServer server(io_context, 12345);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}