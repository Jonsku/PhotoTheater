#ifndef SCENESELECTOR_H
#define SCENESELECTOR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include "ScrollArea.h"
#include "Scene.h"

class SceneSelector: public QWidget
{
    Q_OBJECT
public:
    explicit SceneSelector(QWidget *parent = 0);
protected:
    // The scrollable review area
    HScrollArea *scrollArea;

    // Three labels contained within it
    // Most of the time these labels are displaying thumbnail pixmaps.
    QLabel *leftLabel, *middleLabel, *rightLabel;
    QPushButton * selectButton;

    // The index of the currently visible photo
    int current;
    std::vector<Scene *> scenes;
private:
    // The label that shows the current scene index (e.g. "7/9")
    QLabel * sceneIndexLabel;
    void manageMemory();

signals:
    void sceneSelected(Scene *);

public slots:
    void handleScrolled(int idx);
    void loadSceneThumbnail(Scene * scene);
    void updateThumbnails();
    void selectScene();
    void addScene(Scene * scene);
};


#endif // SCENESELECTOR_H
