#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>

using namespace std;

struct vec3d
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

struct triangle
{
    vec3d p[3];
    wchar_t sym;
    short col;
};

struct mesh
{
    vector<triangle> tris;

    bool LoadObjectFile(string sFilename)
    {
        ifstream f(sFilename);
        if (!f.is_open())
            return false;

        // Local cache of vertices
        vector<vec3d> verts;

        //Make sure it hasn't reached end of file
        while (!f.eof())
        {
            // Assumes each line doesn't exceed 128 characters
            char line[128];
            f.getline(line, 128);

            //Turn into string stream
            strstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                vec3d v;
                //Ignore first character "v", but keep the x, y and z coords
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }

            if (line[0] == 'f')
            {
                int f[3];
                // populate triangle with the vertices
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            }
        }

        return true;
    }
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
    vec3d vCamera;
    float fTheta;

    vec3d Matrix_MultiplyVector(matrix4x4 &m, vec3d& i)
    {
        vec3d v;
        v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
        v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
        v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
        v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
        return v;
    }

    matrix4x4 Matrix_MakeIdentity()
    {
        matrix4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    matrix4x4 Matrix_MakeRotationX(float fAngleRad)
    {
        matrix4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[1][2] = sinf(fAngleRad);
        matrix.m[2][1] = -sinf(fAngleRad);
        matrix.m[2][2] = cosf(fAngleRad);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    matrix4x4 Matrix_MakeRotationY(float fAngleRad)
    {
        matrix4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][2] = sinf(fAngleRad);
        matrix.m[2][0] = -sinf(fAngleRad);
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = cosf(fAngleRad);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    matrix4x4 Matrix_MakeRotationZ(float fAngleRad)
    {
        matrix4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][1] = sinf(fAngleRad);
        matrix.m[1][0] = -sinf(fAngleRad);
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    matrix4x4 Matrix_MakeTranslation(float x, float y, float z)
    {
        matrix4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        matrix.m[3][0] = x;
        matrix.m[3][1] = y;
        matrix.m[3][2] = z;
        return matrix;
    }

    matrix4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
    {
        float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
        matrix4x4 matrix;
        matrix.m[0][0] = fAspectRatio * fFovRad;
        matrix.m[1][1] = fFovRad;
        matrix.m[2][2] = fFar / (fFar - fNear);
        matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matrix.m[2][3] = 1.0f;
        matrix.m[3][3] = 0.0f;
        return matrix;
    }

    matrix4x4 Matrix_MultiplyMatrix(matrix4x4& m1, matrix4x4& m2)
    {
        matrix4x4 matrix;
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
        return matrix;
    }

    matrix4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up)
    {
        // Calculate new forward direction
        vec3d newForward = Vector_Sub(target, pos);
        newForward = Vector_Normalise(newForward);

        // Calculate new Up direction
        vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
        vec3d newUp = Vector_Sub(up, a);
        newUp = Vector_Normalise(newUp);

        // New Right direction is easy, its just cross product
        vec3d newRight = Vector_CrossProduct(newUp, newForward);

        // Construct Dimensioning and Translation Matrix	
        matrix4x4 matrix;
        matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
        return matrix;

    }

    matrix4x4 Matrix_QuickInverse(matrix4x4& m) // Only for Rotation/Translation Matrices
    {
        matrix4x4 matrix;
        matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
        matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
        matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    vec3d Vector_Add(vec3d& v1, vec3d& v2)
    {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    vec3d Vector_Sub(vec3d& v1, vec3d& v2)
    {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }

    vec3d Vector_Mul(vec3d& v1, float k)
    {
        return { v1.x * k, v1.y * k, v1.z * k };
    }

    vec3d Vector_Div(vec3d& v1, float k)
    {
        return { v1.x / k, v1.y / k, v1.z / k };
    }

    float Vector_DotProduct(vec3d& v1, vec3d& v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    float Vector_Length(vec3d& v)
    {
        return sqrtf(Vector_DotProduct(v, v));
    }

    vec3d Vector_Normalise(vec3d& v)
    {
        float l = Vector_Length(v);
        return { v.x / l, v.y / l, v.z / l };
    }

    vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2)
    {
        vec3d v;
        v.x = v1.y * v2.z - v1.z * v2.y;
        v.y = v1.z * v2.x - v1.x * v2.z;
        v.z = v1.x * v2.y - v1.y * v2.x;
        return v;
    }

    CHAR_INFO GetColour(float lum)
    {
        short bg_col, fg_col;
        wchar_t sym;
        int pixel_bw = (int)(13.0f * lum);
        switch (pixel_bw)
        {
        case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

        case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
        case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
        case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
        case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

        case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
        case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
        case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
        case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

        case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
        case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
        case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
        case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
        default:
            bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
        }

        CHAR_INFO c;
        c.Attributes = bg_col | fg_col;
        c.Char.UnicodeChar = sym;
        return c;
    }

public:
    bool OnUserCreate() override
    {
        meshCube.LoadObjectFile("VideoShip.obj");

        matrixProjection = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);


        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        // Background Colour
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);


        // For rotating the cube to show off different perspectives
        matrix4x4 matrixRotationZ, matrixRotationX;

        fTheta += 1.0f * fElapsedTime;

        matrixRotationZ = Matrix_MakeRotationZ(fTheta);
        matrixRotationX = Matrix_MakeRotationX(fTheta);

        matrix4x4 matrixTranslated;
        matrixTranslated = Matrix_MakeTranslation(0.0f, 0.0f, 16.0f);

        matrix4x4 matrixWorld;
        matrixWorld = Matrix_MakeIdentity();
        matrixWorld = Matrix_MultiplyMatrix(matrixRotationZ, matrixRotationX);
        matrixWorld = Matrix_MultiplyMatrix(matrixWorld, matrixTranslated);

        vector<triangle> vecTrianglesToRaster;

        // Transform/transformation loop
        for (auto tri : meshCube.tris)
        {
            triangle triProjected, triTransformed;

            triTransformed.p[0] = Matrix_MultiplyVector(matrixWorld, tri.p[0]);
            triTransformed.p[1] = Matrix_MultiplyVector(matrixWorld, tri.p[1]);
            triTransformed.p[2] = Matrix_MultiplyVector(matrixWorld, tri.p[2]);

            vec3d normal, line1, line2;

            line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
            line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

            normal = Vector_CrossProduct(line1, line2);
            normal = Vector_Normalise(normal);

            vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

            //Only project is triangle is in view
            if(Vector_DotProduct(normal, vCameraRay) < 0.0f)
            { 

                //Illumination to see edges of filled in cube
                vec3d light_direction = { 0.0f, 0.0f, -1.0f };
                light_direction = Vector_Normalise(light_direction);

                // Dot Product
                float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

                //Extract symbol and character info to represent shade of grey within the the console window (No RGB)
                CHAR_INFO c = GetColour(dp);
                triTransformed.col = c.Attributes;
                triTransformed.sym = c.Char.UnicodeChar;

                // Project triangles from 3D into 2D
                triProjected.p[0] = Matrix_MultiplyVector(matrixProjection, triTransformed.p[0]);
                triProjected.p[1] = Matrix_MultiplyVector(matrixProjection, triTransformed.p[1]);
                triProjected.p[2] = Matrix_MultiplyVector(matrixProjection, triTransformed.p[2]);
                triProjected.col = triTransformed.col;
                triProjected.sym = triTransformed.sym;

                triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
                triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
                triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

                //Scale the data into view
                vec3d vOffsetView = { 1,1,0 };
                triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
                triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
                triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

                triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
                triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
                triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
                triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
                triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
                triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

                //Store triangle for sorting
                vecTrianglesToRaster.push_back(triProjected);

            }
        }

        // Sort triangles from back to front
        sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
            {
                float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
                float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
                return z1 > z2;
            });

        // Drawing loop
        for (auto& triProjected : vecTrianglesToRaster)
        {
            // Rasterize the Triangle
            FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
                triProjected.p[1].x, triProjected.p[1].y,
                triProjected.p[2].x, triProjected.p[2].y,
                triProjected.sym, triProjected.col);

            //Used to show wire frames of triangles being drawn (If needed)
            /*DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                triProjected.p[1].x, triProjected.p[1].y,
                triProjected.p[2].x, triProjected.p[2].y,
                PIXEL_SOLID, FG_BLACK);*/
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
