// #include loads up library files, the order can matter generally load glut.h last
#include "stdafx.h" // this library contains the source file that includes just the standard includes
#include <stdio.h> // this library is for standard input and output
#include <stdlib.h> /* this library defines four variable types, several macros, and various
functions for performing general functions */
#include <time.h> /* this library defines four variable types, two macro and various functions
for manipulating date and time*/
#include <math.h> /* declares a set of functions to compute common mathematical operations
and transformations */
#include "glut.h" // this library is for glut the OpenGL Utility Toolkit

// this defines a constant for the array size
#define RAINSIZE 200
#define FLAMESIZE 500

float theta; // declaring variable for circle

// rotate first (left) wind turbine
float first_current_angle = 0.0; // variable for the current angle of the wind turbine
float first_step_angle = 0.8; // rotation angle per frame
// set the center of the wind turbine
float first_center_x = 168;
float first_center_y = 204;

// rotate second (right) wind turbine
float second_current_angle = 0.0;
float second_step_angle = 0.8;
float second_center_x = 558;
float second_center_y = 204;

// initialise variables to the starting position
float squareX = 0.0f;
float squareY = -0.3f;
float squareZ = 0.0f;

// the size of the screen
int winWidth = 700;
int winHeight = 600;

// set timer for flame particle
int counter = 0;
time_t t;

float rotationAngle = 0;
float angle = 90; /* the angle of the flame: 0 degrees is to the left, 90 degrees straight up,
180 to the right etc */
float flameWidth = 30; // the width of the flame in degrees
float flameCenterX, flameCenterY; // the position of the flame
float fr = 1, fg = 1, fb = 1; // variables for flame colour, set once per flame
// the gravity vector
float gx = 0;
float gy = -0.0005;
float flameStartX = winWidth - 100, flameStartY = winHeight - 100; // the position of the partcle system emitter, wher the rocket should be drawn

struct drop {
	float x = 400;
	float y = 400;
	float inc = 0.01;
	float radius = 5;
	float scale = 1.0;
	float rotationAngle = 0;
	float rotationInc = 1;
};

// the properties of a flame particle are defined in a struct
struct flameParticle {
	float x = 0; // current position x
	float y = 0; // current position y
	float startx = 0; // birth position x
	float starty = 0; // birth position y
	int startTime; // a birthtime in frames when it will be born
	int startRange = 100; // the maximum time at which a birth can happen
	bool started = false; // tracks whether the particle has benn born or not
	float speed = 0.1;
	float radius;
	float startxd = 0; // starting direction vector x value
	float startyd = 0; // starting direction vestor y value
	float xd = 0; // current direction vector x value
	float yd = 0; // current direction vector x value
	float alpha = 1.0; // transparency
};

// creating the flame shape
void circle(double radius, double xc, double yc) {
	int i;
	double angle = 2 * 3.1415 / 20; // circle is drawn using 20 lines.
	double circle_xy[100][40];
	circle_xy[0][0] = radius + xc;
	circle_xy[0][1] = yc;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(5.0, 53.0, 0.0); // set flame position
	glBegin(GL_POLYGON);
	for (i = 1; i < 20; i++) {
		circle_xy[i][0] = radius * cos(i * angle) + xc;
		circle_xy[i][1] = radius * sin(i * angle) + yc;
		glVertex2f(circle_xy[i - 1][0], circle_xy[i - 1][1]);
		glVertex2f(circle_xy[i][0], circle_xy[i][1]);
	}
	glEnd();
	glPopMatrix();
}

flameParticle flame[FLAMESIZE]; // the flame size

void normalise(int i) {
	float mag;
	mag = sqrt((flame[i].xd*flame[i].xd) + (flame[i].yd*flame[i].yd));
	flame[i].xd = flame[i].xd / mag;
	flame[i].yd = flame[i].yd / mag;
}

// we calculate the direction vector of the current particle from the global variable angle and spread
void setDirectionVector(int i) {
	float minAngle, maxAngle, range, newangle;
	double newAngleInRadians; // variable
	int rangeInt;
	minAngle = angle - (flameWidth / 2.0); // calculate the minimum angle the particle could move along
	maxAngle = angle + (flameWidth / 2.0); // calculate the maximum angle
	range = maxAngle - minAngle;
	rangeInt = (int)(range * 100.0);
	newangle = minAngle + ((float)(rand() % rangeInt) / 100.0); // generate a random angle between mi and max angles
	newAngleInRadians = (double)(newangle / 360.0) * (2 * 3.1415); // convert it to radians
	flame[i].xd = (float)cos(newAngleInRadians); // calculate the direction vector x value
	flame[i].yd = (float)sin(newAngleInRadians); // calculate the direction vector y value
}

