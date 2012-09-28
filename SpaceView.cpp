#include "SpaceView.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#define TREE_WIDTH 11

//
/*
QLabel * savingNotice = new QLabel("Saving...", this);
savingNotice->setAutoFillBackground(TRUE);
savingNotice->setAlignment(Qt::AlignCenter);
savingNotice->setGeometry(glWidget->width()/3,glWidget->height()/2-16,glWidget->width()/3, 32);
savingNotice->hide();
QObject::connect(glWidget, SIGNAL(saving()), savingNotice, SLOT(show()) );
QObject::connect(glWidget, SIGNAL(doneSaving()), savingNotice, SLOT(hide()) );
*/

SpaceView::SpaceView(QWidget *parent) :
        QWidget(parent), uiChangedSuppressed(true)
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    glWidget = new GLWidget(this);
    glWidget->setFixedSize(640, 480);
    //glWidget->setGeometry(0,0,640,480);
    hLayout->addWidget(glWidget);
    QObject::connect(glWidget, SIGNAL(cutOutSelected(bool)),
                     this, SLOT(cutOutSelected(bool)));

    QObject::connect(glWidget, SIGNAL(wallSelected()),
                         this, SLOT(wallSelected()));

    //change current operation
    QObject::connect(this, SIGNAL(setOperation(int)),
                     glWidget, SLOT(setOperation(int)));

    // Make the button box
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addStrut(800-640);
    vLayout->setSpacing(0);

    vLayout->addStretch(1);

    vLayout->addWidget(selectButton = new ParameterButton("Select", this));
    vLayout->addWidget(camera.button     = new ParameterButton("View", this));
    vLayout->addWidget(transform.button     = new ParameterButton("Transform", this));
    vLayout->addWidget(walls.button     = new ParameterButton("As wall", this));
    vLayout->addWidget(file.button     = new ParameterButton("File", this));



    camera.orbit = new ModeButton("Orbit", "O:", this, camera.button, glWidget);
    camera.zoom = new ModeButton("Zoom", "Z:", this, camera.button, glWidget);
    camera.button->group.setExclusive(FALSE);
    camera.zoom->setChecked(FALSE);

    transform.remove = new ModeButton("Delete", "D:", this, transform.button, glWidget);
    transform.movexy = new ModeButton("Move(X/Y)", "M:", this, transform.button, glWidget);
    transform.movez = new ModeButton("Move(Z)", "Z:", this, transform.button, glWidget);
    transform.scale = new ModeButton("Scale", "S:", this, transform.button, glWidget);
    transform.rotate = new ModeButton("Rotate", "R:", this, transform.button, glWidget);
    transform.button->group.setExclusive(FALSE);
    transform.rotate->setChecked(FALSE);

    walls.floor = new ModeButton("Floor", "F:", this, walls.button, glWidget);
    walls.back = new ModeButton("Back", "B:", this, walls.button, glWidget);
    walls.right = new ModeButton("Right", "R:", this, walls.button, glWidget);
    walls.left = new ModeButton("Left", "L:", this, walls.button, glWidget);
    walls.button->group.setExclusive(FALSE);
    walls.left->setChecked(FALSE);

    file.upload = new ModeButton("Screenshot", "U:", this, file.button, glWidget);
    file.load = new ModeButton("Load", "L:", this, file.button, glWidget);
    file.save = new ModeButton("Save", "S:", this, file.button, glWidget);
    file.reset = new ModeButton("New", "N:", this, file.button, glWidget);
    file.button->group.setExclusive(FALSE);
    file.reset->setChecked(FALSE);


    parameterButtonGroup = new QButtonGroup();
    parameterButtonGroup->setExclusive(FALSE);


    parameterButtonGroup->addButton(walls.button);
    parameterButtonGroup->addButton(transform.button);
    parameterButtonGroup->addButton(selectButton);
    parameterButtonGroup->addButton(camera.button);
    parameterButtonGroup->addButton(file.button);

    QObject::connect(selectButton, SIGNAL(clicked()),
                     this, SLOT(uiChanged()));
    QObject::connect(parameterButtonGroup, SIGNAL(buttonPressed(QAbstractButton *)),
                     this, SLOT(update()));
    QObject::connect(parameterButtonGroup, SIGNAL(buttonReleased(QAbstractButton *)),
                     this, SLOT(update()));

    selectButton->show();
    // selectButton->setCheckable(true);
    selectButton->setChecked(FALSE);

    walls.button->hide();
    walls.button->setChecked(FALSE);

    transform.button->hide();
    transform.button->setChecked(FALSE);

    file.button->setChecked(FALSE);


    hLayout->addLayout(vLayout);

    this->setLayout(hLayout);
    uiChangedSuppressed = false;
    this->uiChanged();
}

