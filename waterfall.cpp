// Modified by:  Kenny Taylor
// Date:  10/4/18
// Original code by Gordon Griesel

//
// WATERFALL SIMULATION WITH RAIN AND WATER SPOUT FUNCTIONS
//

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

using namespace std;

const int MAX_PARTICLES = 10000;
const float GRAVITY = 0.1;

//some structures

struct Vec
{
    float x, y, z;
};

// This Shape struct will work for circles also, yay
struct Shape
{
    float width, height;
    float radius;
    Vec center;
    char label[50];
};

struct Particle
{
    Shape s;
    Vec velocity;
};

class Global
{
public:
    int xres, yres;
    Shape box[5];
    Shape circle[5];
    Particle particle[MAX_PARTICLES];
    int n, boxCount = 0, circleCount = 0;
    Global() {
        xres = 800;
        yres = 600;
        // First (top) box
        box[0].width = 70;
        box[0].height = 10;
        box[0].center.x = 100;
        box[0].center.y = 500;
        strcpy(box[0].label,"Requirements");
        boxCount++;
        // Second box
        box[1].width = 70;
        box[1].height = 10;
        box[1].center.x = 200;
        box[1].center.y = 400;
        strcpy(box[1].label, "Design");
        boxCount++;
        // Third box
        box[2].width = 70;
        box[2].height = 10;
        box[2].center.x = 300;
        box[2].center.y = 300;
        strcpy(box[2].label, "Coding");
        boxCount++;
        // Fourth box
        box[3].width = 70;
        box[3].height = 10;
        box[3].center.x = 400;
        box[3].center.y = 200;
        strcpy(box[3].label, "Testing");
        boxCount++;
        // Fifth (bottom) box
        box[4].width = 70;
        box[4].height = 10;
        box[4].center.x = 500;
        box[4].center.y = 100;
        strcpy(box[4].label, "Maintenance");
        boxCount++;

        // First circle
        circle[0].radius = 70;
        circle[0].center.x = 600;
        circle[0].center.y = 0;
        circleCount++;
        n = 0;
    }
} g;

class X11_wrapper
{
private:
    Display *dpy;
    Window win;
    GLXContext glc;
public:
    ~X11_wrapper()
    {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
    }
    X11_wrapper()
    {
        GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER,
            None };
        int w = g.xres, h = g.yres;
        dpy = XOpenDisplay(NULL);
        if (dpy == NULL) {
            cout << "\n\tcannot connect to X server\n" << endl;
            exit(EXIT_FAILURE);
        }
        Window root = DefaultRootWindow(dpy);
        XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
        if (vi == NULL) {
            cout << "\n\tno appropriate visual found\n" << endl;
            exit(EXIT_FAILURE);
        } 
        Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
        XSetWindowAttributes swa;
        swa.colormap = cmap;
        swa.event_mask =
            ExposureMask | KeyPressMask | KeyReleaseMask |
            ButtonPress | ButtonReleaseMask |
            PointerMotionMask |
            StructureNotifyMask | SubstructureNotifyMask;
        win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
        set_title();
        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, win, glc);
    }
    void set_title()
    {
        //Set the window title bar.
        XMapWindow(dpy, win);
        XStoreName(dpy, win, "3350 Lab1");
    }
    bool getXPending()
    {
        //See if there are pending events.
        return XPending(dpy);
    }
    XEvent getXNextEvent()
    {
        //Get a pending event.
        XEvent e;
        XNextEvent(dpy, &e);
        return e;
    }
    void swapBuffers()
    {
        glXSwapBuffers(dpy, win);
    }
} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();
void makeParticle(int, int);


//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
    srand(time(NULL));
    init_opengl();
    //Main animation loop
    int r, x, y, done = 0;
    while (!done) {
        //Process external events.
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            check_mouse(&e);
            done = check_keys(&e);
        }

        // GENERATE RAIN
        r = rand() % 2;		// 50% chance of generating rain particle
        if (r == 0) {
            cout << "Generating a rain particle at " << x << ", " << y;
            cout << endl; 
            x = rand() % 800;
            y = 400 + rand() % 200;
            makeParticle(x, y);	// Disable rain for now
        }
        // END GENERATE RAIN

        // WATER SPOUT ON TOP BOX
        r = rand() % 2;			// 50% chance of generating water particle
        if (r == 0) {
            // Randomize drop location +/- 5 pixels
            x = (rand() % 10) - 5;
            y = (rand() % 10) - 5;
            makeParticle(100 + x, 550 + y);
        }
        // END WATER SPOUT

        movement();
        render();
        x11.swapBuffers();
    }
    return 0;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(int x, int y)
{
    if (g.n >= MAX_PARTICLES)
        return;
    cout << "makeParticle() " << x << " " << y << endl;
    //position of particle
    Particle *p = &g.particle[g.n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = -4.0;
    p->velocity.x =  1.0;
    ++g.n;
}

void check_mouse(XEvent *e)
{
    static int savex = 0;
    static int savey = 0;

    if (e->type != ButtonRelease &&
        e->type != ButtonPress &&
        e->type != MotionNotify) {
        //This is not a mouse event that we care about.
        return;
    }
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed
            int y = g.yres - e->xbutton.y;
            makeParticle(e->xbutton.x, y);
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed
            return;
        }
    }
    if (e->type == MotionNotify) {
        //The mouse moved!
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
            savex = e->xbutton.x;
            savey = e->xbutton.y;

            // MAKE MULTIPLE PARTICLES APPEAR ON MOUSEOVER
            //int y = g.yres - e->xbutton.y;
            //makeParticle(e->xbutton.x, y);
            //makeParticle(e->xbutton.x, y);
            //makeParticle(e->xbutton.x, y);
            //makeParticle(e->xbutton.x, y);
            //makeParticle(e->xbutton.x, y);
        }
    }
}

