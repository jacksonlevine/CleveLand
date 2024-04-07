


#include "network.h"


std::vector<OtherPlayer> PLAYERS;

std::promise<void> receive_thread_promise;

bool rcvtpromisesat = false;


std::mutex WRITE_MUTEX;

std::atomic<bool> PLAYERSCHANGED = false;

std::string TYPED_IN_SERVER_IP("");

uint32_t MY_ID = 0;


Message createMessage(MessageType type, float x, float y, float z, uint32_t info, float r) {
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



NameMessage createNameMessage(int id, std::string name, size_t length) {
    NameMessage msg = {0};
    int index = 0;
    for(char &c : name) {
        msg.data[index] = c;
        index++;
    }
    return msg;
}

void TCPClient::start() {
    std::filesystem::create_directories(std::filesystem::path("multiplayer"));
    std::cout << "Client loop starting\n";
    shouldRunReceiveLoop.store(true);
    shouldRunSendLoop.store(true);

    if (receive_thread.joinable()) {
        receive_thread.join();
    }

    receive_thread_promise = std::promise<void>();
    rcvtpromisesat = false;
    receive_thread = std::thread([this]() {
        try {
            while (this->shouldRunReceiveLoop.load()) {
                    this->receive();
            }
        } catch (...) {
            if(rcvtpromisesat == false) {
                receive_thread_promise.set_exception(std::current_exception());
                rcvtpromisesat = true;
            }
            
            shouldRunReceiveLoop.store(false);
        }
        std::cout << "Ended receive thread.\n";
    });

    send_thread = std::thread([this]() {

            while (shouldRunSendLoop.load()) {

                Message heartbeat = createMessage(MessageType::Heartbeat, cameraPos->x, cameraPos->y, cameraPos->z, 0, (*camRot).load());

                try {
                    send(heartbeat);
                } catch (std::exception& e) {
                    std::cerr << "Error sending heartbeat: " << e.what() << std::endl;
                }
                for (int i = 0; i < 1; i++) {
                    if (shouldRunSendLoop.load()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    }
                }
                    
                
            }

            std::cout << "Ended send thread \n";
        });
    
}

void TCPClient::stop() {
    shouldRunReceiveLoop.store(false);
    shouldRunSendLoop.store(false);
    
    socket_.cancel();
    socket_.close();

    if (send_thread.joinable()) {
        send_thread.join();
    }

    if(receive_thread.joinable()) {
        receive_thread.join(); 
    }
}


void clientStringToPlayerList(std::vector<OtherPlayer> &out, std::string in) {
    out.clear();
    std::istringstream stream(in);

    std::string line;
    while(std::getline(stream, line)) {
        //Each line is a player
        std::istringstream linestream(line);
        std::string word;

        int wordIndex = 0;
        OtherPlayer thisPlayer;
        while(linestream >> word) {
            if(wordIndex == 0) {
                thisPlayer.id = std::stoi(word);
            }
            if(wordIndex == 1) {
                thisPlayer.x = std::stof(word);
                thisPlayer.lx = std::stof(word);
            }
            if(wordIndex == 2) {
                thisPlayer.y = std::stof(word);
                thisPlayer.ly = std::stof(word);
            }
            if(wordIndex == 3) {
                thisPlayer.z = std::stof(word);
                thisPlayer.lz = std::stof(word);
            }
            if(wordIndex == 5) {
                thisPlayer.name = word;
            }
            wordIndex++;
        }
        out.push_back(thisPlayer);
    }
}




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
    }
}

TCPClient::TCPClient(boost::asio::io_context& io_context, VoxelWorld *voxworld, std::function<void(float)> *gameTimeSet, glm::vec3 *cameraPos, std::atomic<float> * camRot)
    : io_context_(io_context), socket_(io_context), voxelWorld(voxworld), setGameTime(gameTimeSet), cameraPos(cameraPos), camRot(camRot) {

    

}

void TCPClient::connect() {
    try {

    
    std::cout << "Connecting to: " << TYPED_IN_SERVER_IP << "\n";
    size_t colonPos = TYPED_IN_SERVER_IP.find(":");
    std::string server_ip = TYPED_IN_SERVER_IP.substr(0, colonPos);
    std::string server_port = TYPED_IN_SERVER_IP.substr(colonPos + 1);


    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(server_ip, server_port);

    if (endpoints.empty()) {
    // Resolution failed
    std::cout << "Failed to resolve server IP and port." << std::endl;
    } else {
        // Resolution succeeded, print resolved endpoints
        std::cout << "Resolved endpoints:" << std::endl;
        for (const auto& endpoint : endpoints) {
            std::cout << endpoint.endpoint().address().to_string() << ":" << endpoint.endpoint().port() << std::endl;
        }
    }

    boost::asio::connect(socket_, endpoints);
    
    std::cout << "Client connected\n";
    } catch (std::exception& e) {
        std::cout << "In connect: " << e.what() << "\n";
    }
}