void SpaceView::uiChanged() {
    if (uiChangedSuppressed) return;
    qDebug() << "space view, uiChanged";

    //Select
    if(selectButton->isChecked() ){
        qDebug() << "select pressed";
        /*
        uiChangedSuppressed = true;
        transform.movexy->setChecked(FALSE);
        transform.movez->setChecked(FALSE);
        transform.scale->setChecked(FALSE);
        transform.rotate->setChecked(FALSE);
        camera.orbit->setChecked(FALSE);
        camera.zoom->setChecked(FALSE);
        uiChangedSuppressed = false;
        */
        emit setOperation(OPERATION_SELECT);
    }

        //Transform
        if(transform.button->isChecked()){
            transform.button->click();
            if(transform.remove->isChecked()){
                glWidget->deleteCutout();
                transform.remove->setChecked(FALSE);
            }else if(transform.movexy->isChecked()){
                emit setOperation(OPERATION_MOVE);
                transform.movexy->setChecked(FALSE);
            }else if(transform.movez->isChecked()){
                emit setOperation(OPERATION_MOVE_Z);
                transform.movez->setChecked(FALSE);
            }else if(transform.scale->isChecked()){
                emit setOperation(OPERATION_SCALE);
                transform.scale->setChecked(FALSE);
            }else if(transform.rotate->isChecked()){
                emit setOperation(OPERATION_ROTATE);
                transform.rotate->setChecked(FALSE);
            }

        }

        //Camera
        if(camera.button->isChecked()){
            camera.button->click();
            if(camera.orbit->isChecked()){
                emit setOperation(OPERATION_ORBIT);
                camera.orbit->setChecked(FALSE);
            }else if(camera.zoom->isChecked()){
                emit setOperation(OPERATION_ZOOM);
                camera.zoom->setChecked(FALSE);
            }

        }

        //Walls
        if(walls.button->isChecked()){
            walls.button->click();
            if(walls.floor->isChecked()){
                emit setOperation(SET_FLOOR);
                walls.floor->setChecked(FALSE);
            }else if(walls.back->isChecked()){
                emit setOperation(SET_BACK);
                walls.back->setChecked(FALSE);
            }else if(walls.right->isChecked()){
                emit setOperation(SET_RIGHT);
                walls.right->setChecked(FALSE);
            }else if(walls.left->isChecked()){
                emit setOperation(SET_LEFT);
                walls.left->setChecked(FALSE);
            }

        }

        //File
        if(file.button->isChecked()){
            file.button->click();
            if(file.load->isChecked()){
                emit loadScene();
                file.load->setChecked(FALSE);
            }else if(file.save->isChecked()){
                glWidget->save();
                file.save->setChecked(FALSE);
            }else if(file.upload->isChecked()){
                glWidget->upload();
                file.upload->setChecked(FALSE);
            }else if(file.reset->isChecked()){
                glWidget->reset();
                file.reset->setChecked(FALSE);
            }

        }

    /*
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
*/
    this->update();
    //repaint();
    //uiChangedSuppressed = false;
    //updateParameterLabels();
}


GLWidget * SpaceView::getGlWidget(){
    return glWidget;
}

void SpaceView::wallSelected(){
    cutOutSelected(true);
    walls.button->hide();
    walls.button->setChecked(FALSE);
    transform.rotate->setDisabled(true);
    transform.movexy->setText("Freq. H");
    transform.movez->setText("Freq. V");
}

void SpaceView::cutOutSelected(bool selected){

    if(selected){
        walls.button->show();
        walls.button->setChecked(FALSE);

        transform.button->show();
        transform.button->setChecked(FALSE);
        transform.rotate->setDisabled(false);
        //transform.movez->setDisabled(false);
        transform.movexy->setText("Move(X/Y)");
        transform.movez->setText("Move(Z)");

        //selectButton->setText("Deselect");
        selectButton->setChecked(FALSE);
        /*
       orbitButton->hide();
       zoomButton->hide();

       deleteButton->show();

       selectButton->show();
       selectButton->setChecked(false);

       moveButton->show();
       moveButton->setChecked(true);

       moveZButton->show();
       moveZButton->setChecked(false);

       scaleButton->show();
       scaleButton->setChecked(false);

       rotateButton->show();
       rotateButton->setChecked(false);

       emit setOperation(OPERATION_MOVE);
       */
    }else{
        walls.button->hide();
        walls.button->setChecked(FALSE);

        transform.button->hide();
        transform.button->setChecked(FALSE);
        selectButton->setText("Select");
        selectButton->setChecked(FALSE);
        /*
       orbitButton->show();
       orbitButton->setChecked(false);
       zoomButton->show();
       zoomButton->setChecked(false);

       deleteButton->hide();
       selectButton->setChecked(true);
       moveButton->hide();
       moveZButton->hide();
       scaleButton->hide();
       rotateButton->hide();
       */
    }
    this->update();
}


void SpaceView::paintEvent(QPaintEvent * event) {
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

    int viewWidth = glWidget->width();

    int centerX  = viewWidth +TREE_WIDTH/2;

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
        painter.drawLine(QPoint(centerX, parameterY), QPoint(viewWidth +TREE_WIDTH, parameterY));


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

            painter.drawLine(QPoint(viewWidth, buttonY), QPoint(centerX, buttonY));
            minY = qMin(buttonY, minY);
            maxY = qMax(buttonY, maxY);

        }


        painter.setPen(passivePen);
        painter.drawLine(QPoint(centerX, minY), QPoint(centerX,maxY));
        painter.setPen(activePen);
        painter.drawLine(QPoint(centerX, minSelectedY), QPoint(centerX, maxSelectedY));

        // qDebug() << centerX << ", " << minY << ", " << maxY;
        // qDebug() << centerX << ", " << minSelectedY << ", " << maxSelectedY;
    }

}
