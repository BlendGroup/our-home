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
	float t;
	std::unordered_map<unsigned, events_t*> eventList;
public:
	eventmanager(std::vector<std::pair<unsigned, std::pair<float, float>>> events);
	void recalculateTs();
	void resetT();
	float& operator[](unsigned index);
	void increment();
	friend eventmanager& operator+=(eventmanager& e1, float f);
	friend eventmanager& operator-=(eventmanager& e1, float f);
	float getT();
	~eventmanager();
};

#endif