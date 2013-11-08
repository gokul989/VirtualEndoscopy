#include <QResizeEvent>
#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent){
    setAutoBufferSwap(false);
    QObject::connect( &timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(33);
    isAxisDrawn = false;
    drawWithTexture = false;
    drawGraph = true;
}

inline bool matchString(const char *extensionString, const char *subString)
{
    int subStringLength = strlen(subString);
    return (strncmp(extensionString, subString, subStringLength) == 0)
        && ((extensionString[subStringLength] == ' ') || (extensionString[subStringLength] == '\0'));
}

bool necessaryExtensionsSupported()
{
    const char *extensionString = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
    const char *p = extensionString;

    const int GL_EXT_FBO = 1;
    const int GL_ARB_VS = 2;
    const int GL_ARB_FS = 4;
    const int GL_ARB_SO = 8;
    int extensions = 0;

    while (*p) {
        if (matchString(p, "GL_EXT_framebuffer_object"))
            extensions |= GL_EXT_FBO;
        else if (matchString(p, "GL_ARB_vertex_shader"))
            extensions |= GL_ARB_VS;
        else if (matchString(p, "GL_ARB_fragment_shader"))
            extensions |= GL_ARB_FS;
        else if (matchString(p, "GL_ARB_shader_objects"))
            extensions |= GL_ARB_SO;
        while ((*p != ' ') && (*p != '\0'))
            ++p;
        if (*p == ' ')
            ++p;
    }
    return (extensions == 15);
}

void GLWidget::initializeGL(){
    //glewInit();
    glEnable( GL_DEPTH_TEST);
    glFrontFace( GL_CCW);
    glShadeModel( GL_SMOOTH);
    glEnable( GL_NORMALIZE);
    setLighting( true);
    init3dEng();
    setQuaternion( 1);
    //m_trackBalls = TrackBall(0.0f, QVector3D(0, 1, 0), TrackBall::Plane);
    m_trackBalls = TrackBall(0.0f, QVector3D(0, 1, 0), TrackBall::Sphere);

    //-- sun Light
    sun1.isOn = true;
    sun1.mat.ambi.set( .3, .3, .3, 1.);
    sun1.mat.diff.set( .7, .7, .7, 1.);
    sun1.pos.set( -.5, 1, .5, 0.);
    sun2.isOn = true;
    sun2.mat.ambi.set( .3, .3, .3, 1.);
    sun2.mat.diff.set( .7, .7, .7, 1.);
    sun2.pos.set(  .5, -1, -.5, 0.);

    //-- bulb Light
    bulb.isOn = false;
    bulb.mat.ambi.set( .3, .3, .3, 1.);
    bulb.mat.diff.set( .7, .7, .7, 1.);
    bulb.mat.spec.set( 1., 1., 1., 1.);
    bulb.att.set( 1.0, .01, .0003);

    Texture2d *tex;
    GLubyte *textureImage; //added by sai
     tex = loadBMP( "lap.bmp");
    //bool success = loadPngImage("lap.png",256,256,true,&textureImage);
    //tex = loadBMP( "tex3.bmp");
    //stoma.Load3ds( "stomach.3ds");
    stoma.LoadBIN("mesh.bin");
    stoma.smooth = 1;
     stoma.bindTexture( tex);
    //stoma.bindTexture(&textureImage);
    //stoma.rotate( -90, 0, 0);
    //stoma.scale( Vector3f( .3, .3, .3));
    //stoma.scale( Vector3f( .003, .003, .003));
    stoma.col.set( .6, .6, .2);
    stoma.calcBoundingCube();
    stoma.calcGraph();
    //glTranslatef(0,-160.0,0.0);
stoma.drawGraph();
    skele.Load3ds( "skeleton.3ds");
    skele.smooth = 1;
    skele.col.set( .7, .7, .7);
    skele.calcBoundingCube();
    //stoma += -((skele.cubeBoundMin+skele.cubeBoundMax)/2.0);
    stoma += -((stoma.cubeBoundMin+stoma.cubeBoundMax)/2.0);
    skele += -((skele.cubeBoundMin+skele.cubeBoundMax)/2.0);
    skele.updateDisplayList();
    stoma.lap();
    stoma.saveBIN( "mappedmesh.bin");
    //tex = loadBMP( "lapatlas.bmp");
    //stoma.bindTexture( tex);

    stoma.updateDisplayList();
    skele.isHidden = true;

    model.LoadBIN( "mesh.bin");
    model.col.set( .8, .4, .4);
    model.scale( Vector3f(.1));
    model.calcBoundingCube();
    model += -((model.cubeBoundMin+model.cubeBoundMax)/2.0);
    model.updateDisplayList();
    model.isHidden = true;

    m_mouseClick = 0;
    camS = 0.1;
    camM = 1.0;				// camera zoom
    camT = 0.0; camP = 0.0; // camera theta and phi
    aspectRatio = 1.0;
    freeCam.pos.set( 45., 0., 0.);
    //freeCam.pos.set( 0., 0., -50.);
    freeCam.cnear = 0.01;
    freeCam.cfar = 2000.0;
    freeCam.ang = 45.0/camM;
    freeCam.eye.set( 0., 0., 0.);
    //freeCam.eye = freeCam.pos + Vector3f( sin(camP)*cos(camT), sin(camT), cos(camP)*cos(camT));

    printf("Done\n");
}

