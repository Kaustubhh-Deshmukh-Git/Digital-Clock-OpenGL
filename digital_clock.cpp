/*
 ============================================================
  DIGITAL CLOCK - CG Mini Project
  College : SE IT (2024 Pattern) - SPPU
  Course  : PCC-252-ITT Computer Graphics

  CG Concepts Used:
  1. DDA Line Drawing Algorithm     → Vertical segments
  2. Bresenham's Line Algorithm     → Horizontal segments
  3. 2D Translation Transformation  → Digit positioning
  4. RGB Color Model                → Segment colors
  5. 7-Segment Display Logic        → Digit rendering

  Tools   : C++ with OpenGL + GLUT
  OS      : Linux (Ubuntu) / Windows (MinGW)

  Compile (Linux):
    g++ digital_clock.cpp -o digital_clock -lGL -lGLU -lglut
  Compile (Windows MinGW):
    g++ digital_clock.cpp -o digital_clock -lopengl32 -lglu32 -lfreeglut
  Run:
    ./digital_clock
 ============================================================
*/

#include <GL/glut.h>
#include <cmath>
#include <ctime>
#include <string>
#include <iostream>

// ─── Window Settings ───────────────────────────────────────
#define WIN_WIDTH  900
#define WIN_HEIGHT 400
#define WIN_TITLE  "Digital Clock - CG Mini Project (DDA + Bresenham)"

// ─── Segment Dimensions ────────────────────────────────────
#define SEG_LEN    0.12f   // length of each segment
#define SEG_GAP    0.01f   // gap between segments
#define SEG_THICK  3.5f    // line thickness (pixels)
#define DIGIT_SPACE 0.32f  // space between digits
#define COLON_SPACE 0.12f  // space for colon

// ─── Colors (RGB) ──────────────────────────────────────────
// ON  = Cyan glow  | OFF = Dark teal
#define ON_R  0.0f,  1.0f,  0.9f   // Cyan ON
#define OFF_R 0.05f, 0.18f, 0.17f  // Dark OFF
#define BG_R  0.04f, 0.06f, 0.08f  // Background
#define COL_R 1.0f,  0.3f,  0.4f   // Colon color

// ─── Global State ──────────────────────────────────────────
int  g_hours, g_minutes, g_seconds;
bool g_colonVisible = true;

// ─── 7-Segment Table ───────────────────────────────────────
//  Bit order: [a, b, c, d, e, f, g]
//
//   aaa
//  f   b
//  f   b
//   ggg
//  e   c
//  e   c
//   ddd
//
const int SEGMENTS[10][7] = {
    {1,1,1,1,1,1,0},  // 0
    {0,1,1,0,0,0,0},  // 1
    {1,1,0,1,1,0,1},  // 2
    {1,1,1,1,0,0,1},  // 3
    {0,1,1,0,0,1,1},  // 4
    {1,0,1,1,0,1,1},  // 5
    {1,0,1,1,1,1,1},  // 6
    {1,1,1,0,0,0,0},  // 7
    {1,1,1,1,1,1,1},  // 8
    {1,1,1,1,0,1,1},  // 9
};

