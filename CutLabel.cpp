#include <QtDebug>
#include <QPainter>
#include <QColor>
#include <QPixmap>
#include <QImage>
#include "Utilies.h"
#include "CutLabel.h"

CutLabel::CutLabel(QWidget *parent) :
        QLabel("",parent)
{
    toolMode = TRANSLATE;
    setText("No photos selected. Slide down to select one!");
    zoomLevel = 1.0f;
}

void CutLabel::paintEvent(QPaintEvent *event)
{

    QLabel::paintEvent(event);
    if(this->pixmap()==NULL)
        return;
    QPainter painter(this);

    //apply zoom
    // qDebug() << "zoomLevel is" << zoomLevel;
    painter.fillRect(0,0,width(),height(),QColor(0,0,0,255));
    if(zoomLevel <= 1.0f){


        QRect zoomRect((width()*0.5f)*(1.0f-zoomLevel)+scroll.x(), (height()*0.5f)*(1.0f-zoomLevel)+scroll.y(), width()*zoomLevel, height()*zoomLevel);
        QPixmap zoomed = this->pixmap()->copy(zoomRect).scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(0,0,zoomed);
    }else{
        QRect zoomRect(0, 0, width(), height());
        QPixmap zoomed = this->pixmap()->copy(zoomRect).scaled(width()*1.0f/zoomLevel, height()*1.0f/zoomLevel, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(width()/2.0-zoomed.width()/2.0f,height()/2.0-zoomed.height()/2.0f,zoomed);
    }

    painter.translate(width()*0.5,height()*0.5);
    painter.scale(1.0/zoomLevel, 1.0/zoomLevel);
    painter.translate(-(width()*0.5+scroll.x()),-(height()*0.5+scroll.y()));


    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::yellow, 5*zoomLevel, Qt::DashDotLine, Qt::RoundCap));
    painter.setBrush(QBrush(QColor(0,0,255,70), Qt::SolidPattern));
    if(currentPolygon.size()>0){

        painter.drawPolygon(currentPolygon);
        painter.setPen(QPen(Qt::white, 10*zoomLevel, Qt::DashDotLine, Qt::RoundCap));
        painter.drawPoint(currentPolygon.last());
    }
    //painter.end();

    update();
}

void CutLabel::reset(){
    qDebug() << "reset called";
    currentPolygon.clear();
    emit noSelection();
}

void CutLabel::newImage(QPixmap pmap, QString file){
    fileName = file;
    qDebug() << fileName;
    qDebug() << "Cut out pixmap is " << pmap.width() << " x " << pmap.height();
    setPixmap(pmap);
    emit imageAvailable();
    zoomLevel = 1.0f;
    scroll.setX(0);
    scroll.setY(0);
    reset();
}

/*
void CutLabel::dragEnterEvent ( QDragEnterEvent * event ){
    qDebug() << "drag (enter):" << event->pos().x() << ":" << event->pos().y();
}

void CutLabel::dragMoveEvent( QDragMoveEvent * event ){
    qDebug() << "drag:" << event->pos().x() << ":" << event->pos().y();
}
*/

void CutLabel::mouseMoveEvent( QMouseEvent * event ){
    // qDebug() << "Label: mouse move";
    QPoint transformedMouse(event->pos().x(), event->pos().y());
    transformedMouse -= QPoint(width()*0.5,height()*0.5);
    transformedMouse *= zoomLevel;
    transformedMouse += QPoint(width()*0.5,height()*0.5);

    QRect viewRect((width()*0.5f)*(1.0f-zoomLevel)+(scroll.x()), (height()*0.5f)*(1.0f-zoomLevel)+(scroll.y()), width()*zoomLevel, height()*zoomLevel);
    QPoint tPos(viewRect.x()+event->pos().x()*zoomLevel,viewRect.y()+event->pos().y()*zoomLevel);
    //QPoint tPos(viewRect.x()+(event->pos().x()/(float)width())*viewRect.width(),viewRect.y()+(event->pos().y()/(float)height())*viewRect.height());
    if(tPos.x()<0){
        tPos.setX(0);
    }
    if(tPos.x()>width()){
        tPos.setX(width());
    }
    if(tPos.y()<0){
        tPos.setY(0);
    }
    if(tPos.y()>height()){
        tPos.setX(height());
    }
    if(toolMode == FREE){


        currentPolygon << tPos;//transformedMouse; //event->pos();
        //qDebug() << "Free" << currentPolygon.size();
        //qDebug() << tPos.x() << ", " << tPos.y();
    }else if(toolMode == RULER){
        currentPolygon.setPoint(currentPolygon.size()-1,tPos);//transformedMouse); //event->pos());
        //qDebug() << "Ruler " << currentPolygon.size();
    }else if(toolMode == TRANSLATE && zoomLevel<1.0){
        //TRANSLATE

        qDebug() << "Translating";

        float sX = mouseX-transformedMouse.x();
        float sY = mouseY-transformedMouse.y();

        if(viewRect.x()+sX+viewRect.width()<=width() && viewRect.x()+sX>=0 && viewRect.y()+sY+viewRect.height()<=height() && viewRect.y()+sY>=0){
            scroll.setX(scroll.x()+sX);
            scroll.setY(scroll.y()+sY);
        }
    }
    mouseX = transformedMouse.x();
    mouseY = transformedMouse.y();
}

