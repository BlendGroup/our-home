#ifndef __EVENT_MANAGER__
#define __EVENT_MANAGER__

#include<unordered_map>
#include<vector>

struct events_t {
	float start;
	float duration;
	float t;
};

class eventmanager {
private:
	std::unordered_map<unsigned, events_t> eventList;
public:
	eventmanager(vector<pair<unsigned, pair<float, float>>> events);
	void setT(unsigned eventId, float t);
	float getT(unsigned eventId);
	void updateT(float sceneT);
	~eventmanager();
};

#endif