void initFlame() {
	for (int i = 0; i < FLAMESIZE; i++) {
		flame[i].x = winWidth / 2; // set current start x position
		flame[i].y = 100; // set current start y position
		flame[i].startx = flame[i].x; flame[i].starty = flame[i].y; // set start x and y position
		flame[i].speed = 0.1 + (float)(rand() % 150) / 1000.0; // speed is 0.1 to 0.25
		flame[i].startTime = rand() % flame[i].startRange; // set birth time
		flame[i].radius = (float)(rand() % 15); // random radius
		setDirectionVector(i); // set the current direction vector
		flame[i].startxd = flame[i].xd; flame[i].startyd = flame[i].yd; // set start direction vector to current
	}
	// set colour of flame using rgb
	fr = 1.0;
	fg = 0.4;
	fb = 0.0;
}

void drawFlameParticle(int i) {
	if (!flame[i].started) {
		if (counter == flame[i].startTime) {
			flame[i].started = true;
		}
	}
	if (flame[i].started) {
		glColor4f(fr, fg, fb, flame[i].alpha); // white particles
		circle(flame[i].radius, flame[i].x, flame[i].y);
		// the distance of the points to it origin linear increases
		// decrease the speed by time to smoothly approach to a limit position
		flame[i].y = flame[i].y + (flame[i].yd*flame[i].speed);
		flame[i].yd = flame[i].yd + gy; // this produces a direction vector that is a little longer than 1
		normalise(i); // so the normalize the vector to make length 1
		flame[i].alpha -= 0.0003; // reduce transparency
		flame[i].speed *= 0.982f; // rest the speed when the point is "restarted" at its origin
	}
	if (flame[i].x<0 || flame[i].x>winWidth + 500 ||
		flame[i].y<0 || flame[i].y>winHeight ||
		flame[i].alpha < 1.0f / 256.0f ||
		flame[i].speed < 0.04f) {
		flame[i].x = flame[i].startx; flame[i].y = flame[i].starty;
		flame[i].xd = flame[i].startxd; flame[i].yd = flame[i].startyd;
		flame[i].alpha = 1.0;
		flame[i].speed = 0.1 + (float)(rand() % 150) / 1000.0;
	}
}

void drawFlame() {
	// draw each flame particle
	for (int i = 0; i < FLAMESIZE; i++) {
		drawFlameParticle(i);
	}
	flameStartX += 0.2; // increment rocket position
	// if the rocket is off the screen and 500 pixels to the right, then rest it to 0
	if (flameStartX > winWidth + 500) {
		flameStartX = 0;
	}
	counter++;
}

drop rain[RAINSIZE]; // set rain drop size

void initRain() {
	srand((unsigned)time(&t));
	// set the rain to an infinite loop
	for (int i = 0; i < RAINSIZE; i++) {
		rain[i].x = rand() % winWidth;
		rain[i].y = rand() % winHeight;
		rain[i].inc = 1.5 + (float)(rand() % 100) / 1000.0;
		rain[i].radius = (float)(rand() % 8);
		rain[i].scale = (float)(rand() % 20000) / 1000.0;
		rain[i].rotationAngle = (float)(rand() % 3000) / 1000.0;
		rain[i].rotationInc = (float)(rand() % 100) / 1000.0;
		if ((rand() % 100) > 50) {
			rain[i].rotationInc = -rain[i].rotationInc;
		}
	}
}

void drawParticleShape(int i) {
	glBegin(GL_POINTS);
	glVertex2d(rain[i].x, rain[i].y);
	glEnd();
	glBegin(GL_LINES);
	glVertex2d(rain[i].x, rain[i].y);
	glVertex2d(rain[i].x, rain[i].y + rain[i].radius * 2);
	glEnd();
}

void drawDrop(int i) {
	glColor3f(0.1, 0.6, 0.8); // color of the rain
	glLineWidth(2);
	drawParticleShape(i);
	rain[i].y -= rain[i].inc;
	if (rain[i].y < 0) {
		rain[i].y = winHeight;
	}
}

