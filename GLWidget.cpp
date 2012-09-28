#include "GLWidget.h"

#include "ImportExport.h"
#include "UserDefaults.h"
#include "ImageItem.h"
#include "Utilies.h"
#include "HttpHandler.h"
#include <QDebug>

#include <QGLShader>
#include <QMouseEvent>
#include <QPushButton>
#include <QFile>
#include <QProgressBar>
//#include <QVBoxLayout>
//#include <QFileDialog>

#include <QMessageBox>

#include <cmath>

/*float map(float value, float leftMin, float leftMax, float rightMin, float rightMax){
   // Figure out how 'wide' each range is
    float leftSpan = leftMax - leftMin;
    float rightSpan = rightMax - rightMin;

   // Convert the left range into a 0-1 range (float)
    float valueScaled = (value - leftMin) / leftSpan;

   // Convert the 0-1 range into a value in the right range.
    return rightMin + (valueScaled * rightSpan);
}*/
/*
int closestBiggestPowerOfTwo(int n){
    int x = n-1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}
*/
GLWidget::GLWidget(QWidget *parent) :
        QGLWidget(parent)
{
    httpHandler = new HttpHandler("http://j-u-t-t-u.net",this);

    //operation
    currentOperation = OPERATION_SELECT;
    //
    colorCalibration = true;
    calibrationSquareSize = 60;

    //position of the selected cutout - 1; 0 == nothing selected
    selectedTexture = 0;
    //
    pickTest = false;


    //
    checkerVertices[0]= -1;
    checkerVertices[1]= -1;
    checkerVertices[2]= 0.0f;

    checkerVertices[3]= 1;
    checkerVertices[4]= -1;
    checkerVertices[5]= 0.0f;

    checkerVertices[6]= -1;
    checkerVertices[7]= 1;
    checkerVertices[8]= 0.0f;

    checkerVertices[9]= 1;
    checkerVertices[10]= 1;
    checkerVertices[11]= 0.0f;

    checkerTexcoord[0] = 0;
    checkerTexcoord[1] = 0;

    checkerTexcoord[2] = 1;
    checkerTexcoord[3] = 0;

    checkerTexcoord[4] = 0;
    checkerTexcoord[5] = 1;

    checkerTexcoord[6] = 1;
    checkerTexcoord[7] = 1;


    //define triangle vertices
    vertices << QVector3D(-0.5f, 0.0f, -0.5f);
    vertices << QVector3D(0.5f, 0.0f, -0.5f);
    vertices << QVector3D(-0.5f, 0.0f, 0.5f);
    vertices << QVector3D(0.5f, 0.0f, 0.5f);

    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);

    //  setFixedSize(640, 480);
    /*
#ifndef Q_WS_QWS //windowing mode?
    setMinimumSize(300, 250);
#endif
    */
    this->repaint();
}

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0,0,width,height);
    viewPort[0] = 0;
    viewPort[1] = 0;
    viewPort[2] = width;
    viewPort[3] = height;
}

