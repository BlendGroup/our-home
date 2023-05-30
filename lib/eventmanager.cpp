#include<eventmanager.h>
#include<algorithm>
#include<vmath.h>

using namespace std;
using namespace vmath;

events_t event(pair<float, float> ev) {
	events_t e;
	e.start = ev.first;
	e.duration = ev.second;
	e.t = 0.0f;
	return e;
}

eventmanager::eventmanager(vector<pair<unsigned, pair<float, float>>> events) {
	for(pair<unsigned, pair<float, float>> p : events) {
		this->eventList[p.first] = event(p.second);
	}
}

void eventmanager::updateT(float sceneT) {
	for_each(this->eventList.begin(), this->eventList.end(), [sceneT](events_t ev) {
		ev.t = clamp((sceneT - ev.start) / ev.duration, 0.0f, 1.0f);
	});
}

float eventmanager::getT(unsigned eventId) {
	return this->eventList[eventId].t;
}

void eventmanager::setT(unsigned eventId, float t) {
	this->eventList[eventId].t = t;
}

eventmanager::~eventmanager() {
	this->eventList.clear();
}