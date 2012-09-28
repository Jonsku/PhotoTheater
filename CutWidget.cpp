#include "CutWidget.h"

#include <QtDebug>
#include <QGridLayout>
#include <QFrame>

#include "CutLabel.h"

#define TREE_WIDTH 11

CutWidget::CutWidget(QWidget *parent) :
   QWidget(parent), uiChangedSuppressed(true)
{
    toolMode = FREE;
    QFrame *frame = new QFrame(this);
    //backgorund color for alpha chanel debug purposes
    /*
    QPalette palette = frame->palette();
    palette.setColor( backgroundRole(), QColor( 0, 0, 255 ) );
    frame->setPalette( palette );
    frame->setAutoFillBackground( true );
    */

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    imageLabel = new CutLabel(this);
    imageLabel->setFixedSize(640, 480);

    hLayout->addWidget(imageLabel);

    // Make the button box
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addStrut(800-640);
    vLayout->setSpacing(0);

    vLayout->addStretch(1);
    vLayout->addWidget(tool.button     = new ParameterButton("Tools", this));

    vLayout->addWidget(action.button     = new ParameterButton("Selection", this));





    tool.zoom = new ModeButton("Zoom", "Z:", this, tool.button, this);
    tool.scroll = new ModeButton("Scroll", "S:", this, tool.button, this);
    tool.ruler = new ModeButton("Ruler", "R:", this, tool.button, this);
    tool.pen = new ModeButton("Pen", "P:", this, tool.button, this);
    tool.slider    = new AdjustmentSlider("50%", "300%", this, tool.zoom, this);

    action.cutout = new ModeButton("Cut out", "C:", this, action.button, this);
    action.remove = new ModeButton("Delete", "D:", this, action.button, this);
    action.clear = new ModeButton("Clear", "R:", this, action.button, this);

    action.button->group.setExclusive(FALSE);
    action.clear->setChecked(FALSE);


    parameterButtonGroup = new QButtonGroup();
    parameterButtonGroup->setExclusive(FALSE);


    parameterButtonGroup->addButton(action.button);
    parameterButtonGroup->addButton(tool.button);


    QObject::connect(parameterButtonGroup, SIGNAL(buttonPressed(QAbstractButton *)),
                     this, SLOT(update()));
    QObject::connect(parameterButtonGroup, SIGNAL(buttonReleased(QAbstractButton *)),
                     this, SLOT(update()));

    action.button->hide();
    action.button->setChecked(FALSE);

    tool.button->hide();
    tool.button->setChecked(FALSE);

    hLayout->addLayout(vLayout);
    frame->setLayout(hLayout);


    uiChangedSuppressed = false;


    this->uiChanged();

    // Hook up camera thread image signal to the review widget
    QObject::connect(this, SIGNAL(loadImage(QPixmap, QString)),
                     imageLabel, SLOT(newImage(QPixmap, QString)) );

    QObject::connect(imageLabel, SIGNAL(imageAvailable()),
                     this, SLOT(imageAvailable()) );

    //
    QObject::connect(imageLabel, SIGNAL(newSelection()),
                     this, SLOT(newSelection()));

    //
    QObject::connect(imageLabel, SIGNAL(noSelection()),
                     this, SLOT(noSelection()));

    //change current operation
    QObject::connect(this, SIGNAL(setToolMode(int)),
                     imageLabel, SLOT(setToolMode(int)));
    //
    QObject::connect(imageLabel, SIGNAL(newCutout(QImage, float, float, float, float, float, float, float , float, QPoint, QImage, QString)),
                     this, SLOT(addCutout(QImage, float, float, float, float, float , float, float, float, QPoint, QImage, QString)) );

}