void drawRain() {
	for (int i = 0; i < RAINSIZE; i++) {
		drawDrop(i);
	}
}

// this is the initialisation function, called once only
void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0); // set what colour you want the background to be
	glMatrixMode(GL_PROJECTION); // set the matrix mode
	glLoadIdentity();
	gluOrtho2D(0.0, winWidth, 0.0, winHeight); // set the projection window size in x an y
}

float elapsedTime = 0, base_time = 0, fps = 0, frames;

void calcFPS() {
	elapsedTime = glutGet(GLUT_ELAPSED_TIME);
	if ((elapsedTime - base_time) > 1000.0) {
		fps = frames * 1000.0 / (elapsedTime - base_time);
		base_time = elapsedTime;
		frames = 0;
	}
	frames++;
}

// draw the first background
void drawBackground1() {
	// draw the green ground
	glBegin(GL_POLYGON);
	glColor3f(0.3, 0.7, 0.0);
	glVertex2f(700, 100);
	glVertex2f(700, 0);
	glVertex2f(0, 0);
	glVertex2f(0, 100);
	glVertex2f(700, 100);
	glEnd();
	// draw the blue sky
	glBegin(GL_POLYGON);
	glColor3f(0.7, 0.7, 0.9);
	glVertex2f(700, 100);
	glVertex2f(700, 700);
	glVertex2f(0, 700);
	glVertex2f(0, 100);
	glVertex2f(700, 100);
	glEnd();
}

// draw the second background
void drawBackground2() {
	// draw the green ground
	glBegin(GL_POLYGON);
	glColor3f(0.7, 0.7, 0.9);
	glVertex2f(700, 600);
	glVertex2f(700, 0);
	glVertex2f(0, 0);
	glVertex2f(0, 600);
	glVertex2f(700, 600);
	glEnd();
}

void drawTree() {
	// first tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(12, 90);
	glVertex2f(19, 90);
	glVertex2f(19, 140);
	glVertex2f(12, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(15 + 30 * cos(theta), 190 + 50 * sin(theta));
	}
	glEnd();
	// second tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(60, 100);
	glVertex2f(66, 100);
	glVertex2f(66, 140);
	glVertex2f(60, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(64 + 30 * cos(theta), 160 + 30 * sin(theta));
	}
	glEnd();
	// third tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(104, 80);
	glVertex2f(114, 80);
	glVertex2f(114, 150);
	glVertex2f(104, 150);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(109 + 40 * cos(theta), 210 + 60 * sin(theta));
	}
	glEnd();
	// fourth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(172, 90);
	glVertex2f(179, 90);
	glVertex2f(179, 140);
	glVertex2f(172, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(175 + 30 * cos(theta), 190 + 50 * sin(theta));
	}
	glEnd();
	// fifth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(238, 90);
	glVertex2f(245, 90);
	glVertex2f(245, 140);
	glVertex2f(238, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(240 + 40 * cos(theta), 180 + 40 * sin(theta));
	}
	glEnd();
	// sixth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(296, 100);
	glVertex2f(302, 100);
	glVertex2f(302, 140);
	glVertex2f(296, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(299 + 20 * cos(theta), 180 + 40 * sin(theta));
	}
	glEnd();
	// seventh tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(354, 80);
	glVertex2f(364, 80);
	glVertex2f(364, 150);
	glVertex2f(354, 150);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(357 + 50 * cos(theta), 200 + 50 * sin(theta));
	}
	glEnd();
	// eighth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(430, 80);
	glVertex2f(440, 80);
	glVertex2f(440, 150);
	glVertex2f(430, 150);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(436 + 40 * cos(theta), 210 + 60 * sin(theta));
	}
	glEnd();
	// ninth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(482, 100);
	glVertex2f(488, 100);
	glVertex2f(488, 140);
	glVertex2f(482, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(482 + 30 * cos(theta), 160 + 30 * sin(theta));
	}
	glEnd();
	// tenth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(538, 90);
	glVertex2f(545, 90);
	glVertex2f(545, 140);
	glVertex2f(538, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(540 + 40 * cos(theta), 180 + 40 * sin(theta));
	}
	glEnd();
	// eleventh tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(604, 80);
	glVertex2f(614, 80);
	glVertex2f(614, 150);
	glVertex2f(604, 150);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(609 + 40 * cos(theta), 210 + 60 * sin(theta));
	}
	glEnd();
	// twelveth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(656, 100);
	glVertex2f(662, 100);
	glVertex2f(662, 140);
	glVertex2f(656, 140);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(659 + 20 * cos(theta), 180 + 40 * sin(theta));
	}
	glEnd();
	// thirteenth tree
	// draw the trunk
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.3, 0.2);
	glVertex2f(714, 80);
	glVertex2f(724, 80);
	glVertex2f(724, 150);
	glVertex2f(714, 150);
	glEnd();
	// draw the branches
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.3, 0.1);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(717 + 50 * cos(theta), 200 + 50 * sin(theta));
	}
	glEnd();
}

