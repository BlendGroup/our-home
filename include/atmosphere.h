#ifndef __ATMOSPPHERE__
#define __ATMOSPPHERE__

#include<glmodelloader.h>
#include<string>
#include<vmath.h>
#include<debugcamera.h>
#include<CubeMapRenderTarget.h>
#include<glshaderloader.h>
#include<errorlog.h>

class Atmosphere {
private:  
	glshaderprogram* atmosphereProgram;
	glmodel* sphereModel;
public:
	Atmosphere();
	~Atmosphere();
	void render(const vmath::mat4& viewMatrix, float delta);
	void keyboardfunc(int key);
};

#endif // __ATMOSPPHERE__

