#include <QtDebug>
#include <QPushButton>
#include <QProgressBar>

#include "MainWindow.h"
#include "CutWidget.h"
#include "ScrollArea.h"
#include "SpaceView.h"
#include "ViewFinder.h"
#include "ThumbnailView.h"




MainWindow::MainWindow(QApplication *application, CameraThread *cameraThread, QWidget *parent) : QMainWindow(parent)
{
    app = application;
    scrollArea = new VScrollArea(this);
    scrollArea->setGeometry(0, 0, 800, 480);

    Viewfinder *viewfinder = new Viewfinder(cameraThread);
    SpaceView *spaceView= new SpaceView();
    openGlTab = spaceView->getGlWidget();
    CutWidget *cutWidget = new CutWidget();
    ThumbnailView *review = new ThumbnailView();
    sceneSelector = new SceneSelector();

    scrollArea->addWidget(viewfinder);
    scrollArea->addWidget(spaceView);
    scrollArea->addWidget(cutWidget);
    scrollArea->addWidget(review);
    //makes camera the default view
    scrollArea->jumpTo(1);
    //scrollArea->jumpTo(0);


    /*
    // Hook up the quit button to stop the camera thread
    QObject::connect(this, SIGNAL(mainWindowClose()),
                     cameraThread, SLOT(stop()));
    */
    //display scene selector when load button of space view is pressed
    QObject::connect(spaceView,SIGNAL(loadScene()),this,SLOT(toggleSceneSelector()));

    QObject::connect(scrollArea,SIGNAL(slidTo(int)),this,SLOT(closeSceneSelectorOnSlide()));

    // Hook up camera thread image signal to the review widget
    QLabel * loading = new QLabel("Loading ...",openGlTab);
    loading->setGeometry(40,240-32,540,64);
    //lensCoverWarning->setGeometry(0,0,640,480);
    loading->setAutoFillBackground(true);
    loading->setAlignment(Qt::AlignCenter);
    loading->setVisible(false);

    QLabel * saving = new QLabel("Saving ...",openGlTab);
    saving->setGeometry(40,240-32,540,64);
    //lensCoverWarning->setGeometry(0,0,640,480);
    saving->setAutoFillBackground(true);
    saving->setAlignment(Qt::AlignCenter);
    saving->setVisible(false);

    //hide show progress bar



    QObject::connect(openGlTab, SIGNAL(doneSaving(Scene *)),sceneSelector, SLOT(addScene(Scene *)));

    //When a scene has been chosen, move back to the stage view and start loading
    QObject::connect(sceneSelector,SIGNAL(sceneSelected(Scene *)),
                    loading, SLOT(show()));

    QObject::connect(sceneSelector,SIGNAL(sceneSelected(Scene *)),this,SLOT(toggleSceneSelector()));

    QObject::connect(sceneSelector,SIGNAL(sceneSelected(Scene *)),spaceView,SLOT(repaint()));
    QObject::connect(sceneSelector,SIGNAL(sceneSelected(Scene *)),openGlTab,SLOT(repaint()));
    QObject::connect(sceneSelector,SIGNAL(sceneSelected(Scene *)),openGlTab,SLOT(load(Scene *)));

    QObject::connect(openGlTab, SIGNAL(sceneLoaded(Scene *)), loading, SLOT(hide()));
    QObject::connect(openGlTab, SIGNAL(sceneLoaded(Scene *)), spaceView,SLOT(repaint()));

    QObject::connect(cameraThread, SIGNAL(newImage(ImageItem *)),
                     review, SLOT(newImage(ImageItem *)) );

    //Saving is starting
    QObject::connect(openGlTab,SIGNAL(saving()), saving, SLOT(show()));
    QObject::connect(openGlTab,SIGNAL(saving()),spaceView,SLOT(repaint()));

    QObject::connect(openGlTab,SIGNAL(saveFailed()), saving, SLOT(hide()));
    QObject::connect(openGlTab,SIGNAL(doneSaving(Scene *)), saving, SLOT(hide()));
    QObject::connect(openGlTab, SIGNAL(saveFailed()), spaceView,SLOT(repaint()));
    QObject::connect(openGlTab, SIGNAL(doneSaving(Scene *)), spaceView,SLOT(repaint()));

    // Hook up camera thread image signal to the cut out widget

    QObject::connect(review, SIGNAL(newImage(QPixmap, QString)),
                     cutWidget, SLOT(newImage(QPixmap, QString)) );

    QObject::connect(cutWidget, SIGNAL(loadImage(QPixmap, QString)),
                     this, SLOT(newImage()) );

    //jump to camera label
    QObject::connect(cameraThread, SIGNAL(focusPressed()),
                     this, SLOT(jumpToCamera()) );


    //jump to cutout label
    QObject::connect(cameraThread, SIGNAL(newImage(ImageItem *)),
                     this, SLOT(newPhoto()) );

    //

    QObject::connect(cutWidget, SIGNAL(newCutout(QImage, float, float, float, float, float, float, float, float, QPoint, QImage, QString)),
                     openGlTab, SLOT(newCutout(QImage, float, float, float, float, float, float, float, float, QPoint, QImage, QString)) );
    //
    QObject::connect(cutWidget, SIGNAL(newCutout(QImage, float, float, float, float, float, float, float, float, QPoint, QImage, QString)),
                     this, SLOT(newCutOut()));

    // Once the camera thread stops, quit the app
    QObject::connect(cameraThread, SIGNAL(finished()),
                     app, SLOT(quit()));

}

/*
void MainWindow::closeEvent ( QCloseEvent * event ){
    qDebug() << "Main Window: close";
    emit mainWindowClose();
    event->accept();
}
*/

void MainWindow::mouseDoubleClickEvent ( QMouseEvent * event ){
    qDebug() << "Main Window: double click";

    if(isFullScreen()){
        showNormal();
    }else{
        showFullScreen();
    }

    //reset viewfinder
    //overlay->resize(this->size());


    //emit mainWindowClose();
    event->accept();
}

void MainWindow::newCutOut(){
    qDebug() << "Main Window: new Cutout";
    scrollArea->jumpTo(1);
}

void MainWindow::newPhoto(){
    qDebug() << "Main Window: new Photo";
    scrollArea->jumpTo(3);
}


void MainWindow::newImage(){
    qDebug() << "Main Window: new Image";
    scrollArea->jumpTo(2);
}

void MainWindow::jumpToCamera(){
    scrollArea->jumpTo(0);
}

void MainWindow::toggleSceneSelector(){
    if(scrollArea->numberOfWidgets() > 4){
        scrollArea->removeWidget(4);
        scrollArea->jumpTo(1);
    }else{
        scrollArea->addWidget(sceneSelector);
        scrollArea->jumpTo(4);
    }
}

void MainWindow::closeSceneSelectorOnSlide(){
    if(scrollArea->numberOfWidgets() > 4){
        scrollArea->removeWidget(4);
        scrollArea->jumpTo(1);
    }
}
