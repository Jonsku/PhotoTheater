#include "SceneSelector.h"
#include "ScrollArea.h"
#include "Utilies.h"
#include "UserDefaults.h"
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

SceneSelector::SceneSelector(QWidget *parent): QWidget(parent) {
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    scrollArea = new HScrollArea(this);
    scrollArea->setFixedSize(640, 480);
    hLayout->addWidget(scrollArea);

    leftLabel = new QLabel("", this);
    middleLabel = new QLabel("", this);
    rightLabel = new QLabel("", this);

    scrollArea->addWidget(leftLabel);
    scrollArea->addWidget(middleLabel);
    scrollArea->addWidget(rightLabel);
    scrollArea->jumpTo(middleLabel);


    QObject::connect(scrollArea, SIGNAL(slidTo(int)),
                     this, SLOT(handleScrolled(int)));
    /*
    // Make the button box
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setSpacing(0);

    //QPushButton * quitButton = new QPushButton("X", this);
    //quitButton->setFlat(TRUE);
    //QObject::connect(quitButton, SIGNAL(clicked()),
    //                 cameraThread, SLOT(stop()));
    //vLayout->addWidget(quitButton);

    vLayout->addStretch(1);

    selectButton = new QPushButton("Select", this);
    selectButton->setCheckable(FALSE);
    QObject::connect(selectButton, SIGNAL(clicked()),
                     this, SLOT(selectImage()));
    vLayout->addWidget(selectButton);
    deleteButton = new QPushButton("Trash", this);
    vLayout->addWidget(deleteButton);
    QObject::connect(deleteButton, SIGNAL(clicked()),
                     this, SLOT(trashSelectedPhoto()));
*/
    // Make the button box
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setSpacing(0);

    vLayout->addStretch(1);
    selectButton = new QPushButton("Load", this);
    selectButton->setCheckable(FALSE);
    QObject::connect(selectButton, SIGNAL(clicked()),
                     this, SLOT(selectScene()));
    vLayout->addWidget(selectButton);

    vLayout->addStretch(1);
    sceneIndexLabel = new QLabel("0/0");
    vLayout->addWidget(sceneIndexLabel);

    hLayout->addLayout(vLayout);

    this->setLayout(hLayout);

    QObject::connect(&SceneIOThread::reader(), SIGNAL(loadFinished(Scene *)),
                     this, SLOT(loadSceneThumbnail(Scene *)));

    UserDefaults &userDefaults = UserDefaults::instance();
    QString directoryPath = userDefaults["rawPath"].asString().c_str();
    QDir dir(directoryPath);
    QStringList files = dir.entryList();
    for (int i = 0; i < files.size(); i++) {
        QString f = files.at(i);
        if (f.endsWith(".pht")) {
            scenes.push_back(new Scene(f));
        }
    }

    if (scenes.size() == 0) {
        scenes.push_back(new Scene());
    }

    // The current viewed image is the most recent one
    current = scenes.size() - 1;

    // Prefetch the first two images in the background.
    for (int i = 1; i < 3; i++) {
        int idx = scenes.size() - i;
        while (idx < 0) idx += scenes.size();
        scenes[idx]->loadThumbnailAsync();
    }




    this->updateThumbnails();

/*
    vLayout->addStretch(1);
    photoIndexLabel = new QLabel("0/0");
    vLayout->addWidget(photoIndexLabel);

    hLayout->addLayout(vLayout);

    this->setLayout(hLayout);

    QObject::connect(&IOThread::writer(), SIGNAL(saveFinished(ImageItem *)),
                     this, SLOT(updateThumbnails()));

    QObject::connect(&IOThread::reader(), SIGNAL(loadFinished(ImageItem *)),
                     this, SLOT(updateThumbnails()));

    UserDefaults &userDefaults = UserDefaults::instance();
    QString directoryPath = userDefaults["rawPath"].asString().c_str();
    QDir dir(directoryPath);
    QStringList files = dir.entryList();
    for (int i = 0; i < files.size(); i++) {
        QString f = files.at(i);
        if (f.endsWith(".dng")) {
            images.push_back(new ImageItem(directoryPath + files.at(i)));
        }
    }

    if (images.size() == 0) {
        images.push_back(new ImageItem());
    }

    // The current viewed image is the most recent one
    current = images.size() - 1;

    // Prefetch the first two images in the background.
    for (int i = 1; i < 3; i++) {
        int idx = images.size() - i;
        while (idx < 0) idx += images.size();
        images[idx]->loadThumbnailAsync();
    }




    this->updateThumbnails();
    */
}

