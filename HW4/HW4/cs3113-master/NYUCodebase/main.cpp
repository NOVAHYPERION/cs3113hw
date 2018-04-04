#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include "FlareMap.h"

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
float TILE_SIZE = .1f;
int TEXXNUM = TEXXMAX / 16.0f;
int TEXYNUM = TEXYMAX / 16.0f;
float TEXXSIZE = 1.0f / 24.0f;
float TEXYSIZE = 1.0f / 16.0f;
float FRICTION = 1.0f;
float GRAVITY = -1.0f;
Matrix VIEWMATRIX;

class Vector3 {
public:
	float x;
	float y;
	float z;
	Vector3() {}
	Vector3(float newx, float newy, float newz) {
		x = newx;
		y = newy;
		z = newz;
	}

	bool operator==(Vector3& rhs) {
		return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z);
	}

	void operator=(Vector3& rhs)
	{
		this->x = rhs.x;
		this->y = rhs.y;
		this->z = rhs.z;
	}
};

struct gameObj
{
	float verts[12] = { -0.5f, -0.5f, 0.5f, -.5f, .5f, .5f, -0.5f, -0.5f, 0.5f, 0.5f, -.5f, 0.5f };
	float* texCoords;
	Matrix* model;
	Matrix* view;
	float height = .1f;
	float width = .1f;
	float x = 0.0f;
	float y = 0.0f;
	Vector3 vel;
	Vector3 acc;
	bool alive = true;
	bool ydowncol;
	bool yupcol;
	bool xleftcol;
	bool xrightcol;
	bool right;
	bool left;
	std::string type = "";
	
	gameObj(float newx, float newy, std::string typer)
	{
		model = new Matrix;
		view = &VIEWMATRIX;
		x = newx;
		y = newy;
		if (typer == "TEXT")
		{
			height = .5f;
			width = .5f;
		}
		alive = 1;
		acc.x = 0;
		acc.y = 0;
		vel.y = 0.0f;
		vel.x = 0.0f;
		type = typer;
		ydowncol = false;
		yupcol = false;
		xleftcol = false;
		xrightcol = false;
	}

	float lerp(float v0, float v1, float t) {
		return (1.0 - t)*v0 + t * v1;
	}

	void update(float elapsed, gameObj* obj)
	{
		collision(obj, elapsed);
		if (left)
		{
			vel.x = -.5f;
		}
		else if (right)
		{
			vel.x = .5f;
		}
		
		if (!(xleftcol) && !(xrightcol))
		{
			vel.x = lerp(vel.x, 0.0f, elapsed * FRICTION);
		}
		else
		{
			bool col;
		}
		if (!ydowncol) {
			vel.y = lerp(vel.y, GRAVITY, elapsed * .5f);
		}
		vel.x += acc.x * elapsed;
		vel.y += acc.y * elapsed;
		x += vel.x * elapsed;
		y += vel.y * elapsed;
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
		if (alive && obj->alive && obj->type != type)
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
		if (inrange)
		{
			obj->alive = 0;
		}

		return inrange;
	}

	//bool collisiony() {}