//Creates shaders
void GLWidget::initializeGL(){
    const char *vsrc1 =
            "uniform mat4 matrix;\n"	// combined modelview + projection matrix
            "attribute vec4 a_position;\n" // input vertex position
            "attribute vec2 a_st;\n"	// input texture coordinate
            "varying vec2 v_st;\n" 	// output texture coordinate
            "void main(void)\n"
            "{\n"
            "    v_st = a_st;\n"
            "    gl_Position = matrix * a_position;\n"
            "}\n";

    const char *fsrc1 =
            "#ifdef GL_ES\n"
            "precision highp float;\n"
            "#endif\n"
            "uniform int u_frequency;\n"
            "uniform vec4 u_color0;\n"
            "uniform vec4 u_color1;\n"
            "varying vec2 v_st;\n"
            "void main(void){\n"
            "   vec2 texcoord = mod(floor(v_st * float(u_frequency * 2)), 2.0);\n"
            "   float delta = abs(texcoord.x - texcoord.y);\n"
            "   gl_FragColor = mix(u_color1, u_color0, delta);\n"
            "}\n";
    QGLShader *vshader1 = new QGLShader(QGLShader::Vertex, this);
    vshader1->compileSourceCode(vsrc1);
    QGLShader *fshader1 = new QGLShader(QGLShader::Fragment, this);
    fshader1->compileSourceCode(fsrc1);

    checkerProgram.addShader(vshader1);
    checkerProgram.addShader(fshader1);
    checkerProgram.link();

    checker_matrixUniform = checkerProgram.uniformLocation("matrix");
    checker_frequencyUniform = checkerProgram.uniformLocation("u_frequency");
    checker_color0Uniform = checkerProgram.uniformLocation("u_color0");
    checker_color1Uniform = checkerProgram.uniformLocation("u_color1");

    checker_positionAttr = checkerProgram.attributeLocation("a_position");
    checker_texCoordAttr = checkerProgram.attributeLocation("a_st");

    const char *vsrc2 =
            "uniform mediump mat4 matrix;\n"
            "attribute highp vec4 texCoord;\n"
            "attribute vec4 vPosition;\n"
            "varying highp vec4 texc;\n"
            "void main(void)\n"
            "{\n"
            "   gl_Position = matrix * vPosition;\n"
            "   texc = texCoord;\n"
            "}\n";

    const char *fsrc2 =
            "varying highp vec4 texc;\n"
            "uniform bool solid;\n"
            "uniform bool coloring;\n"
            "uniform highp vec3 colorCode;\n"
            "uniform bool selected;\n"
            "uniform sampler2D tex;\n"
            "uniform highp vec2 freq;\n"
            "void main(void)\n"
            "{\n"
            "    if(coloring){\n"
            "        gl_FragColor = vec4(colorCode, 1.0);\n"
            "    }else{\n"
            "        highp vec4 color = texture2D(tex, texc.st);\n"
            "        if(color.a == 0.0){\n"
            "            discard;\n"
            "        }\n"
            "        if(solid){\n"
            "            color = vec4(colorCode,1.0);\n"
            "        }\n"
            "        if(selected){\n"
            "            color.b = 1.0;\n"
            "        }\n"
            "        gl_FragColor = color;\n"
            "    }\n"
            "}\n";

    QGLShader *vshader2 = new QGLShader(QGLShader::Vertex, this);
    vshader2->compileSourceCode(vsrc2);
    QGLShader *fshader2 = new QGLShader(QGLShader::Fragment, this);
    fshader2->compileSourceCode(fsrc2);

    textureProgram.addShader(vshader2);
    textureProgram.addShader(fshader2);
    textureProgram.link();

    matrixUniform = textureProgram.uniformLocation("matrix");

    vertexAttr2 = textureProgram.attributeLocation("vPosition");
    texCoordAttr = textureProgram.attributeLocation("texCoord");
    textureUniform = textureProgram.uniformLocation("tex");
    selectedUniform = textureProgram.uniformLocation("selected");
    coloringUniform = textureProgram.uniformLocation("coloring");
    solidUniform = textureProgram.uniformLocation("solid");
    colorCodeUniform = textureProgram.uniformLocation("colorCode");

}

void GLWidget::paintGL()
{
    QPainter painter;
    painter.begin(this);

    painter.beginNativePainting();

    if(colorCalibration){
        redStep = 1/((width()-calibrationSquareSize)/calibrationSquareSize);
        greenStep = 1/((height()-calibrationSquareSize)/calibrationSquareSize);
        calibrateColor();
        //initialize matrices
        projection.perspective(60.0f,(float)width()/(float)height(),0.1f,100.0f);
        //matrixStack.push(view);
        colorCalibration = false;
    }else{
        //perform the pick test
        if(pickTest){
            //first paint each cut outs using solid colors
            paint3D(false);

            //then find out which cut out has been clicked
            //TODO: Might be able to make this faster/nicer with glReadPixel (grabFrameBuffer uses it so it shoud work)...
            QImage img = grabFrameBuffer();
            QColor pixColor;
            pixColor = img.pixel(pickMouseX, pickMouseY);

            if(pixColor.red() !=0 && pixColor.green() != 0 && pixColor.blue() != 0 && redMap[pixColor.red()] != 0 && greenMap[pixColor.green()] != 0){
                //calculate the coor code
                int rP = pixColor.red();
                int gP = pixColor.green();
                unsigned int code = (rP << 8) | gP;
                if(selectedTexture == code){
                    //deselect
                    selectedTexture = 0;
                    emit cutOutSelected(false);
                }else{
                    selectedTexture = code;
                    //get the mouse coord on that plane
                    QMatrix4x4 mv;
                    unProject(pickMouseX, pickMouseY, scene.getCutOut(selectedTexture)->getZ(), mv, projection, viewPort,
                              &lastMouseX, &lastMouseY, &lastMouseZ);
                    if(scene.getCutOut(selectedTexture)->isWall()){
                        emit wallSelected();
                    }else{
                        emit cutOutSelected(true);
                    }
                }
            }else{
                selectedTexture = 0;
                emit cutOutSelected(false);
            }
            pickTest = false;
        }
        paint3D(true);
    }

    painter.endNativePainting();
    painter.end();
    swapBuffers();
}

