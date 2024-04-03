#include "soundfxsystem.h"

SoundEffect SoundFXSystem::add(std::string path) {
    SF_INFO sfinfo;
    SNDFILE* sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo);
    bool success = true;
    if (sndfile == nullptr) {
        std::cerr << "Error opening sound effect: " << path << "\n";
        success = false;
    }

    std::vector<float> buffer(sfinfo.frames * sfinfo.channels);

    long long numFramesRead = sf_readf_float(sndfile, buffer.data(), sfinfo.frames);

    if (numFramesRead < sfinfo.frames) {
        std::cerr << "Error reading frames from audio file: " << path << "\n";
        success = false;
    }

    int myID = masterBuffers.size();

    sf_close(sndfile);

    if(success == true) {
        masterBuffers.push_back(buffer);
        outputBuffers.push_back(new RingBuffer(bufferSize));
        return myID;
    } else {
        return -1;
    }
}

void SoundFXSystem::play(SoundEffect sound) {
    while(outputBuffers.at(sound)->count > 0) {
        float trash[512];
        outputBuffers.at(sound)->readOneBuffer(trash);
    }
    outputBuffers.at(sound)->write(masterBuffers.at(sound).data(), masterBuffers.at(sound).size());
}

void SoundFXSystem::playNextInSeries(SoundEffectSeries& series) {
    play(series.series.at(series.front));
    series.front = (series.front + 1) % series.series.size();
}