void CutLabel::mousePressEvent ( QMouseEvent * event ){
    QPoint transformedMouse(event->pos().x(), event->pos().y());
    transformedMouse -= QPoint(width()*0.5,height()*0.5);
    transformedMouse *= zoomLevel;
    transformedMouse += QPoint(width()*0.5,height()*0.5);
    mouseX = transformedMouse.x();
    mouseY = transformedMouse.y();

    QRect viewRect((width()*0.5f)*(1.0f-zoomLevel)+(scroll.x()), (height()*0.5f)*(1.0f-zoomLevel)+(scroll.y()), width()*zoomLevel, height()*zoomLevel);
    //QPoint tPos(viewRect.x()+(event->pos().x()/(float)width())*viewRect.width(),viewRect.y()+(event->pos().y()/(float)height())*viewRect.height());

    QPoint tPos(viewRect.x()+event->pos().x()*zoomLevel,viewRect.y()+event->pos().y()*zoomLevel);
    if(tPos.x()<0){
        tPos.setX(0);
    }
    if(tPos.x()>width()){
        tPos.setX(width());
    }
    if(tPos.y()<0){
        tPos.setY(0);
    }
    if(tPos.y()>height()){
        tPos.setX(height());
    }

    if(toolMode != TRANSLATE){
        currentPolygon  << tPos;//transformedMouse;//event->pos();
        if(currentPolygon.size()==1){
            emit newSelection();
        }
    }else{
        qDebug() << "Translating";
    }
}

void CutLabel::mouseReleaseEvent ( QMouseEvent * event ){

}

void CutLabel::deleteSelection(){
    //for performance reasons, extract a part of the image which sides are power of 2
    int oWidth = currentPolygon.boundingRect().width();
    int oHeight = currentPolygon.boundingRect().height();
    int oX = currentPolygon.boundingRect().x();
    int oY = currentPolygon.boundingRect().y();

    //create a mask
    QImage *mask = new QImage(oWidth,oHeight,QImage::Format_Mono);
    mask->fill(0);

    QPainter painter(mask);           // paint in mask

    QPoint *offset = new QPoint(currentPolygon.boundingRect().topLeft());
    currentPolygon.translate(-offset->x(),-offset->y());
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
    painter.drawPolygon(currentPolygon);
    currentPolygon.translate(offset->x(),offset->y());
    painter.end();

    QImage temp = this->pixmap()->toImage().copy(0,0,width(),height()).convertToFormat(QImage::Format_ARGB32);

    for(int j=0;j<oHeight;j++){
        QRgb *row = (QRgb *)temp.scanLine(oY+j);
        for(int i=0;i<oWidth;i++){
            //Could be optimized
            if((mask->pixel(i,j) & 0xFF) != 0){
                row[oX+i] = qRgba( 0, 0, 0, 0 );
                //temp.setPixel(oX+i,oY+j,);
            }
        }
    }

    delete mask;

    //replace image with new one
    setPixmap(QPixmap::fromImage(temp));
    reset();
}