//calibrate color code for selection
void GLWidget::calibrateColor(){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);

    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST); //no need for 2D drawing

    textureProgram.bind();
    textureProgram.enableAttributeArray(vertexAttr2);
    textureProgram.enableAttributeArray(texCoordAttr);

    GLfloat vVertices[] = {0, 10, 0.0f,
                           10, 10, 0.0f,
                           0, 0, 0.0f,
                           10, 0, 0.0f};

    GLfloat vTexCoord[] = {0, 1,
                           1, 1,
                           0, 0,
                           1, 0};
    //equivalent to glOrtho
    QMatrix4x4 modelView ( 2.0f/width(), 0.0f, 0.0f, -1.0f,
                           0.0f, -2.0f/height(), 0.0f, 1.0f,
                           0.0f, 0.0f, -2.0f, 0.0f,
                           0.0f, 0.0f, 0.0f, 1.0f);

    float r = redStep;
    float g = greenStep;

    //draw the color
    textureProgram.setUniformValue(coloringUniform, true);
    //don't care about highlighting here
    textureProgram.setUniformValue(selectedUniform, false);

    for(int x = 0; x<width()-calibrationSquareSize;x+=calibrationSquareSize){
        for(int y = 0; y<height()-calibrationSquareSize;y+=calibrationSquareSize){
            vVertices[0] = x;
            vVertices[1] = y+calibrationSquareSize;
            vVertices[2] = 0;

            vVertices[3] = x+calibrationSquareSize;
            vVertices[4] = y+calibrationSquareSize;
            vVertices[5] = 0;

            vVertices[6] = x;
            vVertices[7] = y;
            vVertices[8] = 0;

            vVertices[9]  = x+calibrationSquareSize;
            vVertices[10] = y;
            vVertices[11] = 0;

            textureProgram.setUniformValue(matrixUniform, modelView);
            textureProgram.setAttributeArray(vertexAttr2, vVertices, 3);
            textureProgram.setAttributeArray(texCoordAttr, vTexCoord, 2);

            textureProgram.setUniformValue(colorCodeUniform, r, g, 1.0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            g+=greenStep;

        }
        r += redStep;
        g = greenStep;
    }



    QImage img = grabFrameBuffer();
    QColor pixColor;

    r = redStep;
    for(int x = calibrationSquareSize/2; x<width()-calibrationSquareSize/2;x+=calibrationSquareSize){
        pixColor = img.pixel(x, calibrationSquareSize/2);
        redMap[pixColor.red()] = r;
        r += redStep;
    }

    g = greenStep;
    for(int y = calibrationSquareSize/2; y<height()-calibrationSquareSize/2;y+=calibrationSquareSize){
        pixColor = img.pixel(calibrationSquareSize/2, y);
        greenMap[pixColor.green()] = g;
        g += greenStep;
    }


    for(int x = calibrationSquareSize/2; x<width()-calibrationSquareSize/2;x+=calibrationSquareSize){
        for(int y = calibrationSquareSize/2; y<height()-calibrationSquareSize/2;y+=calibrationSquareSize){
            pixColor = img.pixel(x, y);
            int rP = pixColor.red();
            int gP = pixColor.green();
            unsigned int c = (rP << 8) | gP;
            colorCodeStack.push(c);
        }
    }

    //Done drawing
    textureProgram.disableAttributeArray(vertexAttr2);
    textureProgram.disableAttributeArray(texCoordAttr);
    textureProgram.release();

}