// ══════════════════════════════════════════════════════════
//  DDA LINE DRAWING ALGORITHM
//  Used for: Vertical segments (b, c, e, f)
// ══════════════════════════════════════════════════════════
void drawLineDDA(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = (fabs(dx) > fabs(dy)) ? fabs(dx) : fabs(dy);

    // Increment per step
    float xInc = dx / steps;
    float yInc = dy / steps;

    float x = x1;
    float y = y1;

    // Draw using OpenGL points simulating DDA
    // (In practice we use glVertex2f which plots each step)
    glBegin(GL_POINTS);
    for (int i = 0; i <= (int)steps; i++) {
        glVertex2f(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// ══════════════════════════════════════════════════════════
//  BRESENHAM'S LINE DRAWING ALGORITHM
//  Used for: Horizontal segments (a, d, g)
// ══════════════════════════════════════════════════════════
void drawLineBresenham(float x1, float y1, float x2, float y2)
{
    // Convert to integer pixel coordinates (scaled)
    // We scale by 1000 to work with integers
    int ix1 = (int)(x1 * 1000);
    int iy1 = (int)(y1 * 1000);
    int ix2 = (int)(x2 * 1000);
    int iy2 = (int)(y2 * 1000);

    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = (ix1 < ix2) ? 1 : -1;
    int sy = (iy1 < iy2) ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        // Convert back to float for OpenGL
        glVertex2f(ix1 / 1000.0f, iy1 / 1000.0f);

        if (ix1 == ix2 && iy1 == iy2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; ix1 += sx; }
        if (e2 <  dx) { err += dx; iy1 += sy; }
    }
    glEnd();
}

// ══════════════════════════════════════════════════════════
//  DRAW SEGMENT (Selects DDA or Bresenham based on type)
//  isHorizontal → Bresenham | isVertical → DDA
// ══════════════════════════════════════════════════════════
void drawSegment(float x1, float y1, float x2, float y2,
                 bool on, bool isHorizontal)
{
    if (on) {
        glColor3f(ON_R);
        glLineWidth(SEG_THICK);
    } else {
        glColor3f(OFF_R);
        glLineWidth(SEG_THICK * 0.8f);
    }

    // Draw thick line using OpenGL lines (smooth rendering)
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();

    // Also run the actual algorithm (for academic demonstration)
    glPointSize(SEG_THICK);
    if (isHorizontal) {
        // Bresenham for horizontal segments
        drawLineBresenham(x1, y1, x2, y2);
    } else {
        // DDA for vertical segments
        drawLineDDA(x1, y1, x2, y2);
    }

    glPointSize(1.0f);
    glLineWidth(1.0f);
}

// ══════════════════════════════════════════════════════════
//  DRAW DIGIT at position (ox, oy)
//  Uses 2D Translation Transformation:
//    actual_x = ox + relative_x
//    actual_y = oy + relative_y
// ══════════════════════════════════════════════════════════
void drawDigit(int digit, float ox, float oy)
{
    const int* seg = SEGMENTS[digit];
    float L = SEG_LEN;
    float G = SEG_GAP;

    // 2D Translation: each segment position = origin + offset
    // Segment coordinates relative to (ox, oy)
    //
    //   +---a---+
    //   f       b
    //   +---g---+
    //   e       c
    //   +---d---+

    // a → TOP horizontal    (Bresenham)
    drawSegment(ox+G,   oy+2*L, ox+L-G, oy+2*L, seg[0], true);

    // b → TOP-RIGHT vertical (DDA)
    drawSegment(ox+L,   oy+L+G, ox+L,   oy+2*L-G, seg[1], false);

    // c → BOT-RIGHT vertical (DDA)
    drawSegment(ox+L,   oy+G,   ox+L,   oy+L-G,   seg[2], false);

    // d → BOTTOM horizontal  (Bresenham)
    drawSegment(ox+G,   oy,     ox+L-G, oy,       seg[3], true);

    // e → BOT-LEFT vertical  (DDA)
    drawSegment(ox,     oy+G,   ox,     oy+L-G,   seg[4], false);

    // f → TOP-LEFT vertical  (DDA)
    drawSegment(ox,     oy+L+G, ox,     oy+2*L-G, seg[5], false);

    // g → MIDDLE horizontal  (Bresenham)
    drawSegment(ox+G,   oy+L,   ox+L-G, oy+L,     seg[6], true);
}

// ══════════════════════════════════════════════════════════
//  DRAW COLON SEPARATOR
// ══════════════════════════════════════════════════════════
void drawColon(float cx, float cy, bool visible)
{
    if (visible) glColor3f(COL_R);
    else         glColor3f(OFF_R);

    float dotSize = 0.018f;
    float L = SEG_LEN;

    // Upper dot
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy + L * 1.4f);
    for (int i = 0; i <= 20; i++) {
        float angle = 2.0f * M_PI * i / 20;
        glVertex2f(cx + dotSize * cos(angle),
                   cy + L * 1.4f + dotSize * sin(angle));
    }
    glEnd();

    // Lower dot
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy + L * 0.6f);
    for (int i = 0; i <= 20; i++) {
        float angle = 2.0f * M_PI * i / 20;
        glVertex2f(cx + dotSize * cos(angle),
                   cy + L * 0.6f + dotSize * sin(angle));
    }
    glEnd();
}

