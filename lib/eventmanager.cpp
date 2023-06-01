#include<eventmanager.h>
#include<algorithm>
#include<vmath.h>
#include<vector>
#include<iostream>
#include<errorlog.h>
#include<global.h>

using namespace std;
using namespace vmath;

eventmanager::eventmanager(vector<pair<unsigned, pair<float, float>>> events) {
	t = 0.0f;
	for(pair<unsigned, pair<float, float>> p : events) {
		events_t* e = new events_t;
		e->start = p.second.first;
		e->duration = p.second.second;
		e->t = 0.0f;
		this->eventList[p.first] = e;
	}
}

void eventmanager::recalculateTs() {
	for(pair<unsigned, events_t*> ev : this->eventList) {
		ev.second->t = clamp((this->t - ev.second->start) / ev.second->duration, 0.0f, 1.0f);
	};
}

void eventmanager::increment() {
	if(programglobal::isAnimating) {
		this->t += programglobal::deltaTime;
		this->recalculateTs();
	}
}

float eventmanager::getT() {
	return this->t;
}

eventmanager& operator+=(eventmanager&e, float t) {
	e.t += t;
	e.recalculateTs();
	return e;
}

eventmanager& operator-=(eventmanager&e, float t) {
	e.t -= t;
	e.recalculateTs();
	return e;
}

void eventmanager::resetT() {
	this->t = 0.0f;
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