// draw the envelope
void drawHotAirBalloon(void) {
	int cutsegment = 45; // size to cut the circle from the bottom
	// the bottom (south) has an angle of -90 degrees
	int start = -90 + cutsegment / 2; // start angle
	// a full angle has 360 degrees (2*PI radians)
	int end = 270 - cutsegment / 2; // end angle
	// formula to draw the circle shape
	glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 0.0); // set the colour
	for (int i = -45; i <= 225; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(355 + 70 * cos(theta), 225 + 90 * sin(theta));
	}
	glEnd();
	// draw parachute valve cord on the left
	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(320, 95);
	glVertex2f(295, 177);
	glEnd();
	// draw parachute valve cord on the right
	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(415, 180);
	glVertex2f(390, 95);
	glEnd();
	// draw propane burner
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.1, 0.1);
	glVertex2f(335, 140);
	glVertex2f(335, 120);
	glVertex2f(375, 120);
	glVertex2f(375, 140);
	glVertex2f(335, 140);
	glEnd();
	// draw wicker basket
	glBegin(GL_POLYGON);
	glColor3f(0.6, 0.45, 0.1);
	glVertex2f(320, 95);
	glVertex2f(320, 40);
	glVertex2f(390, 40);
	glVertex2f(390, 95);
	glVertex2f(320, 95);
	glEnd();
	// first vertical line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(330, 95);
	glVertex2f(330, 40);
	glEnd();
	// second vertical line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(340, 95);
	glVertex2f(340, 40);
	glEnd();
	// third vertical line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(350, 95);
	glVertex2f(350, 40);
	glEnd();
	// fourth vertical line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(360, 95);
	glVertex2f(360, 40);
	glEnd();
	// fifth vertical line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(370, 95);
	glVertex2f(370, 40);
	glEnd();
	// sixth vertical line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(380, 95);
	glVertex2f(380, 40);
	glEnd();
	// first horizontal line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(390, 80);
	glVertex2f(320, 80);
	glEnd();
	// second horizontal line on basket
	glBegin(GL_LINES);
	glColor3f(0.6, 0.25, 0.1);
	glVertex2f(390, 55);
	glVertex2f(320, 55);
	glEnd();
}

// draw the first (left) wind turbine
void drawLeftTurbine() {
	// draw the tower
	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(163, 60);
	glVertex2f(173, 60);
	glVertex2f(173, 200);
	glVertex2f(163, 200);
	glEnd();
	/* to rotate around a pivot you have to define a model matrix, which displace by the inverted pivot, then
	rotates and final transforms back by to the pivot */
	glPushMatrix();
	glTranslatef(first_center_x, first_center_y, 0.0f);
	glRotatef(first_current_angle, 0, 0, 1); // the rotation axis is the z axis (0, 0, 1) because your geometry is drawn in the xy-plane
	first_current_angle += first_step_angle;
	glTranslatef(-first_center_x, -first_center_y, 0.0f);
	// draw first rotor blade
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(173, 209);
	glVertex2f(163, 209);
	glVertex2f(168, 299);
	glEnd();
	// draw second (right) rotor blade
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(170, 201);
	glVertex2f(175, 207);
	glVertex2f(248, 167);
	glEnd();
	// draw third (left) rotor blade
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(162, 207);
	glVertex2f(167, 201);
	glVertex2f(86, 167);
	glEnd();
	// circle in the middle
	float theta;
	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(168 + 7 * cos(theta), 204 + 6.5 * sin(theta));
	}
	glEnd();
	glPopMatrix();
}

