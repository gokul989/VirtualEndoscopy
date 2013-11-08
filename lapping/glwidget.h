#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QTimer>
#include <QPushButton>
#include "3deng.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    void initializeGL();
    void paintGL();
    void resizeGL( int w, int h);
    void handleKeys(QKeyEvent *);

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private:
    QTimer timer;
    bool isAxisDrawn;
    int w, h, FrameCounter;
    Vector2f m_lastPoint;
    int m_mouseClick;
    Camera3d freeCam;
    float camS;				// camera speed
    float camM;				// camera zoom
    float camT, camP;       // camera theta and phi
    float aspectRatio;
    Obj3ds skele;
    Obj3dsGraph stoma;
    ObjBIN model;
    Light3d sun1;
    Light3d sun2;
    Light3d bulb;
    TrackBall m_trackBalls;
    bool drawWithTexture;
    bool drawGraph;

signals:
    void updateStatus( QString);
    void sunStatusToggled();
    void bulbStatusToggled();

public slots:
    void toggleAxis();
    void handleSunStatusChanged(int);
    void handleBulbStatusChanged(int);
    void handleTextureStatusChanged(int);
    void handleGraphStatusChanged(int);
    void handleModelMode();
    void handleExternalMode();
    void handleInternalMode();
};

#endif // GLWIDGET_H
