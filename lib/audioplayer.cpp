#include <iostream>
#include <audio.h>

/************ audioplayer ************/
using namespace std;

audioplayer::audioplayer(const std::string &wavFilePath) {
    alutInit(0, NULL);
    buffer = alutCreateBufferFromFile(wavFilePath.c_str());
    // buffer = alutCreateBufferHelloWorld();
    alGenSources(1, &source); CHECK_OPENAL_ERROR("alGenSources");
    alSourcei(source, AL_BUFFER, buffer); CHECK_OPENAL_ERROR("alSourcei");
    isPlaying = false;
}

void audioplayer::play(void) {
    if(!isPlaying) {
        alSourcePlay(source);
        isPlaying = true;
    }
}

void audioplayer::pause(void) {
    if(isPlaying) {
        alSourcePause(source);
        isPlaying = false;
    }
}

/*************************************/
