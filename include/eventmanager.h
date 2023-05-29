#ifndef __EVENT_MANAGER__
#define __EVENT_MANAGER__

#include<vector>

struct events_t {
	float start;
	float duration;
	float t;
};

class eventmanager {
private:
	std::vector<events_t> eventList;
public:
	eventmanager(std::vector<std::pair<float, float>> events);
	~eventmanager();
};

#endif