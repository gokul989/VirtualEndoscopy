#include "3deng.h"
#include <lpng163/png.h>
#include <lpng163/pngconf.h>
#include <lpng163/pngdebug.h>
#include <lpng163/pnginfo.h>
#include <lpng163/pngpriv.h>
#include <lpng163/pngstruct.h>
std::vector<Vector3f> myvertex;
//Vector3f pppvertex[Obj3ds::Object3d.numVert];

//-----------------------------------------------------------------------------
//								Texture2d
//-----------------------------------------------------------------------------

static bool RENDER_TEXTURES = true;
static bool DRAW_BOUNDS = false;
static int avTotalTextures = 0;
static unsigned int numPolygons = 0;
static bool IS_LIGHTING = false;
static bool USE_QUATERNION = true;
static bool USE_DISP_LISTS = true;
static int SOFT_SHADOW = 0;

int getTotalTextures()
{
    return avTotalTextures;
}

int getNextTextureID(){
    return ++avTotalTextures;
}

bool isRenderTexture(){
    return RENDER_TEXTURES;
}

void setRenderTexture( bool val){
    RENDER_TEXTURES = val;
}

bool isDrawBounds(){
    return DRAW_BOUNDS;
}

void setDrawBounds(bool v){
    DRAW_BOUNDS = v;
}

unsigned int getNumRenderedPolygons(){
    return numPolygons;
}

void setLighting( bool v){
    IS_LIGHTING = v;
}

bool getLighting(){
    return IS_LIGHTING;
}

int getSoftShadow(){
    return SOFT_SHADOW;
}

void setSoftShadow( int v){
    if( v>=0)
        SOFT_SHADOW = v;
}

void setQuaternion( bool v){
    USE_QUATERNION = v;
}

bool getQuaternion(){
    return USE_QUATERNION;
}

//-----------------------------------------------------------------------------
//								GLSL
//-----------------------------------------------------------------------------

unsigned long Shader::getFileLength(FILE *f){
    unsigned long len = 0;
    if(f != NULL){
        fseek( f, 0, SEEK_END);
        len = ftell(f);
        fseek( f, 0, SEEK_SET);
    }
    return len;
}

int Shader::loadShader(char* filename, GLenum t){
    FILE *f = fopen( filename, "rb");
    if( f == NULL)
        return -1;	// Error: loading file
    length = Shader::getFileLength( f);
    if( !length)
        return -2;	// Error: Empty File
    buf = new char[length+1];
    if( buf == NULL)
        return -3;	// Error: not enough memory
    fread( buf, 1, length, f);
    buf[length] = '\0';

    fclose( f);
    return 0; // No Error
}

void Shader::unloadShader(){
   if( buf != NULL)
     delete [] buf;
   buf = NULL;
}

//-----------------------------------------------------------------------------
//								TEXTURE
//-----------------------------------------------------------------------------

Texture2d* loadBMP( char *fname){
    int i,j, w, h, bits;
    unsigned long l;
    GLubyte c[3];
    Texture2d* tex;
    FILE *fin;
    unsigned char head[54];
    fin = fopen( fname,"rb");
    if( fin == NULL){
        printf("Bitmap '%s' not found\n", fname);
        return NULL;
    }
    fread( head, 54, 1, fin);
    w = (head[21]<<24)|(head[20]<<16)|(head[19]<<8)|head[18];
    h = (head[25]<<24)|(head[24]<<16)|(head[23]<<8)|head[22];
    bits = (head[29]<<8)|head[28];
    if( head[0]!='B' || head[1] != 'M' || bits != 24){
        printf("Bitmap '%s' is invalid\n", fname);
        return NULL;
    }
    tex = new Texture2d[1];
    tex->w = w;
    tex->h = h;
    tex->buf = new GLubyte[h*w*4];
    tex->id = avTotalTextures++;
    for(i=h-1;i>=0;i--){
        l = i*w*4;
        for(j=0;j<w;j++){
            fread( c, 1, 3, fin);
            tex->buf[l++] = c[2];
            tex->buf[l++] = c[1];
            tex->buf[l++] = c[0];
            tex->buf[l++] = 255;
        }
    }
    fclose( fin);
    printf("Bitmap '%s' loaded\n", fname);
    return tex;
}

bool appendBMP( Texture2d *tex, char *fname){
    int i,j, w, h, bits;
    unsigned long l;
    GLubyte c;
    FILE *fin;
    unsigned char head[54];
    fin = fopen( fname,"rb");
    if( fin == NULL){
        printf("Bitmap '%s' not found\n", fname);
        return NULL;
    }
    fread( head, 54, 1, fin);
    w = (head[21]<<24)|(head[20]<<16)|(head[19]<<8)|head[18];
    h = (head[25]<<24)|(head[24]<<16)|(head[23]<<8)|head[22];
    bits = (head[29]<<8)|head[28];
    if( head[0]!='B' || head[1] != 'M' || bits != 8 || w != tex->w || h != tex->h){
        printf("Bitmap '%s' is invalid alpha map\n", fname);
        return NULL;
    }
    fseek( fin, 1024, SEEK_CUR);
    for(i=h-1;i>=0;i--){
        l = i*w*4+3;
        for(j=0;j<w;j++,l += 4){
            fread( &c, 1, 1, fin);
            tex->buf[l] = c;
        }
    }
    fclose( fin);
    printf("Bitmap '%s' loaded\n", fname);
    return true;
}

//-----------------------------------------------------------------------------
//								Poin2f
//-----------------------------------------------------------------------------

Vector2f::Vector2f(){
    x=y=0.0;
}

Vector2f::Vector2f(float a){
    x=y=a;
}

Vector2f::Vector2f(float a, float b){
    x=a;
    y=b;
}

void Vector2f::set(float a, float b){
    x=a;
    y=b;
}

Vector2f& Vector2f::operator = ( const Vector2f &a){
    x = a.x;
    y = a.y;
    return *this;
}

Vector2f operator + ( const Vector2f &a, const Vector2f &b){
    Vector2f c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    return c;
}

Vector2f operator - ( const Vector2f &a, const Vector2f &b){
    Vector2f c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    return c;
}

Vector2f operator * ( const Vector2f &a, const Vector2f &b){
    Vector2f c;
    c.x = a.x * b.x;
    c.y = a.y * b.y;
    return c;
}

Vector2f operator / ( const Vector2f &a, const Vector2f &b){
    Vector2f c;
    c.x = a.x / b.x;
    c.y = a.y / b.y;
    return c;
}

Vector2f& Vector2f::operator += ( const float d){
    x += d;
    y += d;
    return *this;
}

Vector2f& Vector2f::operator += ( const Vector2f &d){
    x += d.x;
    y += d.y;
    return *this;
}

//-----------------------------------------------------------------------------
//								Vector3f
//-----------------------------------------------------------------------------

Vector3f::Vector3f(){
    x=y=z=0.0;
}

Vector3f::Vector3f(float a){
    x=y=z=a;
}

Vector3f::Vector3f(float a, float b, float c){
    x=a;
    y=b;
    z=c;
}

Vector3f::Vector3f( const Vector4f& a){
    x = a.x;
    y = a.y;
    z = a.z;
}

void Vector3f::set(float a, float b, float c){
    x=a;
    y=b;
    z=c;
}

Vector3f& Vector3f::operator = ( const Vector3f &a){
    x = a.x;
    y = a.y;
    z = a.z;
    return *this;
}

Vector3f Vector3f::operator + ( ){
    return *this;
}

Vector3f Vector3f::operator - ( ){
    Vector3f a;
    a.x = -x;
    a.y = -y;
    a.z = -z;
    return a;
}

Vector3f& Vector3f::operator += (const Vector3f &d){
    x += d.x;
    y += d.y;
    z += d.z;
    return *this;
}

Vector3f& Vector3f::operator -= (const Vector3f &d){
    x -= d.x;
    y -= d.y;
    z -= d.z;
    return *this;
}

Vector3f& Vector3f::operator *= (const Vector3f &d){
    x *= d.x;
    y *= d.y;
    z *= d.z;
    return *this;
}

Vector3f& Vector3f::operator /= (const Vector3f &d){
    x /= d.x;
    y /= d.y;
    z /= d.z;
    return *this;
}

Vector3f operator + ( const Vector3f &a, const Vector3f &b){
    Vector3f c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
    return c;
}

Vector3f operator - ( const Vector3f &a, const Vector3f &b){
    Vector3f c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    return c;
}

