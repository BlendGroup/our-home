#ifndef __AUDIO__
#define __AUDIO__

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <string>
#include <errorlog.h>

#define CHECK_OPENAL_ERROR(func) \
    { \
        ALenum error; \
        if((error = alGetError()) != AL_NO_ERROR) { \
            throwErr(std::string(func) + ": failed with code " + std::to_string(error)); \
        } \
    }

class audioplayer {
private:
    ALuint buffer;
    ALuint source;
    bool isPlaying;
    
public:
    audioplayer(const std::string &wavFilePath);
    void play(void);
    void pause(void);
	void restart(void);
};

#endif // __AUDIO__
