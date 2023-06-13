#include <AL/al.h>
#include <iostream>
#include <audio.h>
#include <type_traits>

/************ audioplayer ************/
using namespace std;

audioplayer::audioplayer(const std::string &wavFilePath) {
    buffer = alutCreateBufferFromFile(wavFilePath.c_str());
    alGenSources(1, &source); CHECK_OPENAL_ERROR("alGenSources");
    alSourcei(source, AL_BUFFER, buffer); CHECK_OPENAL_ERROR("alSourcei");
    isPlaying = false;
}

void audioplayer::play(void) {
    ALint state;
    alGetSourcei(this->source,AL_SOURCE_STATE,&state);
    if(state == AL_STOPPED) {
        isPlaying = false;
    }
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

void audioplayer::restart(void) {
	alSourceStop(source);
	alSourcePlay(source);
	isPlaying = true;
}

/*************************************/
