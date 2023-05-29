#include<eventmanager.h>

using namespace std;

events_t event(pair<float, float> ev) {
	events_t e;
	e.start = ev.first;
	e.duration = ev.second;
	e.t = 0.0f;
	return e;
}

eventmanager::eventmanager(vector<pair<float, float>> events) {
	for(pair<float, float> p : events) {
		this->eventList.push_back(event(p));
	}
}

eventmanager::~eventmanager() {
	
}