float operator & ( const Vector3f &a, const Vector3f &b){
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector3f operator * ( const Vector3f &a, const Vector3f &b){
    Vector3f c;
    c.x = a.x * b.x;
    c.y = a.y * b.y;
    c.z = a.z * b.z;
    return c;
}

Vector3f operator ^ ( const Vector3f &a, const Vector3f &b){
    Vector3f c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

Vector3f operator / ( const Vector3f &a, const Vector3f &b){
    Vector3f c;
    c.x = a.x / b.x;
    c.y = a.y / b.y;
    c.z = a.z / b.z;
    return c;
}

Vector3f operator == ( const Vector3f &a, const Vector3f &b){
    if(a.x==b.x && a.y == b.y && a.z == b.z )
    return true;
}

float Vector3f::magnitude()const{
    return sqrt( x*x+y*y+z*z);
}

float Vector3f::distance( const Vector3f& a)const{
    return sqrt( (x-a.x)*(x-a.x) + (y-a.y)*(y-a.y) + (z-a.z)*(z-a.z));
}

Vector3f Vector3f::normalize()const{
    float m = magnitude();
    return Vector3f( x/m, y/m, z/m);
}

Vector3f Vector3f::rotate( float xy, float yz, float zx)const{
    Vector3f tmp = *this;
    float a;
    a	  = tmp.x*cos(xy) - tmp.y*sin(xy);
    tmp.y = tmp.x*sin(xy) + tmp.y*cos(xy);
    tmp.x = a;
    a	  = tmp.y*cos(yz) - tmp.z*sin(yz);
    tmp.z = tmp.y*sin(yz) + tmp.z*cos(yz);
    tmp.y = a;
    a	  = tmp.z*cos(zx) - tmp.x*sin(zx);
    tmp.x = tmp.z*sin(zx) + tmp.x*cos(zx);
    tmp.z = a;
    return tmp;
}

//-----------------------------------------------------------------------------
//								Vector4f
//-----------------------------------------------------------------------------

Vector4f::Vector4f(){
    x=y=z=u=0.0;
}

Vector4f::Vector4f(float a){
    x=y=z=u=a;
}

Vector4f::Vector4f(float a, float b, float c, float d){
    x=a;
    y=b;
    z=c;
    u=d;
}

Vector4f::Vector4f( const Vector3f& a){
    x = a.x;
    y = a.y;
    z = a.z;
    u = 0.0;
}

void Vector4f::set(float a, float b, float c, float d){
    x=a;
    y=b;
    z=c;
    u=d;
}

Vector4f& Vector4f::operator = ( const Vector4f &a){
    x = a.x;
    y = a.y;
    z = a.z;
    u = a.u;
    return *this;
}

Vector4f operator + ( const Vector4f &a, const Vector4f &b){
    Vector4f c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
    c.u = a.u + b.u;
    return c;
}

Vector4f operator - ( const Vector4f &a, const Vector4f &b){
    Vector4f c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    c.u = a.u - b.u;
    return c;
}

Vector4f operator * ( const Vector4f &a, const Vector4f &b){
    Vector4f c;
    c.x = a.x * b.x;
    c.y = a.y * b.y;
    c.z = a.z * b.z;
    c.u = a.u * b.u;
    return c;
}


Vector4f operator / ( const Vector4f &a, const Vector4f &b){
    Vector4f c;
    c.x = a.x / b.x;
    c.y = a.y / b.y;
    c.z = a.z / b.z;
    c.u = a.u / b.u;
    return c;
}

float Vector4f::magnitude()const{
    return sqrt( x*x+y*y+z*z+u*u);
}

Vector4f Vector4f::normalize()const{
    float m = magnitude();
    return Vector4f( x/m, y/m, z/m, u/m);
}

//-----------------------------------------------------------------------------
//								Quaternions
//-----------------------------------------------------------------------------

TrackBall currentTrackBall;
void setActiveViewTrackBall( const TrackBall& t){
    currentTrackBall = t;
}

//-----------------------------------------------------------------------------
//								Material3D
//-----------------------------------------------------------------------------

Material3d::Material3d(){
    ambi = .2;
    diff = .6;
    spec = 1.;
    emis = 0.;
    shin = 32;
}

void Material3d::setColor( const Vector4f& color){
    ambi = .2*color;
    diff = color;
}

//-----------------------------------------------------------------------------
//								Light3D
//-----------------------------------------------------------------------------

static std::list<Light3d*> listLight3d;

Light3d::Light3d(){
    castShadow = false;
    isSpot = false;
    isOn = false;
    att.set( 1, 0, 0);
    listLight3d.push_back( this);
}

Light3d::~Light3d(){
    listLight3d.remove( this);
}

void Light3d::runGLcmd( int light){
    if( isOn){
        glPushMatrix();
        glLightfv( light, GL_AMBIENT, (GLfloat*)&mat.ambi);
        glLightfv( light, GL_DIFFUSE, (GLfloat*)&mat.diff);
        glLightfv( light, GL_SPECULAR, (GLfloat*)&mat.spec);
        glLightfv( light, GL_POSITION, (GLfloat*)&pos);
        glLightf( light, GL_CONSTANT_ATTENUATION, att.x);
        glLightf( light, GL_LINEAR_ATTENUATION, att.y);
        glLightf( light, GL_QUADRATIC_ATTENUATION, att.z);
        if( isSpot){
            glLightf( light, GL_SPOT_CUTOFF, cutoff);
            glLightfv( light, GL_SPOT_DIRECTION, (GLfloat*)&sdir);
        }
        glPopMatrix();
    }
}

//-----------------------------------------------------------------------------
//								Camera3d
//-----------------------------------------------------------------------------

Camera3d activeCamera;

Camera3d::Camera3d(){
    pos.set( 0., 0., 0.);
    eye.set( 0., 0., 1.);
    ori.set( 0., 1., 0.);
    ang = 45.0;
    cnear = 1.0;
    cfar = 500.0;
    ar = 1.0;
    isOrtho = false;
    orthoMin.set( -100, -100, -100);
    orthoMax.set( 100, 100, 100);
}

Camera3d& Camera3d::operator = ( const Camera3d &c){
    pos  = c.pos;
    eye  = c.eye;
    ori = c.ori;
    ang  = c.ang;
    cnear = c.cnear;
    cfar  = c.cfar;
    ar   = c.ar;
    isOrtho = c.isOrtho;
    orthoMin = c.orthoMin;
    orthoMax = c.orthoMax;
    return *this;
}

void setActiveCamera( const Camera3d& cam){
    activeCamera = cam;
}

//-----------------------------------------------------------------------------
//								Object3d
//-----------------------------------------------------------------------------

static std::list<Object3d*> listObject3d;

Object3d::Object3d(){
    objType = 0;
    isCollidable = false;
    isExtreamObj = true;
    isHidden = false;
    tex = NULL;
    //texEnvParam = GL_MODULATE;
    texEnvParam = GL_MODULATE;
    col.set( .7, .7, .7);
    texCol.set( 1., 1., 1.);
    listObject3d.push_back( this);
    dispList = -1;
    factorMul.set( 1., 1., 1.);
    factorRot.set( 0., 0., 0.);
    factorTra.set( 0., 0., 0.);
    mat = NULL;
    castShadow = false;
    envMapping = false;
}

Object3d::~Object3d(){
    listObject3d.remove( this);
}

void Object3d::operatorAssign( const Object3d &c)
{
    this->objType = c.objType;
    this->isCollidable = c.isCollidable;
    this->isExtreamObj = c.isExtreamObj;
    this->isHidden = c.isHidden;
    this->queryCollision = c.queryCollision;
    this->cubeBoundMax = c.cubeBoundMax;
    this->cubeBoundMin = c.cubeBoundMin;
    this->col = c.col;
    this->tex = c.tex;
    this->texEnvParam = c.texEnvParam;
    this->dispList = -1;
    this->factorMul= c.factorMul;
    this->factorRot = c.factorRot;
    this->factorTra = c.factorTra;
    this->mat = c.mat;
    this->castShadow = c.castShadow;
    this->envMapping = c.envMapping;
}

void Object3d::updateDisplayList(){
    if( dispList == -1){
        dispList = glGenLists (1);//avTotalLists++;
    }
    glNewList( dispList, GL_COMPILE);
    GLuint tmp = dispList;
    dispList = -1;
    draw();
    dispList = tmp;
    glEndList();
}

void Object3d::makeBoundsEql( ){
    Vector3f dim = cubeBoundMax - cubeBoundMin;
    float max = dim.x;
    if( max < dim.y)
        max = dim.y;
    if( max < dim.z)
        max = dim.z;
    max/=2.0;
    dim = cubeBoundMin + dim/2.0;
    cubeBoundMin = dim - max;
    cubeBoundMax = dim + max;
}

void Object3d::bindTexture( Texture2d *t)
//void Object3d::bindTexture( GLubyte *t)
{
    if( t == NULL)
        return;
    tex = t;
    //glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->buf);
    //gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, tex->w, tex->h, GL_RGBA, GL_UNSIGNED_BYTE, tex->buf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->buf);
    gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, tex->buf);
    delete [] tex->buf;
    tex->buf = NULL;
}

void Object3d::drawBounds(){
    glBegin(GL_LINE_STRIP);
    glColor3f( .9, .9, .9);
    glVertex3f( cubeBoundMin.x, cubeBoundMin.y, cubeBoundMin.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMin.y, cubeBoundMin.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMin.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMin.x, cubeBoundMin.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMin.x, cubeBoundMin.y, cubeBoundMin.z);
    glVertex3f( cubeBoundMin.x, cubeBoundMax.y, cubeBoundMin.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMax.y, cubeBoundMin.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMin.y, cubeBoundMin.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMax.y, cubeBoundMin.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMax.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMin.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMax.x, cubeBoundMax.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMin.x, cubeBoundMax.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMin.x, cubeBoundMin.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMin.x, cubeBoundMax.y, cubeBoundMax.z);
    glVertex3f( cubeBoundMin.x, cubeBoundMax.y, cubeBoundMin.z);
    glEnd();
}

bool collisionTest( const Vector3f& before, const Vector3f& after){
    for( std::list<Object3d*>::iterator i=listObject3d.begin(); i!=listObject3d.end(); i++)
        if( (**i).isCollidable == true){
            int bxs, bys, bzs, axs, ays, azs; // before and after states
            bxs = ( before.x<=((**i).cubeBoundMin.x-activeCamera.cnear))?-1:(( before.x>=((**i).cubeBoundMax.x+activeCamera.cnear))?1:0);
            bys = ( before.y<=((**i).cubeBoundMin.y-activeCamera.cnear))?-1:(( before.y>=((**i).cubeBoundMax.y+activeCamera.cnear))?1:0);
            bzs = ( before.z<=((**i).cubeBoundMin.z-activeCamera.cnear))?-1:(( before.z>=((**i).cubeBoundMax.z+activeCamera.cnear))?1:0);
            axs = ( after.x<=((**i).cubeBoundMin.x-activeCamera.cnear))?-1:(( after.x>=((**i).cubeBoundMax.x+activeCamera.cnear))?1:0);
            ays = ( after.y<=((**i).cubeBoundMin.y-activeCamera.cnear))?-1:(( after.y>=((**i).cubeBoundMax.y+activeCamera.cnear))?1:0);
            azs = ( after.z<=((**i).cubeBoundMin.z-activeCamera.cnear))?-1:(( after.z>=((**i).cubeBoundMax.z+activeCamera.cnear))?1:0);
            if( bxs != axs && (bys==0||ays==0) && (bzs==0||azs==0) )
                return true;
            if( bys != ays && (bxs==0||axs==0) && (bzs==0||azs==0) )
                return true;
            if( bzs != azs && (bys==0||ays==0) && (bxs==0||axs==0) )
                return true;
        }
    return false;
}

bool collisionTest( Object3d& chr){
    Vector3f cu[9];
    cu[0].set( chr.cubeBoundMin.x, chr.cubeBoundMin.y, chr.cubeBoundMin.z);
    cu[1].set( chr.cubeBoundMax.x, chr.cubeBoundMin.y, chr.cubeBoundMin.z);
    cu[2].set( chr.cubeBoundMax.x, chr.cubeBoundMin.y, chr.cubeBoundMax.z);
    cu[3].set( chr.cubeBoundMin.x, chr.cubeBoundMin.y, chr.cubeBoundMax.z);
    cu[4].set( chr.cubeBoundMin.x, chr.cubeBoundMax.y, chr.cubeBoundMax.z);
    cu[5].set( chr.cubeBoundMin.x, chr.cubeBoundMax.y, chr.cubeBoundMin.z);
    cu[6].set( chr.cubeBoundMax.x, chr.cubeBoundMax.y, chr.cubeBoundMin.z);
    cu[7].set( chr.cubeBoundMax.x, chr.cubeBoundMax.y, chr.cubeBoundMax.z);
    cu[8].set( chr.cubeBoundMin.x, chr.cubeBoundMax.y, chr.cubeBoundMax.z);
    bool oldstate = chr.isCollidable;
    chr.isCollidable = false;
    for( int i=0; i<8; i++)
        if( collisionTest( cu[i], cu[i+1])){
            chr.isCollidable = oldstate;
            return true;
        }
    if( collisionTest(cu[0],cu[3])||collisionTest(cu[0],cu[5])||collisionTest(cu[1],cu[6])||collisionTest(cu[2],cu[7])){
        chr.isCollidable = oldstate;
        return true;
    }
    chr.isCollidable = oldstate;
    return false;
}

void updateDisplayList( ){
    for( std::list<Object3d*>::iterator i=listObject3d.begin(); i!=listObject3d.end(); i++)
        if( (**i).dispList != -1)
            (**i).updateDisplayList();
}

void runForAllObject3d( void(*func)(Object3d*)){
    for( std::list<Object3d*>::iterator i=listObject3d.begin(); i!=listObject3d.end(); i++)
        func(*i);
}

//-----------------------------------------------------------------------------
//								Plane3d
//-----------------------------------------------------------------------------

Plane3d::Plane3d(){
    objType = 1;
    isShadowPlane = false;
    p1.set( -1, -1, 1.);
    p2.set( -1,  1, 1.);
    p3.set(  1,  1, 1.);
    p4.set(  1, -1, 1.);
    nor.set( 0, 0, -1);
    t1.set( 0., 0.);
    t2.set( 0., 1.);
    t3.set( 1., 1.);
    t4.set( 1., 0.);
}

void Plane3d::setPlane( float width, float height, Plane3dOri o){
    if( o == PLAIN_XY){
        p1.set( -width, -height, 0.);
        p2.set( -width,  height, 0.);
        p3.set(  width,  height, 0.);
        p4.set(  width, -height, 0.);
    }
    else if( o == PLAIN_YZ){
        p1.set( 0., -height, -width);
        p2.set( 0.,  height, -width);
        p3.set( 0.,  height,  width);
        p4.set( 0., -height,  width);
    }
    else{
        p1.set( -width, 0., -height);
        p2.set( -width, 0.,  height);
        p3.set(  width, 0.,  height);
        p4.set(  width, 0., -height);
    }
    calcBoundingCube();
}

void Plane3d::calcNormal( bool cw){
    if( cw)
        nor = (p2-p1)^(p3-p2);
    else
        nor = (p2-p3)^(p3-p4);
}

Plane3d& Plane3d::operator = (const Plane3d &c){
    operatorAssign( c);
    this->p1 = c.p1;
    this->p2 = c.p2;
    this->p3 = c.p3;
    this->p4 = c.p4;
    this->t1 = c.t1;
    this->t2 = c.t2;
    this->t3 = c.t3;
    this->t4 = c.t4;
    this->nor = c.nor;
    return *this;
}

Plane3d& Plane3d::operator += (const Vector3f& d){
    p1 += d;
    p2 += d;
    p3 += d;
    p4 += d;
    cubeBoundMax += d;
    cubeBoundMin += d;
    return *this;
}

void Plane3d::setTexture( float scaleX, float scaleY, TextureOri o, bool flipH, bool flipV){
    if( o == TEX_ANGLE_180){
        t1.set( flipH?scaleX:0., flipV?scaleY:0.);
        t2.set( flipH?scaleX:0., flipV?0.:scaleY);
        t3.set( flipH?0.:scaleX, flipV?0.:scaleY);
        t4.set( flipH?0.:scaleX, flipV?scaleY:0.);
    }
    else if( o == TEX_ANGLE_270){
        t2.set( flipH?scaleX:0., flipV?scaleY:0.);
        t3.set( flipH?scaleX:0., flipV?0.:scaleY);
        t4.set( flipH?0.:scaleX, flipV?0.:scaleY);
        t1.set( flipH?0.:scaleX, flipV?scaleY:0.);
    }
    else if( o == TEX_ANGLE_0){
        t3.set( flipH?scaleX:0., flipV?scaleY:0.);
        t4.set( flipH?scaleX:0., flipV?0.:scaleY);
        t1.set( flipH?0.:scaleX, flipV?0.:scaleY);
        t2.set( flipH?0.:scaleX, flipV?scaleY:0.);
    }
    else if( o == TEX_ANGLE_90){
        t4.set( flipH?scaleX:0., flipV?scaleY:0.);
        t1.set( flipH?scaleX:0., flipV?0.:scaleY);
        t2.set( flipH?0.:scaleX, flipV?0.:scaleY);
        t3.set( flipH?0.:scaleX, flipV?scaleY:0.);
    }
}

void Plane3d::draw(){
    numPolygons++;
    if( RENDER_TEXTURES && tex != NULL){
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texEnvParam);
        glBindTexture( GL_TEXTURE_2D, tex->id);
    }
    glBegin(GL_QUADS);
        if( !RENDER_TEXTURES || tex == NULL)
            glColor3fv( (GLfloat*)&col);
        else
            glColor3fv( (GLfloat*)&texCol);
        glNormal3fv( (GLfloat*)&nor);
        if( RENDER_TEXTURES && tex != NULL)
            glTexCoord2fv( (GLfloat*)&t1);
        glVertex3fv( (GLfloat*)&p1);
        if( RENDER_TEXTURES && tex != NULL)
            glTexCoord2fv( (GLfloat*)&t2);
        glVertex3fv( (GLfloat*)&p2);
        if( RENDER_TEXTURES && tex != NULL)
            glTexCoord2fv( (GLfloat*)&t3);
        glVertex3fv( (GLfloat*)&p3);
        if( RENDER_TEXTURES && tex != NULL)
            glTexCoord2fv( (GLfloat*)&t4);
        glVertex3fv( (GLfloat*)&p4);
    glEnd();
    if( DRAW_BOUNDS)
        drawBounds();
    if( RENDER_TEXTURES && tex != NULL)
        glDisable(GL_TEXTURE_2D);
}

