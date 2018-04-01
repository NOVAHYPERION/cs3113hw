#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "FlareMap.cpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"


#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

enum entityType { NONE, PHYSICAL, PLAYER, ENEMY, TEXT};
bool play = false;
float TEXXMAX = 384;
float TEXYMAX = 256;

float TILE_SIZE = .5;
int TEXXNUM = TEXXMAX / 16.0f;
int TEXYNUM = TEXYMAX / 16.0f;
float TEXXSIZE = 1.0f / 24.0f;
float TEXYSIZE = 1.0f / 16.0f;
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
	
	gameObj(float newx, float newy, entityType typer)
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
			if (type == ENEMY || type == TEXT )
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
struct text
{
	float x = 0;
	float y = 0;
	float width = 3.5;
	float height = 2.5;
	float textnum;
	std::string whattext;
	gameObj* alientext[13];
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
			alientext[counterx] = new gameObj(ax, y, TEXT);
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
		for(gameObj* n: alientext)
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
struct worldObjs
{
	FlareMap Map;
	std::vector<float> objBlocks;
	std::vector<float> texCoordData;
	int num = 0;

	float textxcoords(int object)
	{
		float returnval = (float)(((int)object) % TEXXNUM) / (float)TEXXNUM;
		return returnval;
	}
	float textycoords(int object)
	{
		float returnval = (float)(((int)object) / TEXYNUM) / (float)TEXYNUM;
		return returnval;
	}
	void maptime()
	{
		Map.Load("Tiletimemapbutsmallnew.txt");
	}
	void Blocker()
	{
		float offset = 1.0f;
		for (int y = 0; y < Map.mapHeight; y++) {
			for (int x = 0; x < Map.mapWidth; x++) {
				int val = Map.mapData[y][x];
				if (val != 0) {
					num += 6;
					float u = ((float)(((int)Map.mapData[y][x]) % TEXXNUM)) / (float)TEXXNUM;
					float v = ((float)(((int)Map.mapData[y][x]) / TEXYNUM)) / (float)TEXYNUM;
					/*if (val == 63 || val == 64)
					{
						u = textxcoords(val);
					}*/
					objBlocks.insert(objBlocks.end(), {
						(TILE_SIZE * x) -offset, (TILE_SIZE * y) - offset,
						(TILE_SIZE * x) - offset, ((TILE_SIZE * y) - TILE_SIZE) - offset,
						((TILE_SIZE * x) + TILE_SIZE) - offset, ((TILE_SIZE * y) - TILE_SIZE) - offset,
						(TILE_SIZE * x) - offset, (TILE_SIZE * y) - offset,
						((TILE_SIZE * x) + TILE_SIZE) - offset, ((TILE_SIZE * y) - TILE_SIZE) - offset,
						((TILE_SIZE * x) + TILE_SIZE)- offset, (TILE_SIZE * y)- offset
						});

					texCoordData.insert(texCoordData.end(), {
						u, v,
						u, v - (TEXYSIZE),
						u + TEXXSIZE, v - (TEXYSIZE),
						u, v,
						u + TEXXSIZE, v - (TEXYSIZE),
						u + TEXXSIZE, v
						});
					u = 0;
				}
			}
		}
	}

	void create()
	{
		maptime();
		Blocker();
	}

	void drawer(ShaderProgram* program, GLuint img)
	{
		glBindTexture(GL_TEXTURE_2D, img);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, &objBlocks[0]);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, &texCoordData[0]);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, num);
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
	GLuint player = LoadTexture(RESOURCE_FOLDER"dirt-tiles.png");
	GLuint Text = LoadTexture(RESOURCE_FOLDER"font1.png");
	if (player == NULL || Text == NULL)
	{
		assert("IMAGEERROR");
	}
	glUseProgram(program.programID);
	int allyfire = 0;
	text starting(Text, 0, 0, "SLIME INVADER");
	starting.setup();
	glClearColor(.4f, .5f, .4f, 1.0f);
	worldObjs ZaWarudo;
	ZaWarudo.create();
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
				}
				else if (keys[SDL_SCANCODE_LEFT]&& keys[SDL_SCANCODE_SPACE] && play)
				{
				}
				else if (keys[SDL_SCANCODE_RIGHT] && keys[SDL_SCANCODE_SPACE] && play)
				{
				}
				if (keys[SDL_SCANCODE_LEFT] && keys[SDL_SCANCODE_RIGHT])
				{
				}
				if (keys[SDL_SCANCODE_LEFT])
				{
				}
				if (keys[SDL_SCANCODE_RIGHT])
				{
				}
				else if (keys[SDL_SCANCODE_SPACE] && play)
				{
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
			ZaWarudo.drawer(&program, player);
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
