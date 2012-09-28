#ifndef SCENE_H
#define SCENE_H

#include "CutOut.h"
#include <QStack>
#include <QHash>
#include <QQueue>
#include <QThread>
#include <QPixmap>

class Scene : public QObject
{
    Q_OBJECT
public:
    explicit Scene(QObject *parent = 0);
    Scene(QString fileName, QObject *parent = 0);
    enum WallId {Back = 0, Left, Right, Floor};

    QList<unsigned int> getCutOutsIds();
    CutOut * getCutOut(unsigned int code);
    void addCutOut(unsigned int code, CutOut* cutOut);
    CutOut * removeCutOut(unsigned int code);
    void setThumbnail(QPixmap thumbnail);
    QPixmap * getThumbnail();
    bool isThumbnailAvailable();
    void loadThumbnailAsync();
    bool isNew();
    QString getFilename();
    void setFilename(QString fileName);

    bool isWallSet(WallId wall);
    CutOut * getWall(WallId wall);
    unsigned int getWallCode(WallId wall);
    void setWall(WallId wall, unsigned int code);
    unsigned int unsetWall(WallId wall);


    void reset(QStack<unsigned int> * colorStack, QStack<unsigned int> * textureStack);
    void resetZoom();
    void resetOrbit();

    float zoom();
    QVector3D orbitVector();

    void addToZoom(float z);

    void addToOrbitX(float x);
    void addToOrbitY(float y);

protected:
     void reset();
     QHash<unsigned int, CutOut *> cutOuts;
     unsigned int walls[4];
     QPixmap thumbnail;
     QString fileName;
     QVector3D orbit;
     float zoomVal;
};

inline Scene::WallId operator++( Scene::WallId &rs, int ) {
   return rs = (Scene::WallId)(rs + 1);
}

// A background thread that takes care of all the disk IO
class SceneIOThread : public QThread {
    Q_OBJECT;
public:
    //instance for reading
    static SceneIOThread &reader();

    // Enqueues a request to load an image item's thumbnail data from disk
    void loadThumbnail(Scene * scene) {
        queue.enqueue(scene);
    }

public slots:
    // Cleans up the IOThread and allows it to return after draining it's pipeline.
    void stop();

signals:
    // These signals are emitted when their corresponding requests have been fulfilled.
    void loadFinished(Scene *);

protected:
    // The main loop for this thread services its queue of I/O requests.
    void run();
private:
    //extract thumbnail from zipped scene file
    void extractThumbnail(QString fileName);

    //enum RequestType {Load = 0, Save, LoadThumbnail, Stop, Demosaic};

    /*
    struct Request {
        Request(ImageItem *im, RequestType t) : image(im), type(t) {}
        ImageItem *image;
        RequestType type;
    };
*/
   QQueue<Scene *> queue;

   SceneIOThread();
};

#endif // SCENE_H