void CutWidget::paintEvent(QPaintEvent * event) {
    QWidget::paintEvent(event);

    //printf("viewfinder paint event\n");
    // Horrible hack to get the tree drawn
    QPainter painter(this);
    QPalette palette;

    QPen activePen(painter.pen());
    activePen.setWidth(3);
    activePen.setColor(palette.color(QPalette::Highlight));
    QPen passivePen(painter.pen());
    passivePen.setWidth(3);
    passivePen.setColor(QColor(64,64,64));



    int centerX  = imageLabel->width()+TREE_WIDTH/2;

    ParameterButton * selectedParameterButton = NULL;
    foreach (QAbstractButton * button, parameterButtonGroup->buttons()) {
        if (button->isVisible() && (button->isDown() || button->isChecked())) {
            selectedParameterButton = (ParameterButton *) button;
            break;
        }
    }

    if (selectedParameterButton){
        int parameterY = selectedParameterButton->geometry().center().y();
        int minSelectedY = parameterY;
        int maxSelectedY = parameterY;
        int minY = parameterY;
        int maxY = parameterY;
        // hash mark for parameter button
        painter.setPen(activePen);
        painter.drawLine(QPoint(centerX, parameterY), QPoint(imageLabel->width()+TREE_WIDTH, parameterY));

        // hash marks for mode buttons
        foreach (QAbstractButton * modeButton, selectedParameterButton->group.buttons()){
            int buttonY = modeButton->geometry().center().y();
            if (modeButton->isChecked() || modeButton->isDown()) {
                painter.setPen(activePen);
                minSelectedY = qMin(buttonY, minSelectedY);
                maxSelectedY = qMax(buttonY, maxSelectedY);
            } else {
                painter.setPen(passivePen);
            }
            painter.drawLine(QPoint(imageLabel->width(), buttonY), QPoint(centerX, buttonY));
            minY = qMin(buttonY, minY);
            maxY = qMax(buttonY, maxY);
        }
        painter.setPen(passivePen);
        painter.drawLine(QPoint(centerX, minY), QPoint(centerX,maxY));
        painter.setPen(activePen);
        painter.drawLine(QPoint(centerX, minSelectedY), QPoint(centerX, maxSelectedY));
    }

}

void CutWidget::imageAvailable(){
    /*
    zoomInButton->show();
   zoomOutButton->show();
   penButton->show();
   rulerButton->show();
   */
    tool.button->show();
    tool.pen->setChecked(true);
     emit setToolMode(FREE);
    //emit newImage(img);
}

void CutWidget::addCutout(QImage img, float x, float y, float imgW, float imgH, float cutW, float cutH, float w, float h, QPoint pos, QImage mask, QString file){
    emit newCutout(img, x, y, imgW, imgH, cutW, cutH, w ,h, pos, mask, file);
}

void CutWidget::uiChanged() {
    if (uiChangedSuppressed) return;
    //uiChangedSuppressed = true;
    qDebug() << "cut widget, uiChanged";

    qDebug() << (action.button->isVisible()?"Action visible":"Action not visible");
    qDebug() << (tool.button->isVisible()?"Tool visible":"Tool not visible");


    //Action
    if(action.button->isChecked()){
        imageLabel->mutex.lock();
        if(action.clear->isChecked()){
            imageLabel->reset();
            action.button->click();
            action.clear->setChecked(FALSE);


        }else if(action.cutout->isChecked()){
            imageLabel->cutOutSelection();
            action.button->click();
            //imageLabel->reset();

            action.cutout->setChecked(FALSE);
            //action.button->click();
        }else if(action.remove->isChecked()){

            imageLabel->deleteSelection();
            action.button->click();
            //imageLabel->reset();
            action.remove->setChecked(FALSE);
            //action.button->click();
            //action.button->hide();
        }
        imageLabel->mutex.unlock();
        qDebug() << "Action: OK";
    }


    //Tool
    if(tool.button->isChecked()){
        if(tool.pen->isChecked()){
            emit setToolMode(FREE);
            tool.button->click();
        }else if(tool.ruler->isChecked()){
             emit setToolMode(RULER);
              tool.button->click();
        }else if(tool.scroll->isChecked()){
            emit setToolMode(TRANSLATE);
             tool.button->click();
        }else if(tool.zoom->isChecked()){
            emit setToolMode(TRANSLATE);
            float z =tool.slider->value()/1000.0f;
            z = 2.0f - (2.0f - 0.03125f)*z;
            imageLabel->zoom(z);
        }
        qDebug() << "Tool: OK";
    }

    this->update();
    //repaint();
    //uiChangedSuppressed = false;
    //updateParameterLabels();
}


void CutWidget::newSelection(){
    action.button->show();
}

void CutWidget::noSelection(){
    action.button->hide();
}

void CutWidget::newImage(QPixmap pix, QString file){
    emit loadImage(pix, file);
}





