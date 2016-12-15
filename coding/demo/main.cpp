#define GLEW_STATIC
#define FREEGLUT_STATIC

#include <windows.h>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "objreader/objreader.h"
#include "core/objstruct.h"
#include "utilities/path.h"
#include "utilities/camera.h"
#include "utilities/assert.h"
#include "imagereader/imagereader.h"
#include "gameloop/gameloop.h"
#include "io/keyboard.h"
#include "gamescene/gameworld.h"
#include "gamescene/gameobject.h"
#include "gamescene/character.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "gmgl/shaders.h"
#include "utilities/vmath.h"
#include "gmgl/gmgl_func.h"

using namespace gm;

float width = 600;
float height = 300;
GLfloat centerX = 0, centerY = 0, centerZ = 0;
GLfloat eyeX = 0, eyeY = 0, eyeZ = 5;

ObjReader reader(ObjReader::LoadOnly);

GMfloat fps = 60;
GameLoopSettings s = { fps };

GameWorld world;
Character* character;

GMGLShaders shaders;

GLuint VAOs[1], Buffers[1];

GLint render_model_matrix_loc;

class GameHandler : public IGameHandler
{
public:
	void setGameLoop(GameLoop* l)
	{
		m_gl = l;
	}

	void render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Camera& camera = world.getMajorCharacter()->getCamera();

		GMGL::lookAt(camera, shaders, "view_matrix");
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//glColor3f(1, 1, 1);
		//obj.draw();
		//world.renderGameWorld();

		glutSwapBuffers();
	}

	void mouse()
	{
		Camera& camera = world.getMajorCharacter()->getCamera();
		//CameraUtility::fglextlib_gl_LookAt(camera);
		GMGL::lookAt(camera, shaders, "view_matrix");
		int wx = glutGet(GLUT_WINDOW_X),
			wy = glutGet(GLUT_WINDOW_Y);
		camera.mouseReact(wx, wy, width, height);
	}

	void keyboard()
	{
		Character* character = world.getMajorCharacter();
		Camera* camera = &character->getCamera();
		GMfloat dis = 5;
		GMfloat v = dis / fps;
		if (Keyboard::isKeyDown(VK_ESCAPE) || Keyboard::isKeyDown('Q'))
			m_gl->terminate();
		if (Keyboard::isKeyDown('A'))
			camera->moveRight(-v);
		if (Keyboard::isKeyDown('D'))
			camera->moveRight(v);
		if (Keyboard::isKeyDown('W'))
			camera->moveFront(v);
		if (Keyboard::isKeyDown('S'))
			camera->moveFront(-v);
		//if (Keyboard::isKeyDown(VK_SPACE))
		//	camera->jump();
	}

	void logicalFrame()
	{
		//world.simulateGameWorld(m_gl->getSettings().fps);
	}

	GameLoop* m_gl;
};

GameHandler handler;
GameLoop gl(s, &handler);


void init()
{
	world.setGravity(0, 0, 0);

	btTransform charTransform;
	charTransform.setIdentity();
	charTransform.setOrigin(btVector3(0, 0, 10));
	character = new Character(charTransform, 0, 10, 15);
	character->setCanFreeMove(false);
	character->setJumpSpeed(btVector3(0, 50, 0));
	world.setMajorCharacter(character);

	glEnable(GL_LINE_SMOOTH);

	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);

	Camera& camera = world.getMajorCharacter()->getCamera();
	camera.mouseInitReaction(wx, wy, width, height);

	handler.setGameLoop(&gl);
	camera.setSensibility(.25f);

	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	GLfloat pos[] = { 150, 150, 150, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	GLfloat clr[] = { .5, .5, .5, 1 };
	GLfloat b[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, clr);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, clr);
	glLightfv(GL_LIGHT0, GL_SPECULAR, b);
	glEnable(GL_LIGHT0);



	//VAO, VBO
	glGenVertexArrays(1, VAOs);
	glBindVertexArray(VAOs[0]);

	GLfloat  vertices[][2] = {
		{ -0.90f, -0.90f },{ 0.85f, -0.90f },{ -0.90f,  0.85f },  // Triangle 1
		{ 0.90f, -0.85f },{ 0.90f,  0.90f },{ -0.85f,  0.90f }   // Triangle 2
	};

	glGenBuffers(1, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
		vertices, GL_STATIC_DRAW);

	GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, "D:/shaders/test/test.vert" },
		{ GL_FRAGMENT_SHADER, "D:/shaders/test/test.frag" },
	};
	shaders.appendShader(shadersInfo[0]);
	shaders.appendShader(shadersInfo[1]);
	GMGLShadersLoader::loadShaders(shaders);
	shaders.useProgram();

	render_model_matrix_loc = glGetUniformLocation(shaders.getProgram(), "model_matrix");

	using namespace vmath;
	const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
	vmath::mat4 model_matrix(translate(0.0f, 0.0f, -5.0f));

	glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
	GMGL::perspective(30, 2, 1, 500, shaders, "projection_matrix");

	glVertexAttribPointer(0, 2, GL_FLOAT,
		GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void render()
{
}

void resharp(GLint w, GLint h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

int main()
{
	WinMain(NULL, NULL, NULL, 0);
	return 0;
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char * lpCmdLine,
	int nCmdShow
)
{
	int argc = 1;
	char* l = "";
	char* argv[1];
	argv[0] = l;
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(400, 400);
	glutCreateWindow("Render");

	GLenum err = glewInit();
	init();
	glutReshapeFunc(resharp);
	glutDisplayFunc(render);

	GameLoopUtilities::gm_gl_registerGameLoop(gl);

	glutMainLoop();

	return 0;
}