//showTexture: when false paint cutouts with their color codes for picking with the mouse
void GLWidget::paint3D(bool showTexture){

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);


    unsigned int colorCode;
    CutOut *cutOut;

    QMatrix4x4 modelView;
    //set perspective
    modelView *= projection;
    //set view angle
    modelView.translate(0,0,-(3.0f+scene.zoom()));
    modelView.rotate(scene.orbitVector().x(),1,0);
    modelView.rotate(scene.orbitVector().y(),0,1);
    modelView.translate(0,0,3.0f);
    modelView.lookAt(QVector3D(0,0,1),QVector3D(0,0,0),QVector3D(0,1,0));

    //this is to be able to hide the walls when seen from the back
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    //paint the walls
    for(Scene::WallId i= Scene::Back;i<=Scene::Floor;i++){
        if(showTexture && !scene.isWallSet(i)){
            checkerProgram.bind();
            checkerProgram.enableAttributeArray(checker_positionAttr);
            checkerProgram.enableAttributeArray(checker_texCoordAttr);

            checkerProgram.setUniformValue(checker_frequencyUniform,10);
            //default white and gray squares pattern
            checkerProgram.setUniformValue(checker_color0Uniform,1,1,1,1);
            checkerProgram.setUniformValue(checker_color1Uniform,0.5,0.5,0.5,0.5);
        }else if(scene.isWallSet(i)){
            textureProgram.bind();
            textureProgram.enableAttributeArray(vertexAttr2);
            textureProgram.enableAttributeArray(texCoordAttr);
            //draw the color
            textureProgram.setUniformValue(coloringUniform, false);
            //don't care about highlighting here
            textureProgram.setUniformValue(selectedUniform, false);
            //
            textureProgram.setUniformValue(solidUniform, !showTexture);
        }

        matrixStack.push(modelView);
        modelView.translate(0,0,-3.0f);

        switch(i){
        case Scene::Back:
            break;
        case Scene::Left: //left wall
            modelView.translate(-1.0f,0,1.0f);
            modelView.rotate(90,0,1);
            break;
        case Scene::Right: //right wall
            modelView.translate(1.0f,0,1.0);
            modelView.rotate(-90,0,1);
            break;
        case Scene::Floor: //floor
            modelView.translate(0,-1.0f,1.0);
            modelView.rotate(-90,1,0);
            break;
        }

        //modelView.rotate(orbitVector.y(),0,1);
        if(showTexture && !scene.isWallSet(i)){
            checkerProgram.setUniformValue(checker_matrixUniform, modelView);
            checkerProgram.setAttributeArray(checker_positionAttr, checkerVertices, 3);
            checkerProgram.setAttributeArray(checker_texCoordAttr, checkerTexcoord, 2);
        }else if(scene.isWallSet(i)){
            textureProgram.setUniformValue(matrixUniform, modelView);

            textureProgram.setAttributeArray(vertexAttr2, checkerVertices, 3);
            textureProgram.setAttributeArray(texCoordAttr, scene.getWall(i)->getWallTextureCoord(), 2);

            glBindTexture(GL_TEXTURE_2D, scene.getWall(i)->getTextureId());


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            textureProgram.setUniformValue(textureUniform, 0);    // use texture unit 0
            //highlight if selected
            unsigned int wC = scene.getWallCode(i);
            textureProgram.setUniformValue(selectedUniform, (showTexture)&&(selectedTexture == wC));

            textureProgram.setUniformValue(colorCodeUniform, redMap[((wC >> 8) & 0xff)], greenMap[(wC & 0xff)], 1.0);
        }


        if(showTexture && !scene.isWallSet(i)){
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            checkerProgram.disableAttributeArray(checker_positionAttr);
            checkerProgram.disableAttributeArray(checker_texCoordAttr);
            checkerProgram.release();
        }else if(scene.isWallSet(i)){
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            textureProgram.disableAttributeArray(vertexAttr2);
            textureProgram.disableAttributeArray(texCoordAttr);
            textureProgram.release();
        }

        modelView = matrixStack.pop();
    }

    glDisable(GL_CULL_FACE);


    //draw the other cutouts
    textureProgram.bind();
    textureProgram.enableAttributeArray(vertexAttr2);
    textureProgram.enableAttributeArray(texCoordAttr);

    //draw the color
    textureProgram.setUniformValue(coloringUniform, false);
    //don't care about highlighting here
    textureProgram.setUniformValue(selectedUniform, false);
    //
    textureProgram.setUniformValue(solidUniform, !showTexture);

    foreach( colorCode, scene.getCutOutsIds() ){
        cutOut = scene.getCutOut(colorCode);
        //walls have already been painted
        if(cutOut->isWall()){
            continue;
        }
        //move to correct position
        matrixStack.push(modelView);
        cutOut->applyTransformation(&modelView);
        //
        glBindTexture(GL_TEXTURE_2D, cutOut->getTextureId());

        textureProgram.setUniformValue(matrixUniform, modelView);
        textureProgram.setAttributeArray(vertexAttr2, cutOut->getVertices(), 3);
        textureProgram.setAttributeArray(texCoordAttr, cutOut->getTextureCoord(), 2);
        textureProgram.setUniformValue(textureUniform, 0);    // use texture unit 0
        //highlight if selected
        textureProgram.setUniformValue(selectedUniform, (showTexture)&&(selectedTexture == colorCode));
        textureProgram.setUniformValue(colorCodeUniform, redMap[((colorCode >> 8) & 0xff)], greenMap[(colorCode & 0xff)], 1.0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //reset transformation
        modelView = matrixStack.pop();
    }

    textureProgram.disableAttributeArray(vertexAttr2);
    textureProgram.disableAttributeArray(texCoordAttr);
    textureProgram.release();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
}

//Straight out from Glut
bool GLWidget::project(GLint objX, GLint objY, GLdouble objZ,
                       const QMatrix4x4 model, const QMatrix4x4 proj,
                       const GLint *viewport,
                       GLdouble * winx, GLdouble * winy, GLdouble * winz)
{
    QVector4D objP(objX, objY, objZ, 1.0);
    objP = model.map(objP);
    objP = proj.map(objP);

    if (objP.w() == 0.0) return false;

    //Could it be normalizing?
    objP.setX(objP.x()/objP.w());
    objP.setY(objP.y()/objP.w());
    objP.setZ(objP.z()/objP.w());

    /* Map x, y and z to range 0-1 */
    objP.setX(objP.x() * 0.5 + 0.5);
    objP.setY(objP.y() * 0.5 + 0.5);
    objP.setZ(objP.z() * 0.5 + 0.5);

    /* Map x,y to viewport */
    objP.setX(objP.x() * viewport[2] + viewport[0]);
    objP.setY(objP.y() * viewport[3] + viewport[1]);

    *winx=objP.x();
    *winy=objP.y();
    *winz=objP.z();
    return true;
}