void GLWidget::handleModelMode(){
    freeCam.pos.set( 300., 0., 0.);
    //skele.isHidden = false;
    //stoma.isHidden = false;
    //model.isHidden = true;
    setQuaternion( 1);
    if( sun1.isOn == false){
        sun1.isOn = sun2.isOn = true;
        emit sunStatusToggled();
    }
    if( bulb.isOn == true){
        bulb.isOn = false;
        emit bulbStatusToggled();
    }
}

void GLWidget::handleInternalMode(){
    freeCam.pos.set( -2.761694,14.281392,-52.381657);
    camT = -.082869;
    camP = -5.947087;
    //skele.isHidden = true;
    //stoma.isHidden = true;
    //model.isHidden = false;
    setQuaternion( 0);
    if( sun1.isOn == true){
        sun1.isOn = sun2.isOn = false;
        emit sunStatusToggled();
    }
    if( bulb.isOn == false){
        bulb.isOn = true;
        emit bulbStatusToggled();
    }
}

void GLWidget::handleExternalMode(){
    freeCam.pos.set( 300., 0., 0.);
    //skele.isHidden = true;
    //stoma.isHidden = true;
    //model.isHidden = false;
    setQuaternion( 1);
    if( sun1.isOn == false){
        sun1.isOn = sun2.isOn = true;
        emit sunStatusToggled();
    }
    if( bulb.isOn == true){
        bulb.isOn = false;
        emit bulbStatusToggled();
    }
}

void GLWidget::toggleAxis(){
    isAxisDrawn = !isAxisDrawn;
}

void GLWidget::handleSunStatusChanged(int v){
    sun1.isOn = v;
    sun2.isOn = v;
}

void GLWidget::handleTextureStatusChanged(int v){
    drawWithTexture = v;
}

void GLWidget::handleGraphStatusChanged(int v){
    drawGraph = v;
}

void GLWidget::handleBulbStatusChanged(int v){
    bulb.isOn = v;
}

void drawAxis(){
    glLineWidth( 2.0);
    glBegin(GL_LINES);
    glColor3f( 1., .0, 0.);//x axis red
    glVertex3f(-1000,0,0);
    glVertex3f(1000,0,0);
    glColor3f( .0, 1., 0.); // y axis green
    glVertex3f(0,-1000,0);
    glVertex3f(0,1000,0);
    glColor3f( .0, .0, 1.);//z axis blue
    glVertex3f(0,0,-1000);
    glVertex3f(0,0,1000);
    glEnd();
    glLineWidth( 1.0);
    glBegin(GL_LINES);

    for( float i=-5; i<=5; i+=.1){
        glColor3f( 0, 1, 0);
        glVertex3f(i*200,-1000,0);
        glVertex3f(i*200, 1000,0);
        glColor3f( 1, 0., 0.);
        glVertex3f(-1000,i*200,0);
        glVertex3f( 1000,i*200,0);
            }
        glEnd();

        glBegin(GL_LINES);
       // glColor3f( .5, .5, .5);
        for( float i=-5; i<=5; i+=0.1){
            glColor3f( .0, 1, 0.);
            glVertex3f(0,-1000,i*200);
            glVertex3f(0, 1000,i*200);
            glColor3f( .0, 0., 1.);
            glVertex3f(0,i*200,-1000);
            glVertex3f( 0,i*200,1000);
                }
            glEnd();

            glBegin(GL_LINES);
            //glColor3f( .5, .5, .5);
            for( float i=-5; i<=5; i+=0.1){
                glColor3f( 1, 0, 0.);
                glVertex3f(-1000,0,i*200);
                glVertex3f(1000,0,i*200);
                glColor3f( .0, 0., 1.);
                glVertex3f(i*200,0,-1000);
                glVertex3f(i*200,0,1000);
                    }
                glEnd();
}

