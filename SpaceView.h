#ifndef SPACEVIEW_H
#define SPACEVIEW_H

#include <QWidget>
#include <QPushButton>

#include "GLWidget.h"
#include "SettingsTree.h"

class SpaceView : public QWidget
{
    Q_OBJECT
public:
    explicit SpaceView(QWidget *parent = 0);
    GLWidget * getGlWidget();

protected:
    GLWidget *glWidget;
    virtual void paintEvent(QPaintEvent * event);
    bool uiChangedSuppressed;

private:
    // The always-visible parameter buttons, and the child widgets
    // they create when selected
    struct {
        // The always-visible button
        ParameterButton *button;

        // The sometimes-visible mode buttons
        ModeButton *orbit, *zoom;
    } camera;

    struct {
        // The always-visible button
        ParameterButton *button;

        // The sometimes-visible mode buttons
        ModeButton *remove, *movexy, *movez, *scale, *rotate;
    } transform;

    struct {
        // The always-visible button
        ParameterButton *button;

        // The sometimes-visible mode buttons
        ModeButton *floor, *back, *right, *left;
    } walls;

    struct {
        // The always-visible button
        ParameterButton *button;

        // The sometimes-visible mode buttons
        ModeButton *save, *load, *upload, *reset;
    } file;

    // The always-visible button
    ParameterButton *selectButton;

    // A list of all the parameter buttons that makes grabbing
    // the currently selected button easier. Note that this is
    // not a QWidget (it's more like a std::vector of button pointers).
    QButtonGroup * parameterButtonGroup;


    /*
    QPushButton *orbitButton;
    QPushButton *zoomButton;
    QPushButton *deleteButton;

    QPushButton *moveButton;
    QPushButton *moveZButton;
    QPushButton *scaleButton;
    QPushButton *rotateButton;
    */

signals:
    void setOperation(int operation);
    void loadScene();

public slots:
     void uiChanged();
     void cutOutSelected(bool selected);
     void wallSelected();
     /*
     void selectOperation();
     void moveOperation();
     void moveZOperation();
     void scaleOperation();
     void rotateOperation();
     void orbit();
     void zoom();
     */
};

#endif // SPACEVIEW_H
