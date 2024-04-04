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



using boost::asio::ip::udp;
using boost::asio::io_context;



//PortAudio settings
const int FRAMESPERBUFFER = 480;
const int SAMPLERATE = 48000;

//Network settings
std::string SERVER_PORT = "6969";
std::string SERVER_IP = "192.168.1.131";

int PREFERRED_INPUT_DEVICE = 0;
int PREFERRED_OUTPUT_DEVICE = 0;

struct AudioPacket {
    uint8_t sequenceNumber;
    uint8_t incomingBytes;
    std::string id;
    unsigned char buffer[128];
};

struct Person {
    std::string id;
    OpusDecoder *decoder;
    RingBuffer rbuf;
    int opusErr;
    int deleteTimer;
    udp::endpoint endpoint;
    Person();
};

Person::Person() {
    this->rbuf = RingBuffer(480, 1);
    this->decoder = opus_decoder_create(SAMPLERATE, 1, &this->opusErr);
    this->deleteTimer = 0;
}

std::vector<Person> allPeople;







// const size_t BUFFER_SIZE = 1024;
// char recv_buffer[BUFFER_SIZE];
udp::socket* client_socket;
udp::resolver::results_type server_endpoints;

udp::endpoint remote_endpoint;
AudioPacket recvPacket;

io_context context;

PaStream *sendingStream;
PaStream *receivingStream;

//Opus stuffies
OpusEncoder *theFuckinEncoderDude;

PaError startupPortAudio(PaStream *stream, PaStreamCallback *streamCallback, bool input);
PaError stopChatStreams(PaStream *stream);
void printDeviceInfo(PaDeviceIndex index);
void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred);

uint8_t mySequenceNumber = 0;
std::string myID = get_uuid();

static int sendingAudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    const float *in = (const float*) inputBuffer;
    //float *out = (float*) outputBuffer;

    // for (unsigned long i = 0; i < framesPerBuffer; ++i) { 
    //     *out++ = *in++;  // Simple pass-through
    // }

    AudioPacket packet;
    packet.sequenceNumber = mySequenceNumber;
    packet.id = myID;

    opus_int32 encodedLength = opus_encode_float(
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

    return paContinue;
}


static int receivingAudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    float *out = (float*) outputBuffer;

    float mixdown[480] = {};

    for (Person& person : allPeople) {
        int bufCount = person.rbuf.count.load();
        float thisPersonsBuf[480];
        if(bufCount > 1) {
            person.rbuf.readOneBuffer(thisPersonsBuf);
            for(int i = 0; i < 480; ++i) {
                mixdown[i] = std::min(std::max(mixdown[i] + thisPersonsBuf[i], -1.0f), 1.0f);
            }
            person.deleteTimer = 0;
        } else {
            person.deleteTimer++;
        }
    }

    allPeople.erase(
        std::remove_if(
            allPeople.begin(), 
            allPeople.end(),
            [](const Person& person) {
                return person.deleteTimer > 50;  // Condition to remove the person
            }
        ), 
        allPeople.end()
    );

    std::copy(std::begin(mixdown), std::end(mixdown), out);

    return paContinue;
}



void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        //std::cout << "Message sent" << std::endl;
    } else {
        std::cerr << "Error sending message: " << error.message() << std::endl;
    }
}

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);


void start_receive() {
    //printf("Started receive");
    client_socket->async_receive_from(
        boost::asio::buffer(&recvPacket, sizeof(recvPacket)), remote_endpoint,
        handle_receive);
}

void promptForChoices() {
    std::cout << "Please enter the number corresponding to the input device you want to use:" << '\n';

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(numDevices) << '\n';
    } else {
        for (int i = 0; i < numDevices; i++) {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
            if (deviceInfo && deviceInfo->maxInputChannels > 0) {
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

    std::cin >> SERVER_IP;

    std::cout << "Enter the port:" << '\n';

    std::cin >> SERVER_PORT;

    std::cout << "Connecting..." << '\n';
}


 
std::atomic<bool> runChatThread = false;

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error && bytes_transferred == sizeof(AudioPacket)) {

        auto &personIt = std::find_if(
            allPeople.begin(),
            allPeople.end(),
            [](const Person& person) {
                return person.id == recvPacket.id;
            }
        );

        if(personIt == allPeople.end()) {
            Person newPerson;
            newPerson.id = recvPacket.id;
            newPerson.endpoint = remote_endpoint;
            allPeople.push_back(newPerson);
            personIt = std::prev(allPeople.end());
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

PaError startupPortAudio(PaStream *stream, PaStreamCallback *streamCallback, bool input) {

    PaError err;// = Pa_Initialize();

    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;

    inputParameters.channelCount = 1;
    inputParameters.device = PREFERRED_INPUT_DEVICE;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.channelCount = 1;
    outputParameters.device = PREFERRED_OUTPUT_DEVICE;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, SAMPLERATE, FRAMESPERBUFFER, paClipOff, streamCallback, NULL);
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

PaError stopChatStreams(PaStream *stream) {
    PaError err;
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        return err;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        return err;
    }

    
    std::cout << "Chat streams terminated." << '\n';
    return err;
}

void printDeviceInfo(PaDeviceIndex index) {
    if (index == paNoDevice) {
        std::cout << "No default device." << std::endl;
    } else {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(index);
        std::cout << "Device name: " << deviceInfo->name << std::endl;
    }
}





std::thread networkThread;



void connectToChat() {
    runChatThread.store(true);
    static auto networkFunc = [](){
        udp::socket socket(context, udp::endpoint(udp::v4(), 0));
        try {

            client_socket = &socket;

            udp::resolver resolver(context);
            server_endpoints = resolver.resolve(udp::v4(), SERVER_IP, SERVER_PORT);
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


void disconnectFromChat() {
    runChatThread.store(false);
    if(networkThread.joinable()) {
        networkThread.join();
    }
    stopChatStreams(sendingStream);
}



#endif