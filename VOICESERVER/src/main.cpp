#define _WIN32_WINNT 0x0601
#include <boost/asio.hpp>
#include <portaudio.h>
#include <opus/opus.h>
#include <iostream>
#include <unordered_map>
#include <vector>

#define RINGBUFFER_IMPLEMENTATION
#include "ringbuffer.h"

using boost::asio::ip::udp;
using boost::asio::io_context;

struct AudioPacket {
    uint32_t servId;
    uint8_t sequenceNumber;
    uint8_t incomingBytes;
    std::string id;
    unsigned char buffer[128];
};

//PortAudio settings
const int FRAMESPERBUFFER = 480;
const int SAMPLERATE = 48000;

PaStream *outStream;

short PORT = 6969;

AudioPacket recvPacket;

udp::socket* server_socket;
udp::endpoint remote_endpoint;
io_context context;

std::vector<udp::endpoint> connectedEndpoints;

struct Person {
    std::string id;
    uint32_t servId;
    OpusDecoder *decoder;
    RingBuffer rbuf;
    int opusErr;
    int deleteTimer;
    udp::endpoint endpoint;
    Person();
};

Person::Person() {
    this->decoder = opus_decoder_create(SAMPLERATE, 1, &this->opusErr);
    this->deleteTimer = 0;
}

std::vector<Person> allPeople;

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);

void start_receive() {
    server_socket->async_receive_from(
        boost::asio::buffer(&recvPacket, sizeof(recvPacket)), remote_endpoint,
        handle_receive);
}

void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        //std::cout << "Message sent" << std::endl;
    } else {
        std::cerr << "Error sending message: " << error.message() << std::endl;
    }
}

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error && bytes_transferred >= sizeof(AudioPacket)) {

        for(Person &person : allPeople) {
            if(person.id != recvPacket.id) {
                server_socket->async_send_to(
                    boost::asio::buffer(&recvPacket, sizeof(recvPacket)),
                    person.endpoint,
                    handle_send
                );
            }
        }

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
            newPerson.servId = recvPacket.servId;
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

        writeToRingBuffer(personIt->rbuf, newBuf);

    }

    start_receive(); // Start another async receive
}

static int receivingAudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    //float *out = (float*) outputBuffer;

    //float mixdown[480] = {};

    for (Person& person : allPeople) {
        int bufCount = person.rbuf.count.load();
        float thisPersonsBuf[480];
        if(bufCount > 3) {
            readFromRingBuffer(person.rbuf, thisPersonsBuf);
            // for(int i = 0; i < 480; ++i) {
            //     mixdown[i] = std::min(std::max(mixdown[i] + thisPersonsBuf[i], -1.0f), 1.0f);
            // }
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

    printf("People: %zu \n", allPeople.size());

    //std::copy(std::begin(mixdown), std::end(mixdown), out);

    return paContinue;
}

PaStream *blankStream;

PaError startupPortAudio(PaStream *stream, PaStreamCallback *streamCallback, bool input) {

    PaError err = Pa_Initialize();
    if (err != paNoError) {
    const char* errorText = Pa_GetErrorText(err);
    std::cerr << "PortAudio error: " << errorText << std::endl;
}


    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;


        inputParameters.channelCount = 1;
        inputParameters.device = Pa_GetDefaultInputDevice();
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;

        outputParameters.channelCount = 1;
        outputParameters.device = Pa_GetDefaultOutputDevice();
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

    std::cout << "Stream started." << std::endl;
}


int main() {

    std::cout << "Enter the preferred hosting port:" << '\n';
    std::cin >> PORT;
    std::cout << "Starting server..." << '\n';

    udp::socket socket(context, udp::endpoint(udp::v4(), PORT));
            startupPortAudio(blankStream, receivingAudioCallback, false);
    try {
        server_socket = &socket;

        start_receive();
        context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

}