void CutLabel::cutOutSelection(){
    /*float z = zoomLevel;
    zoom(1.0f);*/

    //for performance reasons, extract a part of the image which sides are power of 2
    int oWidth = Utilies::closestBiggestPowerOfTwo(currentPolygon.boundingRect().width());
    int oHeight = Utilies::closestBiggestPowerOfTwo(currentPolygon.boundingRect().height());
    float xOffset = (oWidth-currentPolygon.boundingRect().width())/2.0f;
    int oX = currentPolygon.boundingRect().x()-qRound(xOffset);
    float yOffset = (oHeight-currentPolygon.boundingRect().height())/2.0f;
    int oY = currentPolygon.boundingRect().y()-qRound(yOffset);

    //extract selection and add an alpha channel
    QImage cutImage = this->pixmap()->toImage().copy(oX,oY,oWidth,oHeight).convertToFormat(QImage::Format_ARGB32);


    //create a mask
    QImage mask(oWidth,oHeight,QImage::Format_Mono);
    mask.fill(0);

    // paint in mask
    QPainter painter(&mask);

    QPoint *offset = new QPoint(currentPolygon.boundingRect().topLeft());
    currentPolygon.translate(-offset->x()+xOffset,-offset->y()+yOffset);
    //painter.drawRect(polygons.last().boundingRect());
    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
    painter.drawPolygon(currentPolygon);
    currentPolygon.translate(offset->x()-xOffset,offset->y()-yOffset);
    painter.end();


    /*
    for(int i=0;i<mask.width();i++){
        for(int j=0;j<mask.height();j++){
            //Could be optimized
            if((mask.pixel(i,j) & 0xFF) == 0){
                cutImage.setPixel(i,j,qRgba( 0, 0, 0, 0 ));
            }else if(qAlpha(cutImage.pixel(i,j))==0){ // && qGreen(cutImage.pixel(i,j))==0 && qBlue(cutImage.pixel(i,j))==255
                qDebug() << "It's working!";
                mask.setPixel(i,j,0);
            }
        }
    }
    */
    for(int j=0;j<mask.height();j++){
        QRgb *row = (QRgb *)cutImage.scanLine(j);
        for(int i=0;i<mask.width();i++){
            //Could be optimized
            if((mask.pixel(i,j) & 0xFF) == 0){
                row[i] = qRgba( 0, 0, 0, 0 );
                //temp.setPixel(oX+i,oY+j,);
            }else if(qAlpha(row[i]) == 0){
                mask.setPixel(i,j,0);
            }
        }
    }

    //mask = cutImage.alphaChannel();
    //delete mask;
    //zoom(z);
    //calculate 3D coordinates and texture mapping coordinates
    float xU = (oX+oWidth*0.5f)/640.0f;
    float yU = (oY+oHeight*0.5f)/480.0f;

    float wU = currentPolygon.boundingRect().width()*1.0f/640.0f;
    float hU = currentPolygon.boundingRect().height()*1.0f/480.0f;

    //used to calculate texture coordinates
    float imgW = cutImage.width();
    float imgH = cutImage.height();

    float cutW = currentPolygon.boundingRect().width();
    float cutH = currentPolygon.boundingRect().height();

    //tell the OpenGl tab
    //qDebug() << "It's working!";
    emit newCutout(cutImage, xU, yU, imgW, imgH, cutW, cutH, wU, hU, QPoint(oX,oY), mask, fileName);
    //reset();

}


void CutLabel::setToolMode(int toolType){
    toolMode = toolType;
}

void CutLabel::zoom(float z){
    zoomLevel = z;
    if(zoomLevel<0.03125f){
        zoomLevel = 0.03125f;
    }else if(zoomLevel>=1.0f){
        scroll.setX(0);
        scroll.setY(0);
    }

}
/*
void CutLabel::zoomIn(){
    zoomLevel /= 2.0;
    //constrain zoom
    if(zoomLevel<0.03125f){
        zoomLevel = 0.03125f;
    }else if(zoomLevel>=1.0f){
        scroll.setX(0);
        scroll.setY(0);
        zoomLevel = 1.0f;
    }

}

void CutLabel::zoomOut(){

    zoomLevel *= 2.0;
    //constrain zoom
    if(zoomLevel>=1.0f){
        scroll.setX(0);
        scroll.setY(0);
        zoomLevel = 1.0f;
    }
}
*/