void Plane3d::calcBoundingCube(){
    cubeBoundMin.set( OBJINFINITY, OBJINFINITY, OBJINFINITY);
    cubeBoundMax.set( -OBJINFINITY, -OBJINFINITY, -OBJINFINITY);
    if( p1.x < cubeBoundMin.x)
        cubeBoundMin.x = p1.x;
    else if( p1.x > cubeBoundMax.x)
        cubeBoundMax.x = p1.x;
    if( p1.y < cubeBoundMin.y)
        cubeBoundMin.y = p1.y;
    else if( p1.y > cubeBoundMax.y)
        cubeBoundMax.y = p1.y;
    if( p1.z < cubeBoundMin.z)
        cubeBoundMin.z = p1.z;
    else if( p1.z > cubeBoundMax.z)
        cubeBoundMax.z = p1.z;
    if( p2.x < cubeBoundMin.x)
        cubeBoundMin.x = p2.x;
    else if( p2.x > cubeBoundMax.x)
        cubeBoundMax.x = p2.x;
    if( p2.y < cubeBoundMin.y)
        cubeBoundMin.y = p2.y;
    else if( p2.y > cubeBoundMax.y)
        cubeBoundMax.y = p2.y;
    if( p2.z < cubeBoundMin.z)
        cubeBoundMin.z = p2.z;
    else if( p2.z > cubeBoundMax.z)
        cubeBoundMax.z = p2.z;
    if( p3.x < cubeBoundMin.x)
        cubeBoundMin.x = p3.x;
    else if( p3.x > cubeBoundMax.x)
        cubeBoundMax.x = p3.x;
    if( p3.y < cubeBoundMin.y)
        cubeBoundMin.y = p3.y;
    else if( p3.y > cubeBoundMax.y)
        cubeBoundMax.y = p3.y;
    if( p3.z < cubeBoundMin.z)
        cubeBoundMin.z = p3.z;
    else if( p3.z > cubeBoundMax.z)
        cubeBoundMax.z = p3.z;
    if( p4.x < cubeBoundMin.x)
        cubeBoundMin.x = p4.x;
    else if( p4.x > cubeBoundMax.x)
        cubeBoundMax.x = p4.x;
    if( p4.y < cubeBoundMin.y)
        cubeBoundMin.y = p4.y;
    else if( p4.y > cubeBoundMax.y)
        cubeBoundMax.y = p4.y;
    if( p4.z < cubeBoundMin.z)
        cubeBoundMin.z = p4.z;
    else if( p4.z > cubeBoundMax.z)
        cubeBoundMax.z = p4.z;
    //printPoint3f("cubeBoundMin",cubeBoundMin);
    //printPoint3f("cubeBoundMax",cubeBoundMax);
}

//-----------------------------------------------------------------------------
//								Sphere3d
//-----------------------------------------------------------------------------

Sphere::Sphere(){
    objType = 2;
    cen.set( 0., 0., 0.);
    r = 1.;
    hs = 8;
    vs = 4;
    lon.set( 0., 1.);
    lat.set( 0., 1.);
    off.set( 0., 0.);
    tc.set( 1., 1.);
}

Sphere& Sphere::operator += (const Vector3f d){
    cen += d;
    return *this;
}
void Sphere::setTexture( float scaleX, float scaleY, float longitude, float latitude){
    tc.set( scaleX, scaleY);
    off.set( longitude, latitude);
}

void Sphere::draw(){
    float t, th, ph;
    Vector3f n,v;
    numPolygons +=hs*vs;
    if( USE_DISP_LISTS && dispList != -1){
        glCallList( dispList);
        return;
    }
    if( r < 0 )
        r = -r;
    if( hs <0 )
        hs = -hs;
    if( vs < 0)
        vs = -vs;
    if( RENDER_TEXTURES && tex != NULL){
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texEnvParam);
        glBindTexture( GL_TEXTURE_2D, tex->id);
    }
    int xs = lon.x*hs;
    int xe = lon.y*hs;
    int ys = lat.x*vs;
    int ye = lat.y*vs;
    if( ye <= ys)
        ye = ys + 1;
    if( xe <= xs)
        xe = xs + 1;
    if( !RENDER_TEXTURES || tex == NULL)
        glColor3fv( (GLfloat*)&col);
    else
        glColor3fv( (GLfloat*)&texCol);
    for( int j=ys; j<ye; j++)
    {
        t = HALF_PI + off.y - PI*((float)(j)/(float)(vs));
        glBegin( GL_QUAD_STRIP);
        for( int i=xs; i<=xe; i++)
        {
            th = t;
            ph = off.x + 2.0*PI*((float)(i)/(float)(hs));
            n.set( cos(th)*cos(ph), sin(th), cos(th)*sin(ph));
            v = cen + r*n;
            glNormal3fv( (GLfloat*)&n);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2f( tc.x*(float)(i)/(float)(hs), tc.y *(float)(j)/(float)(vs));
            glVertex3fv( (GLfloat*)&v);
            th = t - PI/(float)(vs);
            n.set( cos(th)*cos(ph), sin(th), cos(th)*sin(ph));
            v = cen + r*n;
            glNormal3fv( (GLfloat*)&n);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2f( tc.x*(float)(i)/(float)(hs), tc.y*(float)(j+1)/(float)(vs));
            glVertex3fv( (GLfloat*)&v);
        }
        glEnd();
        if( DRAW_BOUNDS)
            drawBounds();
    }
    if( RENDER_TEXTURES && tex != NULL)
        glDisable(GL_TEXTURE_2D);
}

void Sphere::calcBoundingCube()
{
    cubeBoundMin = cen - r;
    cubeBoundMax = cen + r;
}

//-----------------------------------------------------------------------------
//								Obj3ds
//-----------------------------------------------------------------------------

Obj3ds::Obj3ds(){
    objType = 10;
    numVert = numPoly = 0;
    vertex = NULL;
    //myvertex = NULL;
    normal = NULL;
    mapcoord = NULL;
    triangle = NULL;
    name[0] = '\0';
    smooth = false;
    Vector3f *pppvertex = new Vector3f[numVert];
}

bool Obj3ds::Load3ds( char *filename){
    int i;			//Index variable
    FILE *fin;		//File pointer

    unsigned short cid;		// Chunk identifier
    unsigned int   clen;		// Chunk lenght
    unsigned short ushortv;
    unsigned char  chr;			// Char variable
    unsigned short n;			// Number of elements in each chunk
    unsigned short faceflags;	//Flag that stores some face information

    if ((fin=fopen(filename, "rb"))== NULL) {
        printf("3DS '%s' not found", filename);
        return false;			// Unable to open the file
    }

    while ( ftell(fin) < filelength( fileno(fin))){
        fread (&cid , 2, 1, fin);			//Read the chunk header
        fread (&clen, 4, 1, fin);			//Read the lenght of the chunk

        switch (cid){
            // Description: Main chunk, contains all the other chunks
            // Chunk Lenght: 0 + sub chunks
            case 0x4d4d:
            break;

            // Description: 3D Editor chunk, objects layout info
            // Chunk Lenght: 0 + sub chunks
            case 0x3d3d:
            break;

            // Description: Object block, info for each object
            // Chunk Lenght: len(object name) + sub chunks
            case 0x4000:
                for( i=0,chr=1; i<20 && chr!=0; i++){
                    fread (&chr, 1, 1, fin);
                    name[i] = chr;
                }
                printf("3DS %s (src:%s)\n", name, filename);
            break;

            // Description: Triangular mesh, contains chunks for 3d mesh info
            // Chunk Lenght: 0 + sub chunks
            case 0x4100:
            break;

            // Description: Vertices list
            // Chunk Lenght: 1 x unsigned short (number of vertices)
            //             + 3 x float (vertex coordinates) x (number of vertices)
            //             + sub chunks
            case 0x4110:
                fread (&n, 2, 1, fin);
                numVert = n;
                vertex = new Vector3f[numVert];
               // myvertex = new Vector3f[numPoly];
                printf(" + -- Number of vertices: %d\n",numVert);
                for (i=0; i<numVert; i++){
                    fread (&vertex[i].x, sizeof(float), 1, fin);
                    fread (&vertex[i].z, sizeof(float), 1, fin);
                    fread (&vertex[i].y, sizeof(float), 1, fin);
                    vertex[i].z = - vertex[i].z;
                }
                break;

            // Description: Polygons (faces) list
            // Chunk Lenght: 1 x unsigned short (number of polygons)
            //             + 3 x unsigned short (polygon points) x (number of polygons)
            //             + sub chunks
            case 0x4120:
                fread (&n, 2, 1, fin);
                numPoly = n;
                triangle = new TriMesh[numPoly];
                printf(" + -- Number of polygons: %d\n",numPoly);
                for (i=0; i<numPoly; i++){
                    fread (&ushortv, 2, 1, fin); triangle[i].a = ushortv;
                    fread (&ushortv, 2, 1, fin); triangle[i].b = ushortv;
                    fread (&ushortv, 2, 1, fin); triangle[i].c = ushortv;
                    fread (&faceflags, 2, 1, fin);
                }
                break;

            // Description: Mapping cordinates (Vertices list)
            // Chunk Lenght: 1 x unsigned short (number of mapping points)
            //             + 2 x float (mapping coordinates) x (number of mapping points)
            //             + sub chunks
            case 0x4140:
                fread (&n, sizeof (unsigned short), 1, fin);
                printf(" + -- Number of maps: %d\n",n);
                mapcoord = new Vector2f[n];
                for (i=0; i<n; i++)
                {
                    fread (&mapcoord[i].x, sizeof (float), 1, fin);
                    fread (&mapcoord[i].y, sizeof (float), 1, fin);
                    mapcoord[i].y = 1.0 - mapcoord[i].y;
                }
                break;

            //----------- Skip unknow chunks ------------
            default:
                 fseek( fin, clen-6, SEEK_CUR);
        }
    }
    fclose (fin);
    calcNormals();
    calcBoundingCube();
    return true;
}