void GLWidget::paintGL(){
    // Lights
    bulb.pos = freeCam.pos;
    bulb.pos.u = 1.0;

    // Camera
    freeCam.ar = aspectRatio;
    if( !getQuaternion())
        freeCam.eye = freeCam.pos + Vector3f( sin(camP)*cos(camT), sin(camT), cos(camP)*cos(camT));
    setActiveCamera( freeCam);
    if( getQuaternion())
        setActiveViewTrackBall( m_trackBalls);

    stoma.isHidden = !drawWithTexture;
    render3dObjects();
    if( drawGraph)
        stoma.drawGraph();
    if( isAxisDrawn)
    {
        glPushMatrix();
        glTranslatef(80.0,130.0,0.0);
        drawAxis();
        glPopMatrix();
    }

    swapBuffers();
}

void GLWidget::resizeGL( int width, int height){
    w = width;
    h = height;
    aspectRatio = (float)(w)/(float)(h);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluPerspective(45.,((GLfloat)width)/((GLfloat)height),0.1f,1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLWidget::mousePressEvent(QMouseEvent *e){
    float x = 2.0 * (float(e->x()) / w) - 1.0;
    float y = 1.0 - 2.0 * (float(e->y()) / h);
    m_lastPoint.set( x, y);
    QPointF p(x, y);
    m_trackBalls.push( p, QQuaternion());
    m_mouseClick = 1;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e){
    if ( m_mouseClick){
        float x = 2.0 * (float(e->x()) / w) - 1.0;
        float y = 1.0 - 2.0 * (float(e->y()) / h);
        QPointF p(x, y);
        m_trackBalls.release( p, QQuaternion());
        m_mouseClick = 0;
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *e){
    if( m_mouseClick){
        float x = 2.0 * (float(e->x()) / w) - 1.0;
        float y = 1.0 - 2.0 * (float(e->y()) / h);
        QPointF p(x, y);
        Vector2f now( x, y);
        if( getQuaternion())
            m_trackBalls.move( p, QQuaternion());
        else{
            float dX = x - m_lastPoint.x;
            float dY = y - m_lastPoint.y;
            camP -= dX*.5;
            camT += dY*.5;
        }
        m_lastPoint = now;
        char s[100];
        sprintf( s, "Position: <%f,%f,%f>,%f,%f,<%f,%f>", freeCam.pos.x, freeCam.pos.y, freeCam.pos.z, camT, camP, m_lastPoint.x, m_lastPoint.y);
        emit updateStatus(s);
    }
}

void GLWidget::wheelEvent(QWheelEvent *e){
    if( getQuaternion()){
        freeCam.pos.x += e->delta()/8.0;
        setActiveCamera( freeCam);
    }
}

void GLWidget::handleKeys(QKeyEvent *e){
    if( e->key() == Qt::Key_W)
        freeCam.pos = freeCam.pos + Vector3f( camS*sin(camP), camS*sin(camT), camS*cos(camP));
    else if( e->key() == Qt::Key_S)
        freeCam.pos = freeCam.pos - Vector3f( camS*sin(camP), camS*sin(camT), camS*cos(camP));
    else if( e->key() == Qt::Key_A)
        freeCam.pos = freeCam.pos + Vector3f( camS*sin(camP+HALF_PI), camS*sin(camT), camS*cos(camP+HALF_PI));
    else if( e->key() == Qt::Key_D)
        freeCam.pos = freeCam.pos - Vector3f( camS*sin(camP+HALF_PI), camS*sin(camT), camS*cos(camP+HALF_PI));
    char s[100];
    sprintf( s, "Position: <%f,%f,%f>,%f,%f,<%f,%f>", freeCam.pos.x, freeCam.pos.y, freeCam.pos.z, camT, camP, m_lastPoint.x, m_lastPoint.y);
    emit updateStatus(s);
}
