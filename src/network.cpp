


#include "network.h"


std::vector<OtherPlayer> PLAYERS;

std::promise<void> receive_thread_promise;

bool rcvtpromisesat = false;

void UDPClient::start() {
    std::cout << "Client loop starting\n";
    shouldRunReceiveLoop.store(true);

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
    
}

void UDPClient::stop() {
    shouldRunReceiveLoop.store(false);
    socket_.cancel();
    socket_.close();
    // if(receive_thread.joinable()) {
    //     receive_thread.join(); 
    // }
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
            }
            if(wordIndex == 2) {
                thisPlayer.y = std::stof(word);
            }
            if(wordIndex == 3) {
                thisPlayer.z = std::stof(word);
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
    }
}

UDPClient::UDPClient(boost::asio::io_context& io_context, VoxelWorld *voxworld)
    : io_context_(io_context), socket_(io_context, udp::endpoint(udp::v4(), 0)), voxelWorld(voxworld) {

    

}

void UDPClient::connect() {
    try {

    socket_ = udp::socket(io_context_, udp::endpoint(udp::v4(), 0));
    // Hardcode the server's IP address and port
    std::string server_ip = "127.0.0.1"; // Change this to your server's IP address
    std::string server_port = "12345"; // Change this to your server's port

    udp::resolver resolver(io_context_);
    udp::resolver::results_type endpoints = resolver.resolve(udp::v4(), server_ip, server_port);
    server_endpoint_ = *endpoints.begin();
    std::cout << "Client connected\n";
    } catch (std::exception& e) {
        std::cout << "In connect: " << e.what() << "\n";
    }
}

void UDPClient::disconnect() {
    socket_.close();
}

void UDPClient::send(const Message& message) {
    socket_.send_to(boost::asio::buffer(&message, sizeof(Message)), server_endpoint_);
}

void UDPClient::receive() {
    udp::endpoint sender_endpoint;
    std::vector<char> recv_buffer(65536); // A large buffer, e.g., 64KB

    try {
        size_t length = socket_.receive_from(boost::asio::buffer(recv_buffer), sender_endpoint);
        if(rcvtpromisesat == false) {
            receive_thread_promise.set_value();
            rcvtpromisesat = true;
        }
            
            

        // Now you can process the received data, which is stored in recv_buffer and has a length of 'length' bytes
        // For example, if you're expecting a Message struct, you can check if 'length' is at least sizeof(Message)
        if (length == sizeof(Message)) {
            
            Message* message = reinterpret_cast<Message*>(recv_buffer.data());
            std::cout << "Received: " << getMessageTypeString(*message) << " from " << sender_endpoint << std::endl;

            std::filesystem::create_directories(std::filesystem::path("multiplayer"));

            if (message->type == MessageType::WorldString) {
                std::cout << "Expecting " << std::to_string(message->info) << " bytes\n";
                std::vector<char> ws_buffer(static_cast<size_t>(message->info) + 1, '\0');
                size_t len = socket_.receive_from(boost::asio::buffer(ws_buffer), sender_endpoint);
                std::string receivedString(ws_buffer.data(), len);
                std::cout << "World string: \n" <<
                    receivedString << "\n";
                std::ofstream outputFile("multiplayer/world.save", std::ios::trunc); // Change the file name as needed
                if (outputFile.is_open()) {
                    outputFile << receivedString;
                    outputFile.close();
                    std::cout << "Received world string of length " << std::to_string(len) << std::endl;
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
                size_t len = socket_.receive_from(boost::asio::buffer(ps_buffer), sender_endpoint);

                std::string receivedString(ps_buffer.data(), len);
                std::cout << "Players string: \n" <<
                    receivedString << "\n";

                clientStringToPlayerList(PLAYERS, receivedString);
            }

            if (message->type == MessageType::BlockSet) {
                //LOCK AND AFFECT THE VOXELWORLD
                voxelWorld->setBlockAndQueueRerender(BlockCoord{
                    static_cast<int>(message->x),
                    static_cast<int>(message->y),
                    static_cast<int>(message->z),
                    }, static_cast<uint32_t>(message->info));
            }
            if (message->type == MessageType::PlayerMove) {
                auto playerIt = std::find_if(PLAYERS.begin(), PLAYERS.end(), [message](OtherPlayer& play) {
                    return play.id == static_cast<int>(message->info);
                    });

                if (playerIt == PLAYERS.end()) {
                    PLAYERS.push_back(OtherPlayer{
                        static_cast<int>(message->info),
                        message->x,
                        message->y,
                        message->z
                        });
                }
                else {
                    playerIt->x = message->x;
                    playerIt->y = message->y;
                    playerIt->z = message->z;
                }
            }

            // Additional processing...
        }
        else {
            // Handle the case where the received data is not large enough to be a valid Message
            std::cout << "Not long enough to be Message\n";
        }
    }
    catch (std::exception& e) {
        std::cout << "From this? " << e.what() << "\n";
        stop();
    }


}