void Obj3ds::calcNormals(){
    if( normal != NULL)
        delete [] normal;
    normal = new Vector3f[numVert];
    int* neigh = new int[numVert];
    for (int i=0; i<numVert; i++){
        neigh[i] = 0;
        normal[i].set (0., 0., 0.);
    }
    for ( int i=0; i<numPoly; i++){
        TriMesh p = triangle[i];
        Vector3f v12 = vertex[p.b]-vertex[p.a];
        Vector3f v23 = vertex[p.c]-vertex[p.b];
        //Vector3f v31 = vertex[p.a]-vertex[p.c];
        Vector3f nor = v12^v23;
        normal[p.a] += nor; neigh[p.a]++;
        normal[p.b] += nor; neigh[p.b]++;
        normal[p.c] += nor; neigh[p.c]++;
    }
    /*for (i=0; i<numVert; i++)
        if( neigh[i] != 0)
            normal[i] = normal[i]/double(neigh[i]);*/
    delete [] neigh;
}

void Obj3ds::calcBoundingCube(){
    cubeBoundMin.set( OBJINFINITY, OBJINFINITY, OBJINFINITY);
    cubeBoundMax.set( -OBJINFINITY, -OBJINFINITY, -OBJINFINITY);
    for( int i=0; i<numVert; i++){
        if( vertex[i].x < cubeBoundMin.x)
            cubeBoundMin.x = vertex[i].x;
        else if( vertex[i].x > cubeBoundMax.x)
            cubeBoundMax.x = vertex[i].x;
        if( vertex[i].y < cubeBoundMin.y)
            cubeBoundMin.y = vertex[i].y;
        else if( vertex[i].y > cubeBoundMax.y)
            cubeBoundMax.y = vertex[i].y;
        if( vertex[i].z < cubeBoundMin.z)
            cubeBoundMin.z = vertex[i].z;
        else if( vertex[i].z > cubeBoundMax.z)
            cubeBoundMax.z = vertex[i].z;
    }
}

Obj3ds& Obj3ds::operator = ( const Obj3ds &c)
{
    operatorAssign( c);
    for(int i=0; i<24; i++)
        this->name[i] = c.name[i];
    this->numPoly = c.numPoly;
    this->numVert = c.numVert;
    this->triangle = c.triangle;
    this->mapcoord = c.mapcoord;
    if( this->vertex != NULL)
        delete [] this->vertex;
    this->vertex = new Vector3f[c.numVert];
    for( int i=0; i<c.numVert; i++)
        this->vertex[i] = c.vertex[i];
    this->normal = new Vector3f[c.numVert];
    this->calcNormals ();
    return *this;
}

Obj3ds& Obj3ds::operator += (const Vector3f &d){
    for( int i=0; i<numVert; i++)
        vertex[i] += d;
    cubeBoundMax += d;
    cubeBoundMin += d;
    calcNormals();
    return *this;
}

void Obj3ds::scale( const Vector3f &d){
    Vector3f mid = (cubeBoundMax - cubeBoundMin)/2.0;
    cubeBoundMin.set( OBJINFINITY, OBJINFINITY, OBJINFINITY);
    cubeBoundMax.set( -OBJINFINITY, -OBJINFINITY, -OBJINFINITY);
    for( int i=0; i<numVert; i++){
        Vector3f tmp = vertex[i] - mid;
        tmp *= d;
        vertex[i] = tmp + mid;
        if( vertex[i].x < cubeBoundMin.x)
            cubeBoundMin.x = vertex[i].x;
        else if( vertex[i].x > cubeBoundMax.x)
            cubeBoundMax.x = vertex[i].x;
        if( vertex[i].y < cubeBoundMin.y)
            cubeBoundMin.y = vertex[i].y;
        else if( vertex[i].y > cubeBoundMax.y)
            cubeBoundMax.y = vertex[i].y;
        if( vertex[i].z < cubeBoundMin.z)
            cubeBoundMin.z = vertex[i].z;
        else if( vertex[i].z > cubeBoundMax.z)
            cubeBoundMax.z = vertex[i].z;
    }
    calcNormals();
}

void Obj3ds::rotate( float xy, float yz, float zx){
    xy *= PI/180.0;
    yz *= PI/180.0;
    zx *= PI/180.0;
    factorRot += Vector3f( xy, yz, zx);
    Vector3f mid = (cubeBoundMax - cubeBoundMin)/2.0;
    cubeBoundMin.set( OBJINFINITY, OBJINFINITY, OBJINFINITY);
    cubeBoundMax.set( -OBJINFINITY, -OBJINFINITY, -OBJINFINITY);
    for( int i=0; i<numVert; i++){
        Vector3f tmp = (vertex[i] - mid).rotate( xy, yz, zx);
        vertex[i] = tmp + mid;
        if( vertex[i].x < cubeBoundMin.x)
            cubeBoundMin.x = vertex[i].x;
        else if( vertex[i].x > cubeBoundMax.x)
            cubeBoundMax.x = vertex[i].x;
        if( vertex[i].y < cubeBoundMin.y)
            cubeBoundMin.y = vertex[i].y;
        else if( vertex[i].y > cubeBoundMax.y)
            cubeBoundMax.y = vertex[i].y;
        if( vertex[i].z < cubeBoundMin.z)
            cubeBoundMin.z = vertex[i].z;
        else if( vertex[i].z > cubeBoundMax.z)
            cubeBoundMax.z = vertex[i].z;
    }
    calcNormals();
}

void Obj3ds::draw(){
    numPolygons +=numPoly;
    if( vertex == NULL )
        return;
    if( USE_DISP_LISTS && dispList != -1){
        glCallList( dispList);
        return;
    }
    if( RENDER_TEXTURES && tex != NULL){
        glEnable( GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texEnvParam);
        glBindTexture( GL_TEXTURE_2D, tex->id);
        glColor3fv( (GLfloat*)&texCol);
    }
    else{
        glColor3fv( (GLfloat*)&col);
        if( mat != NULL){
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat*)&(mat->ambi));
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&(mat->diff));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat*)&(mat->spec));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat*)&(mat->shin));
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (GLfloat*)&(mat->emis));
        }
    }
    if( smooth)
    {
        glEnableClientState( GL_VERTEX_ARRAY);
        glVertexPointer( 3, GL_FLOAT, 0, (GLfloat*)vertex);
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer( GL_FLOAT, 0, (GLfloat*)normal);
        if( RENDER_TEXTURES && tex != NULL){
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer( 2, GL_FLOAT, 0, (GLfloat*)mapcoord);
        }
        glDrawElements( GL_TRIANGLES, numPoly * 3, GL_UNSIGNED_INT, triangle);
        if( RENDER_TEXTURES && tex != NULL)
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    else{
        glBegin(GL_TRIANGLES);
        for( int i=0; i<numPoly; i++){
            TriMesh p = triangle[i];
            Vector3f nor = (vertex[p.a]-vertex[p.b])^(vertex[p.b]-vertex[p.c]);
            glNormal3fv( (GLfloat*)&nor);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2fv( (GLfloat*)&mapcoord[p.a]);
            glVertex3fv( (GLfloat*)&vertex[p.a]);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2fv( (GLfloat*)&mapcoord[p.b]);
            glVertex3fv( (GLfloat*)&vertex[p.b]);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2fv( (GLfloat*)&mapcoord[p.c]);
            glVertex3fv( (GLfloat*)&vertex[p.c]);
        }
        glEnd();
    }
    if( DRAW_BOUNDS)
        drawBounds();
    if( RENDER_TEXTURES && tex != NULL)
        glDisable(GL_TEXTURE_2D);
}

Obj3dsGraph::Obj3dsGraph(){
    elist = NULL;
}

Obj3dsGraph::~Obj3dsGraph(){
    if( elist != NULL)
        delete [] elist;
}

struct TriMeshTriplet{
    int u, v, t;

    TriMeshTriplet( int a, int b, int c){
        t = c;
        if( a < b){
            u = a; v = b;
        }
        else{
            u = b; v = a;
        }
    }

    bool operator < (const TriMeshTriplet& x) const{
        if( u < x.u)
            return true;
        else if( u == x.u)
            return v < x.v;
        return false;
    }
};

void Obj3dsGraph::calcGraph(){
    //Vector3f myvertex[numPoly];


    clock_t ct = clock();

    elist = new TriMesh[numPoly];
    for( int i=0; i<numPoly; i++)
        elist[i].a = elist[i].b = elist[i].c = -1;

    std::vector<TriMeshTriplet> ttv;

    for( int i=0; i<numPoly; i++){
        TriMesh *t = &triangle[i];
        TriMeshTriplet tt1(t->a, t->b, i);
        TriMeshTriplet tt2(t->b, t->c, i);
        TriMeshTriplet tt3(t->c, t->a, i);
        ttv.push_back( tt1);
        ttv.push_back( tt2);
        ttv.push_back( tt3);
    }

    std::sort( ttv.begin(), ttv.end());
    FILE *jjjj = fopen("ttv.txt","w");

    for(int i=0;i<numPoly;i++)
    {
        fprintf(jjjj,"%d %d %d\n",ttv[i].u,ttv[i].v,ttv[i].t);
    }
    fclose(jjjj);

    for( int k=0; k<ttv.size()-1; k++)
    {
        if( ttv[k].u == ttv[k+1].u && ttv[k].v == ttv[k+1].v)
        {
            int i = ttv[k].t;
            int j = ttv[k+1].t;

            if( elist[i].a == -1)
                elist[i].a = j;
            else if( elist[i].b == -1)
                elist[i].b = j;
            else
                elist[i].c = j;

            if( elist[j].a == -1)
                elist[j].a = i;
            else if( elist[j].b == -1)
                elist[j].b = i;
            else
                elist[j].c = i;

            //k++;
        }
    }

    TriMesh q;
    Vector3f qq;
    for(int i=0;i<numPoly;i++)
    {
        q = triangle[i];
        qq = (vertex[q.a] + vertex[q.b] + vertex[q.c] ) / Vector3f(3.0);
        myvertex.push_back( qq);
    }

    FILE *jjjjj = fopen("elist.txt","w");

    for(int i=0;i<numPoly;i++)
    {
        fprintf(jjjjj,"%d %d %d\n",elist[i].a,elist[i].b,elist[i].c);
    }
    fclose(jjjjj);

    double td = ((double) (clock() - ct)) / CLOCKS_PER_SEC;

    printf("Graph Calculated. %f sec\n", td);
}

/*void Obj3dsGraph::calcGraph(){
    elist = new TriMesh[numPoly];
    for( int i=0; i<numPoly; i++)
        elist[i].a = elist[i].b = elist[i].c = -1;

    for( int i=0; i<numPoly; i++){
        for( int j=i+1; j<numPoly; j++){
            TriMesh *t1 = &triangle[i];
            TriMesh *t2 = &triangle[j];
            int match = 0;
            if( t1->a == t2->a || t1->a == t2->b || t1->a == t2->c )
                match++;
            if( t1->b == t2->a || t1->b == t2->b || t1->b == t2->c )
                match++;
            if( t1->c == t2->a || t1->c == t2->b || t1->c == t2->c )
                match++;
            if( match == 2){

                if( elist[i].a == -1)
                    elist[i].a = j;
                else if( elist[i].b == -1)
                    elist[i].b = j;
                else
                    elist[i].c = j;

                if( elist[j].a == -1)
                    elist[j].a = i;
                else if( elist[j].b == -1)
                    elist[j].b = i;
                else
                    elist[j].c = i;
            }
        }
    }
    printf("Graph Calculated.\n");
}*/

std::vector<unsigned int> seedt;

void Obj3dsGraph::drawGraph(){
    //Obj3ds::draw();
    //glColor3f( 1., 1., 1.);
    glColor3f( 1., 0., 0.);
    glLineWidth(17.0);
    glBegin(GL_LINES);
    for( int c=0; c<seedt.size(); c++){
        unsigned int i = seedt[c];
        glVertex3fv( (GLfloat*)&vertex[triangle[i].a]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].b]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].b]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].c]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].c]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].a]);
    }
    glEnd();

    glColor3f( 0., 1., 1.);
    glLineWidth(1.0);
   glBegin(GL_LINES);
   //int i=0;
   //for( int i=0; i<3; i++)
    for( int i=0; i<numPoly; i++)
   {
        glVertex3fv( (GLfloat*)&vertex[triangle[i].a]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].b]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].b]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].c]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].c]);
        glVertex3fv( (GLfloat*)&vertex[triangle[i].a]);
    }
    glEnd();

