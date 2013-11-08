#ifndef __3DOBJECT_ENGINE__
#define __3DOBJECT_ENGINE__

#include <io.h>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <list>
#include <queue>
#include <iterator>
//#include <GL/glu.h>
#include "glextensions.h"
#include "trackball.h"

//#include <GL/glew.h>
//#include <GL/glut.h>

#define HALF_PI		1.57079632679
#define PI			3.14159265358
#define OBJINFINITY 99999.9

#define printPoint3f(msg,p) printf("%s: <%f,%f,%f>\n",msg,p.x,p.y,p.z)

enum TextureOri{
    TEX_ANGLE_0,
    TEX_ANGLE_90,
    TEX_ANGLE_180,
    TEX_ANGLE_270
};

enum Plane3dOri{
    PLAIN_XY,
    PLAIN_YZ,
    PLAIN_ZX
};

class Vector2f{
    public:
        float x;
        float y;
        Vector2f();
        Vector2f(float a);
        Vector2f(float a, float b);
        void set(float a, float b);
        //-- unary
        //Vector2f operator + ();
        //Vector2f operator - ();
        //--
        Vector2f& operator = ( const Vector2f &a);
        friend Vector2f operator + ( const Vector2f &a, const Vector2f &b);
        friend Vector2f operator - ( const Vector2f &a, const Vector2f &b);
        friend Vector2f operator * ( const Vector2f &a, const Vector2f &b);
        friend Vector2f operator / ( const Vector2f &a, const Vector2f &b);
        Vector2f& operator += (const float d);
        Vector2f& operator += (const Vector2f &d);
};

class Vector4f;

class Vector3f{
    public:
        float x;
        float y;
        float z;
        Vector3f();
        Vector3f(float a);
        Vector3f(float a, float b, float c);
        Vector3f( const Vector4f& v);
        void set(float a, float b, float c);
        Vector3f& operator = ( const Vector3f &a);
        Vector3f  operator + ();
        Vector3f  operator - ();
        Vector3f& operator += (const Vector3f &d);	// element by element addition
        Vector3f& operator -= (const Vector3f &d);	// element by element addition
        Vector3f& operator *= (const Vector3f &d);	// element by element multiplication
        Vector3f& operator /= (const Vector3f &d);	// element by element multiplication
        friend Vector3f operator + ( const Vector3f &a, const Vector3f &b);	// element by element addition
        friend Vector3f operator - ( const Vector3f &a, const Vector3f &b);	// element by element subtraction
        friend float operator & ( const Vector3f &a, const Vector3f &b);	// dot product
        friend Vector3f operator * ( const Vector3f &a, const Vector3f &b);	// element by element multiplication
        friend Vector3f operator ^ ( const Vector3f &a, const Vector3f &b);	// cross product
        friend Vector3f operator / ( const Vector3f &a, const Vector3f &b);	// element by element division
        float distance( const Vector3f& a)const;
        float magnitude() const;
        Vector3f normalize()const;
        Vector3f rotate( float xy, float yz, float zx)const;
};

class Vector4f{
    public:
        float x;
        float y;
        float z;
        float u;
        Vector4f();
        Vector4f(float a);
        Vector4f(float a, float b, float c, float d);
        Vector4f( const Vector3f& a);
        void set(float a, float b, float c, float d);
        Vector4f& operator = ( const Vector4f &a);
        friend Vector4f operator + ( const Vector4f &a, const Vector4f &b);	// element by element addition
        friend Vector4f operator - ( const Vector4f &a, const Vector4f &b);	// element by element subtraction
        friend Vector4f operator * ( const Vector4f &a, const Vector4f &b);	// element by element multiplication
        friend Vector4f operator / ( const Vector4f &a, const Vector4f &b);	// element by element division
        Vector4f normalize()const;
        float magnitude()const;
};

void setQuaternion( bool v);
bool getQuaternion();
void setActiveViewTrackBall( const TrackBall& t);

struct Texture2d
{
    GLuint id;
    long w;				// width
    long h;				// height
    unsigned char *buf;
};

class Shader
{
    public:
        GLuint id;
        GLenum type;
        GLint compiledStatus;
        unsigned long length;
        char *buf;

    Shader();
    static unsigned long getFileLength( FILE *);
    int loadShader(char* filename, GLenum type);
    void unloadShader();
    int compile();
};

Texture2d* loadBMP( char *fname);
bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
bool appendBMP( Texture2d *tex, char *fname);
int getTotalTextures();
int getNextTextureID();
bool isRenderTexture();
void setRenderTexture(bool);
bool isDrawBounds();
void setDrawBounds(bool);
void setSoftShadow( int v);
int getSoftShadow();

class Camera3d{
    public:
        Vector3f pos;	// position of the camera
        Vector3f eye;	// where they are pointing
        Vector3f ori;	// orientation
        float ang;		// angle;
        float cnear;     // camera near
        float cfar;      // camera far
        float ar;		// aspect ratio
        bool isOrtho;
        Vector3f orthoMin;	// left, bottom, near
        Vector3f orthoMax;	// right, top, far

        Camera3d();
        Camera3d& operator = ( const Camera3d &c);
};

void setActiveCamera( const Camera3d& cam);

