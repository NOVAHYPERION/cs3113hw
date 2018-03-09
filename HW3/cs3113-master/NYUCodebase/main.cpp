#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"


#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

struct Vector3
{
	Vector3() {}
	Vector3(float newx, float newy, float newz)
	{
		x = newx;
		y = newy;
		z = newz;
	}
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
};

enum entityType { NONE, BULLET, PLAYER, ENEMY, TEXT };
bool play = false;
struct gameObj
{
	float verts[12] = { -0.5f, -0.5f, 0.5f, -.5f, .5f, .5f, -0.5f, -0.5f, 0.5f, 0.5f, -.5f, 0.5f };
	float* texCoords;
	Matrix* model;
	Matrix* view;
	float height = .5f;
	float width = .5f;
	float x = 0.0f;
	float y = 0.0f;
	float maxy = 1.9f;
	float maxx = 3.7f;
	float speed;
	bool alive = true;
	entityType type = NONE;
	
	void changeTex(float texx, float texy, float texwidth, float texheight, float texxsize, float texysize)
	{
		float textx = texx / texxsize;
		float texty = texy / texysize;
		float textxpl = (texx + texwidth) / texxsize;
		float textypl = (texy + texheight) / texysize;
		texCoords =  new float[12]{ textxpl, texty, textx, texty, textx, textypl, /*half*/ textxpl, texty, textx, textypl, textxpl, textypl  };
	}

	bool collision(gameObj* obj, float elapsed)
	{
		bool inrange = false;
		if (alive && obj->alive)
		{
			float paddleroof = obj->y + (obj->height / 2.0f);
			float paddlefloor = obj->y - (obj->height / 2.0f);
			float paddleleft = obj->x - (obj->width / 2.0f);
			float paddleright = obj->x + (obj->width / 2.0f);

			float ballroof = y + (height / 2);
			float ballfloor = y - (height / 2);
			float ballright = x + (width / 2);
			float ballleft = x - (width / 2);
			bool ballxpaddle = ((ballright >= paddleleft && ballleft <= paddleright) || (ballright >= paddleleft && ballleft < paddleleft) || (ballleft <= paddleright && ballright > paddleright));
			inrange = (ballfloor <= paddleroof && ballxpaddle && ballroof >= paddlefloor);
		}

		return inrange;
	}

