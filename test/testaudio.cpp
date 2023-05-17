#include <testaudio.h>

using namespace std;

static audioplayer *audioPlayer1 = NULL;
static audioplayer *audioPlayer2 = NULL;
static const string audioFile1 = "resources/audio/test1.wav";
static const string audioFile2 = "resources/audio/test2.wav";

void initTestAudio(void) {
	audioPlayer1 = new audioplayer(audioFile1);
	audioPlayer2 = new audioplayer(audioFile2);
}

void renderTestAudio(bool isAudioPlaying) {
    if(isAudioPlaying) {
        audioPlayer1->play();
        audioPlayer2->play();
    }
    else {
        audioPlayer1->pause();
        audioPlayer2->pause();
    }
}

void uninitTestAudio(void) {
	if(audioPlayer1) {
        delete audioPlayer1;
        audioPlayer1 = NULL;
    }
    if(audioPlayer2) {
        delete audioPlayer2;
        audioPlayer2 = NULL;
    }
}