/*
Written with infos from:
            http://www.siggraph.org/education/materials/HyperGraph/raytrace/rayplane_intersection.htm
            http://www.opengl.org/resources/faq/technical/selection.htm
and the GLut library.
*/
//mouse z is the the coordinate of the plane
bool GLWidget::unProject(GLint mouse_x, GLint mouse_y, GLdouble mouse_z,
                         const QMatrix4x4 model, const QMatrix4x4 proj,
                         const GLint *viewport,
                         GLdouble * objx, GLdouble * objy, GLdouble * objz)
{


    // calculate inverse transformation
    QMatrix4x4 A(model);
    A = A.inverted();
    A *= proj.inverted();

    // normalize cordinates between -1 and 1
    int window_y = (viewport[3] - mouse_y) - viewport[3]/2;
    double norm_y = double(window_y)/double(viewport[3]/2.0);
    int window_x = mouse_x - viewport[2]/2;
    double norm_x = double(window_x)/double(viewport[2]/2.0);

    float y =  norm_y;
    float x =  double(viewport[2]/viewport[3]) * norm_x;

    //get the direction vector in object coordinate
    QVector4D rayV(x, y, -0.1,1);
    rayV =  A.map(rayV);


    QVector3D planeNormal(0,0,1);
    QVector3D rayPoint(0,0,0); //that's where the camera is
    QVector3D rayVector(rayV);
    rayVector.normalize();

    float vD = QVector3D::dotProduct(planeNormal, rayVector);
    //is the ray parrallel to the plane?
    if(vD >= 0){
        return false;
    }

    //NOTE: in the online material it states that the sign should be opposite, but here it isn't...
    float v0 = (QVector3D::dotProduct(planeNormal, rayPoint)+mouse_z);
    float t = v0/vD;
    //is the intersection point behind the ray?
    if(t<=0){
        return false;
    }

    //calculate interesction point
    *objx = rayPoint.x() + rayVector.x()*t;
    *objy = rayPoint.y() + rayVector.y()*t;
    *objz = rayPoint.z() + rayVector.z()*t; //The sign could be wrong, but it doesn't really matter..
    return true;
}


void GLWidget::mousePressEvent(QMouseEvent * event){

    lastMouseX = event->x();
    lastMouseY = event->y();
    lastMouseZ = 0;
    if(currentOperation==OPERATION_ORBIT || currentOperation==OPERATION_ZOOM){
        return;
    }



    if(currentOperation==OPERATION_SELECT){
        pickMouseX = event->x();
        pickMouseY = event->y();
        pickTest = true;
        repaint();
    }else if(selectedTexture!=0){
        CutOut * selectedCutout = scene.getCutOut(selectedTexture);
        /*
         TODO: This doesn't work too well as the view tranformations are not taken into account.
             Itis still functional, but should be fixed at some point.
         */
        QMatrix4x4 mv;
        unProject(lastMouseX, lastMouseY, selectedCutout->getZ(), mv, projection, viewPort,
                  &lastMouseX, &lastMouseY, &lastMouseZ);

        if(currentOperation == OPERATION_ROTATE){
            //in this case lastMouseX contains the initial rotation and lastMouseY the reference angle)
            lastMouseY = atan2(lastMouseY-selectedCutout->getY(),lastMouseX-selectedCutout->getX());
            lastMouseX = selectedCutout->getRotation();
        }
    }

}

