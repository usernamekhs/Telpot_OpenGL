#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

// 회전 및 줌 변수
float angleY = 0.0f, angleX = 0.0f;
float zoom = 3.0f;
int lastMouseX, lastMouseY;
bool isDragging = false;
bool autoRotate = true;  // 자동 회전 on/off

// 색상 관련
float hue = 0.0f;
float r = 1.0f, g = 0.0f, b = 0.0f;
int colorChangeCounter = 0;

// 연기
const int NUM_SMOKE_PARTICLES = 100;
float smokeY[NUM_SMOKE_PARTICLES];

void hsvToRgb(float h, float s, float v, float& r, float& g, float& b) {
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    switch (i % 6) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
    }
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, zoom, 0, 0, 0, 0, 1, 0);

    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    // 조명 위치 설정
    GLfloat lightPos[] = { 2.0f, 2.0f, 2.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // 주전자 색상
    glColor3f(r, g, b);
    glutSolidTeapot(0.5);

    // 연기 입자
    glDisable(GL_LIGHTING); // 연기는 조명 영향 X
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_SMOKE_PARTICLES; ++i) {
        float x = (rand() % 100 - 50) / 100.0f * 0.2f;
        float z = (rand() % 100 - 50) / 100.0f * 0.2f;
        glColor4f(0.8f, 0.8f, 0.8f, 0.3f);
        glVertex3f(x, smokeY[i], z);
    }
    glEnd();
    glEnable(GL_LIGHTING); // 조명 다시 켜기

    glutSwapBuffers();
}

void timer(int value) {
    if (autoRotate) {
        angleY += 1.5f;  // 자동 회전 속도 조절 가능
        if (angleY > 360.0f) angleY -= 360.0f;
    }

    hue += 0.005f;
    if (hue > 1.0f) hue -= 1.0f;
    hsvToRgb(hue, 1.0f, 1.0f, r, g, b);

    for (int i = 0; i < NUM_SMOKE_PARTICLES; ++i) {
        smokeY[i] += 0.01f;
        if (smokeY[i] > 1.5f) {
            smokeY[i] = 0.5f;
        }
    }

    if (++colorChangeCounter > 200) {
        r = (rand() % 100) / 100.0f;
        g = (rand() % 100) / 100.0f;
        b = (rand() % 100) / 100.0f;
        colorChangeCounter = 0;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = (float)w / (float)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, ratio, 1.0, 100.0);
    glViewport(0, 0, w, h);
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        isDragging = (state == GLUT_DOWN);
        lastMouseX = x;
        lastMouseY = y;
    }
    else if (button == 3) { // 마우스 휠 업 (확대)
        zoom -= 0.1f;
        if (zoom < 1.5f) zoom = 1.5f;
    }
    else if (button == 4) { // 마우스 휠 다운 (축소)
        zoom += 0.1f;
        if (zoom > 10.0f) zoom = 10.0f;
    }
}

void mouseMotion(int x, int y) {
    if (isDragging) {
        angleY += (x - lastMouseX);
        angleX += (y - lastMouseY);
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'r' || key == 'R') {
        autoRotate = !autoRotate;  // r 키 누르면 자동 회전 토글
    }
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_SMOKE_PARTICLES; ++i) {
        smokeY[i] = 0.5f + (rand() % 100) / 100.0f * 0.5f;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Fancy Teapot with Lighting");

    initLighting();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
