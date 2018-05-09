#ifdef _WINDOWS
#include <GL/glew.h>
#endif

//#include <GL/glew.h>
#include <list>
#include <cstdlib>
#include <ctime>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
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

enum curState { NONE, PHYSICAL, PLAYER, ENEMY, TEXT };
bool play = false;
float TEXXMAX = 384;
float TEXYMAX = 256;
float TILE_SIZE = .1f;
int TEXXNUM = TEXXMAX / 16.0f;
int TEXYNUM = TEXYMAX / 16.0f;
float TEXXSIZE = 1.0f / 24.0f;
float TEXYSIZE = 1.0f / 16.0f;
float FRICTION = 3.0f;
float GRAVITY = -1.0f;
Matrix VIEWMATRIX;
float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
	float retVal = dstMin + ((value - srcMin) / (srcMax - srcMin) * (dstMax - dstMin));
	if (retVal < dstMin) {
		retVal = dstMin;
	}
	if (retVal > dstMax) {
		retVal = dstMax;
	}
	return retVal;
}
float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t * v1;
}
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
struct Particle
{
	Particle(float x, float y, float velx, float vely, float timealive);
	float xy[2];
	Vector3 vel;
	float alivetime;
	bool alive = true;
};
/*struct particlemitter*/
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
	bool back = false;
	bool yback = false;
	float bscalex = 0.0f;
	float bscalie = 0.0f;
	float scalex = 0.0f;
	float scalie = 0.0f;
	Vector3 vel;
	Vector3 acc;
	bool alive = true;
	bool ydowncol;
	bool yupcol;
	bool xleftcol;
	bool xrightcol;
	bool right = false;
	bool left = false;
	int inverser = 1;
	std::string type = "";
	float penx = 0.0f;
	float peny = 0.0f;
	bool leftstop = false;
	bool rightstop = false;
	float scale_y = 0.0f;
	float scale_x = 0.0f;
	bool jump = false;
	bool dying = false;
	bool slapsoundx = false;
	bool slapsoundy = false;
	//std::list<particlemitter*> particles;
	bool animatingdown = false;
	bool animatingup = false;
	bool doneanimating = false;
	float rotation = -90 * ((3.1459) / 180);
	int soundplayer = 0;
	bool player = true;

	gameObj();
	gameObj(float newx, float newy, std::string typer);
	float lerp(float v0, float v1, float t);
	void update(float elapsed, gameObj* obj);
	void updatex(float elapsed);
	void updatey(float elapsed);
	void changeTex(float texx, float texy, float texwidth, float texheight, float texxsize, float texysize);
	bool collision(gameObj* obj);
	void draw(ShaderProgram* program, GLuint img, float elapsed);
	void fullcollisiony(float elapsed);//, gameObj* obj);
	void fullcollisionx(float elapsed);//, gameObj* obj);
									   // takes in world coordinates, and tile coordinates, and see if those world coordinates points collide with the tiles
	bool testcollision(float worldx, float worldy, int x, int y);
	void animator(float elapsed);
	void deathanimator(float elapsed);
	void soundhandler(float elapsed, ShaderProgram* program);
	//void particleupdater(float elapsed, ShaderProgram* program);
};
struct Sounder
{
	std::vector<Mix_Chunk*> sounds;
	std::vector<Mix_Music*> music;
	~Sounder();
	void initsounds();
};

struct Enemies : gameObj
{
	bool flipper = false;
	int buffer = 0;
	Enemies(float newx, float newy, std::string typer);
	void EdgeCollider();
	void AIish(gameObj* player);
	float reacting = false;
	int reaction = rand() % 4;
};
struct worldObjs
{
	FlareMap Map;
	std::vector<float> objBlocks;
	std::vector<float> texCoordData;
	std::vector<Enemies> entities;
	gameObj* Player;
	Matrix* view = &VIEWMATRIX;
	int num = 0;