class Material3d{
    public:
        Vector4f ambi;
        Vector4f diff;
        Vector4f spec;
        Vector4f emis;
        GLfloat shin;

        Material3d();
        void setColor( const Vector4f& color);
};

class Light3d{
    public:
        bool castShadow;
        bool isSpot;
        bool isOn;
        Material3d mat;
        Vector4f pos;		// position or direction in case of directional light
        Vector3f sdir;		// spot direction
        GLfloat cutoff;		// spot cutoff, default:45
        GLfloat exp;		// exponent, default:2.0
        GLfloat cone;		// cone alpha, default:0.0
        Vector3f att;		// Attenuation x + y*dist + z*dist*dist

        Light3d();
        ~Light3d();
        void runGLcmd(int light);
};

void setLighting( bool v);
bool getLighting( void);

class Object3d{
    public:
        int objType;			// Default: 0 (none)
        bool isCollidable;		// Default: false
        bool isExtreamObj;		// true if it's an immovable object or has unstoppable force, Default: true
        bool isHidden;			// Default: false
        bool queryCollision;	// True: if it's a hit
        bool castShadow;
        bool envMapping;
        Vector3f cubeBoundMin;	// Bounding Box Minimum
        Vector3f cubeBoundMax;	// Bounding Box Maximum
        Vector3f col;			// color, Default: .7, .7, .7
        Vector3f texCol;		// texture color, Default: 1,1,1
        Texture2d* tex;			// texture
        GLfloat texEnvParam;	// texture environment parameter. Default: GL_MODULATE
        GLuint dispList;		// Display List
        Vector3f factorRot;		// orietation
        Vector3f factorMul;		// scaling of object
        Vector3f factorTra;		// 3d offset from origin
        Material3d *mat;		// Material

        Object3d();
        ~Object3d();
        void operatorAssign( const Object3d &c);
        void bindTexture( Texture2d *t);
        void drawBounds();
        void makeBoundsEql();
        virtual void updateDisplayList();
        virtual void draw(){};
        virtual void calcBoundingCube(){};
};

bool collisionTest( const Vector3f& before, const Vector3f& after);
bool collisionTest( Object3d& obj);
void updateDisplayList();
void runForAllObject3d( void(*func)(Object3d*));
unsigned int getNumRenderedPolygons();

// objType = 1
class Plane3d: public Object3d{
    public:
        Vector3f p1, p2, p3, p4; // vertex positions
        Vector3f nor;			// normal vector
        Vector2f t1, t2, t3, t4; // texture coordinates
        bool isShadowPlane;

        Plane3d();
        void setPlane( float width, float height, Plane3dOri o);
        void calcNormal( bool cw=true);
        Plane3d& operator = (const Plane3d &c);
        Plane3d& operator += (const Vector3f& d);
        void setTexture( float scaleX, float scaleY, TextureOri o = TEX_ANGLE_0, bool flipH=false, bool flipV=false);
        void draw();
        void calcBoundingCube();
};

// objType = 2
class Sphere: public Object3d{
    public:
        Vector3f cen;	// center positions
        float r;		// radius
        long hs, vs;	// horizontal segment & vetical segment
        Vector2f lon;	// longitude start and end position
        Vector2f lat;	// latitude start and end position
        Vector2f tc;		// texture scaling
        Vector2f off;	// longitude & latitude texture offset

        Sphere();
        Sphere& operator += (const Vector3f d);
        void setTexture( float scaleX, float scaleY, float longitude, float latitude);
        void draw();
        void calcBoundingCube();
};

typedef struct{
    unsigned long a;
    unsigned long b;
    unsigned long c;
}TriMesh;

// objType = 10
class Obj3ds: public Object3d{
    public:
        char name[24];
        unsigned long numVert;
        Vector3f *vertex;
        //Vector3f *myvertex;   //added by gokul.
        Vector3f *normal;
        unsigned long numPoly;
        TriMesh *triangle;
        Vector2f *mapcoord;
        bool smooth;

        Obj3ds();
        bool Load3ds( char *filename);
        void calcNormals();
        Obj3ds& operator = ( const Obj3ds &c);
        Obj3ds& operator += (const Vector3f &d);
        void scale(const Vector3f &d);
        void rotate( float xy, float yz, float zx);
        virtual void draw();
        void calcBoundingCube();
};

// objType = 11
class ObjBIN: public Object3d{
    public:
        unsigned long numVert;
        Vector3f *vertex;
        //Vector3f *myvertex;  //added by gokul.
        Vector3f *normal;
        unsigned long numPoly;
        TriMesh *triangle;
        Vector2f *mapcoord;
        bool smooth;

        ObjBIN();
        bool LoadBIN( const char *filename);
        bool saveBIN( const char *filename);
        void calcNormals();
        ObjBIN& operator = ( const ObjBIN &c);
        ObjBIN& operator += (const Vector3f &d);
        void scale(const Vector3f &d);
        void rotate( float xy, float yz, float zx);
        void draw();
        void calcBoundingCube();
};

class Obj3dsGraph: public ObjBIN{
    public:
        TriMesh *elist;

        Obj3dsGraph();
        ~Obj3dsGraph();
        void calcGraph();
        void drawGraph();
        void lap();
};

void render3dObjects( void);
void init3dEng( void);
float* calcPlaneEqn(float*,float*,float*);

#endif