// draw the second (right) wind turbine
void drawRightTurbine() {
	// draw the tower
	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(553, 60);
	glVertex2f(563, 60);
	glVertex2f(563, 200);
	glVertex2f(553, 200);
	glEnd();
	// rotate object
	glPushMatrix();
	glTranslatef(second_center_x, second_center_y, 0.0f);
	glRotatef(second_current_angle, 0, 0, 1);
	second_current_angle += second_step_angle;
	glTranslatef(-second_center_x, -second_center_y, 0.0f);
	// draw first rotor blade
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(563, 209);
	glVertex2f(553, 209);
	glVertex2f(558, 299);
	glEnd();
	// draw second (right) rotor blade
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(560, 201);
	glVertex2f(565, 207);
	glVertex2f(638, 167);
	glEnd();
	// draw third (left) rotor blade
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(562, 212);
	glVertex2f(567, 206);
	glVertex2f(486, 192);
	glEnd();
	// circle in the middle
	float theta;
	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
	for (int i = 0; i <= 360; i++) {
		theta = i * 3.142 / 180;
		glVertex2f(558 + 7 * cos(theta), 204 + 6.5 * sin(theta));
	}
	glEnd();
	glPopMatrix();
}

void renderSpacedBitmapString(float x, float y, void *font, char *string) {
	char *c;
	int x1 = x;
	for (c = string; *c != '\0'; c++) {
		glRasterPos2f(x1, y);
		glutBitmapCharacter(font, *c);
		x1 = x1 + glutBitmapWidth(font, *c);
	}
}

void drawSlogan1() {
	int red = rand() % 255; // variable to set red colors
	glColor3f(float(red) / 255.0, 0.0, 0.0); /* flashing red colors */
	char buf[100] = { 0 };
	// fills the buffer according to the format string and the values given in the additional parameters
	sprintf_s(buf, "Fly High With Virgin!"); // what you want the text to say
	float scale = 1.2f; // increase to add more space to the text
	glPushMatrix();
	glScalef(scale, scale, 1.0f); // multiply the current matrix by a general scaling matrix
	renderSpacedBitmapString(250.0f / scale, 520.0f / scale, GLUT_BITMAP_HELVETICA_18, buf); // renders a bitmap character
	glPopMatrix();
}

void drawSlogan2() {
	int red = rand() % 255; // variable to set red colors
	glColor3f(float(red) / 255.0, 0.0, 0.0); /* flashing red colors */
	char buf[100] = { 0 };
	// fills the buffer according to the format string and the values given in the additional parameters
	sprintf_s(buf, "Dream Big and Float \n For Less In 2019!"); // what you want the text to say
	float scale = 1.2f; // increase to add more space to the text
	glPushMatrix();
	glScalef(scale, scale, 1.0f); // multiply the current matrix by a general scaling matrix
	renderSpacedBitmapString(150.0f / scale, 50.0f / scale, GLUT_BITMAP_HELVETICA_18, buf); // renders a bitmap character
	glPopMatrix();
}

void drawSlogan3() {
	int red = rand() % 255; // variable to set red colors
	glColor3f(float(red) / 255.0, 0.0, 0.0); /* flashing red colors */
	char buf[100] = { 0 };
	// fills the buffer according to the format string and the values given in the additional parameters
	sprintf_s(buf, "Fly Anywhere!"); // what you want the text to say
	float scale = 1.2f; // increase to add more space to the text
	glPushMatrix();
	glScalef(scale, scale, 1.0f); // multiply the current matrix by a general scaling matrix
	renderSpacedBitmapString(280.0f / scale, 50.0f / scale, GLUT_BITMAP_HELVETICA_18, buf); // renders a bitmap character
	glPopMatrix();
}

void drawLogo() {
	glColor3f(1.0, 1.0, 1.0); /* flashing red colors */
	char buf[100] = { 0 };
	// fills the buffer according to the format string and the values given in the additional parameters
	sprintf_s(buf, "VIRGIN"); // what you want the text to say
	float scale = 1.2f; // increase to add more space to the text
	glPushMatrix();
	glScalef(scale, scale, 1.0f); // multiply the current matrix by a general scaling matrix
	renderSpacedBitmapString(308.0f / scale, 230.0f / scale, GLUT_BITMAP_TIMES_ROMAN_24, buf); // renders a bitmap character
	glPopMatrix();
}

// global variables
int state = 1;
bool object_visible = true;