//    for(int i=0;i<=8;i++)
//    {
//     glColor3f(1, 1., 1.);
//glPointSize(18.0);
//    glBegin(GL_POINTS );
//    glVertex3fv((GLfloat*) &vertex[0]);
//    glEnd();
//    }

    glTranslated(212,189,-513);
    glColor3f( 0., 1, 0.);
    glLineWidth(1.0);

   //int i=0;
  // for( int i=0; i<3; i++)
    for( int i=0; i<numPoly; i++)
   {
        if(elist[i].a != -1)
        {
        glBegin(GL_LINES);
        glVertex3f((GLfloat) myvertex[i].x,(GLfloat) myvertex[i].y,(GLfloat) myvertex[i].z);
        glVertex3f((GLfloat) myvertex[elist[i].a].x,(GLfloat) myvertex[elist[i].a].y,(GLfloat)myvertex[elist[i].a].z);
        glEnd();

      }
         if(elist[i].b != -1)
         {
        glBegin(GL_LINES);
        glVertex3f((GLfloat) myvertex[i].x,(GLfloat) myvertex[i].y,(GLfloat) myvertex[i].z);
        glVertex3f((GLfloat) myvertex[elist[i].b].x,(GLfloat)myvertex[elist[i].b].y,(GLfloat)myvertex[elist[i].b].z);
        glEnd();
         }

          if(elist[i].c != -1)
          {
        glBegin(GL_LINES);
        glVertex3f((GLfloat) myvertex[i].x,(GLfloat) myvertex[i].y,(GLfloat) myvertex[i].z);
        glVertex3f((GLfloat) myvertex[elist[i].c].x,(GLfloat)myvertex[elist[i].c].y,(GLfloat)myvertex[elist[i].c].z);
        glEnd();
          }
    }



//    glColor3f(1, 1., 1.);
//    glLineWidth(1.0);

//   glBegin(GL_LINES);
//   //int i=0;

//    for( int i=3; i<numPoly; i++)
//   {
//        glVertex3fv( (GLfloat*)&vertex[triangle[i].a]);
//        glVertex3fv( (GLfloat*)&vertex[triangle[i].b]);
//        glVertex3fv( (GLfloat*)&vertex[triangle[i].b]);
//        glVertex3fv( (GLfloat*)&vertex[triangle[i].c]);
//        glVertex3fv( (GLfloat*)&vertex[triangle[i].c]);
//        glVertex3fv( (GLfloat*)&vertex[triangle[i].a]);
//    }
//    glEnd();


    glColor3f( 1., .0, .0);
    glBegin(GL_LINES);
    //for( int i=0; i<numPoly; i++){
    /*for( int i=1000; i<1100; i++){
        TriMesh *s = &triangle[i];
        Vector3f cs = ( vertex[s->a] + vertex[s->b] + vertex[s->c] ) / Vector3f(3.0);
        glVertex3fv( (GLfloat*)&cs);
        //Vector3f cs2 = cs + .2;
        //glVertex3fv( (GLfloat*)&cs2);
        if( elist[i].a != -1 && elist[i].a < numPoly){
            TriMesh *d1 = &triangle[elist[i].a];
            Vector3f c1 = ( vertex[d1->a] + vertex[d1->b] + vertex[d1->c] ) / Vector3f(3.0);
            glVertex3fv( (GLfloat*)&cs);
            glVertex3fv( (GLfloat*)&c1);
        }
        if( elist[i].b != -1 && elist[i].b < numPoly){
            TriMesh *d2 = &triangle[elist[i].b];
            Vector3f c2 = ( vertex[d2->a] + vertex[d2->b] + vertex[d2->c] ) / Vector3f(3.0);
            glVertex3fv( (GLfloat*)&cs);
            glVertex3fv( (GLfloat*)&c2);
        }
        if( elist[i].c != -1 && elist[i].c < numPoly){
            TriMesh *d3 = &triangle[elist[i].c];
            Vector3f c3 = ( vertex[d3->a] + vertex[d3->b] + vertex[d3->c] ) / Vector3f(3.0);
            glVertex3fv( (GLfloat*)&cs);
            glVertex3fv( (GLfloat*)&c3);
        }
    }*/
    glEnd();
}

