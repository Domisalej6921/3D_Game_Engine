#include "olcConsoleGameEngine.h"
using namespace std;

struct vec3d
{
    float x, y, z;
};

struct triangle
{
    vec3d p[3];
};

struct mesh
{
    vector<triangle> tris;
};

class GameEngine : public olcConsoleGameEngine
{
public:
    GameEngine()
    {
        appName = L"Demo_3D_Game_Engine";
    }

public:
    bool onUserCreate() override
    {
        return true;
    }

    bool onUserUpdate(float fElapsedTime) override
    {
        return true;
    }
};

int main()
{
    GameEngine demo;
    if (demo.ConstructConsole(256, 240, 4, 4))
        demo.Start();
    //else it will result in an error (Add error handling)
    return 0;
}