void GLWidget::mouseMoveEvent( QMouseEvent * event ){

    if(currentOperation == OPERATION_SELECT){
        //event->ignore(); //does it make the view slide?
        return;
    }

    if(selectedTexture != 0){
        CutOut * selectedCutout = scene.getCutOut(selectedTexture);
        GLdouble mouseX, mouseY, mouseZ;
        QMatrix4x4 mv;
        /*
         TODO: This doesn't work too well as the view tranformations are not taken into account.
             Itis still functional, but should be fixed at some point.
         */
        unProject(event->x(), event->y(), selectedCutout->getZ(), mv, projection,
                  viewPort,
                  &mouseX, &mouseY, &mouseZ);
        float yTrans = mouseY-lastMouseY;
        float xTrans = mouseX-lastMouseX;
        float rot = atan2(event->y()-height()*0.5,event->x()-width()*0.5);
        float s = (mouseX-lastMouseX)/2.0f;
        switch(currentOperation){
        case OPERATION_MOVE:
            //reverse translation depending on camera position
            if(scene.orbitVector().y() >= 90.0f && scene.orbitVector().y() <= 270.0f){
                xTrans *= -1;
            }
            if(scene.orbitVector().x() >= 90.0f && scene.orbitVector().x() <= 270.0f){
                yTrans *= -1;
            }
            selectedCutout->moveCenterBy(xTrans,yTrans);
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            break;
        case OPERATION_MOVE_Z:
            //reverse translation depending on camera position
            if(scene.orbitVector().y() >= 90.0f && scene.orbitVector().y() <= 270.0f){
                xTrans *= -1;
            }
            if(scene.orbitVector().x() >= 90.0f && scene.orbitVector().x() <= 270.0f){
                yTrans *= -1;
            }
            selectedCutout->moveZBy(-yTrans);
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            break;
        case OPERATION_SCALE:

            if(selectedCutout->isWall()){
                selectedCutout->addToWallScale(s);
            }else{
                selectedCutout->addToScale(s);
            }
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            break;
        case OPERATION_ROTATE:
            //reverse rotation depending on camera position
            if(scene.orbitVector().y() >= 90.0f && scene.orbitVector().y() <= 270.0f){
                rot *= -1;
            }
            if(scene.orbitVector().x() >= 90.0f && scene.orbitVector().x() <= 270.0f){
                rot *= -1;
            }
            //in this case lastMouseX contains the initial rotation and lastMouseX the reference angle, do not update them
            selectedCutout->setRotation(lastMouseX-(rot)*(180.0)/PI);// addToRotation(float((lastMouseY-event->x()))/float(width())*180.0f);
            break;
        }
        repaint();
    }
    if(currentOperation == OPERATION_ORBIT){
        scene.addToOrbitX(float((lastMouseY-event->y()))/float(height())*180.0f);
        scene.addToOrbitY(float((lastMouseX-event->x()))/float(width())*180.0f);
        /*
        orbitVector.setX(orbitVector.x()+float((lastMouseY-event->y()))/float(height())*180.0f);
        orbitVector.setY(orbitVector.y()+float((lastMouseX-event->x()))/float(width())*180.0f);
        //limit angle to the range [0, 360]
        if(orbitVector.x()>=360){
            orbitVector.setX(orbitVector.x()-360);
        }else if(orbitVector.x()<0){
            orbitVector.setX(360+orbitVector.x());
        }

        if(orbitVector.y()>=360){
            orbitVector.setY(orbitVector.y()-360);
        }else if(orbitVector.y()<0){
            orbitVector.setY(360+orbitVector.y());
        }
*/
        lastMouseY = event->y();
        lastMouseX = event->x();
        repaint();
    }else if(currentOperation == OPERATION_ZOOM){
        scene.addToZoom((lastMouseY-event->y())/float(height()));
        /*
                zoom += (lastMouseY-event->y())/float(height());
        if(zoom>0){
            zoom = 0;
        }else if(zoom<-3){
            zoom = -3;
        }
*/
        lastMouseY = event->y();
        lastMouseX = event->x();
        repaint();
    }else{
        return;
    }

}


void GLWidget::newCutout(QImage img, float x, float y, float imgW, float imgH, float cutW, float cutH, float wU, float hU,  QPoint pos,  QImage mask, QString filename){
    if(colorCodeStack.size()==0){ //no more color to allocate
        return; //TODO: Tell user the limit has been reached
    }

    unsigned int colorCode = colorCodeStack.pop();
    //get a valid texture id
    GLuint texId = 0;
    if(textureIdStack.empty()){
        //need to generate a new one
        glGenTextures(1,&texId);
    }else{
        texId = textureIdStack.pop();
    }

    //turn upside down
    img = QGLWidget::convertToGLFormat(img);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA,GL_UNSIGNED_BYTE, img.bits());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    /*
     GLuint texId = bindTexture(img);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
*/

    //calculate the texture coordinates of the cutout
    float tX = (imgW-cutW)/2.0f;
    float tY = (imgH-cutH)/2.0f;
    float bX = tX+cutW;
    float bY = tY+cutH;

    //texture coordinates are from 0.0 to 1.0
    tX /= imgW;
    bX /= imgW;
    tY /= imgH;
    bY /= imgH;

    float aspectRatio = 480.0f/640.0f;
    scene.addCutOut(colorCode, new CutOut(texId, img.width(), img.height(), bX, bY, tX, tY, wU, hU, aspectRatio, QVector3D(x-0.5f,-y+0.5f,-2.0), pos, mask, filename));
    //make it the current selction
    selectedTexture = colorCode;
    emit cutOutSelected(true);
}