void TCPClient::disconnect() {
    socket_.close();
}

void TCPClient::send(const Message& message) {
    std::cout << "Sending " << getMessageTypeString(static_cast<Message>(message)) << "\n";
    std::lock_guard<std::mutex> lock(WRITE_MUTEX);
    boost::asio::write(socket_, boost::asio::buffer(&message, sizeof(Message)));
}

void TCPClient::processMessage(Message* message) {
    
            if(message->type == MessageType::TellYouYourID) {
                MY_ID = message->info;
            }

            if (message->type == MessageType::WorldString) {
                std::cout << "Expecting " << std::to_string(message->info) << " bytes\n";
                std::vector<char> ws_buffer(static_cast<size_t>(message->info) + 1, '\0');

                boost::asio::read(socket_, boost::asio::buffer(ws_buffer, message->info));

                std::string receivedString(ws_buffer.data(), message->info);
                
                std::cout << "World string: \n" <<
                    receivedString << "\n";
                std::ofstream outputFile("multiplayer/world.save", std::ios::trunc); // Change the file name as needed
                if (outputFile.is_open()) {
                    outputFile << receivedString;
                    outputFile.close();
                    std::cout << "Received world string of length " << std::to_string(message->info) << std::endl;
                    receivedWorld.store(true);
                    outputFile.close();
                }
                else {
                    std::cerr << "Failed to open file for writing." << std::endl;
                }
                
            }

            if (message->type == MessageType::PlayerList) {
                std::cout << "Expecting " << std::to_string(message->info) << " bytes\n";
                std::vector<char> ps_buffer(static_cast<size_t>(message->info) + 1, '\0');
                boost::asio::read(socket_, boost::asio::buffer(ps_buffer, message->info));

                std::string receivedString(ps_buffer.data(), message->info);
                std::cout << "Players string: \n" <<
                    receivedString << "\n";

                clientStringToPlayerList(PLAYERS, receivedString);
            }

            if (message->type == MessageType::BlockSet) {
                //LOCK AND AFFECT THE VOXELWORLD
                std::cout << "Block set at " << std::to_string(message->x) << " " << std::to_string(message->y) << " " << std::to_string(message->z) << " of type " << std::to_string(message->info) << "\n";
                voxelWorld->setBlockAndQueueRerender(BlockCoord{
                    static_cast<int>(message->x),
                    static_cast<int>(message->y),
                    static_cast<int>(message->z),
                    }, static_cast<uint32_t>(message->info));
            }

            if (message->type == MessageType::TimeUpdate) {
                (*setGameTime)(message->x);
                //std::cout << "Updated time to " << std::to_string(message->x) << "\n";
            }
            
            if (message->type == MessageType::Disconnect) {
                int idToRemove = message->info;
                auto it = std::find_if(PLAYERS.begin(), PLAYERS.end(), [idToRemove](const OtherPlayer& player) {
                    return player.id == idToRemove;
                });
                if (it != PLAYERS.end()) {
                    PLAYERS.erase(it);
                }
            }

            if (message->type == MessageType::PlayerMove) {
                auto playerIt = std::find_if(PLAYERS.begin(), PLAYERS.end(), [message](OtherPlayer& play) {
                    return play.id == static_cast<int>(message->info);
                    });

                if (playerIt == PLAYERS.end()) {
                    PLAYERS.push_back(
                        {
                        static_cast<int>(message->info),
                        message->x,
                        message->y,
                        message->z,

                        message->x,
                        message->y,
                        message->z,

                        0.0f,

                        message->rot,
                        message->rot,
                        std::string("Brewhaha")

                        });
                    
                }
                else {
                    playerIt->lx = playerIt->x;
                    playerIt->ly = playerIt->y;
                    playerIt->lz = playerIt->z;

                    playerIt->x = message->x;
                    playerIt->y = message->y;
                    playerIt->z = message->z;

                    playerIt->t = 0.0f;

                    playerIt->lrot = playerIt->rot;
                    playerIt->rot = message->rot;
                }
                PLAYERSCHANGED.store(true);
                //printf("Player %i moved to %f %f %f", message->info, message->x, message->y, message->z);
            }
}



void TCPClient::receive() {

    try {


        Message message;

        boost::asio::read(socket_, boost::asio::buffer(&message, sizeof(Message)));


        if(rcvtpromisesat == false) {
            receive_thread_promise.set_value();
            rcvtpromisesat = true;
        }
            
            
        if(message.type != PlayerMove && message.type != TimeUpdate) {
            std::cout << "Received: " << getMessageTypeString(message) << std::endl;
        }


        processMessage(&message);




    }
    catch (boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            std::cout << "EOF encountered, continuing receive loop." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            return; // Continue the receive loop
        }

        std::cout << "Boost System Error: " << e.what() << "\n";

        //stop();
    }
    catch (std::exception& e) {
        std::cout << "From this? " << e.what() << "\n";

        //stop();
    }


}