	void draw(ShaderProgram* program, GLuint img, float elapsed)
	{
		if (alive)
		{
			model->Identity();
			model->Translate(x, y, 0.0f);
			model->Scale(width, height, 0);
			if (type == "ENEMY" || type == "TEXT" )
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
			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);
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
			alientext[counterx] = new gameObj(ax, y, "TEXT");
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

	void draw(ShaderProgram* program, float elapsed)
	{
		for(gameObj* n: alientext)
		{
			n->draw(program, img,elapsed);
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
	std::vector<gameObj> entities;
	Matrix* view = &VIEWMATRIX;
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
		Map.Load("Tilemap.txt");
	}
	void Blocker()
	{
		float offset = 0.0f;
		for (int y = 0; y < Map.mapHeight; y++) {
			for (int x = 0; x < Map.mapWidth; x++) {
				int val = Map.mapData[y][x];
				if (val != 0) {
					num += 6;
					float u = ((float)(((int)Map.mapData[y][x]) % TEXXNUM)) / (float)TEXXNUM;
					float v = ((float)(((int)Map.mapData[y][x]) / TEXXNUM)) / (float)TEXYNUM;

					objBlocks.insert(objBlocks.end(), {
						(TILE_SIZE * x) -offset, (-TILE_SIZE * y) - offset,
						(TILE_SIZE * x) - offset, ((-TILE_SIZE * y) - TILE_SIZE) - offset,
						((TILE_SIZE * x) + TILE_SIZE) - offset, ((-TILE_SIZE * y) - TILE_SIZE) - offset,
						(TILE_SIZE * x) - offset, (-TILE_SIZE * y) - offset,
						((TILE_SIZE * x) + TILE_SIZE) - offset, ((-TILE_SIZE * y) - TILE_SIZE) - offset,
						((TILE_SIZE * x) + TILE_SIZE)- offset, (-TILE_SIZE * y)- offset
						});

					texCoordData.insert(texCoordData.end(), {
						u, v,
						u, v + (TEXYSIZE),
						u + TEXXSIZE, v + (TEXYSIZE),
						u, v,
						u + TEXXSIZE, v + (TEXYSIZE),
						u + TEXXSIZE, v
						});
				}
			}
		}
	}

	void collisionx(gameObj* obj)
	{
		int xvals = (int)(obj->x/ TILE_SIZE);
		int yvals = (int)(-obj->y / TILE_SIZE);
		//worldToTileCoordinates(obj->x, obj->y, xvals, yvals);
		bool leftcol = false;
		bool rightcol = false;

		for (int y = 0; y < Map.mapHeight; y++)
		{
			for (int x = 0; x < Map.mapWidth; x++)
			{
				int val = Map.mapData[y][x];
				if (val != 0 && val != 20 && val != 21 && val != 22 && 23 && val != 44 && val != 45 && val != 46 && val != 47 && val != 68 && val != 69 && val != 70 && val != 71 && val != 92 && val != 93 && val != 94 && val != 95 && val != 117 && val != 118 && val != 141 && val != 142 && val != 165 && val != 166) {
					//bool fuck = ((obj->x - fabs(obj->width / 2)) <= (TILE_SIZE * x) + TILE_SIZE && xvals >= x && yvals == y);
					if ((obj->x - fabs(obj->width / 2)) <= (TILE_SIZE * x) + TILE_SIZE && xvals >= x && yvals == y)//((obj->y + fabs(obj->height / 2)) <= (-TILE_SIZE * y ) && (obj->y - fabs(obj->height/2) >= (-TILE_SIZE *y + TILE_SIZE ))))
					{
						obj->vel.x = 0;
						obj->x += fabs(((obj->x - (fabs(obj->width) / 2))) - ((TILE_SIZE * x) + TILE_SIZE)) + 0.0000000001f;
						leftcol = true;
					}
					if ((obj->x + fabs(obj->width / 2)) >= (TILE_SIZE * x) && xvals <= x && yvals == y)//(obj->y - (obj->height / 2)) == (-TILE_SIZE * y - TILE_SIZE))
					{
						obj->vel.x = 0;
						obj->x -= fabs((obj->x + (fabs(obj->width) / 2))) - ((TILE_SIZE * x)) + 0.0000000001f;
						rightcol = true;
					}
				}
			}
		}
		obj->xleftcol = leftcol;
		obj->xrightcol = rightcol;
	}

	void collisiony(gameObj* obj)
	{
		int xvals = (int)(obj->x / TILE_SIZE);
		int yvals = (int)(-obj->y / TILE_SIZE);
		bool downcol = false;
		bool upcol = false;
		for (int y = 0; y < Map.mapHeight; y++)
		{
			for (int x = 0; x < Map.mapWidth; x++)
			{
				int val = Map.mapData[y][x];
				if (val != 0 && val != 20 && val!= 21 && val != 22 && 23 && val != 44 && val != 45 && val != 46 && val != 47 && val != 68 && val != 69 && val != 70 && val != 71 && val != 92 && val != 93 && val != 94 && val != 95 && val != 117 && val != 118 && val != 141 && val != 142 && val != 165 && val != 166) {
					if ((obj->y - (obj->height / 2)) <= (-TILE_SIZE * y) && xvals == x && obj->y + (obj->height/2) > (-TILE_SIZE * y) )
					{
						obj->vel.y = 0;
						obj->y += fabs((obj->y - (fabs(obj->width) / 2)) - (-TILE_SIZE * y)) + .0000001f;
						downcol = true;
					}

					else if ((obj->y + (obj->height / 2)) >= (-TILE_SIZE * y) - TILE_SIZE && xvals == x && obj->y - (obj->height / 2) < (-TILE_SIZE * y) - TILE_SIZE)
					{
						obj->vel.y = 0;
						obj->y -= fabs((obj->y + (fabs(obj->width) / 2)) - ((-TILE_SIZE * y))- TILE_SIZE) - 0.0000001f;
						upcol = true;
					}
				}
			}
		}
		obj->ydowncol = downcol;
		obj->yupcol = upcol;
	}

	void docollisions(gameObj* obj)
	{
		collisionx(obj);
		collisiony(obj);
	}

	void entityCollisions()
	{
		for (int i = 0; i< 2; i++)
		{
			docollisions(&entities[i]);
		}
	}

	void create()
	{
		maptime();
		Blocker();
		entitygenerate();
	}

	void drawer(ShaderProgram* program, GLuint img)
	{
		Matrix model;

		program->SetModelMatrix(model);
		program->SetViewMatrix(*view);
		glBindTexture(GL_TEXTURE_2D, img);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, &objBlocks[0]);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, &texCoordData[0]);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, num);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}

	void entitydrawer(ShaderProgram* program, GLuint img, float elapsed)
	{
		for (int i = 0; i< 2; i++)
		{
			entities[i].draw(program, img,elapsed);
		}
	}

	void entityupdater(float elapsed)
	{
		for (int i = 0; i< 2; i++)
		{
			entities[i].update(elapsed, &entities[i]);
		}
	}

	void entitygenerate()
	{
		int p = 0;
		float u;
		float v;
		for (FlareMapEntity i : Map.entities)
		{
			if (i.type == "Enemy") {
				u = ((float)(((int)90) % TEXXNUM)) / (float)TEXXNUM;
				v = ((float)(((int)90) / TEXXNUM)) / (float)TEXYNUM;
			}
			else if (i.type == "Ally") {
				u = ((float)(((int)42) % TEXXNUM)) / (float)TEXXNUM;
				v = ((float)(((int)42) / TEXXNUM)) / (float)TEXYNUM;
			}
			gameObj newObj = *(new gameObj(i.x*TILE_SIZE,i.y*-TILE_SIZE, i.type));
			newObj.texCoords = new float[12]{
				u, v,
				u, v + (TEXYSIZE),
				u + TEXXSIZE, v + (TEXYSIZE),
				u, v,
				u + TEXXSIZE, v + (TEXYSIZE),
				u + TEXXSIZE, v
			};
  			entities.push_back(newObj);
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
	program.SetProjectionMatrix(projectionMatrix);
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
				if (keys[SDL_SCANCODE_LEFT])
				{
					if (keys[SDL_SCANCODE_SPACE] && play)
					{
						if (ZaWarudo.entities[1].ydowncol)
						{
							ZaWarudo.entities[1].vel.y += .2f;
						}
					}
					ZaWarudo.entities[1].left = true;
					ZaWarudo.entities[1].right = false;
				}
				else if (keys[SDL_SCANCODE_RIGHT])
				{
					if (keys[SDL_SCANCODE_SPACE] && play)
					{
						if (ZaWarudo.entities[1].ydowncol)
						{
							ZaWarudo.entities[1].vel.y += .2f;
						}
					}
					ZaWarudo.entities[1].left = false;
					ZaWarudo.entities[1].right = true;
				}
				else
				{
					if (keys[SDL_SCANCODE_SPACE] && play)
					{
						if (ZaWarudo.entities[1].ydowncol)
						{
							ZaWarudo.entities[1].vel.y += .2f;
						}
					}
					ZaWarudo.entities[1].left = false;
					ZaWarudo.entities[1].right = false;
				}
				if (keys[SDL_SCANCODE_SPACE] && play)
				{
					if (ZaWarudo.entities[1].ydowncol)
					{
						ZaWarudo.entities[1].vel.y += .2f;
					}
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
		glClearColor(.4f, .5f, .4f, 1.0f);
		if (play)
		{
			ZaWarudo.entityupdater(elapsed);
			ZaWarudo.entityCollisions();
			ZaWarudo.entities[1].collision(&ZaWarudo.entities[0], elapsed);
			VIEWMATRIX.Identity();
			VIEWMATRIX.Scale(1 / TILE_SIZE, 1 / TILE_SIZE, 1);
			VIEWMATRIX.Translate(-ZaWarudo.entities[1].x, -ZaWarudo.entities[1].y, (0.0f));
			program.SetViewMatrix(VIEWMATRIX);
			ZaWarudo.drawer(&program, player);
			ZaWarudo.entitydrawer(&program, player,elapsed);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
			SDL_GL_SwapWindow(displayWindow);
		}
		else
		{
			starting.draw(&program,elapsed);
			glClearColor(.4f, .5f, .4f, 1.0f);
			SDL_GL_SwapWindow(displayWindow);
		}
	}
	SDL_Quit();
	return 0;
}