void GLWidget::deleteCutout(){
    if(selectedTexture == 0){
        return;
    }
    CutOut * selectedCutout = scene.getCutOut(selectedTexture);
    if(selectedCutout->isWall()){
        //selectedCutout->setAsWall(false);
        for(Scene::WallId i= Scene::Back;i<=Scene::Floor;i++){
            if(scene.getWallCode(i) == selectedTexture){
                scene.unsetWall(i);
                break;
            }
        }
        emit cutOutSelected(false);
        emit cutOutSelected(true);
    }else{
        //push back the texture id
        textureIdStack.push(selectedCutout->getTextureId());
        scene.removeCutOut(selectedTexture);
        colorCodeStack.push(selectedTexture);
        selectedTexture = 0;
        currentOperation = OPERATION_SELECT;
        emit cutOutSelected(false);
    }
    repaint();
}

void GLWidget::setOperation(int operation){
    if(currentOperation == operation){
        //reset scaling/rotation
        switch(currentOperation){
        case OPERATION_SCALE:
            scene.getCutOut(selectedTexture)->setScale(1.0f);
            break;
        case OPERATION_ROTATE:
            scene.getCutOut(selectedTexture)->setRotation(0.0f);
            break;
        case OPERATION_ORBIT:
            scene.resetOrbit();
            break;
        case OPERATION_ZOOM:
            scene.resetZoom();
            break;
        }
        repaint();
    }else{
        switch(operation){
        case SET_FLOOR:
            scene.setWall(Scene::Floor,selectedTexture);
            emit wallSelected();
            break;
        case SET_BACK:
            scene.setWall(Scene::Back,selectedTexture);
            emit wallSelected();
            break;
        case SET_RIGHT:
            scene.setWall(Scene::Right,selectedTexture);
            emit wallSelected();
            break;
        case SET_LEFT:
            scene.setWall(Scene::Left,selectedTexture);
            emit wallSelected();
            break;
        default:;
        }

        currentOperation = operation;
    }
}

//load a scene
void GLWidget::load(Scene * sceneToLoad){
    emit loadingScene();
    //reset the scene
    scene.reset(&colorCodeStack, &textureIdStack);
    scene.setFilename(sceneToLoad->getFilename());
   // UserDefaults &userDefaults = UserDefaults::instance();
   // QString directoryPath = userDefaults["rawPath"].asString().c_str();

   /* QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open scene"), directoryPath, tr("(*.pht)"));
                                                    */
    ImportExport * iE = new ImportExport(&scene,false);
    bool r = false;
    CutOut * cO = iE->nextCutOut(&r);
    while(r){
       // qDebug() << cO->fileName;
        importTexture(cO,iE->getWalls());
        cO = iE->nextCutOut(&r);
    }
    delete cO;
    iE->clean();
    delete iE;
    //set up camera
    /*
scene.resetZoom();
    scene.addToZoom(sceneToLoad->zoom());
    scene.resetOrbit();
    scene.addToOrbitX(sceneToLoad->orbitVector().x());
    scene.addToOrbitY(sceneToLoad->orbitVector().y());
*/
    repaint();
    emit sceneLoaded(&scene);
}

void GLWidget::importTexture(CutOut * cO, unsigned int * wallz){
    if(colorCodeStack.size()==0){ //no more color to allocate
        return; //TODO: Tell user the limit has been reached
    }

    float hRatio = 640.0f/2592.0f;
    float vRatio = 480.0f/1968.0f;

    QImage img = QImage("/home/user/.phototheater/temp/"+cO->fileName).convertToFormat(QImage::Format_ARGB32);
    //moves the temporary image to the PhotoTheater folder
    UserDefaults &userDefaults = UserDefaults::instance();
    QString directoryPath = userDefaults["rawPath"].asString().c_str();
    if(! img.save(directoryPath + cO->fileName)){
        qDebug("Saving the image failed...");
    }
    /*
    QImage mask(img.width(),img.height(),QImage::Format_Mono);

    //apply alpha mask
    for(int j=0;j<mask.height();j++){
        QRgb * imgScanLine = (QRgb *)(img.scanLine(j));
        for(int i=0;i<mask.width();i++){
            if(qAlpha(imgScanLine[i]) == 0){
                mask.setPixel(i,j, 0);
            }else{
                mask.setPixel(i,j, 1);
            }
        }
    }
    */
    img = img.scaled(Utilies::closestBiggestPowerOfTwo(img.width()*hRatio),Utilies::closestBiggestPowerOfTwo(img.height()*vRatio),Qt::IgnoreAspectRatio, Qt::FastTransformation);




    img = QGLWidget::convertToGLFormat(img);

    // Load the texture
    QGLWidget::makeCurrent();
    GLuint texId = 0;
    if(textureIdStack.empty()){
        //need to generate a new one
        glGenTextures(1,&texId);
    }else{
        texId = textureIdStack.pop();
    }
    //glGenTextures(1,&texId);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA,GL_UNSIGNED_BYTE, img.bits());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint oldId = cO->getTextureId();
    cO->setTextureId(texId);
   // qDebug() << "Texture created";
    //get next color code
    int nextCode = colorCodeStack.pop();
    //cO->setAsWall(false);

    scene.addCutOut(nextCode,cO);
    //check if the cut out is applied to a wall
    for(Scene::WallId i= Scene::Back;i<=Scene::Floor;i++){

        if(wallz[i] == oldId){
      //      qDebug() << "Setting wall";
            scene.setWall(i,nextCode);
            break;
        }
    }
    // cO->fileName = texId+".png";
    //cO->setTextureId(texId);
   // scene.addCutOut(nextCode,cO);
}

