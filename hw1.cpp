//Name: David A. Hernandez II
//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

extern "C" {
#include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 5000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures
struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape box[5];
    Shape circle;
    Particle particle[MAX_PARTICLES];
    int n;
    int lastMousex, lastMousey;
    bool bubbler;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows(); 
    init_opengl();
    //declare game object
    Game game;
    game.n=0;
    game.bubbler = false;

    for (int i = 0; i < 5; i++) {
        game.box[i].width = 100;
        game.box[i].height = 15;
        game.box[i].center.x = 130 + (i*85);
        game.box[i].center.y = 475 - (i*75);
    }
    game.circle.radius = 140.0;
    game.circle.center.x = 400 + (5*65);
    game.circle.center.y = 300 - (5*60);

    //this big loop is called "the controller"
    //start animation "event loop"
    while(!done) {
        while(XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e, &game);
        }
        movement(&game);//checks if anything moves
        render(&game);//renders images to the screen
        glXSwapBuffers(dpy, win);//swaps buffers
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "David A. Hernandez II CS335 Lab1 Waterfall Model");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    //allows fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

#define rnd() (float) rand() / (float) RAND_MAX

void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
        return;
    //std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd() - 0.5;
    p->velocity.x = rnd() + 0.5;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed
            int y = WINDOW_HEIGHT - e->xbutton.y;
            makeParticle(game, e->xbutton.x, y);
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed
            return;
        }
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        game->lastMousex = savex;
        game->lastMousey = WINDOW_HEIGHT - savey;
        if (++n < 10)
            return;
        int y = WINDOW_HEIGHT - e->xbutton.y;
        makeParticle(game, e->xbutton.x, y);
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
        //You may check other keys here.
        if (key == XK_b) {
            game->bubbler = !game->bubbler;
        }
    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;
    if (game->bubbler) {
        for (int i = 0; i <= MAX_PARTICLES; i++) {
            makeParticle(game, game->lastMousex, game->lastMousey);
        }
    }

    if (game->n <= 0)
        return;

    for (int i = 0; i < game->n; i++) {
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= GRAVITY;

        //check for collision with shapes...
        Shape *s;
        for (int j = 0; j < 5; j++) {
            s = &game->box[j];
            if (p->s.center.y >= s->center.y - (s->height) &&
                    p->s.center.y <= s->center.y + (s->height) &&
                    p->s.center.x >= s->center.x - (s->width) &&
                    p->s.center.x <= s->center.x + (s->width)) {
                p->s.center.y = p->s.center.y + (s->height*2);
                p->velocity.x += 0.06;
                p->velocity.y *= rnd() * -0.3;
            } 
        }
        //check for off-screen
        if (p->s.center.y < 0.0) {
            //std::cout << "off screen" << std::endl;
            game->particle[i] = game->particle[game->n-1];  //value at end of array switches with
            //particle that left the screen
            game->n--;

        }

        //check for collision with circle
        Shape *c;
        c = &game->circle;
        float diffx, diffy, distance;
        diffx = p->s.center.x - c->center.x;
        diffy = p->s.center.y - c->center.y;
        distance = sqrt((diffx*diffx) + (diffy*diffy));
        if (distance < c->radius) {
            //moving particle to circle edge
            p->s.center.x = c->center.x + (diffx/distance) * c->radius + 1.01;
            p->s.center.y = c->center.y + (diffy/distance) * c->radius + 1.01;
            //collision
            p->velocity.x -= 0.08;
            p->velocity.y *= rnd() * -0.3;
        }
    }
}

void render(Game *game)
{
    float w, h;
    Rect r;
    glClear(GL_COLOR_BUFFER_BIT);//Clears the screen

    //Draw shapes...
    //creating text for boxes
    const char *text[5] = {"Requirement", "Design", "Coding", "Testing", "Maintenance"};
    //draw box
    Shape *s;
    for (int j = 0; j < 5; j++) {
        glColor3ub(255,0,255);
        s = &game->box[j];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        r.left = s->width - 100 ;
        r.bot = s->height - 20 ;
        glBegin(GL_QUADS);
        glVertex2i(-w,-h);
        glVertex2i(-w, h);
        glVertex2i( w, h);
        glVertex2i( w,-h);
        glEnd();
        ggprint8b(&r, 32, 0xffffffff, text[j]);
        glPopMatrix();
    }

    //draw all particles here
    glPushMatrix();
    for (int i = 0; i < game->n; i++) {
        //Giving different shades of blue to particles
        glColor3ub(0,0,255 - 5*i);
        Vec *c = &game->particle[i].s.center;
        w = 3;
        h = 3;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
        glPopMatrix();
    }

    //draw circle
    for (int k = 0; k < 1; k++) {
        Shape *c;
        c = &game->circle;
        float x, y, radius;
        x = c->center.x;
        y = c->center.y;
        radius = c->radius;
        glPushMatrix();
        glColor3ub(255,0,0);
        int triangleAmount = 90;
        float twicePi = 2.0f * M_PI;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x,y);    //center of circle
        for (int i = 0; i <= triangleAmount; i++) {
            glVertex2f(x + (radius * cos(i * twicePi / triangleAmount)),
                    y + (radius * sin(i * twicePi / triangleAmount)));
        }
        glEnd();
        glPopMatrix();
    }

}
