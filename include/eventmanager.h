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
	std::unordered_map<unsigned, events_t*> eventList;
public:
	eventmanager(std::vector<std::pair<unsigned, std::pair<float, float>>> events);
	void updateT(float sceneT);
	float& operator[](unsigned index);
	~eventmanager();
};

#endif