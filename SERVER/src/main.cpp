#include <boost/asio.hpp>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "voxelworld.h"

using boost::asio::ip::udp;

const char* worldPath = "world";

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
};




std::unordered_map<int, Client> CLIENTS;


std::string clientListToString(int excludedID) {
    std::ostringstream outputStream;

    for(auto &[key, val] : CLIENTS) {
        if(key != excludedID) {
            outputStream << key << " " << val.x << " " << val.y << " " << val.z << "\n";
        }
    }

    return outputStream.str();
}



std::vector<BlockChange> BLOCK_CHANGE_QUEUE;

std::mutex BLOCK_CHANGE_QUEUE_LOCK;
std::mutex FILE_LOCK;
std::mutex WORLD_LOCK;

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
    }
}

VoxelWorld world;

class UDPServer {
public:
    UDPServer(boost::asio::io_context& io_context, unsigned short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)) {
        start_receive();
    }

private:
    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(&recv_message_, sizeof(Message)), remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::cout << "Received: " << getMessageTypeString(recv_message_) << " from " << remote_endpoint_ << std::endl;
                    
                    bool playerMove = false;
                    glm::vec3 newPlayerPosition(0,0,0);

                    //Find the client in client map if we have it
                    auto it = std::find_if(CLIENTS.begin(), CLIENTS.end(), [this](const auto& pair) {
                        return pair.second.endpoint == this->remote_endpoint_;
                    });

                    int thisPlayersID = 0;

                    if (it != CLIENTS.end()) {
                        //Client is already in list. Update their position if we have new pos
                        if(playerMove == true) {
                            it->second.x = newPlayerPosition.x;
                            it->second.y = newPlayerPosition.y;
                            it->second.z = newPlayerPosition.z;
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
                                newPlayerPosition.z
                            });
                        } else {
                            CLIENTS.insert_or_assign(currentID, Client{
                                remote_endpoint_,
                                currentID,
                                0,
                                0,
                                0
                            });
                        }
                        thisPlayersID = currentID;
                        //IMPORTANT: INCREMENT THE ID
                        currentID++;
                    }


                    if(recv_message_.type == MessageType::RequestWorldString) {
                        Message m;
                        m.type = MessageType::WorldString;
                        m.x = 0; m.y = 0; m.z = 0; m.info = worldString.size();
                        socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), remote_endpoint_);

                        std::cout << "Sending world string of size " << std::to_string(worldString.size())
                        << "\n";
                        std::cout << "World string: \n" <<
                        worldString << "\n";
                        socket_.send_to(boost::asio::buffer(worldString), remote_endpoint_);
                    }


                    if(recv_message_.type == MessageType::RequestPlayerList) {
                        Message m;
                        m.type = MessageType::PlayerList;

                        std::string playerListString = clientListToString(thisPlayersID);

                        m.x = 0; m.y = 0; m.z = 0; m.info = playerListString.size();
                        socket_.send_to(boost::asio::buffer(&m, sizeof(Message)), remote_endpoint_);

                        std::cout << "Sending client string of size " << std::to_string(playerListString.size())
                        << "\n";
                        std::cout << "Client string: \n" <<
                        playerListString << "\n";

                        socket_.send_to(boost::asio::buffer(playerListString), remote_endpoint_);
                    }
                    
                    
                    if(recv_message_.type == MessageType::BlockSet) {
                        world.setBlock(BlockCoord{
                            static_cast<int>(recv_message_.x),
                            static_cast<int>(recv_message_.y),
                            static_cast<int>(recv_message_.z)}, 
                            static_cast<uint32_t>(recv_message_.info));
                        world._saveWorldToFile(worldPath);
                    }
                    if(recv_message_.type == MessageType::PlayerMove) {
                        playerMove = true;
                        newPlayerPosition = glm::vec3(
                            recv_message_.x,
                            recv_message_.y,
                            recv_message_.z
                        );
                    }

                    
                    
                    if(recv_message_.type != MessageType::RequestWorldString && recv_message_.type != MessageType::RequestPlayerList) {
                        if(recv_message_.type == MessageType::PlayerMove) {
                            //If its a player move, set the info field to the id
                            recv_message_.info = currentID;
                        }
                        // Forward the message to all other clients
                        for (const auto&[key, value] : CLIENTS) {
                            if (value.endpoint != remote_endpoint_) {
                                socket_.send_to(boost::asio::buffer(&recv_message_, sizeof(Message)), value.endpoint);
                            }
                        }
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

int main() {

    world.loadOrCreateSaveGame(worldPath);

    try {
        boost::asio::io_context io_context;
        UDPServer server(io_context, 12345);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}