#ifndef CUTLABEL_H
#define CUTLABEL_H

#include <QLabel>
//#include <QImage>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPolygon>

#include "ImageItem.h"

const int FREE = 0;
const int RULER = 1;
const int TRANSLATE = 2;

class CutLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CutLabel(QWidget *parent = 0);
    QMutex mutex;

protected:
    void paintEvent(QPaintEvent *event);
    /*
    void dragEnterEvent ( QDragEnterEvent * event );
    void dragMoveEvent ( QDragMoveEvent * event );
    */
    void mousePressEvent ( QMouseEvent * event );
    void mouseMoveEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );

private:
    QPolygon currentPolygon;
    int toolMode;

    float mouseX, mouseY;
    float zoomLevel;
    QPoint scroll;

    QString fileName;

signals:
 void noSelection();
 void newSelection();
 void newCutout(QImage img, float x, float y, float imgW, float imgH, float cutW, float cutH, float w, float h, QPoint position, QImage mask, QString file);
 void imageAvailable();

public slots:
    // Hook up camera thread image signal to the review widget
    void newImage(QPixmap pmap, QString file);
    void reset();
    //delete the pixels of the current selection
    void deleteSelection();
    //send current selection to 3D scene
    void cutOutSelection();
    //change tool mode
    void setToolMode(int toolType);
    /*
    //Zoom In
    void zoomIn();
    //Zoom Out
    void zoomOut();
    */
    void zoom(float z);
};

#endif // CUTLABEL_H
