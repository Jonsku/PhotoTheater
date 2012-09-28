#include "CutOut.h"
#include <QDebug>
#include <QMatrix4x4>

CutOut::CutOut(unsigned int textureId, int oWidth, int oHeight, float bX, float bY, float tX, float tY, float width, float height, float viewPortRatio, const QVector3D position, QPoint topLeft, QImage mask, QString fileName, QObject *parent) :
   QObject(parent), m_uiTexture(textureId), centerPosition(position), isAWall(false), wallScale(1.0f), scale(1.0f), rotation(0), topLeft(topLeft), mask(mask), fileName(fileName), originalWidth(oWidth), originalHeight(oHeight)
{
    //centerPosition = position;
   // centerPosition->setZ(-2.0f);
   // scale = 1.0f;
    //rotation = 0;
    //aspectRatio = viewPortRatio;
    //centerPosition.setX(centerPosition->x());

    //some calculation needs to be done to keep the same width/height ratio
    //and to handle the fact that the texture source is bigger than the cutout for optimization reasons
    /*
    qDebug() << "Position: " << centerPosition->x() << ", " << centerPosition->y() << ", " << centerPosition->z();
    qDebug() << "Original width: " << width;
    qDebug() << "Original height: " << height;
    qDebug() << "viewPortRatio: " << viewPortRatio;
    */

    //m_uiTexture = textureId;

    //texture coordinates (s,t)

    texCoord = new float[8];
     //bottom left
    texCoord[0] = tX;
    texCoord[1] = tY;

     //bottom right
    texCoord[2] = bX;
    texCoord[3] = tY;

    //top left
    texCoord[4] = tX;
    texCoord[5] = bY;

    //top right
    texCoord[6] = bX;
    texCoord[7] = bY;

    //create wall Texture coordinates
    addToWallScale(0);

    width *= (1.0f/viewPortRatio);
    //calculate the quad's vertices positions
    vertices = new float[12];
        vertices[0] = -width/2.0f;
        vertices[1] = -height/2.0f;
        vertices[2] = 0.0f;

        //bottom right
        vertices[3] = width/2.0f;
        vertices[4] = -height/2.0f;
        vertices[5] = 0.0f;

        //top left
        vertices[6] = -width/2.0f;
        vertices[7] = height/2.0f;
        vertices[8] = 0.0f;

        //top right
        vertices[9] = width/2.0f;
        vertices[10] = height/2.0f;
        vertices[11] = 0.0f;
}

CutOut::CutOut(QString fileName, int oWidth, int oHeight, const QVector3D position, float r, float s, float ws, QObject *parent):
        QObject(parent), centerPosition(position), wallScale(ws), scale(s), rotation(r), topLeft(0,0), originalWidth(oWidth), originalHeight(oHeight), isAWall(false){
    this->fileName = fileName;

}

void CutOut::setVertices(float vs[12]){
    vertices = vs;
}

void CutOut::setTextureCoord(float ts[8], bool setWall){
    texCoord = ts;
    if(setWall){
        addToWallScale(0);
    }
}

void CutOut::applyTransformation(QMatrix4x4 *modelView){
    modelView->translate(centerPosition);
    modelView->rotate(rotation,0,0,1);
    modelView->scale(scale);
}

void CutOut::setTextureId(unsigned int tId){
    m_uiTexture = tId;
}

unsigned int CutOut::getTextureId(){
    return m_uiTexture;
}

float * CutOut::getVertices(){
    return vertices;
}

float * CutOut::getTextureCoord(){
    return texCoord;
}

float * CutOut::getWallTextureCoord(){
    return wallTexCoord;
}

bool CutOut::containsPoint(float pX, float pY){
    return pX>=vertices[0] && pX<=vertices[3] && pY>=vertices[1] && pY<=vertices[7];
}

void CutOut::setCenterTo(float pX, float pY){
    centerPosition.setX(pX);
    centerPosition.setY(pY);
}

void CutOut::moveCenterBy(float pX, float pY){
    centerPosition.setX(centerPosition.x()+pX);
    centerPosition.setY(centerPosition.y()+pY);
}

void CutOut::moveZBy(float pZ){
    centerPosition.setZ(centerPosition.z()+pZ);
}

void CutOut::addToScale(float s){
    scale += s;
}

void CutOut::addToRotation(float r){
    rotation += r;
}

void CutOut::setScale(float s){
    scale = s;
}

void CutOut::setRotation(float r){
    rotation = r;
}

float CutOut::getScale(){
    return scale;
}

float CutOut::getRotation(){
    return rotation;
}

float CutOut::getX(){
    return centerPosition.x();
}

float CutOut::getY(){
    return centerPosition.y();
}

float CutOut::getZ(){
    return centerPosition.z();
}

int CutOut::getOriginalWidth(){
    return originalWidth;
}

int CutOut::getOriginalHeight(){
    return originalHeight;
}

float CutOut::getWallScale(){
    return wallScale;
}

void CutOut::addToWallScale(float s){

    wallScale += s;
    if(wallScale>1.0){
         wallScale = 1.0f;
     }else if(wallScale<-1.0){
         wallScale = -1.0f;
     }
    float w = texCoord[2]-texCoord[0];
    float h = texCoord[5]-texCoord[1];
    float x = (texCoord[0]+w*0.5f)-(w*0.5f*wallScale);
    float y = (texCoord[3]+h*0.5f)-(h*0.5f*wallScale);
    w *= wallScale;
    h *= wallScale;

    //bottom left
    wallTexCoord[0] = x;
    wallTexCoord[1] = y;

    //bottom right
    wallTexCoord[2] = x+w;
    wallTexCoord[3] = y;

    //top left
    wallTexCoord[4] = x;
    wallTexCoord[5] = y+h;

    //top right
    wallTexCoord[6] = x+w;
    wallTexCoord[7] = y+h;

}

bool CutOut::isWall(){
    return isAWall;
}

void CutOut::setAsWall(bool w){
    isAWall = w;
}

QString CutOut::toString(){
     QString s;
     s.sprintf("s: %f, r: %f, p: %f,%f,%f", scale, rotation, centerPosition.x(),centerPosition.y(),centerPosition.z());

    return s;
}