// ══════════════════════════════════════════════════════════
//  DRAW TEXT (using GLUT bitmap fonts)
// ══════════════════════════════════════════════════════════
void drawText(float x, float y, const std::string& text,
              void* font = GLUT_BITMAP_HELVETICA_12)
{
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

// ══════════════════════════════════════════════════════════
//  DRAW BACKGROUND GRID (decorative)
// ══════════════════════════════════════════════════════════
void drawGrid()
{
    glColor4f(0.0f, 1.0f, 0.9f, 0.04f);
    glLineWidth(0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (float x = -1.0f; x <= 1.0f; x += 0.08f) {
        glBegin(GL_LINES);
        glVertex2f(x, -1.0f);
        glVertex2f(x,  1.0f);
        glEnd();
    }
    for (float y = -1.0f; y <= 1.0f; y += 0.08f) {
        glBegin(GL_LINES);
        glVertex2f(-1.0f, y);
        glVertex2f( 1.0f, y);
        glEnd();
    }
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
}

// ══════════════════════════════════════════════════════════
//  DISPLAY CALLBACK
// ══════════════════════════════════════════════════════════
void display()
{
    // Clear with background color
    glClearColor(BG_R, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw grid
    drawGrid();

    // ── Get current time ──────────────────────────────
    time_t now = time(0);
    tm* t = localtime(&now);
    g_hours   = t->tm_hour;
    g_minutes = t->tm_min;
    g_seconds = t->tm_sec;

    // ── Extract individual digits ─────────────────────
    int h1 = g_hours   / 10,  h2 = g_hours   % 10;
    int m1 = g_minutes / 10,  m2 = g_minutes % 10;
    int s1 = g_seconds / 10,  s2 = g_seconds % 10;

    // ── 2D Translation: Starting x position ───────────
    // Layout: [H1][H2] : [M1][M2] : [S1][S2]
    float startX = -0.82f;
    float startY = -0.15f;
    float dSpace = DIGIT_SPACE;
    float cSpace = COLON_SPACE;

    // Draw Hour digits
    drawDigit(h1, startX,              startY);
    drawDigit(h2, startX + dSpace,     startY);

    // Draw colon 1 (H:M)
    drawColon(startX + dSpace*2 + cSpace*0.3f,
              startY, g_colonVisible);

    // Draw Minute digits
    drawDigit(m1, startX + dSpace*2 + cSpace, startY);
    drawDigit(m2, startX + dSpace*3 + cSpace, startY);

    // Draw colon 2 (M:S)
    drawColon(startX + dSpace*4 + cSpace*1.3f,
              startY, g_colonVisible);

    // Draw Second digits
    drawDigit(s1, startX + dSpace*4 + cSpace*2, startY);
    drawDigit(s2, startX + dSpace*5 + cSpace*2, startY);

    // ── Draw Labels ───────────────────────────────────
    glColor3f(0.3f, 0.6f, 0.55f);
    drawText(-0.84f, -0.35f, "HH",   GLUT_BITMAP_HELVETICA_10);
    drawText(-0.26f, -0.35f, "MM",   GLUT_BITMAP_HELVETICA_10);
    drawText( 0.33f, -0.35f, "SS",   GLUT_BITMAP_HELVETICA_10);

    // ── Title ─────────────────────────────────────────
    glColor3f(0.2f, 0.5f, 0.45f);
    drawText(-0.38f, 0.62f,
             "DIGITAL CLOCK  |  DDA + Bresenham",
             GLUT_BITMAP_HELVETICA_18);

    // ── Footer info ───────────────────────────────────
    glColor3f(0.15f, 0.35f, 0.32f);
    drawText(-0.72f, -0.75f,
             "CG Mini Project  |  PCC-252-ITT  |  SE IT 2024 Pattern  |  SPPU",
             GLUT_BITMAP_HELVETICA_10);

    // ── Algorithm labels ──────────────────────────────
    glColor3f(0.6f, 0.3f, 0.35f);
    drawText(-0.95f, -0.55f,
             "Horizontal Segs: Bresenham  |  Vertical Segs: DDA  |  Positions: 2D Translation",
             GLUT_BITMAP_HELVETICA_10);

    glutSwapBuffers();
}

// ══════════════════════════════════════════════════════════
//  TIMER CALLBACK — updates every 500ms for colon blink
// ══════════════════════════════════════════════════════════
void timer(int value)
{
    g_colonVisible = !g_colonVisible;
    glutPostRedisplay();
    glutTimerFunc(500, timer, 0);
}

// ══════════════════════════════════════════════════════════
//  RESHAPE CALLBACK
// ══════════════════════════════════════════════════════════
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)w / h;
    glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ══════════════════════════════════════════════════════════
//  KEYBOARD CALLBACK — Press ESC or Q to quit
// ══════════════════════════════════════════════════════════
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27 || key == 'q' || key == 'Q') exit(0);
}