	float textxcoords(int object);
	float textycoords(int object);
	void maptime(std::string world);
	void Blocker();
	//void collisionx(gameObj* obj);
	//void collisiony(gameObj* obj);
	//void docollisions(gameObj* obj);
	void entityCollisions(float elapsed);
	void create(std::string world);
	void drawer(ShaderProgram* program, GLuint img);
	void entitydrawer(ShaderProgram* program, GLuint img, float elapsed);
	void entityupdater(float elapsed);
	void entitygenerate();
	//void dealwithxy(gameObj* obj);
};
worldObjs* CURRENTWORLD;
Sounder* ALLSOUNDS;
//game Objects
gameObj::gameObj()
{
	model = new Matrix;
	view = &VIEWMATRIX;
	alive = 1;
	acc.x = 0;
	acc.y = 0;
	vel.y = 0.0f;
	vel.x = 0.0f;
	ydowncol = false;
	yupcol = false;
	xleftcol = false;
	xrightcol = false;
	animatingup = false;
	animatingdown = false;
	//// << alive << std::endl;
}
gameObj::gameObj(float newx, float newy, std::string typer)
{
	model = new Matrix;
	view = &VIEWMATRIX;
	x = newx;
	y = newy;
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
	scale_x = 10;
	scale_y = 10;
}
float gameObj::lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t * v1;
}
void gameObj::update(float elapsed, gameObj* obj)
{
	//collision(obj, elapsed);
	if (left)
	{
		vel.x = -.1f;
	}
	else if (right)
	{
		vel.x = .1f;
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
	//// << type << x << " : " << y << std::endl;
}
void gameObj::changeTex(float texx, float texy, float texwidth, float texheight, float texxsize, float texysize)
{
	float textx = texx / texxsize;
	float texty = texy / texysize;
	float textxpl = (texx + texwidth) / texxsize;
	float textypl = (texy + texheight) / texysize;
	texCoords = new float[12]{ textxpl, texty, textx, texty, textx, textypl, /*half*/ textxpl, texty, textx, textypl, textxpl, textypl };
}
bool gameObj::collision(gameObj* obj)//                  /
{//                           
	//// << xleftcol << " " << xrightcol << " " << yupcol << " " << ydowncol << std::endl;
	bool inrange = false;//                        /
	if (alive && obj->alive && obj->type != type)// don't mind this. this is just collision stuff.
	{//                                          /
		float paddleroof = obj->y + //          /
			(obj->height / 2.0f); // legacy code. Paddle is the other object.
		float paddlefloor = obj->y - //       /
			(obj->height / 2.0f);
		float paddleleft = obj->x - //      /
			(obj->width / 2.0f);
		float paddleright = obj->x + //   /
			(obj->width / 2.0f);
		//                              /
		float ballroof = y + (height / 2); // ball is this object.
		float ballfloor = y - (height / 2);
		float ballright = x + (width / 2);
		float ballleft = x - (width / 2);
		bool ballxpaddle = ((ballright >= paddleleft && ballleft <= paddleright) || (ballright >= paddleleft && ballleft < paddleleft) || (ballleft <= paddleright && ballright > paddleright));
		inrange = (ballfloor <= paddleroof && ballxpaddle && ballroof >= paddlefloor);
	}
	if (inrange)
	{
		if (!(dying))
		{
			dying = true;
			if (obj->width < .16)
			{
				obj->width += .01;
				obj->height += .01;
			}
		}
	}
	return inrange;
}
void gameObj::draw(ShaderProgram* program, GLuint img, float elapsed)
{
	if (alive)
	{
		if (dying)
		{
			deathanimator(elapsed);
		}
		if (animatingdown)
		{
			scale_x = lerp(scale_x, 1, elapsed * 2);
			scale_y = lerp(scale_y, 1, elapsed * 2);
			if (scale_x <= 1.5 || scale_y <= 1.5)
			{
				animatingdown = false;
				rotation = -90 * ((3.1459) / 180);
			}
		}
		else if (animatingup)
		{
			scale_x = lerp(scale_x, 200, elapsed * .1);
			scale_y = lerp(scale_y, 200, elapsed * .1);
			rotation = lerp(rotation, (90 * ((3.1459) / 180)), elapsed * .1);
			//// << scale_x << std::endl;
			if (scale_x >= 50 || scale_y >= 50)
			{
				Mix_PlayChannel(-1, ALLSOUNDS->sounds[4], 0);
				doneanimating = true;
			}
		}
		else
		{
			if (player)
			{
				animator(elapsed);
			}
		}
		//// << vel.x << " " << vel.y << std::endl;
		model->Identity();
		model->Translate(x, y, 0.0f);
		model->Scale(width, height, 0);
		model->Scale(scale_x, scale_y, 0);
		model->Rotate(rotation);
		//// << xleftcol << xrightcol << yupcol << ydowncol << std::endl;
		soundhandler(elapsed, program);
		//model->SetRotation(cos(vel.x)/sin(vel.y));
		//// << scalex << " " << scalie << std::endl;
		//scalex = lerp(scalex, 0.0f, elapsed *.01);
		//scalie = lerp(scalie, 0.0f, elapsed * .01);
		//scalex = 0;
		//scalie = 0;
		//particleupdater(elapsed, program);
		if (type == "ENEMY" || type == "TEXT")
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
bool gameObj::testcollision(float worldx, float worldy, int tilex, int tiley)
{

	int xvals = (int)(worldx / TILE_SIZE);
	int yvals = (int)(-worldy / TILE_SIZE);

	float tileRoofc = (-TILE_SIZE * tiley);
	float tileFloorc = (-TILE_SIZE * tiley) - TILE_SIZE;
	float tileLeftc = (TILE_SIZE * tilex);
	float tileRightc = (TILE_SIZE * tilex) + TILE_SIZE;
	int val = CURRENTWORLD->Map.mapData[tiley][tilex];
	if (val != 0
		&& val != 20
		&& val != 21
		&& val != 22
		&& val != 23
		&& val != 44
		&& val != 45
		&& val != 46
		&& val != 47
		&& val != 68
		&& val != 69
		&& val != 70
		&& val != 71
		&& val != 92
		&& val != 93
		&& val != 94
		&& val != 95
		&& val != 117
		&& val != 118
		&& val != 141
		&& val != 142
		&& val != 165
		&& val != 166
		&& val != 116
		&& val != 140
		&& val != 164
		&& val != 119
		&& val != 143
		&& val != 167)
	{
		if (worldx <= tileRightc && worldx >= tileLeftc && worldy >= tileFloorc && worldy <= tileRoofc)
		{
			return true;
		}
		if ((val == 173 || val == 174 || val == 175))
		{
			dying = true;
		}
		if ((val == 145 || val == 169) && player)
		{
			if (!(animatingup))
			{
				Mix_PlayChannel(-1, ALLSOUNDS->sounds[4], 0);
			}
			animatingup = true;
		}
	}
	return false;

}
void gameObj::fullcollisionx(float elapsed)//, gameObj* obj)
{
	updatex(elapsed);
	int xvals = (int)(x / TILE_SIZE);
	int yvals = (int)(-y / TILE_SIZE);
	float left = (x - fabs(width / 2));
	float right = (x + fabs(width / 2));
	float roof = (y + fabs(height / 2));
	float floor = (y - fabs(height / 2));
	bool posleft = false;
	bool posright = false;
	//bool movedleft = false;
	//bool movedright = false;
	bool stoppedleft = leftstop;
	bool stoppedright = rightstop;
	//// << stoppedleft << " " << stoppedright << std::endl;
	//left collisions
	// So I for some reason it seems like rounding has effected the corners. 
	//If not for the - .0001f on the next line, the x collision would detect and react before the y collision,
	// making you jerk to the right if you hit a corner.
	// weird! but cool!
	if (testcollision(left, roof - .0001f, xvals - 1, yvals - 1) || testcollision(left, floor + .0001, xvals - 1, yvals + 1) || testcollision(left, y, xvals - 1, yvals))
	{
		//// << "Left Col" << std::endl;
		if (!back)
		{
			scalex = fabs(vel.x);
		}
		vel.x = 0;
		penx += fabs((x - (fabs(width) / 2)) - ((TILE_SIZE * (xvals - 1)) + TILE_SIZE));
		posleft = true;
		stoppedleft = true;
		//Mix_PlayChannel(-1,ALLSOUNDS->sounds[0],0);
	}
	if (testcollision(right, roof - .0001f, xvals + 1, yvals - 1) || testcollision(right, floor + .0001, xvals + 1, yvals + 1) || testcollision(right, y, xvals + 1, yvals))
	{
		//// << "Right Col" << std::endl;
		if (!back)
		{
			scalex = fabs(vel.x);
		}
		vel.x = 0;
		penx -= fabs(((x + (fabs(width) / 2))) - (TILE_SIZE * (xvals + 1)));
		posright = true;
		stoppedright = true;
		//Mix_PlayChannel(-1, ALLSOUNDS->sounds[0], 0);
	}
	// Stuff for animations and stopping at a wall, so that it doesn't calculate if it's "stopped"
	if (stoppedleft && vel.x > 0)
	{
		stoppedleft = false;
	}
	if (stoppedright && vel.x < 0)
	{
		stoppedright = false;
	}
	// Time to deal with all the penetration issues.
	x += penx;
	penx = 0.0f;
	xleftcol = posleft;
	xrightcol = posright;
	leftstop = stoppedleft;
	rightstop = stoppedright;
}
void gameObj::fullcollisiony(float elapsed)//, gameObj* obj)
{
	updatey(elapsed);
	int xvals = (int)(x / TILE_SIZE);
	int yvals = (int)(-y / TILE_SIZE);
	//// << yvals << std::endl;
	float left = (x - fabs(width / 2));
	float right = (x + fabs(width / 2));
	float roof = (y + fabs(height / 2));
	float floor = (y - fabs(height / 2));
	bool posup = false;
	bool posdown = false;
	//// << "it happen" << std::endl;
	//// << "down " << xvals << " " << yvals << " " << x << " " << y << std::endl;

	//down collisions
	if ((testcollision(left + .0001f, floor, xvals - 1, yvals + 1) || testcollision(x, floor, xvals, yvals + 1) || testcollision(right - .0001f, floor, xvals + 1, yvals + 1)))
	{
		//// << "down col" << std::endl;
		if (!yback)
		{
			scalie = fabs(vel.y);
		}
		vel.y = 0;
		peny += fabs((y - (fabs(height) / 2)) - (-TILE_SIZE * (yvals + 1)));
		posdown = true;
		//Mix_PlayChannel(-1, ALLSOUNDS->sounds[0], 0);
	}
	//// << "thisfar" << std::endl;
	//up col
	//// << "up" << std::endl;
	//// << " Left " << testcollision(left, roof, xvals - 1, yvals - 1) << std::endl;
	//// << " mid " << testcollision(x, roof, xvals, yvals - 1) << std::endl;
	//// << " right " << testcollision(right, roof, xvals + 1, yvals - 1) << std::endl;
	if ((testcollision(left + .0001, roof, xvals - 1, yvals - 1)) || testcollision(x, roof, xvals, yvals - 1) || testcollision(right - .0001f, roof, xvals + 1, yvals - 1))
	{
		//// << "up col" << std::endl;
		if (!yback)
		{
			scalie = fabs(vel.y);
		}
		vel.y = 0;
		peny -= fabs((y + (fabs(height)) / 2) - ((-TILE_SIZE * (yvals))));
		posup = true;
		//Mix_PlayChannel(-1, ALLSOUNDS->sounds[0], 0);
	}
	// Time to deal with all the penetration issues.
	y += peny;
	peny = 0.0f;
	yupcol = posup;
	ydowncol = posdown;
}
void gameObj::updatex(float elapsed)
{
	if (!animatingup && !animatingdown)
	{
		if (left && !(xleftcol))
		{
			vel.x += -.02f;
			//x += -.0002f;
		}
		else if (right && !(xrightcol))
		{
			vel.x += .02f;
			//x += .0002f;
		}
		//// << jump << " " << leftstop << " " << rightstop << std::endl;
		if (jump)
		{
			if (xleftcol)
			{
				vel.x += .4;
			}
			else if (xrightcol)
			{
				vel.x -= .4;
			}
		}
		if (!(xleftcol) && !(xrightcol))
		{
			vel.x = lerp(vel.x, 0.0f, elapsed * FRICTION);
			if (fabs(vel.x) < .001f)
			{
				vel.x = 0.0f;
			}
		}
		vel.x += acc.x * elapsed * inverser;
		x += vel.x * elapsed * inverser;
		jump = false;
		if (!player)
		{
			//// << inverser << std::endl;
		}
	}
}
void gameObj::updatey(float elapsed)
{
	if (!animatingup && !animatingdown)
	{
		if (!(ydowncol))
		{
			if (xleftcol || xrightcol)
			{
				vel.y = lerp(vel.y, GRAVITY, elapsed * .02f);
				if (vel.y > 0)
				{
					vel.y = 0;
				}
				if (fabs(vel.y) >= .03)
				{
					if (vel.y < 0)
					{
						vel.y = -.03;
					}
					else
					{
						vel.y = .03;
					}
				}
			}
			else
			{
				vel.y = lerp(vel.y, GRAVITY, elapsed * .6f);
			}
		}
		if (jump)
		{
			vel.y += .6f;
			//jump = false;
		}
		vel.y += acc.y * elapsed;
		y += vel.y * elapsed;
	}
}
void gameObj::animator(float elapsed)
{
	if (!(back))
	{
		bscalex = lerp(bscalex, scalex, elapsed * 10);
		if (bscalex <= scalex - .01f)
		{
			back = true;
		}
		//// << scalex << std::endl;
	}
	else
	{
		scalex = lerp(scalex, 0.0f, elapsed * 10);
		bscalex = scalex;
		if (scalex < .003f)
		{
			scalex = 0;
			bscalex = 0;
			back = false;
		}
	}
	if (!(yback))
	{
		bscalie = lerp(bscalie, scalie, elapsed * 10);
		if (bscalie < scalie - .1f)
		{
			yback = true;
		}
	}
	else
	{
		scalie = lerp(scalie, 0.0f, elapsed * 10);
		bscalie = scalie;
		//// << scalie << std::endl;
		if (scalie < .003f)
		{
			scalie = 0;
			bscalie = 0;
			yback = false;
		}
	}
	//// << bscalie << std::endl;
	scale_y = mapValue(-(fabs(vel.x * 50)) + bscalex * 6.0f, 0.0, 5.0, 1.0, 1.6);
	scale_x = mapValue(-fabs(vel.y * 50) + bscalie * 12.0f, 0.0, 5.0, 1.0, 1.6);
}
void gameObj::deathanimator(float elapsed)
{
	width = lerp(width, 0.0f, elapsed * 2);
	height = lerp(width, 0.0f, elapsed * 2);
	if (width <= .001f || height <= .001f)
	{
		dying = false;
		alive = false;
	}
}
void gameObj::soundhandler(float elapsed, ShaderProgram* program)
{
	if (player)
	{
		soundplayer = rand() % 3;
	}
	if (xleftcol || xrightcol)
	{
		if (!(slapsoundx))
		{
			//particles.push_back(new particlemitter(vel, x, y, elapsed, program));
			Mix_PlayChannel(-1, ALLSOUNDS->sounds[soundplayer], 0);
		}
		slapsoundx = true;
	}
	else
	{
		slapsoundx = false;
	}
	if ((yupcol || ydowncol))
	{
		if (!(slapsoundy))
		{
			//particles.push_back(new particlemitter(vel, x, y, elapsed, program));
			Mix_PlayChannel(-1, ALLSOUNDS->sounds[soundplayer], 0);
		}
		slapsoundy = true;
	}
	else
	{
		slapsoundy = false;
	}
}
/*void gameObj::particleupdater(float elapsed, ShaderProgram* program)
{
	bool popped = false;
	for (particlemitter* i : particles)
	{
		if (i->done && (!popped))
		{
			particles.pop_front();
			popped = true;
		}
		i->shoot(elapsed, program);
	}
}*/

//text box

struct textbox
{
	float verts[12] = { -0.5f, -0.5f, 0.5f, -.5f, .5f, .5f, -0.5f, -0.5f, 0.5f, 0.5f, -.5f, 0.5f };
	Matrix model;
	Matrix* view;
	float* texCoords;
	float x;
	float y;
	float scale_x = 1;
	float scale_y = 1;
	float width = .5;
	float height = .5;


	~textbox()
	{
		delete(verts);
		//delete(texCoords);
	}
	textbox(float newx, float newy)
	{
		x = newx;
		y = newy;
		view = &VIEWMATRIX;
	}

	void animate(float to, float elapsed)
	{
		scale_x = lerp(scale_x, to, elapsed*.2);
		scale_y = lerp(scale_y, to, elapsed*.2);
	}
	void changeTex(float texx, float texy, float texwidth, float texheight, float texxsize, float texysize)
	{
		float textx = texx / texxsize;
		float texty = texy / texysize;
		float textxpl = (texx + texwidth) / texxsize;
		float textypl = (texy + texheight) / texysize;
		texCoords = new float[12]{ textxpl, texty, textx, texty, textx, textypl, /*half*/ textxpl, texty, textx, textypl, textxpl, textypl };
	}

	void draw(ShaderProgram* program, GLuint img, float elapsed, float to)
	{
		//::cout << "Identity!" << x << " " << y << std::endl
		view->Identity();
		model.Identity();
		model.Translate(x, y, 0.0f);
		//animate(to, elapsed);
		model.Rotate(180.0f * (3.1415926f / 180.0f));
		model.Scale(width, height, 0);
		model.Scale(scale_x, scale_y, 0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		program->SetModelMatrix(model);
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

};

//text 
struct text
{
	float x = 0;
	float y = 0;
	float width = 3.5;
	float height = 2.5;
	float textnum;
	std::string whattext;
	std::vector<textbox*> alientext;
	GLuint img;

	/*~text()
	{
		for (int i = 0; i < textnum - 1; i++)
		{
			delete(alientext[i]);
		}
	}*/
	text(GLuint spritesheet, float nx, float ny, std::string thetext)
	{
		img = spritesheet;
		x = nx;
		y = ny;
		whattext = thetext;
		textnum = thetext.size();
	}
	void printall()
	{
		// << x << " " << y << " " << width << " " << height << " " << textnum << " " << whattext << " " << img << std::endl;
	}
	float textxcoords(char letter)
	{
		int stringcor = (int)letter;
		float returnval = ((float)(stringcor % 16) / 16.0f);
		return returnval;
	}

	float textycoords(char letter)
	{
		int stringcor = (int)letter;
		float returnval = ((float)(stringcor / 16) / 16.0f);
		return returnval;
	}

	void populate(float offsetx, float offsety)
	{
		int counterx = 0;
		int offset = 0;
		if (textnum > 13)
		{ 
			offset = 1;
		}
		for (float ax = x + -((width+offset) / 2); ax < x + ((width+offset) / 2); ax += ((x + width+offset) / (textnum )))
		{
			alientext.push_back(new textbox(ax + offsetx, y + offsety));
			counterx++;
		}
	}

	void texturer()
	{
		int counterx = 0;
		for (float ax = x + -((x + width) / 2); ax < x + ((x + width) / 2); ax += ((x + width) / textnum))
		{
			alientext[counterx]->changeTex(textxcoords(whattext[counterx]), textycoords(whattext[counterx]), (1.0f / 16.0f), (1.0f / 16.0f), 1, 1);
			counterx++;
		}
	}

	void draw(ShaderProgram* program, float elapsed)
	{
		for (int i = 0; i < textnum;i++)
		{
			// << textnum << " " << alientext.size() << std::endl;
			alientext[i]->draw(program, img, elapsed, 1);
		}
	}

	void setup(float offsetx, float offsety)
	{
		populate(offsetx, offsety);
		texturer();
	}
};

//Enemies
Enemies::Enemies(float newx, float newy, std::string typer)
{
	x = newx;
	y = newy;
	if (typer == "TEXT")
	{
		height = .5f;
		width = .5f;
	}
	type = typer;
	scale_x = 1;
	scale_y = 1;
	soundplayer = 3;
	player = false;
}
void Enemies::EdgeCollider()
{
	int xvals = (int)abs((x / TILE_SIZE));
	int yvals = (int)abs((int)(y / TILE_SIZE));
	//// << CURRENTWORLD->Map.mapData[yvals + 1][xvals + 1] << std::endl;

		if ((CURRENTWORLD->Map.mapData[yvals + 1][xvals + 1] == 0 || CURRENTWORLD->Map.mapData[yvals + 1][xvals - 1] == 0 || xleftcol || xrightcol) && !(flipper) && !(reacting)) {
			flipper = true;
			//// << "yes" << std::endl;
			//// << inverser << std::endl;
			inverser *= -1;
		}
		if (flipper)
		{
			//// << buffer << std::endl;
			buffer += 1;
		}
		if (buffer == 10)
		{
			//// << "Yeah" << std::endl;
			flipper = false;
			buffer = 0;
		}
		if (ydowncol && reaction != 3)
		{
			vel.x = .1f + acc.x * inverser;
			//// << vel.x << " " << std::endl;
			//// << vel.x << " " << std::endl;
		}
}
void Enemies::AIish(gameObj* player)
{
	float distance = sqrt(((x - player->x)*(x - player->x)) + ((y - player->y)* (y - player->y)));
	//// << distance << std::endl;
	if (distance <= .7)
	{

		switch (reaction)
		{
		case 1:
		{
			float in = x - player->x;
			if (in < 0)
			{
				inverser = -1;
			}
			else
			{
				inverser = 1;
			}
			//// << "run at" << std::endl;
			acc.x = .2 * inverser;
		}
		break;
		case 2:
		{
			float out = x - player->x;
			if (out < 0)
			{
				inverser = 1;
			}
			else
			{
				inverser = -1;
			}
			//// << "run at " << std::endl;
			acc.x = .2 * inverser;
		}
		break;
		case 3:
		{
			scale_x = (rand() % 10) / 15.0f + .5;
			scale_y = (rand() % 10)/15.0f + .5;
			//// << scale_x << " " << scale_y << std::endl;
			vel.x = 0;
			vel.y = 0;
		}
		break;
		}
		reacting = true;
	}
}
//World Obj
float worldObjs::textxcoords(int object)
{
	float returnval = (float)(((int)object) % TEXXNUM) / (float)TEXXNUM;
	return returnval;
}
float worldObjs::textycoords(int object)
{
	float returnval = (float)(((int)object) / TEXYNUM) / (float)TEXYNUM;
	return returnval;
}
void worldObjs::maptime(std::string world)
{
	Map.Load(world);
}
void worldObjs::Blocker()
{
	float offset = 0.0f;
	for (int y = 0; y < Map.mapHeight; y++) {
		for (int x = 0; x < Map.mapWidth; x++) {
			//// << "x: " << "y: " << std::endl;
			int val = Map.mapData[y][x];
			if (val != 0) {
				num += 6;
				float u = ((float)(((int)Map.mapData[y][x]) % TEXXNUM)) / (float)TEXXNUM;
				float v = ((float)(((int)Map.mapData[y][x]) / TEXXNUM)) / (float)TEXYNUM;

				objBlocks.insert(objBlocks.end(), {
					(TILE_SIZE * x) - offset, (-TILE_SIZE * y) - offset,
					(TILE_SIZE * x) - offset, ((-TILE_SIZE * y) - TILE_SIZE) - offset,
					((TILE_SIZE * x) + TILE_SIZE) - offset, ((-TILE_SIZE * y) - TILE_SIZE) - offset,
					(TILE_SIZE * x) - offset, (-TILE_SIZE * y) - offset,
					((TILE_SIZE * x) + TILE_SIZE) - offset, ((-TILE_SIZE * y) - TILE_SIZE) - offset,
					((TILE_SIZE * x) + TILE_SIZE) - offset, (-TILE_SIZE * y) - offset
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
	//// << "OUT OF BLOCKER" << std::endl;
	//// << objBlocks.size() << std::endl;
	//// << objBlocks.max_size() << std::endl;
}
//void worldObjs::docollisions(gameObj* obj)
//
	//collisiony(obj);
	//collisionx(obj);
	//dealwithxy(obj);
//
void worldObjs::entityCollisions(float elapsed)
{
	//// << "INTO ENTITY COLLISION" << std::endl;
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i].alive || entities[i].dying)
		{
			entities[i].EdgeCollider();
			entities[i].fullcollisionx(elapsed);
			entities[i].fullcollisiony(elapsed);
			entities[i].collision(Player);
			entities[i].AIish(Player);
		}
	}
	//// << Player->jump << std::endl;
	Player->fullcollisiony(elapsed);
	Player->fullcollisionx(elapsed);

	//// << "OUTTA ENTITY COLLISION" << std::endl;
}
void worldObjs::create(std::string world)
{
	maptime(world);
	Blocker();
	entitygenerate();
}
void worldObjs::drawer(ShaderProgram* program, GLuint img)
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
void worldObjs::entitydrawer(ShaderProgram* program, GLuint img, float elapsed)
{
	//// << "INTO ENTITY DRAWIN" << std::endl;
	for (int i = 0; i< entities.size(); i++)
	{
		entities[i].draw(program, img, elapsed);
	}
	Player->draw(program, img, elapsed);
	//// << "OUTTA ENTITY DRAWIN" << std::endl;
}
void worldObjs::entityupdater(float elapsed)
{
	//// << "INTO ENTITY UPDATIN" << std::endl;
	for (int i = 0; i< entities.size(); i++)
	{
		entities[i].update(elapsed, &entities[i]);
		if (entities[0].acc.x != 0 || entities[0].vel.x != 0)
		{
			//// << entities[0].acc.x << entities[0].vel.x << std::endl;
		}
	}
	Player->update(elapsed, Player);
	//// << "OUTTA ENTITY UPDATIN" << std::endl;
}
void worldObjs::entitygenerate()
{
	//// << "INTO ENTITY GENERATION" << std::endl;
	int p = 0;
	float u;
	float v;
	//// << Map.entities.size() << std::endl;
	for (FlareMapEntity i : Map.entities)
	{
		//::cout << i.type << std::endl;
		if (i.type == "Enemy") {
			u = ((float)(((int)90) % TEXXNUM)) / (float)TEXXNUM;
			v = ((float)(((int)90) / TEXXNUM)) / (float)TEXYNUM;
			Enemies newObj = *(new Enemies(i.x*TILE_SIZE, i.y*-TILE_SIZE, i.type));
			newObj.texCoords = new float[12]{
				u, v,
				u, v + (TEXYSIZE),
				u + TEXXSIZE, v + (TEXYSIZE),
				u, v,
				u + TEXXSIZE, v + (TEXYSIZE),
				u + TEXXSIZE, v
			};
			//// << i.type << std::endl;
			entities.push_back(newObj);
		}
		if (i.type == "Player") {
			u = ((float)(((int)42) % TEXXNUM)) / (float)TEXXNUM;
			v = ((float)(((int)42) / TEXXNUM)) / (float)TEXYNUM;
			Player = new gameObj(i.x*TILE_SIZE, i.y*-TILE_SIZE, i.type);
			gameObj* fuck = new gameObj();
			//// << "hello" << std::endl;
			//// << Player << " fuck" << std::endl;
			//// << "hello" << std::endl;
			//// << fuck << std::endl;
			//// << "goodbye" << std::endl;
			Player->texCoords = new float[12]{
				u, v,
				u, v + (TEXYSIZE),
				u + TEXXSIZE, v + (TEXYSIZE),
				u, v,
				u + TEXXSIZE, v + (TEXYSIZE),
				u + TEXXSIZE, v
			};
		}
	}
	//// << Player << std::endl;
}

//sounder
void Sounder::initsounds()
{
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	//// << "sound " << std::endl;
	sounds.push_back((Mix_LoadWAV("slapsound.wav")));
	sounds.push_back((Mix_LoadWAV("slapsound2.wav")));
	sounds.push_back((Mix_LoadWAV("slapsound3.wav")));
	sounds.push_back((Mix_LoadWAV("slapsound4.wav")));
	sounds.push_back((Mix_LoadWAV("tim.wav")));
	music.push_back(Mix_LoadMUS("d_e1m1.mid"));
	music.push_back(Mix_LoadMUS("d_e1m8.mid"));
	music.push_back(Mix_LoadMUS("d_e2m1.mid"));
	music.push_back(Mix_LoadMUS("d_e2m6.mid"));
	music.push_back(Mix_LoadMUS("d_e3m6.mid"));
	music.push_back(Mix_LoadMUS("d_e1m9.mid"));
	music.push_back(Mix_LoadMUS("d_e2m7.mid"));	
	music.push_back(Mix_LoadMUS("d_e3m4.mid"));
	//music.push_back(Mix_LoadMUS("d_e2m7.mid"));

	//// << sounds.size() << std::endl;
	for (Mix_Chunk* i : sounds)
	{
		Mix_VolumeChunk(i, 35);
	}
		Mix_VolumeMusic(10);
}
Sounder::~Sounder()
{
	for (Mix_Chunk* i : sounds)
	{
		delete(i);
	}
	for (Mix_Music* d : music)
	{
		delete(d);
	}
}
//particleer
/*
void particlemitter::Update(float elapsed)
{
	int i = 0;
	for (int p = 0; p <= particles.size(); p++)
	{

		particles[p]->alivetime -= elapsed;
		if (particles[p]->alivetime <= 0 || particles[p]->alive == false)
		{
			i++;
			particles[p]->alive = false;
		}
		else
		{
			particles[p]->vel.x = lerp(particles[p]->vel.x, 0.0f, elapsed*.001f);
			particles[p]->vel.y = lerp(particles[p]->vel.y, GRAVITY, elapsed*.1f);
			particles[p]->xy[0] += particles[p]->vel.x;
			particles[p]->xy[1] += particles[p]->vel.y;
		}
	}
	if (i >= particles.size())
	{
		done = true;
	}
}
void particlemitter::Render(ShaderProgram* program)
{
	for (Particle* i : particles)
	{
		if (!i->alive)
		{
			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, i->xy);
			glEnableVertexAttribArray(program->positionAttribute);
			glDrawArrays(GL_POINTS, 0, 2);
		}
	}
}
void particlemitter::initparticles()
{
	int ter;
	int time;
	float randomy;
	float randomx;
	ter = rand() % 20 + 5;
	time = rand() % 10 + 5;
	randomy = (rand()) / 10;
	randomx = (rand()) / 10;
	for (int i = 0; i < ter; i++)
	{
		particles.push_back(new Particle(position.x, position.y, vel.x * randomx, vel.y* randomy, time));
	}
}
void particlemitter::shoot(float elapsed, ShaderProgram* program)
{
	Update(elapsed);
	Render(program);
}
particlemitter::particlemitter(Vector3 newvel, float posx, float posy, float elapsed, ShaderProgram* program)
{
	vel.x = newvel.x;
	vel.y = newvel.y;
	vel.z = newvel.z;
	position.x = posx;
	position.y = posy;
	initparticles();
	//shoot(elapsed, program);
}
particlemitter::~particlemitter()
{
	for (Particle* i : particles)
	{
		delete(i);
	}
}
Particle::Particle(float x, float y, float velx, float vely, float timealive)
{
	xy[0] = x;
	xy[1] = y;
	vel.x = velx;
	vel.y = vely;
	alivetime = timealive;
}*/

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
	displayWindow = SDL_CreateWindow("Slime Invader", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 576, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	SDL_Event event;
	bool done = false;
	srand(time(NULL));
	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Matrix projectionMatrix;
	Matrix playerModelMatrix;
	ALLSOUNDS = new Sounder();
	ALLSOUNDS->initsounds();
	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	float lastFrameTicks = 0.0f;
	GLuint player = LoadTexture(RESOURCE_FOLDER"dirt-tiles.png");
	GLuint Texter = LoadTexture(RESOURCE_FOLDER"font1.png");
	if (player == NULL || Texter == NULL)
	{
		assert("IMAGEERROR");
	}
	glUseProgram(program.programID);
	int music = 0;
	int currworld = 0;
	bool musicplayin = false;
	text starting(Texter, 0, 0, "SLIME INVADER");
	text ending(Texter, 0, 0, "GAME OVER X TO QUIT");
	text pause(Texter, 0, 0, "PAUSED X TO QUIT");
	text youwin(Texter, 0, 0, "YOU WIN! PRESS X TO QUIT");
	//starting.printall();
	//ending.printall();
	//text moretext(Text, 0, 0, "E");
	starting.setup(0, 0);
	ending.setup(0, 0);
	pause.setup(0, 0);
	youwin.setup(0, 0);
	//moretext.setup(0, 0);
	glClearColor(.4f, .5f, .4f, 1.0f);
	std::vector<worldObjs*> Zawarudo;
	Zawarudo.push_back(new worldObjs());
	Zawarudo.push_back(new worldObjs());
	Zawarudo.push_back(new worldObjs());
	Zawarudo.push_back(new worldObjs());
	//worldObjs ZaWarudo3;
	Zawarudo[0]->create("TileMapLevelTwo.txt");
	CURRENTWORLD = Zawarudo[0];
	//::cout << CURRENTWORLD->Player << std::endl;
	//// << fuck << std::endl;
	Zawarudo[1]->create("TileMapLevelOne.txt");
	Zawarudo[2]->create("TheNest.txt");
	Zawarudo[3]->create("Finale.txt");
	program.SetProjectionMatrix(projectionMatrix);
	int state = 0;
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		glClear(GL_COLOR_BUFFER_BIT);
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			if (state == 3 || state == 2 || state == 4)
			{
				if (keys[SDL_SCANCODE_X])
				{
					done = true;
				}
			}
			if (state == 1)
			{
				/*if (keys[SDL_SCANCODE_UP])
				{
				CURRENTWORLD->Player->y += .01f;
				}
				if (keys[SDL_SCANCODE_DOWN])
				{
				CURRENTWORLD->Player->y -= .01f;
				}*/
				if (keys[SDL_SCANCODE_P])
				{
					state = 3;
				}
				if (keys[SDL_SCANCODE_LEFT])
				{
					if (keys[SDL_SCANCODE_SPACE] && state == 1)
					{
						if (CURRENTWORLD->Player->ydowncol || (CURRENTWORLD->Player->xleftcol || CURRENTWORLD->Player->xrightcol))
						{
							//// << CURRENTWORLD->Player->ydowncol << CURRENTWORLD->Player->leftstop << CURRENTWORLD->Player->rightstop << std::endl;
							CURRENTWORLD->Player->jump = true;
						}
					}
					CURRENTWORLD->Player->left = true;
					CURRENTWORLD->Player->right = false;
				}
				else if (keys[SDL_SCANCODE_RIGHT])
				{

					if (keys[SDL_SCANCODE_SPACE] && state == 1)
					{
						if (CURRENTWORLD->Player->ydowncol || (CURRENTWORLD->Player->xleftcol || CURRENTWORLD->Player->xrightcol))
						{
							//// << CURRENTWORLD->Player->ydowncol << CURRENTWORLD->Player->leftstop << CURRENTWORLD->Player->rightstop << std::endl;
							CURRENTWORLD->Player->jump = true;
						}
					}
					CURRENTWORLD->Player->left = false;
					CURRENTWORLD->Player->right = true;
				}
				else
				{
					if (keys[SDL_SCANCODE_SPACE] && state == 1)
					{
						//// << "JUMPING?" << std::endl;
						if (CURRENTWORLD->Player->ydowncol || (CURRENTWORLD->Player->xleftcol || CURRENTWORLD->Player->xrightcol))
						{
							CURRENTWORLD->Player->jump = true;
						}
					}
					CURRENTWORLD->Player->left = false;
					CURRENTWORLD->Player->right = false;
				}
				if (keys[SDL_SCANCODE_SPACE] && state == 1)
				{
					if (CURRENTWORLD->Player->ydowncol || (CURRENTWORLD->Player->xleftcol || CURRENTWORLD->Player->xrightcol))
					{
						CURRENTWORLD->Player->jump = true;
					}
				}
				if (keys[SDL_SCANCODE_0] && state == 1)
				{
					//CURRENTWORLD = &ZaWarudo2;
				}
			}
			else
			{
				if (keys[SDL_SCANCODE_SPACE] && state == 0)
				{
					state = 1;
				}
			}
		}
		glClearColor(0, 0, 0, 0);
		switch (state)
		{
		case 1:
		{
			if (!Mix_PlayingMusic())
			{
				Mix_PlayMusic((ALLSOUNDS->music[music]), -1);
			}
			//// << "INTO PLAY" << std::endl;
			//CURRENTWORLD->entityupdater(elapsed);
			CURRENTWORLD->entityCollisions(elapsed);
			//// << "ENTITY VECTOR" << std::endl;
			//CURRENTWORLD->Player->collision(&CURRENTWORLD->entities[0], elapsed);
			VIEWMATRIX.Identity();
			float viewsc = .5f;
			VIEWMATRIX.Scale((1 / TILE_SIZE) * viewsc, (1 / TILE_SIZE) * viewsc, 1);
			//// << "ENTITY VECTOR, TRANSLATION" << std::endl;
			VIEWMATRIX.Translate(-CURRENTWORLD->Player->x, -CURRENTWORLD->Player->y, (0.0f));
			program.SetViewMatrix(VIEWMATRIX);
			CURRENTWORLD->drawer(&program, player);
			CURRENTWORLD->entitydrawer(&program, player, elapsed);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);
			if (!(CURRENTWORLD->Player->alive))
			{
				//// << "dead" << std::endl;
				state = 2;
			}
			// << currworld << std::endl;
			if (currworld <= 3 && CURRENTWORLD->Player->doneanimating)
			{
				state = 4;
			}
			else if (CURRENTWORLD->Player->doneanimating)
			{
				currworld += 1;
				Zawarudo[currworld]->Player->animatingdown = true;
				Zawarudo[currworld]->Player->scale_x = 180;
				Zawarudo[currworld]->Player->scale_y = 180;
				CURRENTWORLD = Zawarudo[currworld];
				Mix_PlayMusic((ALLSOUNDS->music[currworld]), -1);
			}
			SDL_GL_SwapWindow(displayWindow);
		}
		break;
		case 2:
		{
			//Mix_PlayMusic(ALLSOUNDS->music[6], -1);
			//// << "dead " << std::endl;
			ending.draw(&program, elapsed);
			//			glClearColor(.4f, .5f, .4f, 1.0f);
			SDL_GL_SwapWindow(displayWindow);
		}
		break;
		case 3:
		{
			Mix_Pause(-1);
			pause.draw(&program, elapsed);
			SDL_GL_SwapWindow(displayWindow);
		}
		break;
		case 0:
		{
			Mix_PlayMusic(ALLSOUNDS->music[6], -1);
			starting.draw(&program, elapsed);
			//				glClearColor(.4f, .5f, .4f, 1.0f);
			SDL_GL_SwapWindow(displayWindow);
		}
		break;
		case 4:
		{
			youwin.draw(&program, elapsed);
			glClearColor(0, 0, 0, 0);
			SDL_GL_SwapWindow(displayWindow);

		}
		}

	}
	for (worldObjs* i : Zawarudo)
	{
		delete(i);
	}
	SDL_Quit();
	return 0;
}
