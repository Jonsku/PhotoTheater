#ifndef CUTWIDGET_H
#define CUTWIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>

#include "SettingsTree.h"
#include "AdjustmentWidget.h"
#include "ImageItem.h"
#include "CutLabel.h"

class CutWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CutWidget(QWidget *parent = 0);

private:
    // The always-visible parameter buttons, and the child widgets
    // they create when selected

    struct {
        // The always-visible button
        ParameterButton *button;

        // The sometimes-visible mode buttons
        ModeButton *pen, *ruler, *zoom, *scroll;

        // The slider for manual mode
        AdjustmentSlider *slider;
    } tool;

    struct {
        // The always-visible button
        ParameterButton *button;

        // The sometimes-visible mode buttons
        ModeButton *cutout, *remove, *clear;
    } action;
    // A list of all the parameter buttons that makes grabbing
    // the currently selected button easier. Note that this is
    // not a QWidget (it's more like a std::vector of button pointers).
    QButtonGroup * parameterButtonGroup;

    CutLabel *imageLabel;
    int toolMode;

protected:
    virtual void paintEvent(QPaintEvent * event);
    bool uiChangedSuppressed;

signals:
    // A photograph was taken
    void loadImage(QPixmap pix, QString file);
    void newCutout(QImage img, float x, float y, float imgW, float imgH, float cutW, float cutH, float w, float h, QPoint position, QImage mask, QString file);
    void setToolMode(int toolType);

public slots:
    // The UI changed, need to update the underlying parameters
    void uiChanged();

    void noSelection();
    void newSelection();

    // Hook up camera thread image signal to the review widget
    void imageAvailable();
    //void saveImage(ImageItem *);
    void addCutout(QImage img, float x, float y, float imgW, float imgH, float cutW, float cutH, float w, float h, QPoint position, QImage mask, QString file);
    void newImage(QPixmap pix, QString file);

};

#endif // CUTWIDGET_H
