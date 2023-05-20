#include<vmath.h>
#include<iostream>

class modelplacer {
	vmath::vec3 translateBy;
	float scaleBy;
	vmath::vec3 rotateBy;
	float multiplier;
public:
	modelplacer();
	modelplacer(vmath::vec3 t, vmath::vec3 r, float s);
	vmath::mat4 getModelMatrix(void);
	void keyboardfunc(int key);
	friend std::ostream& operator<<(std::ostream &out, modelplacer* m);
};
