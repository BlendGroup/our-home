#include <testaudio.h>

using namespace std;

static audioplayer *audioPlayer;
static const string audioFile = "resources/audio/test.wav";

void initTestAudio(void) {
	audioPlayer = new audioplayer(audioFile);
}

void renderTestAudio(bool isAudioPlaying) {
    if(isAudioPlaying)
        audioPlayer->play();
    else
        audioPlayer->pause();
}

void uninitTestAudio(void) {
	if(audioPlayer) {
        delete audioPlayer;
        audioPlayer = NULL;
    }
}
