#include <QPixmap>
#include <QDir>
#include "Scene.h"
//#include "CutOut.h"
#include "UserDefaults.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

Scene::Scene(QObject *parent) :
        QObject(parent), thumbnail()
{
    reset();
    thumbnail = 0;
}

Scene::Scene(QString fileName, QObject *parent) :
        QObject(parent), thumbnail()
{
    reset();
    this->fileName = fileName;
    thumbnail = 0;
}

CutOut * Scene::getCutOut(unsigned int code){
    return cutOuts[code];
}

void Scene::addCutOut(unsigned int code, CutOut* cutOut){
    cutOuts.insert(code,cutOut);
}

CutOut * Scene::removeCutOut(unsigned int code){
    CutOut * c = cutOuts[code];
    cutOuts.remove(code);
    return c;
}

QList<unsigned int> Scene::getCutOutsIds(){
    return cutOuts.keys();
}

void Scene::setThumbnail(QPixmap thumbnail){
    this->thumbnail = thumbnail;
}

void Scene::setFilename(QString fileName){
    this->fileName = fileName;
}
bool Scene::isThumbnailAvailable(){
    return !thumbnail.isNull();
}

QPixmap * Scene::getThumbnail(){
    return &thumbnail;
}

QString Scene::getFilename(){
    return fileName;
}

bool Scene::isWallSet(WallId wall){
    return walls[wall] != 0;
}

CutOut * Scene::getWall(WallId wall){
    return getCutOut(walls[wall]);
}

unsigned int Scene::getWallCode(WallId wall){
    return walls[wall];
}

void Scene::setWall(WallId wall, unsigned int code){
    walls[wall] = code;
    cutOuts[walls[wall]]->setAsWall(true);
}

unsigned int Scene::unsetWall(WallId wall){
    unsigned int w = walls[wall];
    if(w!=0){
        cutOuts[w]->setAsWall(false);
    }
    walls[wall] = 0;
    return w;
}

//reset the scene
void Scene::reset(QStack<unsigned int> * colorStack, QStack<unsigned int> * textureStack){
    unsigned int colorCode;
    foreach( colorCode, cutOuts.keys() ){
        colorStack->push(colorCode);
        textureStack->push(cutOuts[colorCode]->getTextureId());
    }
    reset();

}

void Scene::reset(){
    cutOuts.clear();
    walls[0] = 0;
    walls[1] = 0;
    walls[2] = 0;
    walls[3] = 0;
    resetZoom();
    resetOrbit();
    fileName = "untitled";
}

void Scene::resetZoom(){
    zoomVal = 0;
}

float Scene::zoom(){
    return zoomVal;
}

QVector3D Scene::orbitVector(){
    return orbit;
}

void Scene::addToZoom(float z){
    zoomVal += z;
    if(zoomVal>0){
        zoomVal = 0;
    }else if(zoomVal<-3){
        zoomVal = -3;
    }
}

void Scene::addToOrbitX(float x){
    orbit.setX(orbit.x()+x);

    //limit angle to the range [0, 360]
    if(orbit.x()>=360){
        orbit.setX(orbit.x()-360);
    }else if(orbit.x()<0){
        orbit.setX(360+orbit.x());
    }

}

void Scene::addToOrbitY(float y){
    orbit.setY(orbit.y()+y);

    //limit angle to the range [0, 360]
    if(orbit.y()>=360){
        orbit.setY(orbit.y()-360);
    }else if(orbit.y()<0){
        orbit.setY(360+orbit.y());
    }
}

void Scene::resetOrbit(){
    orbit = QVector3D(0,0,0);
}

void Scene::loadThumbnailAsync(){
    SceneIOThread::reader().loadThumbnail(this);
}

bool Scene::isNew(){
    return fileName == "untitled";
}

//--------------- SceneIOThread ----------------
void SceneIOThread::stop() {
    queue.enqueue(0);
}


SceneIOThread::SceneIOThread() : QThread(NULL) {

}

SceneIOThread &SceneIOThread::reader() {
    static SceneIOThread *_instance = NULL;
    if (!_instance) {
        _instance = new SceneIOThread();
        _instance->start(QThread::IdlePriority);
    }
    return *_instance;
}

void SceneIOThread::run() {
    while (1) {
        if(!queue.empty()){
            Scene * scene = queue.dequeue();
            if(scene == 0){
                return;
            }
            if(scene->isNew()){
                loadFinished(scene);
            }else{
                extractThumbnail(scene->getFilename());
                loadFinished(scene);
            }
        }
    }
}

void SceneIOThread::extractThumbnail(QString fileName){
    UserDefaults &userDefaults = UserDefaults::instance();
    /* ZIP IT */
    QString directoryPath = userDefaults["rawPath"].asString().c_str();

    QuaZip zip(directoryPath+fileName);
    if(!zip.open(QuaZip::mdUnzip)) {
        qWarning("testRead(): zip.open(): %d", zip.getZipError());
        return;
    }
    zip.setFileNameCodec("IBM866");
    //printf("%d entries\n", zip.getEntriesCount());
    // printf("Global comment: %s\n", zip.getComment().toLocal8Bit().constData());
    QuaZipFileInfo info;
    //printf("name\tcver\tnver\tflags\tmethod\tctime\tCRC\tcsize\tusize\tdisknum\tIA\tEA\tcomment\textra\n");
    QuaZipFile file(&zip);
    QFile out;
    QString name;
    char c;
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
        if(!zip.getCurrentFileInfo(&info)) {
            qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
            return;
        }
        /*
        printf("%s\t%hu\t%hu\t%hu\t%hu\t%s\t%u\t%u\t%u\t%hu\t%hu\t%u\t%s\t%s\n",
               info.name.toLocal8Bit().constData(),
               info.versionCreated, info.versionNeeded, info.flags, info.method,
               info.dateTime.toString(Qt::ISODate).toLocal8Bit().constData(),
               info.crc, info.compressedSize, info.uncompressedSize, info.diskNumberStart,
               info.internalAttr, info.externalAttr,
               info.comment.toLocal8Bit().constData(), info.extra.constData());*/
        if(info.name != QString("snapshot.png")){
            continue;
        }
        //printf("Preview %s found",info.name.toLocal8Bit().constData());

        QDir dir("/home/user/.phototheater");
        if(!file.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", file.getZipError());
            return;
        }
        name=file.getActualFileName();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return;
        }
        QString dirn = dir.absoluteFilePath(name);
        if (name.contains('/')) { // subdirectory support
            // there must be a more elegant way of doing this
            // but I couldn't find anything useful in QDir
            dirn.chop(dirn.length() - dirn.lastIndexOf("/"));
            QDir().mkpath(dirn);
        }
        out.setFileName(dir.absoluteFilePath(fileName+".png"));
        out.open(QIODevice::WriteOnly);
        char buf[4096];
        int len = 0;
        while (file.getChar(&c)) {
            // we could just do this, but it's about 40% slower:
            // out.putChar(c);
            buf[len++] = c;
            if (len >= 4096) {
                out.write(buf, len);
                len = 0;
            }
        }
        if (len > 0) {
            out.write(buf, len);
        }
        out.close();
        if(!file.atEnd()) {
            qWarning("testRead(): read all but not EOF");
            //return false;
        }
        file.close();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.close(): %d", file.getZipError());
            //   return false;
        }
        zip.close();
        return;
        //return new QPixmap(dir.absoluteFilePath(name));
    }
    zip.close();
    if(zip.getZipError()!=UNZ_OK) {
        qWarning("testRead(): zip.close(): %d", zip.getZipError());
        //return false;
    }
    return;
}