// ══════════════════════════════════════════════════════════
//  MAIN
// ══════════════════════════════════════════════════════════
int main(int argc, char** argv)
{
    std::cout << "=========================================\n";
    std::cout << " Digital Clock - CG Mini Project\n";
    std::cout << " PCC-252-ITT | SE IT 2024 | SPPU\n";
    std::cout << "=========================================\n";
    std::cout << " Algorithms: DDA + Bresenham\n";
    std::cout << " Concepts  : 2D Translation, RGB Color\n";
    std::cout << " Press Q or ESC to exit\n";
    std::cout << "=========================================\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(WIN_TITLE);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(500, timer, 0);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glutMainLoop();
    return 0;
}

/*
 ============================================================
  THEORY FOR LAB JOURNAL
 ============================================================

  1. DDA (Digital Differential Analyzer) Algorithm:
     - Calculate dx = x2-x1, dy = y2-y1
     - steps = max(|dx|, |dy|)
     - xInc = dx/steps, yInc = dy/steps
     - Plot point (x, y) and increment by (xInc, yInc)
     - Used for VERTICAL segments (b, c, e, f)

  2. Bresenham's Line Algorithm:
     - Uses only integer arithmetic
     - err = dx - dy
     - Adjust error term at each step
     - More efficient than DDA
     - Used for HORIZONTAL segments (a, d, g)

  3. 2D Translation Transformation:
     - Matrix: | 1  0  tx |
               | 0  1  ty |
               | 0  0  1  |
     - New position: x' = x + tx, y' = y + ty
     - Used to place each digit at correct position

  4. 7-Segment Display:
     -  aaa        Segment | Algorithm
        f   b      --------+----------
        f   b      a (top) | Bresenham
         ggg       b (TR)  | DDA
        e   c      c (BR)  | DDA
        e   c      d (bot) | Bresenham
         ddd       e (BL)  | DDA
                   f (TL)  | DDA
                   g (mid) | Bresenham

  5. RGB Color Model:
     - ON  segments : RGB(0.0, 1.0, 0.9) → Cyan
     - OFF segments : RGB(0.05, 0.18, 0.17) → Dark teal
     - Background   : RGB(0.04, 0.06, 0.08) → Near black
 ============================================================
*/