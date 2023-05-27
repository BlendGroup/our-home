#include<crossfade.h>

glshaderprogram* crossfader::programCrossFade;
GLuint crossfader::emptyvao;

void crossfader::init() {
	crossfader::programCrossFade = new glshaderprogram({"shaders/fsquad.vert", "shaders/crossfade.frag"});
	glGenVertexArrays(1, &crossfader::emptyvao);
}

void crossfader::render(GLuint tex, GLfloat t) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		crossfader::programCrossFade->use();
		glUniform1f(crossfader::programCrossFade->getUniformLocation("alpha"), 1.0f - t);
		glUniform1i(crossfader::programCrossFade->getUniformLocation("texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glBindVertexArray(crossfader::emptyvao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
}

void crossfader::uninit() {
	glDeleteVertexArrays(1, &crossfader::emptyvao);
	delete crossfader::programCrossFade;
}
