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

struct matrix4x4
{
    float m[4][4] = { 0 };
};

class GameEngine : public olcConsoleGameEngine
{
public:
    GameEngine()
    {
        m_sAppName = L"Demo_3D_Game_Engine";
    }

private:
    mesh meshCube;
    matrix4x4 matrixProjection;
    float fTheta;

    void MultiplyMatrixVector(vec3d& i, vec3d& o, matrix4x4& m)
    {
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

        if (w != 0.0f)
        {
            o.x /= w; o.y /= w; o.z /= w;
        }
    }

public:
    bool OnUserCreate() override
    {
        meshCube.tris = {

            // South
            { 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f },

            // East
            { 1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f },

            // North
            { 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f },

            // West
            { 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f },

            // Top
            { 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f },

            // Bottom
            { 1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f },

        };

        // Projection Matrix
        float fNear = 0.1f;
        float fFar = 1000.0f;
        float fFov = 90.0f;
        float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
        float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

        matrixProjection.m[0][0] = fAspectRatio * fFovRad;
        matrixProjection.m[1][1] = fFovRad;
        matrixProjection.m[2][2] = fFar / (fFar - fNear);
        matrixProjection.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matrixProjection.m[2][3] = 1.0f;
        matrixProjection.m[3][3] = 0.0f;


        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        // Background Colour
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);


        // For rotating the cube to show off different perspectives
        matrix4x4 matrixRotationZ, matrixRotationX;

        fTheta += 1.0f * fElapsedTime;

        // Rotation Z
        matrixRotationZ.m[0][0] = cosf(fTheta);
        matrixRotationZ.m[0][1] = sinf(fTheta);
        matrixRotationZ.m[1][0] = -sinf(fTheta);
        matrixRotationZ.m[1][1] = cosf(fTheta);
        matrixRotationZ.m[2][2] = 1;
        matrixRotationZ.m[3][3] = 1;

        // Rotation X
        matrixRotationX.m[0][0] = 1;
        matrixRotationX.m[1][1] = cosf(fTheta * 0.5f);
        matrixRotationX.m[1][2] = sinf(fTheta * 0.5f);
        matrixRotationX.m[2][1] = -sinf(fTheta * 0.5f);
        matrixRotationX.m[2][2] = cosf(fTheta * 0.5f);
        matrixRotationX.m[3][3] = 1;

        // Draw Triangles
        for (auto tri : meshCube.tris)
        {
            triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

            //Rotate in Z-axis
            MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matrixRotationZ);
            MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matrixRotationZ);
            MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matrixRotationZ);

            //Rotate in X-axis
            MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matrixRotationX);
            MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matrixRotationX);
            MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matrixRotationX);

            // Offset onto the screen
            triTranslated = triRotatedZX;
            triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
            triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
            triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

            // Project triangles from 3D into 2D
            MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matrixProjection);
            MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matrixProjection);
            MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matrixProjection);

            //Scale the data into view
            triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
            triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
            triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

            triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
            triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
            triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
            triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
            triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
            triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

            DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                triProjected.p[1].x, triProjected.p[1].y,
                triProjected.p[2].x, triProjected.p[2].y,
                PIXEL_SOLID, FG_WHITE);

        }

        return true;
    }
};

int main()
{
    GameEngine demo;
    if (demo.ConstructConsole(256, 240, 4, 4))
        demo.Start();
    else
    {
        wcerr << L"Error: Could not construct console." << endl;
        return -1;
    }
    return 0;
}