int check_keys(XEvent *e)
{
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
        switch (key) {
            case XK_1:
                //Key 1 was pressed
                break;
            case XK_a:
                //Key A was pressed
                break;
            case XK_Escape:
                //Escape key was pressed
                return 1;
        }
    }
    return 0;
}

void movement()
{
    int i, j;
    cout << "  entering movement(), checking " << g.n << " particles..";
    cout << endl;
    if (g.n <= 0)
        return;
    
    for (i = 0; i < g.n; i++) {
        Particle *p = &g.particle[i];
        //cout << "    particle(" << i << ")" << endl;
        //cout << "      original position/velocity:  (" << p->s.center.x << ", " << p->s.center.y << ") [" << p->velocity.x << ", " << p->velocity.y << "]" << endl;
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;

        p->velocity.y -= GRAVITY;

        // Check for collision with rectangles...
        cout << "      checking for collisions with " << g.boxCount;
        cout << " rectangles" << endl;
        for (j = 0; j < g.boxCount; j++) {
            Shape *s = &g.box[j];
            // Reverse y velocity if particle center enters the box!
            if (p->s.center.y < (s->center.y + s->height) &&			// Below top of the box
                p->s.center.x > s->center.x - s->width &&			// Right of left side of the box
                p->s.center.x < s->center.x + s->width &&			// Left of right side of the box
                p->s.center.y > (s->center.y - s->height))			// Above bottom of the box
            {
                p->s.center.y = (s->center.y + s->height);			// Added to fix particles getting stuck in the box
                p->velocity.y *= -1.0;		// Reverse y velocity when hitting the box
                p->velocity.y *= 0.3;		//Absorb energy from hitting the box.  Use 0.3 to look like water
            }
        }

        // Check for collision with each circle
        cout << "      checking for collisions with " << g.circleCount;
        cout << " circles" << endl;
        for (j = 0; j < g.circleCount; j++) {
            Shape *s = &g.circle[j];
            if (p->s.center.x > (s->center.x - s->radius) &&                                  // Right of left side of the circle
                p->s.center.x < (s->center.x + s->radius) &&                                  // Left of right side of the circle
                // Under the upper arc of the circle (general form)
                p->s.center.y < (sqrt(pow(s->radius,2) - pow(p->s.center.x,2) + 2*s->center.x*p->s.center.x - pow(s->center.x,2)) - s->center.y) &&
                // Above the lower arc of the circle (general form)
                p->s.center.y > -(sqrt(pow(s->radius,2) - pow(p->s.center.x,2) + 2*s->center.x*p->s.center.x - pow(s->center.x,2)) - s->center.y))
            {
                // Added to fix particles getting stuck unter the circle
                p->s.center.y = (sqrt(pow(s->radius,2) - pow(p->s.center.x,2) + 2*s->center.x*p->s.center.x - pow(s->center.x,2)) - s->center.y);
                //p->velocity.x = what?
                //p->velocity.y = what?
            }
        }

        //check for off-screen
        if (p->s.center.y < 0.0) {
            cout << "off screen" << endl;
            g.particle[i] = g.particle[g.n - 1];
            --g.n;
        }
        //cout << "      final position/velocity:  (" << p->s.center.x << ", " << p->s.center.y << ") [" << p->velocity.x << ", " << p->velocity.y << "]" << endl;
	}
    cout << "  leaving movement()" << endl;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...
    //
    //draw a box
    Shape *s;
    float w, h;
    int i;
    glColor3ub(90,140,90);
    for (i = 0; i < g.boxCount; i++) {
        s = &g.box[i];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        glBegin(GL_QUADS);
            glVertex2i(-w, -h);
            glVertex2i(-w,  h);
            glVertex2i( w,  h);
            glVertex2i( w, -h);
        glEnd();
        glPopMatrix();
    }
    //

    //Draw the particle here
    //glPushMatrix();
    //glColor3ub(150,160,220);
    for (i = 0; i < g.n; i++) {
        glPushMatrix();
        glColor3ub(150,160,220);
        Vec *c = &g.particle[i].s.center;
        w =
        h = 2;
        glBegin(GL_QUADS);
            glVertex2i(c->x-w, c->y-h);
            glVertex2i(c->x-w, c->y+h);
            glVertex2i(c->x+w, c->y+h);
            glVertex2i(c->x+w, c->y-h);
        glEnd();
        glPopMatrix();
    }

    // Draw the circles
    for (i = 0; i < g.circleCount; i++) {
        glPushMatrix();
        glColor3ub(160, 82, 45);
        glTranslatef(g.circle[i].center.x, g.circle[i].center.y, 0.0);
        // Sweep the full 360 degrees with one vertex per degree
        glBegin(GL_POLYGON);
        for (int j = 0; j < 360; j++) {
            glVertex3f(g.circle[i].radius*cos(j),
                g.circle[i].radius*sin(j), 0.0);            
        }
        glEnd();
        glPopMatrix();
    }
	
    //Draw your 2D text here
    unsigned int c = 0x00ffff44;
    Rect r;
    for (i = 0; i < g.boxCount; i++) {
        s = &g.box[i];
        r.bot= s->center.y - s->height/2;
        r.left = s->center.x;
        ggprint8b(&r, 16, c, s->label);
    }

}