//save the scene
void GLWidget::save(){
    if(scene.getCutOutsIds().size() == 0){
        //nothing to save!
        return;
    }
    emit saving();

    ImportExport * iE = new ImportExport(&scene);


    //takes a snapshot of the scene
    QImage thumbnail = QGLWidget::grabFrameBuffer();
    QString s = "/home/user/.phototheater/temp/snapshot.png";
    qDebug() << "Saving to " << s;

    if(! thumbnail.save(s,"PNG",-1)){
        qDebug() << "Failed to create Ramyah's snapshot";
    }

    unsigned int colorCode;
    foreach( colorCode, scene.getCutOutsIds() ){
      //  iE->addCutout(scene.getCutOut(colorCode)); // cutOuts[colorCode]
        if(!exportTexture(scene.getCutOut(colorCode))){
            qDebug() << "save operation failed ...";
            iE->clean();
            delete iE;
            emit saveFailed();
            return;
        }
    }
    iE->saveToFile(&scene);
    iE->clean();
    delete iE;
    emit doneSaving(&scene);
/*
    for(Scene::WallId i= Scene::Back;i<=Scene::Floor;i++){
        if(scene.isWallSet(i)){
            iE->addWall(i,scene.getWall(i)->getTextureId());
        }
    }
    */




}

bool GLWidget::exportTexture(CutOut * cutOut){

    UserDefaults &userDefaults = UserDefaults::instance();
    QString directoryPath = userDefaults["rawPath"].asString().c_str();
    qDebug() << "Export Texture: " << cutOut->fileName.split(".").at(1).trimmed();
    if(cutOut->fileName.split(".").at(1).trimmed() == "png"){
        qDebug() << "from png";
        //simply move the file from PhotoTheater to temp folder
        QFile * mvFile = new QFile(directoryPath + cutOut->fileName);
        QString mv;
        mv.sprintf("/home/user/.phototheater/temp/%d.png",cutOut->getTextureId());
        if(mvFile->rename(mv)){
            qDebug() << "File moved successfully.";
            return true;
        }else{
            return false;
        }

    }
    ImageItem * imageItem = new ImageItem(directoryPath + cutOut->fileName);
    QPixmap pMap = imageItem->fullResPixmap();


    //the polygon's vertices need to be converted from 640 x 480 to the full resolution
    float hRatio = float(pMap.width())/float(width());
    float vRatio = float(pMap.height())/float(height());

    QRect bRect(cutOut->topLeft.x()*hRatio, cutOut->topLeft.y()*vRatio, cutOut->mask.width()*hRatio, cutOut->mask.height()*vRatio);

    QImage cutImage = pMap.toImage().copy(bRect).convertToFormat(QImage::Format_ARGB32);

    //create a mask
    QImage scaledMask = cutOut->mask.scaled(cutImage.width(), cutImage.height() ,Qt::IgnoreAspectRatio, Qt::FastTransformation);

    for(int j=0;j<cutImage.height();j++){
        QRgb *row = (QRgb *)cutImage.scanLine(j);
        for(int i=0;i<cutImage.width();i++){
            if((scaledMask.pixel(i,j) & 0xFF) == 0){
                row[i] = qRgba( 0, 0, 0, 0 );
            }
        }
    }

    QString s;

    s.sprintf("/home/user/.phototheater/temp/%d.png", cutOut->getTextureId());

    if(! cutImage.save(s,"PNG",-1)){
        qDebug()<< "saving texture to file failed!";
        return false;
    }else{
        return true;
    }

}

void GLWidget::upload(){
    qDebug() << "Upload";
    QMessageBox msgBox;
     msgBox.setText("The screenshot will be shown on http://j-u-t-t-u.net/PhotoTheater/");
     msgBox.setInformativeText("Images on this page are available for anyone to see and download, do you agree and want to continue?");
     msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
     msgBox.setDefaultButton(QMessageBox::Yes);
     int ret = msgBox.exec();
     if(ret==QMessageBox::Yes){
        httpHandler->sendImage(QGLWidget::grabFrameBuffer());
     }
}

void GLWidget::reset(){
    scene.reset(&colorCodeStack,&textureIdStack);
}