void Obj3dsGraph::lap()
{
    FILE *fout = fopen("out.txt", "w");

    for( int i=0; i<numPoly; i++)
                fprintf(fout,"%ld %ld %ld \n",elist[i].a , elist[i].b,  elist[i].c);
    clock_t ct = clock();
    clock_t ct2;
   // double td2=0.0;
    //int t=0;

    if( mapcoord == NULL)
        mapcoord = new Vector2f[numVert];

    for( int i=0; i<numVert; i++)
        mapcoord[i].x = mapcoord[i].y = 0;

    float tscale = .05;
    //float tscale = 1.0;
    int seed = 1000;

    char *flg = new char[numPoly];
    memset( flg, 1, numPoly);

    //std::queue<unsigned int> sq;
    //std::queue<unsigned int> tq;
    //std::queue<unsigned int> tqd;
   // tq.push(seed);
    //tqd.push(0);

    //while(!tq.empty()){
    {
       // unsigned int c = tq.front();
       // tq.pop();
      //  unsigned int cd = tqd.front();
       // tqd.pop();
       // if( !flg[c])
        //    ;//continue;
        unsigned int c=seed;
        flg[c] = 0;
        //if( cd%3 == 0)
        //{
         //   sq.push(c);
            seedt.push_back(c);
        //}
       // cd++;
       /* if( elist[c].a != -1 && elist[c].a < numPoly && flg[elist[c].a]){
            tq.push(elist[c].a);
            tqd.push(cd);
        }
        if( elist[c].b != -1 && elist[c].b < numPoly && flg[elist[c].b]){
            tq.push(elist[c].b);
            tqd.push(cd);
        }
        if( elist[c].c != -1 && elist[c].c < numPoly && flg[elist[c].c]){
            tq.push(elist[c].c);
            tqd.push(cd);
        }*/
    }

    int notTouched = 0;
    for( int i=0; i<numPoly; i++)
        if( flg[i])
            notTouched++;

    delete [] flg;
    flg = new char[numVert];
    memset( flg, 1, numVert);

    printf("mapping done, seeds = %d, untouched = %d\n", seedt.size(), notTouched);

    char *flag = new char[numVert];
    memset( flag, 1, numVert);

    Vector3f *tvertex = new Vector3f[numVert];
    int textureCount = 0;
    int hello=0;
    //for( int i=0; i<50; i++){
  //  while(!sq.empty())
    {
    //{
        //seed = rand()%numPoly;
        //seed = sq.front();
        //sq.pop();
        printf("the seed is %d\n",seed);
        TriMesh p = triangle[seed];
        //TriMesh pa, pb, pc;
        //pa = pb = pc = p;
        /*if( elist[seed].a != -1 && elist[seed].a < numPoly )
            pa = triangle[elist[seed].a];
        if( elist[seed].b != -1 && elist[seed].b < numPoly )
            pb = triangle[elist[seed].b];
        if( elist[seed].c != -1 && elist[seed].c < numPoly )
            pc = triangle[elist[seed].c];*/

        if( !flag[p.a] || !flag[p.b] || !flag[p.c])/* ||
                !flag[pa.a] || !flag[pa.b] || !flag[pa.c] ||
                !flag[pb.a] || !flag[pb.b] || !flag[pb.c] ||
                !flag[pc.a] || !flag[pc.b] || !flag[pc.c] )*/
            ;//continue;

        memcpy( tvertex, vertex, numVert*sizeof(Vector3f));
        std::queue<unsigned int> q;
        Vector3f nor = (vertex[p.a]-vertex[p.b])^(vertex[p.b]-vertex[p.c]);
        Vector3f mid = (vertex[p.a] + vertex[p.b] + vertex[p.c] ) / Vector3f(3.0);
       // q.push( seed);
        flag[p.a] = flag[p.b] = flag[p.c] = 0;
/*
       // for( int i=0; i<20 && (!q.empty()) && q.size() < 28; )
       // for(int i=0;(!q.empty());)
        {
            unsigned int cur = q.front();
            q.pop();
            printf("cur=%d\n",cur);
            TriMesh *t = &triangle[cur];
            char chk = flag[t->a] + flag[t->b] + flag[t->c];
            printf("chk=%d\n",chk);
            if( chk == 1)
            {
                printf("hhh\n");
                Vector3f nor1 = (vertex[t->a]-vertex[t->b])^(vertex[t->b]-vertex[t->c]);
                float th = acos( nor.normalize() & nor1.normalize());
                float ct = cos(th), rt = 1 - cos(th), st = sin(th);
                //printf("---1");
                unsigned int rp = (flag[t->a])?t->a:((flag[t->b])?t->b:t->c);
                unsigned int s1 = (t->a==rp)?t->b:t->a;
                unsigned int s2 = (t->c==rp)?t->b:t->c;
                Vector3f p1 = tvertex[rp];
                Vector3f l1 = tvertex[s1];
                Vector3f l2 = tvertex[s2];
                Vector3f u = (l2 - l1).normalize();
                //printf("---2");
                float rm[3][3] =
                {   {ct+u.x*u.x*rt,     u.x*u.y*rt-u.z*st,  u.x*u.z*rt+u.y*st},
                    {u.y*u.x*rt+u.z*st, ct+u.y*u.y*rt,      u.y*u.z*rt-u.x*st},
                    {u.z*u.x*rt-u.y*st, u.z*u.y*rt+u.x*st,  ct+u.z*u.z*rt}
                };
                float pm[3] = { p1.x - l1.x, p1.y - l1.y, p1.z - l1.z};
                float result[3] = {
                    pm[0]*rm[0][0]+pm[1]*rm[0][1]+pm[2]*rm[0][2],
                    pm[0]*rm[1][0]+pm[1]*rm[1][1]+pm[2]*rm[1][2],
                    pm[0]*rm[2][0]+pm[1]*rm[2][1]+pm[2]*rm[2][2]
                };
                //printf("---3");
                tvertex[rp].set( result[0]+l1.x, result[1]+l1.y, result[2]+l1.z );
                flag[t->a] = flag[t->b] = flag[t->c] = 0;
               // i++;
            }
            //printf("---4");
            if( elist[cur].a != -1 && elist[cur].a < numPoly)
                q.push(elist[cur].a);
            if( elist[cur].b != -1 && elist[cur].b < numPoly)
                q.push(elist[cur].b);
            if( elist[cur].c != -1 && elist[cur].c < numPoly)
                q.push(elist[cur].c);
            //printf("---5");
        }

        while( !q.empty())
            q.pop();
*/
float th;
Vector3f u;

        th = acos( nor.normalize() & Vector3f( 0, 0, 1)) ; //dot product



        float ct = cos(th), rt = 1 - cos(th), st = sin(th);

        u = (nor.normalize() ^ Vector3f( 0, 0, 1)).normalize();//cross product
       // u = (vertex[p.a] - vertex[p.b]).normalize();
        float ninety = acos(nor.normalize() & u);

        printf("theta = %f  costh = %f ninety = %f\n", th, ct,ninety);
       // u = Vector3f(0,0,1).normalize();

        float rm[3][3] =
        {   {ct+u.x*u.x*rt,     u.x*u.y*rt-u.z*st,  u.x*u.z*rt+u.y*st},
            {u.y*u.x*rt+u.z*st, ct+u.y*u.y*rt,      u.y*u.z*rt-u.x*st},
            {u.z*u.x*rt-u.y*st, u.z*u.y*rt+u.x*st,  ct+u.z*u.z*rt}
        };



//                      th = acos( nor.normalize() & Vector3f( 0, 0, 1));



//                  u = (nor.normalize() ^ Vector3f( 0, 0, 1)).normalize();



//            ct = cos(th), rt = 1 - cos(th), st = sin(th);




//            float rm1[3][3] =
//            {   {ct+u.x*u.x*rt,     u.x*u.y*rt-u.z*st,  u.x*u.z*rt-u.y*st},
//                {u.y*u.x*rt-u.z*st, ct+u.y*u.y*rt,      u.y*u.z*rt-u.x*st},
//                {u.z*u.x*rt-u.y*st, u.z*u.y*rt-u.x*st,  ct+u.z*u.z*rt}
//            };//flattening matrix



            float result[3];
       // int k=numVert/4;
           //
           // flg[seed]=0;
          //  for( int i=0; i<3; i++)
            for(int i=0;i<numVert;i++)
            flg[i]=1;
          //  int j=1;
            FILE *fileout = fopen("mapcoords.txt", "w");

            /*

     for( int i=0; i<numVert; i++)

       {
         //printf("%d\n",numVert);
        // j=0;
//         for(int k=0;k<numVert;k++)
//            { if(flg[k]==1)
//                 j=1;}

            if(flg[i])
            {
                //TriMesh p1 = triangle[i];
                Vector3f p1 = tvertex[i];

               // float th = acos(nor1.normalize() & nor.normalize());
               // float ct = cos(th), rt = 1 - cos(th), st = sin(th);

               // Vector3f u = (vertex[p.a] - vertex[p.b]).normalize();
                float pm[3] = { p1.x - mid.x, p1.y - mid.y, p1.z - mid.z};
               // float pm[3] = { p1.a, p1.b, p1.c};
//                float rm[3][3] =
//                {   {ct+u.x*u.x*rt,     u.x*u.y*rt-u.z*st,  u.x*u.z*rt+u.y*st},
//                    {u.y*u.x*rt+u.z*st, ct+u.y*u.y*rt,      u.y*u.z*rt-u.x*st},
//                    {u.z*u.x*rt-u.y*st, u.z*u.y*rt+u.x*st,  ct+u.z*u.z*rt}
//                };
               // if(i<=k || (i>=2*k && i< 3*k))
                //{
                    result[0]=pm[0]*rm[0][0]+pm[1]*rm[0][1]+pm[2]*rm[0][2];
                    result[1]=pm[0]*rm[1][0]+pm[1]*rm[1][1]+pm[2]*rm[1][2];
                    result[2]=pm[0]*rm[2][0]+pm[1]*rm[2][1]+pm[2]*rm[2][2];
               // }



                //flattening operation
//                result[0]=pm[0]*rm[0][0]+pm[1]*rm[1][0]+pm[2]*rm[2][0];
//                result[1]=pm[0]*rm[0][7]+pm[1]*rm[1][1]+pm[2]*rm[2][1];
//                result[2]=pm[0]*rm[0][2]+pm[1]*rm[1][2]+pm[2]*rm[2][2];


//                float result[3] = {
//                    pm[0]*rm[0][0]+pm[1]*rm[0][1]+pm[2]*rm[0][2],
//                    pm[0]*rm[1][0]+pm[1]*rm[1][1]+pm[2]*rm[1][2],
//                    pm[0]*rm[2][0]+pm[1]*rm[2][1]+pm[2]*rm[2][2]
//                };

              //  else
//                {
//                    result[0]=pm[0]*rm1[0][0]+pm[1]*rm1[0][1]+pm[2]*rm1[0][2];
//                    result[1]=pm[0]*rm1[1][0]+pm[1]*rm1[1][1]+pm[2]*rm1[1][2];
//                    result[2]=pm[0]*rm1[2][0]+pm[1]*rm1[2][1]+pm[2]*rm1[2][2];
//                }

//                  float  result[3] = {
//                        pm[0]*rm1[0][0]+pm[1]*rm1[0][1]+pm[2]*rm1[0][2],
//                        pm[0]*rm1[1][0]+pm[1]*rm1[1][1]+pm[2]*rm1[1][2],
//                        pm[0]*rm1[2][0]+pm[1]*rm1[2][1]+pm[2]*rm1[2][2]
//                    };

                //float mapBaseX = textureCount%1400;
                //float mapBaseY = (textureCount/1400)%14;
                tvertex[i].set( result[0]+mid.x, result[1]+mid.y, result[2]+mid.z );
                // printf("%d  %f %f %f \n",i,vertex[i].x,vertex[i].y,vertex[i].z);
                //printf("%d  %f %f %f \n",i,tvertex[i].x,tvertex[i].y,tvertex[i].z);
               // tvertex[i].set( result[0], result[1], result[2] );
//                myvertex[triangle[i].a]= result[0];
//                myvertex[triangle[i].b]= result[1];
//                myvertex[triangle[i].c]= result[2];
                if(i<9)
                printf("%d  %f %f %f \n",i,tvertex[i].x,tvertex[i].y,tvertex[i].z);
               // tvertex[i].z=0;

                Vector3f aaa= tvertex[i]-mid;

                    float hee = acos(aaa.normalize() & nor.normalize());
                     if(i<3)
             printf("hee = %f \n",hee);

                //mapcoord[i].x = ((tvertex[i].x - mid.x)*tscale + .5)/14.0 + mapBaseX/13.0;
                //mapcoord[i].y = ((tvertex[i].y - mid.y)*tscale + .5)/14.0 + mapBaseY/13.0;
                mapcoord[i].x = (tvertex[i].x )*tscale ;//+ 2.52 ;//+ mapBaseX/13.0;
                mapcoord[i].y = (tvertex[i].y )*tscale;//+ 2.52 ;//+ mapBaseY/13.0;
               // if(i<300)



                fprintf(fileout,"%d %f %f  \n\n",i,mapcoord[i].x,mapcoord[i].y);
                flg[i] = 0;
            }

          // t++;
       }

     */
            int test,kk;
            kk=seed;
            std::queue<unsigned int> q1;


            for( int i=0; i<2000; i++)

              {
                //printf("%d\n",numVert);
               // j=0;
       //         for(int k=0;k<numVert;k++)
       //            { if(flg[k]==1)
       //                 j=1;}
                q1.push(elist[kk].a);
                 q1.push(elist[kk].b);
                  q1.push(elist[kk].c);

                  test = q1.front();
                  q1.pop();

                  TriMesh mm = triangle[test];
                  unsigned long kl[3]= {mm.a,mm.b,mm.c};
                  for(int j=0;j<3;j++)
                  {
                      if(flg[kl[j]])
                   {

                       Vector3f p1 = tvertex[kl[j]];


                       float pm[3] = { p1.x - mid.x, p1.y - mid.y, p1.z - mid.z};


                           result[0]=pm[0]*rm[0][0]+pm[1]*rm[0][1]+pm[2]*rm[0][2];
                           result[1]=pm[0]*rm[1][0]+pm[1]*rm[1][1]+pm[2]*rm[1][2];
                           result[2]=pm[0]*rm[2][0]+pm[1]*rm[2][1]+pm[2]*rm[2][2];

                       tvertex[kl[j]].set( result[0]+mid.x, result[1]+mid.y, result[2]+mid.z );


                       Vector3f aaa= tvertex[kl[j]]-mid;

                           float hee = acos(aaa.normalize() & nor.normalize());


                       //mapcoord[i].x = ((tvertex[i].x - mid.x)*tscale + .5)/14.0 + mapBaseX/13.0;
                       //mapcoord[i].y = ((tvertex[i].y - mid.y)*tscale + .5)/14.0 + mapBaseY/13.0;
                       mapcoord[kl[j]].x = (tvertex[kl[j]].x )*.1 ;//+ 2.52 ;//+ mapBaseX/13.0;
                       mapcoord[kl[j]].y = (tvertex[kl[j]].y )*.1;//+ 2.52 ;//+ mapBaseY/13.0;
                      // if(i<300)



                       fprintf(fileout,"%d %f %f  \n\n",i,mapcoord[kl[j]].x,mapcoord[kl[j]].y);
                       flg[kl[j]] = 0;

                   }


                  }

                   kk=test;


              }

               //}
       //td2 += (((double) (clock() - ct2)) / CLOCKS_PER_SEC);
        textureCount++;
    }
    delete [] tvertex;

    /*FILE *fout = fopen("out.off", "w");
    fprintf( fout, "OFF\n");
    fprintf( fout, "%d %d 0\n", numVert, numPoly);
    for( int i=0; i<numVert; i++)
        fprintf( fout, "%f %f %f\n", tvertex[i].x, tvertex[i].y, tvertex[i].z);
    for( int i=0; i<numPoly; i++)
        fprintf( fout, "3 %d %d %d\n", triangle[i].a, triangle[i].b, triangle[i].c);
    fclose(fout);*/

    delete [] flag;
    delete [] flg;

    double td = ((double) (clock() - ct)) / CLOCKS_PER_SEC;

    printf("lapping done, texture Count = %d (%f sec)\n", textureCount, td);
   // printf("lapping done, texture Count = %d (%f sec)\n", t, td2);
}

/*void Obj3dsGraph::lap(){
    for( int i=0; i<numVert; i++)
        mapcoord[i].x = mapcoord[i].y = 0;

    float tscale = .2;
    int seed = 1000;
    char *flag = new char[numVert];
    memset( flag, 1, numVert);

    for( int i=0; i<50; i++){
        Vector3f *tvertex = new Vector3f[numVert];
        memcpy( tvertex, vertex, numVert*sizeof(Vector3f));
        seed = rand()%numPoly;
        std::queue<unsigned int> q;
        TriMesh p = triangle[seed];
        Vector3f nor = (vertex[p.a]-vertex[p.b])^(vertex[p.b]-vertex[p.c]);
        Vector3f mid = (vertex[p.a] + vertex[p.b] + vertex[p.c] ) / Vector3f(3.0);
        q.push( seed);
        flag[p.a] = flag[p.b] = flag[p.c] = 0;

        for( int i=0; i<10 && (!q.empty()); ){
            unsigned int cur = q.front();
            q.pop();
            TriMesh *t = &triangle[cur];
            char chk = flag[t->a] + flag[t->b] + flag[t->c];
            if( chk == 1){
                Vector3f nor1 = (vertex[t->a]-vertex[t->b])^(vertex[t->b]-vertex[t->c]);
                float th = acos( nor.normalize() & nor1.normalize());
                float ct = cos(th), rt = 1 - cos(th), st = sin(th);
                //printf("---1");
                unsigned int rp = (flag[t->a])?t->a:((flag[t->b])?t->b:t->c);
                unsigned int s1 = (t->a==rp)?t->b:t->a;
                unsigned int s2 = (t->c==rp)?t->b:t->c;
                Vector3f p1 = tvertex[rp];
                Vector3f l1 = tvertex[s1];
                Vector3f l2 = tvertex[s2];
                Vector3f u = (l2 - l1).normalize();
                //printf("---2");
                float rm[3][3] =
                {   {ct+u.x*u.x*rt,     u.x*u.y*rt-u.z*st,  u.x*u.z*rt+u.y*st},
                    {u.y*u.x*rt+u.z*st, ct+u.y*u.y*rt,      u.y*u.z*rt-u.x*st},
                    {u.z*u.x*rt-u.y*st, u.z*u.y*rt+u.x*st,  ct+u.z*u.z*rt}
                };
                float pm[3] = { p1.x - l1.x, p1.y - l1.y, p1.z - l1.z};
                float result[3] = {
                    pm[0]*rm[0][0]+pm[1]*rm[0][1]+pm[2]*rm[0][2],
                    pm[0]*rm[1][0]+pm[1]*rm[1][1]+pm[2]*rm[1][2],
                    pm[0]*rm[2][0]+pm[1]*rm[2][1]+pm[2]*rm[2][2]
                };
                //printf("---3");
                tvertex[rp].set( result[0]+l1.x, result[1]+l1.y, result[2]+l1.z );
                flag[t->a] = flag[t->b] = flag[t->c] = 0;
                i++;
            }
            //printf("---4");
            if( elist[cur].a != -1 && elist[cur].a < numPoly)
                q.push(elist[cur].a);
            if( elist[cur].b != -1 && elist[cur].b < numPoly)
                q.push(elist[cur].b);
            if( elist[cur].c != -1 && elist[cur].c < numPoly)
                q.push(elist[cur].c);
            //printf("---5");
        }

        float th = acos( nor.normalize() & Vector3f( 0, 0, 1));
        float ct = cos(th), rt = 1 - cos(th), st = sin(th);
        Vector3f u = (nor.normalize() ^ Vector3f( 0, 0, 1)).normalize();
        float rm[3][3] =
        {   {ct+u.x*u.x*rt,     u.x*u.y*rt-u.z*st,  u.x*u.z*rt+u.y*st},
            {u.y*u.x*rt+u.z*st, ct+u.y*u.y*rt,      u.y*u.z*rt-u.x*st},
            {u.z*u.x*rt-u.y*st, u.z*u.y*rt+u.x*st,  ct+u.z*u.z*rt}
        };
        for( int i=0; i<numVert; i++)
            if( !flag[i]){
                Vector3f p1 = tvertex[i];
                float pm[3] = { p1.x - mid.x, p1.y - mid.y, p1.z - mid.z};
                float result[3] = {
                    pm[0]*rm[0][0]+pm[1]*rm[0][1]+pm[2]*rm[0][2],
                    pm[0]*rm[1][0]+pm[1]*rm[1][1]+pm[2]*rm[1][2],
                    pm[0]*rm[2][0]+pm[1]*rm[2][1]+pm[2]*rm[2][2]
                };
                tvertex[i].set( result[0]+mid.x, result[1]+mid.y, result[2]+mid.z );
                mapcoord[i].x = (tvertex[i].x - mid.x)*tscale + .5;
                mapcoord[i].y = (tvertex[i].y - mid.y)*tscale + .5;
            }

        delete [] tvertex;
    }

    delete [] flag;
}*/