// moves the hot air balloon
// the variable state, states the direction of the current movement
void update(int value) {
	// 1 : move up
	// if the object reaches a certain position has reached, then the state has to be changed and a the new start position can be set.
	if (state == 1) {
		squareY += 1.0f;
		if (squareY > 650.0) {
			state = 2;
			squareX = -150.0f;
			squareY = 150.0f;
		}
	}
	// 2 : move right
	else if (state == 2) {
		squareX += 1.0f;
		if (squareX > 300.0) {
			state = 3;
			squareX = 0.0f;
			squareY = 600.0f;
		}
	}
	// 3 : move down
	else if (state == 3) {
		squareY -= 1.0f;
		if (squareY < 0.0) {
			state = 0; // stop at the final position
		}
	}
	glutTimerFunc(25, update, 0); // update every 25 seconds
	object_visible = state != 2; // make the objects disappear in scene 2
}

void display() {
	float app_aspcet = (float)winWidth / (float)winHeight;
	/* with glutGet, using the parameters GLUT_WINDOW_WIDTH respectively GLUT_WINDOW_HEIGHT the size of the current window can be get
	and the current aspect ratio can be calculated */
	int currWidth = glutGet(GLUT_WINDOW_WIDTH);
	int currHeight = glutGet(GLUT_WINDOW_HEIGHT);
	// the view can be perfectly centered to the viewport
	float window_aspcet = (float)currWidth / (float)currHeight;
	if (window_aspcet > app_aspcet) {
		int width = (int)((float)currWidth * app_aspcet / window_aspcet + 0.5f);
		glViewport((currWidth - width) / 2, 0, width, currHeight);
	}
	else {
		int height = (int)((float)currHeight * window_aspcet / app_aspcet + 0.5f);
		glViewport(0, (currHeight - height) / 2, currWidth, height);
	}
	glClear(GL_COLOR_BUFFER_BIT); // clear the color plane of the default framebuffer
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// a switch case based on state with appropriate calls
	switch (state) {
	case 0:
		drawBackground1(); // the first background to appear
		break;
	case 1:
		drawBackground1(); // the second background to appear
		drawSlogan1(); // display the first slogan
		break;
	case 2:
		drawBackground2(); // the third background to appear
		drawSlogan2(); // display the second slogan
		break;
	case 3:
		drawBackground1(); // the last background to appear
		drawSlogan3(); // display the last slogan
		break;
	}
	// draw the turbines dependent on the state
	if (object_visible) {
		drawTree(); // display the trees
		drawLeftTurbine(); // display first (left) wind turbine
		drawRightTurbine(); // display first (right) wind turbine
	}
	glPushMatrix(); // to push (save) the model view matrix before the object is drawn
	/*glTranslatef(squareX, squareY, squareZ);*/
	glTranslatef(squareX, squareY, 0);
	float rectScale = 1.0f + (squareX / 200.0f);
	glScalef(rectScale, rectScale, 1.0f);
	drawFlame(); // display flame
	drawHotAirBalloon(); // display hot air balloon
	drawLogo(); // display logo
	glPopMatrix(); // pop (restore) the model view matrix after the object is drawn
	drawRain(); // display rain drops
	calcFPS();
	counter++;
	glFlush(); // force all drawing to finish
	glutSwapBuffers();
	glutPostRedisplay();
}

// this is the idle function it is called whenever the program is idle
// this has the effect of repeatedly calling the display function
void idle() {
	display();
}

// as with many programming languages the main() function is the entry point for execution of the program
int main(int argc, char** argv) {
	srand(1);
	initRain();
	srand((unsigned)time(&t));
	initFlame(); // initialise first flame work
	glutInit(&argc, argv); // perform the GLUT initialization
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA); // more initialisation
	glutInitWindowSize(winWidth, winHeight); // set window position
	glutInitWindowPosition(100, 100); // set window size
	glutCreateWindow("Hot Air Balloon"); // create a display with a given caption for the title bar
	glEnable(GL_BLEND); // enable blending.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // set blending function.
	init(); // call init function defined above
	glutIdleFunc(idle); // define what function to call when the program is idle
	glutDisplayFunc(display); // define what function to call to draw
	glutTimerFunc(25, update, 0); // update every 25 seconds
	glutMainLoop(); // the last function in the program puts the program into infinite loop
	return 0; // this line exits the program
}