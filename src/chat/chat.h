#ifndef CHAT_H

#define CHAT_H



#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <portaudio.h>
#include <opus/opus.h>
#include <atomic>


#include "ringbuffer.h"

#include "uuid.h"

#include "chatstuff.h"



using boost::asio::ip::udp;
using boost::asio::io_context;






//PortAudio settings
constexpr int FRAMESPERBUFFER = 480;
constexpr int SAMPLERATE = 48000;

//Network settings
inline std::string SERVER_PORT = "6970";

inline int PREFERRED_INPUT_DEVICE = 0;
inline int PREFERRED_OUTPUT_DEVICE = 0;

struct AudioPacket {
    uint32_t servId{};
    uint8_t sequenceNumber{};
    uint8_t incomingBytes{};
    std::string id;
    unsigned char buffer[128]{};
};

struct Person {
    std::string id;
    uint32_t servId{};
    OpusDecoder *decoder;
    RingBuffer rbuf;
    int opusErr{};
    int deleteTimer;
    udp::endpoint endpoint;
    Person();
};

inline Person::Person() {
    this->rbuf = RingBuffer(480, 1);
    this->decoder = opus_decoder_create(SAMPLERATE, 1, &this->opusErr);
    this->deleteTimer = 0;
}

inline std::vector<Person> allPeople;







// const size_t BUFFER_SIZE = 1024;
// char recv_buffer[BUFFER_SIZE];
inline udp::socket* client_socket;
inline udp::resolver::results_type server_endpoints;

inline udp::endpoint remote_endpoint;
inline AudioPacket recvPacket;

inline io_context context;

inline PaStream *sendingStream;
inline PaStream *receivingStream;

//Opus stuffies
inline OpusEncoder *theFuckinEncoderDude;

PaError startupPortAudio(PaStream *stream, PaStreamCallback *streamCallback, bool input);
PaError stopChatStreams(PaStream *stream);
void printDeviceInfo(PaDeviceIndex index);
void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred);

inline uint8_t mySequenceNumber = 0;
inline std::string myID = get_uuid();



 
inline std::atomic<bool> runChatThread = false;

static int sendingAudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    const auto in = static_cast<const float*>(inputBuffer);
    //float *out = (float*) outputBuffer;

    // for (unsigned long i = 0; i < framesPerBuffer; ++i) { 
    //     *out++ = *in++;  // Simple pass-through
    // }
    if(runChatThread.load()) {
         AudioPacket packet;
        packet.sequenceNumber = mySequenceNumber;
        packet.id = myID;
        packet.servId = MY_ID;

        const opus_int32 encodedLength = opus_encode_float(
            theFuckinEncoderDude,
            in,
            FRAMESPERBUFFER,
            packet.buffer,
            128
        );

        packet.incomingBytes = static_cast<uint8_t>(encodedLength);

        auto sendBuf = boost::asio::buffer(&packet, sizeof(packet));

        client_socket->async_send_to(sendBuf, *server_endpoints.begin(), handle_send);

        mySequenceNumber = (mySequenceNumber + 1) % 255;

    }
   
    return paContinue;
}


static int receivingAudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    auto *out = static_cast<float*>(outputBuffer);

    if(runChatThread.load()) {

        float mixdown[480] = {};

        for (Person& person : allPeople) {

            float volume = 0.0f;

            volume = volumeByProximity(person.servId);



            if(person.rbuf.count.load() > 1) {
                float thisPersonsBuf[480];
                person.rbuf.readOneBuffer(thisPersonsBuf);
                for(int i = 0; i < 480; ++i) {
                    mixdown[i] = std::min(std::max(mixdown[i] + thisPersonsBuf[i] * volume, -1.0f), 1.0f);
                }
                person.deleteTimer = 0;
            } else {
                person.deleteTimer++;
            }
        }

        std::erase_if(
            allPeople,
            [](const Person& person) {
                return person.deleteTimer > 50;  // Condition to remove the person
            }
        );

        std::ranges::copy(mixdown, out);
    }

    return paContinue;
}



inline void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        //std::cout << "Message sent" << std::endl;
    } else {
        std::cerr << "Error sending message: " << error.message() << std::endl;
    }
}

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);


inline void start_receive() {
    //printf("Started receive");
    client_socket->async_receive_from(
        boost::asio::buffer(&recvPacket, sizeof(recvPacket)), remote_endpoint,
        handle_receive);
}

