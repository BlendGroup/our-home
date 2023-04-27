#include<scenes/lab.h>
#include<glmodelloader.h>

glmodel* labModel;

void labscene::setupProgram() {
	
}

void labscene::setupCamera() {

}

void labscene::init() {
	labModel = new glmodel("resources/models/spaceship/SpaceLab.fbx", 0, false);
}

void labscene::render() {

}

void labscene::uninit() {

}