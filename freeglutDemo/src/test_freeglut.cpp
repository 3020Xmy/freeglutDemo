#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include <string>

static HMODULE hFreeglut;

typedef void (*PF_glutInit)(int*, char**);
typedef void (*PF_glutInitWindowSize)(int, int);
typedef void (*PF_glutInitDisplayMode)(unsigned int);
typedef int (*PF_glutCreateWindow)(const char*);
typedef void (*PF_glutDisplayFunc)(void (*)(void));
typedef void (*PF_glutMainLoop)();
typedef void (*PF_glutTimerFunc)(unsigned int, void (*)(int), int);
typedef void (*PF_glutSwapBuffers)();
typedef void (*PF_glutPostRedisplay)();

static PF_glutInit p_glutInit;
static PF_glutInitWindowSize p_glutInitWindowSize;
static PF_glutInitDisplayMode p_glutInitDisplayMode;
static PF_glutCreateWindow p_glutCreateWindow;
static PF_glutDisplayFunc p_glutDisplayFunc;
static PF_glutMainLoop p_glutMainLoop;
static PF_glutTimerFunc p_glutTimerFunc;
static PF_glutSwapBuffers p_glutSwapBuffers;
static PF_glutPostRedisplay p_glutPostRedisplay;

static FARPROC loadSym(const char* name) {
    FARPROC p = GetProcAddress(hFreeglut, name);
    if (!p) {
        char buf[64];
        for (int i = 0; i <= 64; ++i) buf[i] = 0;
        std::snprintf(buf, sizeof(buf), "%s@%d", name, 4);
        p = GetProcAddress(hFreeglut, buf);
    }
    return p;
}

static bool loadFreeglut(const char* dllPath) {
    hFreeglut = LoadLibraryA(dllPath);
    if (!hFreeglut) return false;
    p_glutInit = (PF_glutInit)loadSym("glutInit");
    p_glutInitWindowSize = (PF_glutInitWindowSize)loadSym("glutInitWindowSize");
    p_glutInitDisplayMode = (PF_glutInitDisplayMode)loadSym("glutInitDisplayMode");
    p_glutCreateWindow = (PF_glutCreateWindow)loadSym("glutCreateWindow");
    p_glutDisplayFunc = (PF_glutDisplayFunc)loadSym("glutDisplayFunc");
    p_glutMainLoop = (PF_glutMainLoop)loadSym("glutMainLoop");
    p_glutTimerFunc = (PF_glutTimerFunc)loadSym("glutTimerFunc");
    p_glutSwapBuffers = (PF_glutSwapBuffers)loadSym("glutSwapBuffers");
    p_glutPostRedisplay = (PF_glutPostRedisplay)loadSym("glutPostRedisplay");
    return p_glutInit && p_glutInitWindowSize && p_glutInitDisplayMode && p_glutCreateWindow && p_glutDisplayFunc && p_glutMainLoop && p_glutTimerFunc && p_glutSwapBuffers && p_glutPostRedisplay;
}

static void display() {
    glViewport(0, 0, 800, 600);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex2f(-0.6f, -0.4f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex2f(0.6f, -0.4f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex2f(0.0f, 0.6f);
    glEnd();
    p_glutSwapBuffers();
}

static void quitTimer(int) {
    std::exit(0);
}

static void tick(int) {}

static std::string exeDir() {
    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(GetModuleHandleA(nullptr), path, MAX_PATH);
    if (!len) return std::string();
    std::string s(path, len);
    size_t pos = s.find_last_of("\\/");
    if (pos == std::string::npos) return std::string();
    return s.substr(0, pos);
}

int main(int argc, char** argv) {
    std::string dir = exeDir();
    auto exists = [](const std::string& p) -> bool {
        DWORD attr = GetFileAttributesA(p.c_str());
        return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
    };
#ifdef _DEBUG
    const char* conf = "Debug";
    const char* name = "freeglutd.dll";
#else
    const char* conf = "Release";
    const char* name = "freeglut.dll";
#endif
    std::string dll;
    std::string candidates[] = {
        dir + std::string("\\..\\..\\freeglutDemo\\freeglut3.8.0_MSVC_Win\\") + conf + "\\" + name,
        dir + std::string("\\..\\freeglutDemo\\freeglut3.8.0_MSVC_Win\\") + conf + "\\" + name,
        dir + std::string("\\..\\freeglut3.8.0_MSVC_Win\\") + conf + "\\" + name,
        dir + std::string("\\..\\..\\freeglut3.8.0_MSVC_Win\\") + conf + "\\" + name
    };
    for (const auto& c : candidates) {
        if (exists(c)) { dll = c; break; }
    }
    if (dll.empty() || !loadFreeglut(dll.c_str())) {
        std::fprintf(stderr, "Failed to load Freeglut: %s\n", dll.c_str());
        return 1;
    }
    p_glutInit(&argc, argv);
    const unsigned int GLUT_RGB = 0u;
    const unsigned int GLUT_DOUBLE = 2u;
    p_glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    p_glutInitWindowSize(800, 600);
    p_glutCreateWindow("Freeglut Test");
    p_glutDisplayFunc(display);
    p_glutTimerFunc(30000, quitTimer, 0);
    p_glutMainLoop();
    return 0;
}

