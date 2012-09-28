#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QHBoxLayout>

#include "CameraThread.h"
#include "OverlayWidget.h"
#include "GLWidget.h"
#include "ScrollArea.h"
#include "ImageItem.h"
#include "SceneSelector.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QApplication *application, CameraThread * cameraThread, QWidget *parent = 0);

protected:
    //void closeEvent( QCloseEvent * event);
    void mouseDoubleClickEvent ( QMouseEvent * event );

private:
     QApplication *app;
     VScrollArea *scrollArea;     
     OverlayWidget *overlay;
     QHBoxLayout *cameraTabLayout;
     GLWidget *openGlTab;
     SceneSelector * sceneSelector;

signals:
     //void mainWindowClose();

public slots:
     void newCutOut();
     void newPhoto();
     void newImage();
     void jumpToCamera();
     void toggleSceneSelector();
     void closeSceneSelectorOnSlide();

};

#endif // MAINWINDOW_H