	void draw(ShaderProgram* program, GLuint img)
	{
		if (alive)
		{
			model->Identity();
			model->Translate(x, y, 0.0f);
			model->Scale(width, height, 0);
			if (type == ENEMY || type == TEXT)
			{
				model->Rotate(180.0f * (3.1415926f / 180.0f));
			}
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			program->SetModelMatrix(*model);
			program->SetViewMatrix(*view);
			glBindTexture(GL_TEXTURE_2D, img);
			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, verts);
			glEnableVertexAttribArray(program->positionAttribute);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(program->texCoordAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
};

struct bullet : gameObj
{
	float dir = 1;
	bullet()
	{
		model = new Matrix;
		view = new Matrix;
		alive = false;
		speed = 2.0f;
		type = BULLET;
		height = .1;
		width = .1;
		changeTex(856, 421, 9,54,1024,1024);
		maxy += .2;
	}
	
	void shoot(gameObj* Shooter)
	{
		alive = true;
		if (Shooter->type == PLAYER)
		{
			dir = 1;
		}
		else
		{
			dir = -1;
		}
		x = Shooter->x;
		y = Shooter->y + .001*dir;
	}

	void pewpew(float elapsed)
	{
		if (alive)
		{
			float tmpy = y + speed * elapsed * dir;
			if (tmpy <= 0)
			{
				if (fabs(tmpy) <= maxy)
				{
					y = tmpy;
				}
				else
				{
					y = tmpy;
					alive = 0;
				}
			}
			else if (tmpy <= maxy)
			{
				y = tmpy;
			}
			else
			{
				y = tmpy;
				alive = 0;
			}
		}
	}
};

struct alien : gameObj
{

	alien(float newx, float newy, entityType typer)
	{
		model = new Matrix;
		view = new Matrix;
		x = newx;
		y = newy;
		alive = 1;
		speed = 2;
		height = (.25);
		width = (.25);
		type = typer;
	}

	bool getHit(gameObj *bang,float elapsed)
	{
		if (collision(bang,elapsed))
		{
			alive = 0;
			return true;
		}
		else { return false; }
	}

	~alien()
	{
		delete model;
		delete view;
	}
	
	void move(float elapsed, float dir)
	{
		float tmpx = x + dir * speed * elapsed;
		if (tmpx < 0)
		{
			if (fabs(tmpx) <= maxx)
			{
				x = tmpx;
			}
			else
			{
				x = -1 * maxx;

			}
		}
		else if (tmpx <= maxx)
		{
			x = tmpx;
		}
		else
		{
			x = maxx;
		}
	}
};

struct alienArmy : gameObj
{
	//float verts[12] = { -0.25f, -0.25f, -0.25f, .25f, .25f, .25f, 0.25f, 0.25f, -0.25f, -0.25f, .25f, -0.25f };
	int rows = 5;
	int columns = 5;
	alien *armyItself[5][5];
	bool alivearray[5];
	float xarray[5];
	int lboundary = 0;
	int rboundary = 4;
	bool alive = 1;
	int dir = 1;
	alienArmy()
	{
		y = 1.0f;
		height = 1.5;
		width = 2.5;
	}
	
	void mover(float elapsed)
	{
		for (int i = 0; i < 5; i++)
		{
			for (alien* n : armyItself[i])
			{
				checkBoundaries();
				if (xarray[rboundary] >= maxx || abs(xarray[lboundary]) >= maxx)
				{
					dir *= -1;
				}
				n->move(elapsed, dir);
			}
		}
	}

	void checkHits(gameObj* well,float elapsed)
	{
		for (int i = 0; i < 5; i++)
		{
			for (alien* n : armyItself[i])
			{
					if (n->getHit(well, elapsed))
					{
						well->alive = false;
						return;
					}
			}
		}
	}

	void populate()
	{
		int counterx = 0;
		int countery = 0;
		for(float ax =  x + -((x+width) / 2); ax < x + ((x+ width) / 2); ax += ((x+width) / 5))
		{
			for (float ay = y + (-((height) / 2)); ay < y + (height / 2); ay += (height / 5))
			
				{
					armyItself[counterx][countery] = new alien(ax, ay,ENEMY);
					countery += 1;
				}
			counterx += 1;
			countery = 0;
		}
	}

	void draw(ShaderProgram* program, GLuint img)
	{
		int counterx = 0;
		int countery = 0;
		for (float x = -(width / 2); x < (width / 2); x += (width / 5))
		{
			for (float y = -(height / 2); y < (height / 2); y += (height / 5))
			{
				armyItself[counterx][countery]->draw(program, img);
				countery += 1;
			}
			counterx += 1;
			countery = 0;
		}
	}

	void checkBoundaries()
	{
		for (int i = 0; i < 5; i++)
		{
			for (alien *n : armyItself[i])
			{
				xarray[i] = n->x;
				if (n->alive)
				{
					alivearray[i] = true;
				}
			}
		}
		int rightmost = 0;
		int leftmost = 0;
		for (int i = 0; i < 5; i++)
		{
			bool leftallocated = false;
			if (alivearray[i] && rightmost < i)
			{
				rightmost = i;
			}
			if (alivearray[i] && leftallocated)
			{
				leftmost = i;
			}
		}
		rboundary = rightmost;
		lboundary = leftmost;
	}

	void massChangeTexture(float texx, float texy, float texwidth, float texheight, float texxsize, float texysize)
	{
		int counterx = 0;
		int countery = 0;
		for (float x = -(width / 2); x < (width / 2); x += (width / 5))
		{
			for (float y = -(height / 2); y < (height / 2); y += (height / 5))
			{
				armyItself[counterx][countery]->changeTex(texx, texy, texwidth, texheight, texxsize,texysize);
				countery += 1;
			}
			counterx += 1;
			countery = 0;
		}
	}


};

struct player : gameObj
{
	float speed = 120.0f;
	player(Matrix* modelm, Matrix* viewm)
	{
		model = modelm;
		view = viewm;
		type = PLAYER;
		y = -1.5f;
	}

	void move(float elapsed, float dir)
	{
		float tmpx = x + dir * speed * elapsed;
		if (tmpx < 0)
		{
			if (fabs(tmpx) <= maxx)
			{
				x = tmpx;
			}
			else
			{
				x = -1 * maxx;
			}
		}
		else if (tmpx <= maxx)
		{
			x = tmpx;
		}
		else
		{
			x = maxx;
		}
	}
};
struct text
{
	float x = 0;
	float y = 0;
	float width = 3.5;
	float height = 2.5;
	float textnum;
	std::string whattext;
	alien* alientext[13];
	GLuint img;

	text(GLuint spritesheet, float nx, float ny, std::string thetext)
	{
		img = spritesheet;
		x = nx;
		y = ny;
		whattext = thetext;
	}

	float textxcoords(char letter)
	{
		int stringcor = (int)letter;
		float returnval =  ((float)(stringcor % 16) / 16.0f);
		return returnval;
	}

	float textycoords(char letter)
	{
		int stringcor = (int)letter;
		float returnval = ((float)(stringcor / 16) / 16.0f);
		return returnval;
	}

	void populate()
	{
		int counterx = 0;
		for (float ax = x + -((width) / 2); ax < x + ((width) / 2); ax += ((x + width) / 13))
		{
			alientext[counterx] = new alien(ax, y, TEXT);
			counterx++;
		}
	}

	void texturer()
	{
		int counterx = 0;
		for (float ax = x + -((x + width) / 2); ax < x + ((x + width) / 2); ax += ((x + width) / 13))
		{
			alientext[counterx]->changeTex(textxcoords(whattext[counterx]), textycoords(whattext[counterx]), (1.0f/16.0f), (1.0f / 16.0f), 1, 1);
			counterx++;
		}
	}

	void draw(ShaderProgram* program)
	{
		for(alien* n: alientext)
		{
			n->draw(program, img);
		}
	}

	void setup()
	{
		populate();
		texturer();
	}
};
struct bulletpool
{
	bullet* bullets;
	bulletpool()
	{
		bullets = new bullet();
		bullets->alive = false;
	}
	void shootemup(gameObj* shooter)
	{
		if (!(bullets->alive))
		{
			bullets->shoot(shooter);
		}
	}
};
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL)
	{
		return -1;
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	
	SDL_Event event;
	bool done = false;
	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Matrix projectionMatrix;
	Matrix playerModelMatrix;
	Matrix playerViewMatrix;
	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	float lastFrameTicks = 0.0f;
	player shooter(&playerModelMatrix, &playerViewMatrix);
	GLuint player = LoadTexture(RESOURCE_FOLDER"sheet.png");
	GLuint Text = LoadTexture(RESOURCE_FOLDER"font1.png");
	if (player == NULL)
	{
		assert("IMAGEERROR");
	}
	glUseProgram(program.programID);
	bulletpool friendbullets;
	int allyfire = 0;
	alienArmy aliens;
	aliens.populate();
	aliens.massChangeTexture(423.0f,728.0f,93.0f,84.0f,1024.0f,1024.0f);
	shooter.changeTex(120.0f, 520.0f, 104.0f, 84.0f, 1024.0f, 1024.0f);
	text starting(Text, 0, 0, "SPACE INVADER");
	starting.setup();
	glClearColor(.4f, .5f, .4f, 1.0f);
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		glClear(GL_COLOR_BUFFER_BIT);
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&event)){
			
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
			}
			if (play)
			{
				if (keys[SDL_SCANCODE_LEFT] && keys[SDL_SCANCODE_RIGHT] && keys[SDL_SCANCODE_SPACE] && play)
				{
					shooter.move(elapsed, 1.0f);
					shooter.move(elapsed, -1.0f);
					friendbullets.shootemup(&shooter);
				}
				else if (keys[SDL_SCANCODE_LEFT]&& keys[SDL_SCANCODE_SPACE] && play)
				{
					shooter.move(elapsed, -1.0f);
					friendbullets.shootemup(&shooter);
				}
				else if (keys[SDL_SCANCODE_RIGHT] && keys[SDL_SCANCODE_SPACE] && play)
				{
					shooter.move(elapsed, 1.0f);
					friendbullets.shootemup(&shooter);
				}
				if (keys[SDL_SCANCODE_LEFT] && keys[SDL_SCANCODE_RIGHT])
				{
					shooter.move(elapsed, 1.0f);
					shooter.move(elapsed, -1.0f);
				}
				if (keys[SDL_SCANCODE_LEFT])
				{
					shooter.move(elapsed, -1.0f);
				}
				if (keys[SDL_SCANCODE_RIGHT])
				{
					shooter.move(elapsed, 1.0f);
				}
				else if (keys[SDL_SCANCODE_SPACE] && play)
				{
					friendbullets.shootemup(&shooter);
				}
			}
			else
			{
				if (keys[SDL_SCANCODE_SPACE])
				{
					play = true;
				}
			}
		}
		program.SetProjectionMatrix(projectionMatrix);
		glClearColor(.4f, .5f, .4f, 1.0f);
		if (play)
		{
			friendbullets.bullets->pewpew(elapsed);
			shooter.draw(&program, player);
			aliens.draw(&program, player);
			aliens.mover(elapsed);
			friendbullets.bullets->draw(&program, player);
			aliens.checkHits(friendbullets.bullets,elapsed);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
			SDL_GL_SwapWindow(displayWindow);
		}
		else
		{
			starting.draw(&program);
			glClearColor(.4f, .5f, .4f, 1.0f);
			SDL_GL_SwapWindow(displayWindow);
		}
	}
	SDL_Quit();
	return 0;
}