//-----------------------------------------------------------------------------
//								ObjBIN
//-----------------------------------------------------------------------------

ObjBIN::ObjBIN(){
    objType = 11;
    numVert = numPoly = 0;
    vertex = NULL;
    normal = NULL;
    triangle = NULL;
    mapcoord = NULL;
    smooth = true;
}

bool ObjBIN::LoadBIN( const char *filename){
    std::fstream fin;
    fin.open( filename, std::fstream::in|std::fstream::binary);
    if (  !fin.is_open()) {
        printf("error loading file '%s'\n", filename);
        return false;			// Unable to open the file
    }
    printf("BIN (src:%s)\n", filename);

    unsigned int uint32v;
    fin.read( (char*)&uint32v, sizeof(unsigned int));
    numVert = uint32v;
    fin.read( (char*)&uint32v, sizeof(unsigned int));
    numPoly = uint32v;
    printf( " + -- Number of vertices: %d\n", numVert);
    printf( " + -- Number of polygons: %d\n", numPoly);
    vertex = new Vector3f[ numVert];
    triangle = new TriMesh[ numPoly];
    fin.read( (char*)vertex, numVert*sizeof(Vector3f));
    fin.read( (char*)triangle, numPoly*sizeof(TriMesh));
    fin.close();

    calcNormals();
    calcBoundingCube();
    return true;
}

bool ObjBIN::saveBIN( const char *filename){
    std::fstream fin;
    fin.open( filename, std::fstream::out|std::fstream::binary);
    if (  !fin.is_open()) {
        printf("error loading file '%s'\n", filename);
        return false;			// Unable to open the file
    }

    unsigned int uint32v = numVert;
    fin.write( (char*)&uint32v, sizeof(unsigned int));
    uint32v = numPoly;
    fin.write( (char*)&uint32v, sizeof(unsigned int));
    fin.write( (char*)vertex, numVert*sizeof(Vector3f));
    fin.write( (char*)triangle, numPoly*sizeof(TriMesh));
    if( mapcoord != NULL)
        fin.write( (char*)mapcoord, numVert*sizeof(Vector2f));
    fin.close();

    printf("BIN (src:%s)new saved.\n", filename);
    return true;
}

void ObjBIN::calcNormals(){
    if( normal != NULL)
        delete [] normal;
    normal = new Vector3f[numVert];
    float* neigh = new float[numVert];
    for (int i=0; i<numVert; i++){
        neigh[i] = 0;
        normal[i].set (0., 0., 0.);
    }
    for ( int i=0; i<numPoly; i++){
        TriMesh p = triangle[i];
        Vector3f v12 = vertex[p.b]-vertex[p.a];
        Vector3f v23 = vertex[p.c]-vertex[p.b];
        //Vector3f v31 = vertex[p.a]-vertex[p.c];
        Vector3f nor = v12^v23;
        float weight = nor.magnitude();
        normal[p.a] += nor*weight; neigh[p.a]+=weight;
        normal[p.b] += nor*weight; neigh[p.b]+=weight;
        normal[p.c] += nor*weight; neigh[p.c]+=weight;
    }
    for (int i=0; i<numVert; i++)
        if( neigh[i] > 0.1)
            normal[i] = normal[i]/double(neigh[i]);
    delete [] neigh;
}

void ObjBIN::calcBoundingCube(){
    cubeBoundMin.set( OBJINFINITY, OBJINFINITY, OBJINFINITY);
    cubeBoundMax.set( -OBJINFINITY, -OBJINFINITY, -OBJINFINITY);
    for( int i=0; i<numVert; i++){
        if( vertex[i].x < cubeBoundMin.x)
            cubeBoundMin.x = vertex[i].x;
        else if( vertex[i].x > cubeBoundMax.x)
            cubeBoundMax.x = vertex[i].x;
        if( vertex[i].y < cubeBoundMin.y)
            cubeBoundMin.y = vertex[i].y;
        else if( vertex[i].y > cubeBoundMax.y)
            cubeBoundMax.y = vertex[i].y;
        if( vertex[i].z < cubeBoundMin.z)
            cubeBoundMin.z = vertex[i].z;
        else if( vertex[i].z > cubeBoundMax.z)
            cubeBoundMax.z = vertex[i].z;
    }
}

ObjBIN& ObjBIN::operator = ( const ObjBIN &c){
    operatorAssign( c);
    this->numPoly = c.numPoly;
    this->numVert = c.numVert;
    this->triangle = c.triangle;
    if( this->vertex != NULL)
        delete [] this->vertex;
    this->vertex = new Vector3f[c.numVert];
    for( int i=0; i<c.numVert; i++)
        this->vertex[i] = c.vertex[i];
    this->normal = new Vector3f[c.numVert];
    this->calcNormals ();
    return *this;
}

ObjBIN& ObjBIN::operator += (const Vector3f &d){
    for( int i=0; i<numVert; i++)
        vertex[i] += d;
    cubeBoundMax += d;
    cubeBoundMin += d;
    calcNormals();
    return *this;
}

void ObjBIN::scale( const Vector3f &d){
    Vector3f mid = (cubeBoundMax - cubeBoundMin)/2.0;
    cubeBoundMin.set( OBJINFINITY, OBJINFINITY, OBJINFINITY);
    cubeBoundMax.set( -OBJINFINITY, -OBJINFINITY, -OBJINFINITY);
    for( int i=0; i<numVert; i++){
        Vector3f tmp = vertex[i] - mid;
        tmp *= d;
        vertex[i] = tmp + mid;
        if( vertex[i].x < cubeBoundMin.x)
            cubeBoundMin.x = vertex[i].x;
        else if( vertex[i].x > cubeBoundMax.x)
            cubeBoundMax.x = vertex[i].x;
        if( vertex[i].y < cubeBoundMin.y)
            cubeBoundMin.y = vertex[i].y;
        else if( vertex[i].y > cubeBoundMax.y)
            cubeBoundMax.y = vertex[i].y;
        if( vertex[i].z < cubeBoundMin.z)
            cubeBoundMin.z = vertex[i].z;
        else if( vertex[i].z > cubeBoundMax.z)
            cubeBoundMax.z = vertex[i].z;
    }
    calcNormals();
}

void ObjBIN::rotate( float xy, float yz, float zx){
    xy *= PI/180.0;
    yz *= PI/180.0;
    zx *= PI/180.0;
    factorRot += Vector3f( xy, yz, zx);
    Vector3f mid = (cubeBoundMax - cubeBoundMin)/2.0;
    cubeBoundMin.set( OBJINFINITY, OBJINFINITY, OBJINFINITY);
    cubeBoundMax.set( -OBJINFINITY, -OBJINFINITY, -OBJINFINITY);
    for( int i=0; i<numVert; i++){
        Vector3f tmp = (vertex[i] - mid).rotate( xy, yz, zx);
        vertex[i] = tmp + mid;
        if( vertex[i].x < cubeBoundMin.x)
            cubeBoundMin.x = vertex[i].x;
        else if( vertex[i].x > cubeBoundMax.x)
            cubeBoundMax.x = vertex[i].x;
        if( vertex[i].y < cubeBoundMin.y)
            cubeBoundMin.y = vertex[i].y;
        else if( vertex[i].y > cubeBoundMax.y)
            cubeBoundMax.y = vertex[i].y;
        if( vertex[i].z < cubeBoundMin.z)
            cubeBoundMin.z = vertex[i].z;
        else if( vertex[i].z > cubeBoundMax.z)
            cubeBoundMax.z = vertex[i].z;
    }
    calcNormals();
}

