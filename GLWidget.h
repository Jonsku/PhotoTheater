#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QGLShaderProgram>
#include <QStack>

#include "CutOut.h"
#include "Scene.h"
#include "HttpHandler.h"

#define OPERATION_SELECT 0
#define OPERATION_MOVE 1
#define OPERATION_MOVE_Z 2
#define OPERATION_SCALE 3
#define OPERATION_ROTATE 4

#define OPERATION_ORBIT 5
#define OPERATION_ZOOM 6

#define SET_FLOOR 7
#define SET_BACK 8
#define SET_RIGHT 9
#define SET_LEFT 10

#define PI 3.14159265

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);

protected:
    void resizeGL(int width, int height);
    void paintGL();
    void calibrateColor();
    void paint3D(bool textured);
    void initializeGL();

    void importTexture(CutOut * cutOut, unsigned int * wallz);
    bool exportTexture(CutOut * cutOut);

    bool project(GLint objX, GLint objY, GLdouble objZ,
                           const QMatrix4x4 model, const QMatrix4x4 proj,
                           const GLint *viewport,
                           GLdouble * winx, GLdouble * winy, GLdouble * winz);
    bool unProject(GLint mouse_x, GLint mouse_y, GLdouble mouse_z,
                    const QMatrix4x4 model, const QMatrix4x4 proj,
                    const GLint *viewport,
                    GLdouble * objx, GLdouble * objy, GLdouble * objz);

    void mousePressEvent ( QMouseEvent * event );
    void mouseMoveEvent ( QMouseEvent * event );

private:

    //checker walls
    GLfloat checkerVertices[12];
    GLfloat checkerTexcoord[8];
    //textured wall
  //  unsigned int walls[4];

    //color shader
    QGLShaderProgram checkerProgram;
    int checker_matrixUniform;
    int checker_frequencyUniform;
    int checker_color0Uniform;
    int checker_color1Uniform;

    int checker_positionAttr;
    int checker_texCoordAttr;

    QVector<QVector3D> vertices;

    //texture shader
    QGLShaderProgram textureProgram;
    int vertexAttr2;
    int texCoordAttr;
    int textureUniform;
    int matrixUniform;
    int selectedUniform;

    int coloringUniform;
    int colorCodeUniform;
    int solidUniform;

    //cutouts selection and manipulation
    unsigned int selectedTexture;


    GLdouble lastMouseX;
    GLdouble lastMouseY;
    GLdouble lastMouseZ;
    GLdouble winX, winY, winZ;

    float redStep;
    float redMap[256];
    float greenStep;
    float greenMap[256];
    bool colorCalibration;
    float calibrationSquareSize;
    QStack<unsigned int> colorCodeStack;
    QStack<unsigned int> textureIdStack;
    //QHash<unsigned int, CutOut*> cutOuts;
    Scene scene;
    bool pickTest;
    int pickMouseX;
    int pickMouseY;


    GLint viewPort[4];
    QMatrix4x4 projection;
    QMatrix4x4 view;

    QStack<QMatrix4x4> matrixStack;
    int currentOperation;
    float viewPortRatio;

    HttpHandler * httpHandler;




signals:
    void cutOutSelected(bool selected);
    void wallSelected();
    void saving();
    void saveFailed();
    void doneSaving(Scene *);
    void loadingScene();
    void sceneLoaded(Scene *);

public slots:
     //
     void newCutout(QImage img, float x, float y, float imgW, float imgH,  float cutW, float cutH, float w, float h, QPoint, QImage, QString);
     void deleteCutout();
     void setOperation(int operation);
     void upload();
     void save();
     void reset();
     void load(Scene * sceneToLoad);

};

#endif // GLWIDGET_H
