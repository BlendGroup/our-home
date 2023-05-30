#include<eventmanager.h>
#include<algorithm>
#include<vmath.h>
#include<vector>
#include<iostream>
#include<errorlog.h>

using namespace std;
using namespace vmath;

eventmanager::eventmanager(vector<pair<unsigned, pair<float, float>>> events) {
	for(pair<unsigned, pair<float, float>> p : events) {
		events_t* e = new events_t;
		e->start = p.second.first;
		e->duration = p.second.second;
		e->t = 0.0f;
		this->eventList[p.first] = e;
	}
}

void eventmanager::updateT(float sceneT) {
	for(pair<unsigned, events_t*> ev : this->eventList) {
		ev.second->t = clamp((sceneT - ev.second->start) / ev.second->duration, 0.0f, 1.0f);
	};
}

float& eventmanager::operator[](unsigned eventid) {
	if(this->eventList.count(eventid) == 0) {
		throwErr("Invalid Event");
	}
	return this->eventList[eventid]->t;
}

eventmanager::~eventmanager() {
	this->eventList.clear();
}