void ObjBIN::draw(){
    numPolygons +=numPoly;
    if( vertex == NULL )
        return;
    if( USE_DISP_LISTS && dispList != -1){
        glCallList( dispList);
        return;
    }
    if( RENDER_TEXTURES && tex != NULL){
        glEnable( GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texEnvParam);
        glBindTexture( GL_TEXTURE_2D, tex->id);
        glColor3fv( (GLfloat*)&texCol);
    }
    else{
        glColor3fv( (GLfloat*)&col);
        if( mat != NULL){
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat*)&(mat->ambi));
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&(mat->diff));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat*)&(mat->spec));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat*)&(mat->shin));
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (GLfloat*)&(mat->emis));
        }
    }
    if( smooth){
        glEnableClientState( GL_VERTEX_ARRAY);
        glVertexPointer( 3, GL_FLOAT, 0, (GLfloat*)vertex);
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer( GL_FLOAT, 0, (GLfloat*)normal);
        if( RENDER_TEXTURES && tex != NULL){
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer( 2, GL_FLOAT, 0, (GLfloat*)mapcoord);
        }
        glDrawElements( GL_TRIANGLES, numPoly * 3, GL_UNSIGNED_INT, triangle);
        if( RENDER_TEXTURES && tex != NULL)
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    else{
        printf("test test\n");
        glBegin(GL_TRIANGLES);
        for( int i=0; i<numPoly; i++){
            TriMesh p = triangle[i];
            Vector3f nor = (vertex[p.a]-vertex[p.b])^(vertex[p.b]-vertex[p.c]);
            glNormal3fv( (GLfloat*)&nor);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2fv( (GLfloat*)&mapcoord[p.a]);
            glVertex3fv( (GLfloat*)&vertex[p.a]);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2fv( (GLfloat*)&mapcoord[p.b]);
            glVertex3fv( (GLfloat*)&vertex[p.b]);
            if( RENDER_TEXTURES && tex != NULL)
                glTexCoord2fv( (GLfloat*)&mapcoord[p.c]);
            glVertex3fv( (GLfloat*)&vertex[p.c]);
        }
        glEnd();
    }
    if( DRAW_BOUNDS)
        drawBounds();
    if( RENDER_TEXTURES && tex != NULL)
        glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------
//				RENDERING OF OBJECTS
//-----------------------------------------------------------------------------

static void matrixConcatenate (float *result, float *ma, float *mb)
{
    int i;
    double mb00, mb01, mb02, mb03,
           mb10, mb11, mb12, mb13,
           mb20, mb21, mb22, mb23,
           mb30, mb31, mb32, mb33;
    double mai0, mai1, mai2, mai3;

    mb00 = mb[0];  mb01 = mb[1];
    mb02 = mb[2];  mb03 = mb[3];
    mb10 = mb[4];  mb11 = mb[5];
    mb12 = mb[6];  mb13 = mb[7];
    mb20 = mb[8];  mb21 = mb[9];
    mb22 = mb[10];  mb23 = mb[11];
    mb30 = mb[12];  mb31 = mb[13];
    mb32 = mb[14];  mb33 = mb[15];

    for (i = 0; i < 4; i++) {
        mai0 = ma[i*4+0];  mai1 = ma[i*4+1];
        mai2 = ma[i*4+2];  mai3 = ma[i*4+3];

        result[i*4+0] = mai0 * mb00 + mai1 * mb10 + mai2 * mb20 + mai3 * mb30;
        result[i*4+1] = mai0 * mb01 + mai1 * mb11 + mai2 * mb21 + mai3 * mb31;
        result[i*4+2] = mai0 * mb02 + mai1 * mb12 + mai2 * mb22 + mai3 * mb32;
        result[i*4+3] = mai0 * mb03 + mai1 * mb13 + mai2 * mb23 + mai3 * mb33;
    }
}

#define vectorLength(nin) \
    sqrt((nin)[0]*(nin)[0] + (nin)[1]*(nin)[1] + (nin)[2]*(nin)[2])
#define distanceFromPlane(peq,p) \
    ((peq)[0]*(p.x) + (peq)[1]*(p.y) + (peq)[2]*(p.z) + (peq)[3])

static float planeEqs[6][4];

static void calcViewVolumePlanes ()
{
    GLfloat ocEcMat[16], ecCcMat[16], ocCcMat[16];


    /* Get the modelview and projection matrices */
    glGetFloatv (GL_MODELVIEW_MATRIX, ocEcMat);
    glGetFloatv (GL_PROJECTION_MATRIX, ecCcMat);

    /* ocCcMat transforms from OC (object coordinates) to CC (clip coordinates) */
    matrixConcatenate (ocCcMat, ocEcMat, ecCcMat);

    /* Calculate the six OC plane equations. */
    planeEqs[0][0] = ocCcMat[3] - ocCcMat[0];
    planeEqs[0][1] = ocCcMat[7] - ocCcMat[4];
    planeEqs[0][2] = ocCcMat[11] - ocCcMat[8];
    planeEqs[0][3] = ocCcMat[15] - ocCcMat[12];

    planeEqs[1][0] = ocCcMat[3] + ocCcMat[0];
    planeEqs[1][1] = ocCcMat[7] + ocCcMat[4];
    planeEqs[1][2] = ocCcMat[11] + ocCcMat[8];
    planeEqs[1][3] = ocCcMat[15] + ocCcMat[12];

    planeEqs[2][0] = ocCcMat[3] + ocCcMat[1];
    planeEqs[2][1] = ocCcMat[7] + ocCcMat[5];
    planeEqs[2][2] = ocCcMat[11] + ocCcMat[9];
    planeEqs[2][3] = ocCcMat[15] + ocCcMat[13];

    planeEqs[3][0] = ocCcMat[3] - ocCcMat[1];
    planeEqs[3][1] = ocCcMat[7] - ocCcMat[5];
    planeEqs[3][2] = ocCcMat[11] - ocCcMat[9];
    planeEqs[3][3] = ocCcMat[15] - ocCcMat[13];

    planeEqs[4][0] = ocCcMat[3] + ocCcMat[2];
    planeEqs[4][1] = ocCcMat[7] + ocCcMat[6];
    planeEqs[4][2] = ocCcMat[11] + ocCcMat[10];
    planeEqs[4][3] = ocCcMat[15] + ocCcMat[14];

    planeEqs[5][0] = ocCcMat[3] - ocCcMat[2];
    planeEqs[5][1] = ocCcMat[7] - ocCcMat[6];
    planeEqs[5][2] = ocCcMat[11] - ocCcMat[10];
    planeEqs[5][3] = ocCcMat[15] - ocCcMat[14];
}

float* calcPlaneEqn(float* p1,float* p2,float* p3){
    //Array for the plane equation
    float* plane = new float[4];

    //Given two vectors (three points) in the plane
    //the normal can be calculated
    plane[0] = ((p2[1]-p1[1])*(p3[2]-p1[2]))-
                             ((p2[2]-p1[2])*(p3[1]-p1[1]));
    plane[1] = ((p2[2]-p1[2])*(p3[0]-p1[0]))-
                             ((p2[0]-p1[0])*(p3[2]-p1[2]));
    plane[2] = ((p2[0]-p1[0])*(p3[1]-p1[1]))-
                             ((p2[1]-p1[1])*(p3[0]-p1[0]));
    plane[3] = -(plane[0]*p1[0] + plane[1]*p1[1] + plane[2]*p1[2]);

    return plane;
}

float* shadowMatrix( float* plane, float* light_pos) {
       float* shadow_mat = new float[16];
       float dot;

       //Finds dot, which is the product of the light vetcor and the plane's normal
       dot = plane[0] * light_pos[0] +
        plane[1] * light_pos[1] +
        plane[2] * light_pos[2] +
        plane[3] * light_pos[3];

       shadow_mat[0]  = dot - light_pos[0] * plane[0];
       shadow_mat[4]  =		- light_pos[0] * plane[1];
       shadow_mat[8]  =		- light_pos[0] * plane[2];
       shadow_mat[12] =		- light_pos[0] * plane[3];

       shadow_mat[1]  =		- light_pos[1] * plane[0];
       shadow_mat[5]  = dot - light_pos[1] * plane[1];
       shadow_mat[9]  =		- light_pos[1] * plane[2];
       shadow_mat[13] =		- light_pos[1] * plane[3];

       shadow_mat[2]  =		- light_pos[2] * plane[0];
       shadow_mat[6]  =		- light_pos[2] * plane[1];
       shadow_mat[10] = dot - light_pos[2] * plane[2];
       shadow_mat[14] =		- light_pos[2] * plane[3];

       shadow_mat[3]  =		- light_pos[3] * plane[0];
       shadow_mat[7]  =		- light_pos[3] * plane[1];
       shadow_mat[11] =		- light_pos[3] * plane[2];
       shadow_mat[15] = dot - light_pos[3] * plane[3];

       return shadow_mat;
}

void drawObjectsWithLights(){
    if( IS_LIGHTING){
        glEnable( GL_LIGHTING);
        glEnable( GL_COLOR_MATERIAL);
        GLfloat ambientLight[] = { .0, .0, .0};
        glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambientLight);
        glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        int l = GL_LIGHT0;
        for( std::list<Light3d*>::iterator i=listLight3d.begin(); i!=listLight3d.end(); i++){
            if( (**i).isOn){
                glEnable( l);
                (**i).runGLcmd( l);
            }
            else
                glDisable( l);
            l++;
        }
    }
    else{
        glDisable( GL_LIGHTING);
        glDisable( GL_COLOR_MATERIAL);
    }

    calcViewVolumePlanes ();
    for( std::list<Object3d*>::iterator i=listObject3d.begin(); i!=listObject3d.end(); i++)
        if( (**i).isHidden == false ){
            bool show = true;
            Vector3f cu[8];
            cu[0].set( (**i).cubeBoundMin.x, (**i).cubeBoundMin.y, (**i).cubeBoundMin.z);
            cu[1].set( (**i).cubeBoundMax.x, (**i).cubeBoundMin.y, (**i).cubeBoundMin.z);
            cu[2].set( (**i).cubeBoundMax.x, (**i).cubeBoundMin.y, (**i).cubeBoundMax.z);
            cu[3].set( (**i).cubeBoundMin.x, (**i).cubeBoundMin.y, (**i).cubeBoundMax.z);
            cu[4].set( (**i).cubeBoundMin.x, (**i).cubeBoundMax.y, (**i).cubeBoundMin.z);
            cu[5].set( (**i).cubeBoundMax.x, (**i).cubeBoundMax.y, (**i).cubeBoundMin.z);
            cu[6].set( (**i).cubeBoundMax.x, (**i).cubeBoundMax.y, (**i).cubeBoundMax.z);
            cu[7].set( (**i).cubeBoundMin.x, (**i).cubeBoundMax.y, (**i).cubeBoundMax.z);
            for ( int j=0; j<6; j++){
                int cnt=0;
                for( int k=0; k<8; k++)
                    if( distanceFromPlane(planeEqs[j], cu[k]) < 0)
                        cnt++;
                if(cnt==8){
                    show=false;
                    break;
                }
            }
            if( show)
                (**i).draw();
        }
}

void render3dObjects(){
    numPolygons = 0;

    /*for( std::list<Object3d*>::iterator i=listObject3d.begin(); i!=listObject3d.end(); i++)
        if( (**i).isHidden == false && (**i).envMapping && (**i).tex != NULL){
            GLint fbo;
            glGenFramebuffersEXT(1, &fbo);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, (**i).tex->id, 0);

            glMatrixMode( GL_PROJECTION);
            gluPerspective( 80, (**i).tex->w/(**i).tex->h, activeCamera.near, activeCamera.far);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            Vector3f cen = ((**i).cubeBoundMax+(**i).cubeBoundMin)/2.0;
            gluLookAt(  cen.x, cen.y, cen.z,
                        activeCamera.pos.x, activeCamera.pos.y, activeCamera.pos.z,
                        activeCamera.ori.x, activeCamera.ori.y, activeCamera.ori.z);
            (**i).isHidden = true;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawObjectsWithLights();
            (**i).isHidden = false;
            glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0);
            glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0);
        }*/

    glMatrixMode( GL_PROJECTION);
    glLoadIdentity();
    if( activeCamera.isOrtho){
        if( activeCamera.ar<1.)
            glOrtho( activeCamera.orthoMin.x, activeCamera.orthoMax.x,
                     activeCamera.orthoMin.y/activeCamera.ar, activeCamera.orthoMax.y/activeCamera.ar,
                     activeCamera.orthoMin.z, activeCamera.orthoMax.z);
        else
            glOrtho( activeCamera.orthoMin.x*activeCamera.ar, activeCamera.orthoMax.x*activeCamera.ar,
                     activeCamera.orthoMin.y, activeCamera.orthoMax.y,
                     activeCamera.orthoMin.z, activeCamera.orthoMax.z);
    }
    else
        gluPerspective( activeCamera.ang, activeCamera.ar, activeCamera.cnear, activeCamera.cfar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if( USE_QUATERNION){
        QMatrix4x4 view;
        view.rotate(currentTrackBall.rotation());
        view(2, 3) -= 2.0f * exp( 600 / 1200.0f);
        QMatrix4x4 viewRotation(view);
        viewRotation(3, 0) = viewRotation(3, 1) = viewRotation(3, 2) = 0.0f;
        viewRotation(0, 3) = viewRotation(1, 3) = viewRotation(2, 3) = 0.0f;
        viewRotation(3, 3) = 1.0f;
        static GLfloat mat[16];
        const qreal *data = viewRotation.constData();
        for (int index = 0; index < 16; ++index)
            mat[index] = data[index];
        glTranslatef( 0., 0., -activeCamera.pos.x);
        glMultMatrixf( mat);
    }
    else
        gluLookAt( activeCamera.pos.x, activeCamera.pos.y, activeCamera.pos.z,
                activeCamera.eye.x, activeCamera.eye.y, activeCamera.eye.z,
                activeCamera.ori.x, activeCamera.ori.y, activeCamera.ori.z);

    if( SOFT_SHADOW){

        glClear(GL_ACCUM_BUFFER_BIT);
        for( int ssy = 0; ssy<SOFT_SHADOW; ssy++)
        for( int ssx = 0; ssx<SOFT_SHADOW; ssx++){
            for( std::list<Light3d*>::iterator li=listLight3d.begin(); li!=listLight3d.end(); li++){
                // Clear the window with current clearing color
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glPushMatrix();
                glDisable( GL_LIGHTING);
                if( (**li).isOn && (**li).castShadow){
                    float p1[] = { -100, .1,   0 };
                    float p2[] = {  100, .1,   0 };
                    float p3[] = {  100, .1, 500 };
                    float *floor = calcPlaneEqn( p3, p2, p1);//= { 0., 1., 0. , -0.1};
                    Vector4f pos = (**li).pos;
                    if( pos.u > .1){
                        pos.x += .5-(float)(ssx)/(float)(SOFT_SHADOW);
                        pos.z += .5-(float)(ssy)/(float)(SOFT_SHADOW);
                    }
                    else{
                        pos.x += .02*(.5-(float)(ssx)/(float)(SOFT_SHADOW));
                        pos.z += .02*(.5-(float)(ssy)/(float)(SOFT_SHADOW));
                    }
                    float* shadow = shadowMatrix( floor, (float*)&pos);
                    glMultMatrixf(shadow);
                    delete [] shadow;
                    for( std::list<Object3d*>::iterator i=listObject3d.begin(); i!=listObject3d.end(); i++)
                        if( (**i).isHidden == false && (**i).castShadow){
                                int os = RENDER_TEXTURES;
                                RENDER_TEXTURES = false;
                                Vector3f c = (**i).col;
                                (**i).col.set( 0., 0., 0.);
                                USE_DISP_LISTS = false;
                                (**i).draw();
                                USE_DISP_LISTS = true;
                                (**i).col = c;
                                RENDER_TEXTURES = os;
                        }
                }
                glPopMatrix();

                drawObjectsWithLights();
                glAccum(GL_ACCUM, 1.f/(float)(SOFT_SHADOW*SOFT_SHADOW*listLight3d.size()));
            }
        }
        glAccum(GL_RETURN, 1.f);
    }
    else{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawObjectsWithLights();
    }
}

void init3dEng(){

}