inline void promptForChoices()
{
    std::cout << "Please enter the number corresponding to the input device you want to use:" << '\n';

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(numDevices) << '\n';
    } else {
        for (int i = 0; i < numDevices; i++) {
            if (const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i); deviceInfo && deviceInfo->maxInputChannels > 0) {
                std::cout << std::to_string(i) << ": " << deviceInfo->name << '\n'; // Debug output
            }
        }
    }

    int inputDeviceIndex;
    std::cin >> inputDeviceIndex;

    if (inputDeviceIndex >= 0 && inputDeviceIndex < numDevices) {
        const PaDeviceInfo* selectedDeviceInfo = Pa_GetDeviceInfo(inputDeviceIndex);
        if (selectedDeviceInfo && selectedDeviceInfo->maxInputChannels > 0) {
            PREFERRED_INPUT_DEVICE = inputDeviceIndex;
        } else {
            std::cerr << "Invalid selection for input device." << '\n';
            promptForChoices();
        }
    } else {
        std::cerr << "Invalid device index." << '\n';
        promptForChoices();
    }

    std::cout << "Please enter the number corresponding to the output device you want to use:" << '\n';

    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo && deviceInfo->maxOutputChannels > 0) {  // Check for output channels
            std::cout << std::to_string(i) << ": " << deviceInfo->name << '\n';
        }
    }

    int outputDeviceIndex;
    std::cin >> outputDeviceIndex;

    if (outputDeviceIndex >= 0 && outputDeviceIndex < numDevices) {
        const PaDeviceInfo* selectedOutputDeviceInfo = Pa_GetDeviceInfo(outputDeviceIndex);
        if (selectedOutputDeviceInfo && selectedOutputDeviceInfo->maxOutputChannels > 0) {
            PREFERRED_OUTPUT_DEVICE = outputDeviceIndex;
        } else {
            std::cerr << "Invalid selection for output device." << '\n';
            promptForChoices();
        }
    } else {
        std::cerr << "Invalid device index." << '\n';
        promptForChoices();
    }

    std::cout << "Enter the server IP without the port:" << '\n';

    //std::cin >> SERVER_IP;

    std::cout << "Enter the port:" << '\n';

    std::cin >> SERVER_PORT;

    std::cout << "Connecting..." << '\n';
}


inline void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error && bytes_transferred == sizeof(AudioPacket)) {

        auto personIt = std::ranges::find_if(allPeople
                                             ,
                                             [](const Person& person) {
                                                 return person.id == recvPacket.id;
                                             }
        );

        if(personIt == allPeople.end()) {
            Person newPerson;
            newPerson.id = recvPacket.id;
            newPerson.servId = recvPacket.servId;
            newPerson.endpoint = remote_endpoint;
            allPeople.push_back(newPerson);
            personIt = std::prev(allPeople.end());
        } else {
            personIt->servId = recvPacket.servId;
        }

        float newBuf[480];

        opus_decode_float(
            personIt->decoder,
            recvPacket.buffer,
            recvPacket.incomingBytes,
            newBuf,
            480,
            0
        );

        personIt->rbuf.write(newBuf,480);
    }
    if(runChatThread.load()) {
        start_receive(); // Start another async receive
    }
}

inline PaError startupPortAudio(PaStream *stream, PaStreamCallback *streamCallback, bool input) {

    ;// = Pa_Initialize();

    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;

    inputParameters.channelCount = 1;
    inputParameters.device = Pa_GetDefaultInputDevice();
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    outputParameters.channelCount = 1;
    outputParameters.device = Pa_GetDefaultOutputDevice();;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, SAMPLERATE, FRAMESPERBUFFER, paClipOff, streamCallback, nullptr);
    if (err != paNoError) {
        const char* errorText = Pa_GetErrorText(err);
        std::cerr << "PortAudio error: " << errorText << std::endl;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        const char* errorText = Pa_GetErrorText(err);
        std::cerr << "PortAudio error: " << errorText << std::endl;
    }

    return err;
}

inline PaError stopChatStreams(PaStream *stream) {
    PaError err = Pa_StopStream(sendingStream);
    if (err != paNoError) {
        return err;
    }

    err = Pa_CloseStream(sendingStream);
    if (err != paNoError) {
        return err;
    }

    err = Pa_StopStream(receivingStream);
    if (err != paNoError) {
        return err;
    }

    err = Pa_CloseStream(receivingStream);
    if (err != paNoError) {
        return err;
    }

    
    std::cout << "Chat streams terminated." << '\n';
    return err;
}

inline void printDeviceInfo(PaDeviceIndex index) {
    if (index == paNoDevice) {
        std::cout << "No default device." << std::endl;
    } else {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(index);
        std::cout << "Device name: " << deviceInfo->name << std::endl;
    }
}





inline std::thread networkThread;



inline void connectToChat() {
    runChatThread.store(true);
    static auto networkFunc = [](){

        try {
            udp::socket socket(context, udp::endpoint(udp::v4(), 0));
            client_socket = &socket;

            udp::resolver resolver(context);\

            const size_t colonPos = TYPED_IN_SERVER_IP.find(':');
            const std::string server_ip = TYPED_IN_SERVER_IP.substr(0, colonPos);


            server_endpoints = resolver.resolve(udp::v4(), server_ip, "6970");
            std::cout << "Connected." << std::endl;
            startupPortAudio(sendingStream, sendingAudioCallback, true);
            startupPortAudio(receivingStream, receivingAudioCallback, false);

            start_receive();
            context.run();
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    };

    networkThread = std::thread(networkFunc);
    networkThread.detach();
}


inline void disconnectFromChat() {

    runChatThread.store(false);


    stopChatStreams(sendingStream);

    if(networkThread.joinable()) {
        networkThread.join();
    }
}



#endif