void SceneSelector::handleScrolled(int idx) {
    switch (idx) {
    case 0: // scrolled left
        current--;
        if (current < 0) current = scenes.size()-1;
        break;
    case 1: // scrolled to middle
        return;
        break;
    case 2: // scrolled right
        current++;
        if (current == (int)scenes.size()) current = 0;
        break;
    }

    scrollArea->jumpTo(middleLabel);
    updateThumbnails();
}

void SceneSelector::updateThumbnails(){
    // show the appropriate pixmaps
    size_t indices[3];
    for (int i = 0; i < 3; i++) {
        indices[i] = current + i + scenes.size() - 1;
        while (indices[i] >= scenes.size()) indices[i] -= scenes.size();
    }
    if (scenes.size() == 1 && scenes[0]->isNew()) {
        sceneIndexLabel->setText("Scene    0\n       of    0");
    } else {
        sceneIndexLabel->setText(QString().sprintf("Scene % 4d\n       of % 4d", current+1, scenes.size()));
    }
    if (scenes[indices[0]]->isThumbnailAvailable()) {
        leftLabel->setPixmap(*scenes[indices[0]]->getThumbnail());
    } else {
        leftLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        leftLabel->setLineWidth(3);
        if (scenes[indices[0]]->isNew()) {
            leftLabel->setText("No scene found. Save one first!");
        } else if (!scenes[indices[0]]->isThumbnailAvailable()) {
            leftLabel->setText("Loading preview ...");
        } else {
            leftLabel->setText("Error loading preview ...");
        }
    }

    //deleteButton->setEnabled(images[indices[1]]->safeToDelete());
    selectButton->setEnabled(!scenes[indices[1]]->isNew());


    if (scenes[indices[1]]->isThumbnailAvailable()) {
        middleLabel->setPixmap(*scenes[indices[1]]->getThumbnail());
       // zoomableThumbnail->setImage(images[indices[1]]);
    } else {
        middleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        middleLabel->setLineWidth(3);
        if (scenes[indices[1]]->isNew()) {
            middleLabel->setText("No scene found. Save one first!");
        } else if (!scenes[indices[1]]->isThumbnailAvailable()) {
            middleLabel->setText("Loading preview ...");
        } else {
            middleLabel->setText("Error loading preview ...");
        }
    }

    if (scenes[indices[2]]->isThumbnailAvailable()) {
        rightLabel->setPixmap(*scenes[indices[2]]->getThumbnail());
    } else {
        rightLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        rightLabel->setLineWidth(3);
        if (scenes[indices[2]]->isNew()) {
            rightLabel->setText("No scene found. Save one first!");
        } else if (!scenes[indices[2]]->isThumbnailAvailable()) {
            rightLabel->setText("Loading ...");
        } else {
            rightLabel->setText("Error loading preview ...");
        }
    }

    manageMemory();
}

void SceneSelector::manageMemory() {
    // printf("about to loadthumbasync(), item name is %s\n", images[current]->fullPath().toStdString().c_str());
    //scenes[current]->loadThumbnailAsync();
    //printf("loadedthumbasync()\n");
    for (int i = 0; i < (int)scenes.size(); i++) {
        int delta = Utilies::min3(abs(current - i),
                         abs(current - i + scenes.size()),
                         abs(current - i - scenes.size()));
        if (delta <= 3 && delta > 0) {
            if (scenes[i]->isThumbnailAvailable()) {
                //remove the image file from disk
                QDir dir("/home/user/.phototheater");
                QFile f(dir.absoluteFilePath(scenes[i]->getFilename()+".png"));
                f.remove();
            } else {
                scenes[i]->loadThumbnailAsync();
            }
        } else if (delta > 3) {
            //images[i]->discardFrame();
            //images[i]->discardThumbnail();
        }
    }
}

void SceneSelector::loadSceneThumbnail(Scene * scene){
    QDir dir("/home/user/.phototheater");
    //qDebug() << "Load preview from " << dir.absoluteFilePath(scene->getFilename()+".png");

    scene->setThumbnail(QPixmap(dir.absoluteFilePath(scene->getFilename()+".png")));
    //remove file
    QFile thumb(dir.absoluteFilePath(scene->getFilename()+".png"));
    thumb.remove();
    this->updateThumbnails();
}

void SceneSelector::selectScene(){
    emit sceneSelected(scenes[current]);
}

void SceneSelector::addScene(Scene * scene){
    scenes.push_back(scene);
    this->updateThumbnails();
}
