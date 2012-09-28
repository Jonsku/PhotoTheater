#ifndef CUTOUT_H
#define CUTOUT_H

#include <QObject>
#include <QVector3D>
#include <QImage>

class CutOut : public QObject
{
    Q_OBJECT
public:
    explicit CutOut(unsigned int textureId, int oWidth, int oHeight, float bX, float bY, float tX, float tY, float width, float height, float viewPortRatio, const QVector3D position, QPoint topLeft, QImage mask, QString fileName,  QObject *parent = 0);
    CutOut(QString fileName, int oWidth, int oHeight, const QVector3D position, float r, float s, float ws, QObject *parent = 0);

    bool containsPoint(float pX, float pY);
    void setTextureId(unsigned int tId);
    unsigned int getTextureId();
    void setVertices(float vs[12]);
    void setTextureCoord(float ts[8], bool setWall = true);
    float * getVertices();
    float * getTextureCoord();
    float * getWallTextureCoord();
    void applyTransformation(QMatrix4x4 *modelView);

    void setCenterTo(float pX, float pY);   
    float getX();
    float getY();
    float getZ();
    void moveCenterBy(float pX, float pY);
    void moveZBy(float pZ);

    bool isWall();
    void setAsWall(bool w);

    float getScale();
    float getWallScale();
    float getRotation();
    void addToWallScale(float s);
    void addToScale(float s);
    void addToRotation(float r);
    void setScale(float s);
    void setRotation(float r);

    int getOriginalWidth();
    int getOriginalHeight();
    QPoint topLeft;
    QImage mask;
    QString fileName;
    QString toString();



private:
    //
    bool isAWall;
    //
    int originalWidth;
    int originalHeight;
    //
    float scale;
    //
    float wallScale;
    //
    float rotation;
    //
    float * vertices;
    //
    float * texCoord;
    //
    float wallTexCoord[8];
    //
    unsigned int m_uiTexture;
    //
    QVector3D centerPosition;
    //
    //float aspectRatio;


signals:

public slots:

};

#endif // CUTOUT_H
