#include "ImportExport.h"
#include "UserDefaults.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include <QXmlStreamWriter>
#include <QDebug>
#include <QDir>
#include <QPixmap>
#include <QDateTime>


ImportExport::ImportExport(Scene * scene, bool save) : xmlDoc("PhotoTheater0.1")
{
    if(save){
        //create a temporary folder to store the files
        QDir dir;
        dir.mkpath("/home/user/.phototheater/temp");
        //create an xml document
        QDomElement root = xmlDoc.createElement("phototheater");
        xmlDoc.appendChild(root);
        unsigned int colorCode;

        //add the cut outs to the xml document
        foreach( colorCode, scene->getCutOutsIds() ){
            addCutout(scene->getCutOut(colorCode));
        }

        //add the wall info
        for(Scene::WallId i= Scene::Back;i<=Scene::Floor;i++){
            if(scene->isWallSet(i)){
                addWall(i,scene->getWall(i)->getTextureId());
            }
        }

        //add the camera info
        addCamera(scene);
    }else{
        //create a temporary folder to store the files
        QDir dir;
        dir.mkpath("/home/user/.phototheater/temp");

        UserDefaults &userDefaults = UserDefaults::instance();
        QString file = userDefaults["rawPath"].asString().c_str()+scene->getFilename();

        //unzip the scene file
        if(!unzip(file)){
         //   qDebug() << "Failes to open archive.";
            return;
        }
        scene->setThumbnail(QPixmap("/home/user/.phototheater/temp/"));

        QFile xmlfile("/home/user/.phototheater/temp/scene.xml");
        if (!xmlfile.open(QIODevice::ReadOnly))
            return;

        if (!xmlDoc.setContent(&xmlfile))
        {
            xmlfile.close();
            return;
        }
        xmlfile.close();


        QDomElement root = xmlDoc.documentElement();
        QDomNodeList cutoutsElems = root.elementsByTagName("cutout");
      //  qDebug() << "Found " << cutoutsElems.size() << " cut outs.";
        for(int i=0;i<cutoutsElems.size();i++){
            addCutout(cutoutsElems.item(i));
        }
       // qDebug() << "Added " << cutOutStack.size() << " cut outs";

        //load walls information
        walls[0] = 0;
        walls[1] = 0;
        walls[2] = 0;
        walls[3] = 0;
        QDomNodeList wallElems = root.elementsByTagName("wall");
      //  qDebug() << "Found " << wallElems.size() << " walls.";
        for(int i=0;i<wallElems.size();i++){
            walls[wallElems.item(i).attributes().namedItem("wallId").nodeValue().toUInt()] = wallElems.item(i).attributes().namedItem("cutOutId").nodeValue().toUInt();
          //  qDebug() << "wall cutout: " << wallElems.item(i).attributes().namedItem("cutOutId").nodeValue().toUInt();
        }

        //load camera information
        QDomNodeList cameraElems = root.elementsByTagName("camera");
      //   qDebug() << "Found " << cameraElems.size() << " camera.";
         scene->resetZoom();
         scene->resetOrbit();
         if(cameraElems.size()==1){
             scene->addToZoom(cameraElems.item(0).attributes().namedItem("zoom").nodeValue().toFloat());
             scene->addToOrbitX(cameraElems.item(0).attributes().namedItem("orbitX").nodeValue().toFloat());
             scene->addToOrbitY(cameraElems.item(0).attributes().namedItem("orbitY").nodeValue().toFloat());
         }
    }
}
/*
ImportExport::ImportExport(Scene * scene) : xmlDoc("PhotoTheater0.1")
{
    //create a temporary folder to store the files
    QDir dir;
    dir.mkpath("/home/user/.phototheater/temp");

    UserDefaults &userDefaults = UserDefaults::instance();
    QString file = userDefaults["rawPath"].asString().c_str()+scene->getFilename();

    //unzip the scene file
    if(!unzip(file)){
        qDebug() << "Failes ot open archive.";
        return;
    }

    QFile xmlfile("/home/user/.phototheater/temp/scene.xml");
    if (!xmlfile.open(QIODevice::ReadOnly))
        return;

    if (!xmlDoc.setContent(&xmlfile))
    {
        xmlfile.close();
        return;
    }
    xmlfile.close();


    QDomElement root = xmlDoc.documentElement();
    QDomNodeList cutoutsElems = root.elementsByTagName("cutout");
    qDebug() << "Found " << cutoutsElems.size() << " cut outs.";
    for(int i=0;i<cutoutsElems.size();i++){
        addCutout(cutoutsElems.item(i));
    }
    qDebug() << "Added " << cutOutStack.size() << " cut outs";

    //load walls information
    walls[0] = 0;
    walls[1] = 0;
    walls[2] = 0;
    walls[3] = 0;
    QDomNodeList wallElems = root.elementsByTagName("wall");
    qDebug() << "Found " << wallElems.size() << " walls.";
    for(int i=0;i<wallElems.size();i++){
        walls[wallElems.item(i).attributes().namedItem("wallId").nodeValue().toUInt()] = wallElems.item(i).attributes().namedItem("cutOutId").nodeValue().toUInt();
        qDebug() << "wall cutout: " << wallElems.item(i).attributes().namedItem("cutOutId").nodeValue().toUInt();
    }

    //load camera information
    QDomNodeList cameraElems = root.elementsByTagName("camera");
     qDebug() << "Found " << cameraElems.size() << " camera.";
     scene->resetZoom();
     scene->resetOrbit();
     if(cameraElems.size()==1){
         scene->addToZoom(cameraElems.item(0).attributes().namedItem("zoom").nodeValue().toFloat());
         scene->addToOrbitX(cameraElems.item(0).attributes().namedItem("orbitX").nodeValue().toFloat());
         scene->addToOrbitY(cameraElems.item(0).attributes().namedItem("orbitY").nodeValue().toFloat());
     }

}
*/

unsigned int * ImportExport::getWalls(){
    qDebug() << "getWalls";
    for(int i=0;i<4;i++){
        qDebug() << walls[i];
    }
    return &walls[0];
}

CutOut * ImportExport::nextCutOut(bool *r){
    if(cutOutStack.size()==0){
        *r = false;
        return 0;
    }
    *r = true;
    qDebug() << "Stack: " << cutOutStack.size();
    return cutOutStack.pop();
    //qDebug() << "Poping " << c->fileName;
    //return true;
}

void ImportExport::addCutout(CutOut * cutOut){
    QDomElement root = xmlDoc.documentElement();
    QDomElement cutOutTag = xmlDoc.createElement("cutout");
    //texture info
    cutOutTag.setAttribute("texId",cutOut->getTextureId());
    cutOutTag.setAttribute("width",cutOut->getOriginalWidth());
    cutOutTag.setAttribute("height",cutOut->getOriginalHeight());

    //position
    cutOutTag.setAttribute("x",cutOut->getX());
    cutOutTag.setAttribute("y",cutOut->getY());
    cutOutTag.setAttribute("z",cutOut->getZ());

    //transformation
    cutOutTag.setAttribute("rotation",cutOut->getRotation());
    cutOutTag.setAttribute("scale",cutOut->getScale());
    cutOutTag.setAttribute("wallscale",cutOut->getWallScale());

    //texture coordinates
    float * textureCoord = cutOut->getTextureCoord();
    QDomElement textureTag = xmlDoc.createElement("texture");
    QString s;
    for(int i=0;i<8;i++){
        s= "";
        s.sprintf("%.3f", textureCoord[i]);
        QDomElement coordTag = xmlDoc.createElement("coord");
        coordTag.setAttribute("id",i);
        coordTag.appendChild(xmlDoc.createTextNode(s));
        textureTag.appendChild(coordTag);
    }
    cutOutTag.appendChild(textureTag);

    //vertices coordinates
    float * verticesCoord = cutOut->getVertices();
    QDomElement verticesTag = xmlDoc.createElement("vertices");
    for(int i=0;i<12;i++){
        s= "";
        s.sprintf("%.3f", verticesCoord[i]);
        QDomElement coordTag = xmlDoc.createElement("coord");
        coordTag.setAttribute("id",i);
        coordTag.appendChild(xmlDoc.createTextNode(s));
        verticesTag.appendChild(coordTag);
    }
    cutOutTag.appendChild(verticesTag);

    root.appendChild(cutOutTag);
}

void ImportExport::addCutout(QDomNode cutOutTag){

    CutOut * cCutOut = new CutOut(cutOutTag.attributes().namedItem("texId").nodeValue()+".png",
                   cutOutTag.attributes().namedItem("width").nodeValue().toInt(),
                   cutOutTag.attributes().namedItem("height").nodeValue().toInt(),
                   QVector3D(cutOutTag.attributes().namedItem("x").nodeValue().toFloat(), cutOutTag.attributes().namedItem("y").nodeValue().toFloat(), cutOutTag.attributes().namedItem("z").nodeValue().toFloat()),
                   cutOutTag.attributes().namedItem("rotation").nodeValue().toFloat(),
                   cutOutTag.attributes().namedItem("scale").nodeValue().toFloat(),
                   cutOutTag.attributes().namedItem("wallscale").nodeValue().toFloat()
                   );
    cCutOut->setTextureId(cutOutTag.attributes().namedItem("texId").nodeValue().toUInt());
    //
    float  * vertices = new float[12];
    //
    float * texCoord = new float[8];

    QDomNode coordTag;
    //texture
    QDomNode textureTag = cutOutTag.firstChild();// elementsByTagName("texture").item(0);
    //qDebug() << "textureTag:" << textureTag.nodeName();
    //qDebug() << "textureTag:" << textureTag.childNodes().size();
    for(int i=0;i<8;i++){
        coordTag = textureTag.childNodes().item(i);
        texCoord[coordTag.attributes().namedItem("id").nodeValue().toInt()] = coordTag.firstChild().nodeValue().toFloat();
   //     qDebug() << "Tc[" << coordTag.attributes().namedItem("id").nodeValue().toInt() << "] = " <<  texCoord[coordTag.attributes().namedItem("id").nodeValue().toInt()];
    }
    cCutOut->setTextureCoord(texCoord);

    QDomNode verticesTag = cutOutTag.childNodes().item(1);// elementsByTagName("vertices").item(0);
    for(int i=0;i<12;i++){
        coordTag = verticesTag.childNodes().item(i);
        vertices[coordTag.attributes().namedItem("id").nodeValue().toInt()] = coordTag.firstChild().nodeValue().toFloat();
    }

    cCutOut->setVertices(vertices);

    cutOutStack.push(cCutOut);

   // qDebug() << "Putting " << cutOutStack.last()->fileName << " in stack";
    //Cutout cCutOut(cutOutTag.attributes().namedItem(texId).nodeValue(), cutOutNode.attributes().namedItem(width).nodeValue(), cutOutNode.attributes().namedItem(height).nodeValue(), float bX, float bY, float tX, float tY, float width, float height, float viewPortRatio, const QVector3D position, QPoint topLeft, QImage mask, QString fileName, QObject *parent);
}

void ImportExport::addWall(int wall, unsigned int cutOut){
    QDomElement root = xmlDoc.documentElement();
    QDomElement cutOutTag = xmlDoc.createElement("wall");
    //texture info
    cutOutTag.setAttribute("wallId",wall);
    cutOutTag.setAttribute("cutOutId",cutOut);
    root.appendChild(cutOutTag);
}

void ImportExport::addCamera(Scene * s){
    QDomElement root = xmlDoc.documentElement();
    QDomElement cameraTag = xmlDoc.createElement("camera");
    //camera info
    cameraTag.setAttribute("zoom",s->zoom());
    cameraTag.setAttribute("orbitX",s->orbitVector().x());
    cameraTag.setAttribute("orbitY",s->orbitVector().y());
    cameraTag.setAttribute("orbitZ",s->orbitVector().z());
    root.appendChild(cameraTag);
}

bool ImportExport::saveToFile(Scene * scene){
    //qDebug() << xmlDoc.toString();
    QFile outputFile("/home/user/.phototheater/temp/scene.xml");
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QTextStream writer(&outputFile);
        xmlDoc.save(writer,1);
    }else{
        //return WriteError;
        qDebug() << "Failed to open stdout.";
        return false;
    }

    QDir dir("/home/user/.phototheater/temp");

    UserDefaults &userDefaults = UserDefaults::instance();


    /* ZIP IT */
    QString directoryPath = userDefaults["rawPath"].asString().c_str();
    QString epoch = QDateTime::currentDateTime().toString("hh_mm_ss_zzz");
    QuaZip zip(directoryPath+ epoch +".pht");
    qDebug() << zip.getZipName();
    if(!zip.open(QuaZip::mdCreate)) {
        qWarning("testCreate(): zip.open(): %d", zip.getZipError());
        return false;
    }else{
        zip.setComment("A PhotoTheater scene");
        QFileInfoList files=dir.entryInfoList();
        QFile inFile;
        QuaZipFile outFile(&zip);
        char c;
        foreach(QFileInfo file, files) {
            if(!file.isFile()) continue;
            inFile.setFileName(dir.absoluteFilePath(file.fileName()));
            if(!inFile.open(QIODevice::ReadOnly)) {
                qWarning("testCreate(): inFile.open(): %s", inFile.errorString().toLocal8Bit().constData());
                return false;
            }
            if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(file.fileName(), inFile.fileName()))) {
                qWarning("testCreate(): outFile.open(): %d", outFile.getZipError());
                return false;
            }
            while(inFile.getChar(&c) && outFile.putChar(c));
            if(outFile.getZipError()!=UNZ_OK) {
                qWarning("testCreate(): outFile.putChar(): %d", outFile.getZipError());
                return false;
            }
            outFile.close();
            if(outFile.getZipError()!=UNZ_OK) {
                qWarning("testCreate(): outFile.close(): %d", outFile.getZipError());
                return false;
            }
            inFile.close();
        }
        zip.close();
        if(zip.getZipError()!=0) {
            qWarning("testCreate(): zip.close(): %d", zip.getZipError());
            return false;
        }

    }
    scene->setFilename(epoch +".pht");
    return true;
}

bool ImportExport::unzip(QString fileToOpen){
    QDir dir("/home/user/.phototheater/temp");

    QuaZip zip(fileToOpen);
    if(!zip.open(QuaZip::mdUnzip)) {
        qWarning("testRead(): zip.open(): %d", zip.getZipError());
        return false;
    }
    zip.setFileNameCodec("IBM866");
    printf("%d entries\n", zip.getEntriesCount());
    printf("Global comment: %s\n", zip.getComment().toLocal8Bit().constData());
    QuaZipFileInfo info;
   // printf("name\tcver\tnver\tflags\tmethod\tctime\tCRC\tcsize\tusize\tdisknum\tIA\tEA\tcomment\textra\n");
    QuaZipFile file(&zip);
    QFile out;
    QString name;
    char c;
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
        if(!zip.getCurrentFileInfo(&info)) {
            qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
            return false;
        }
        /*
        printf("%s\t%hu\t%hu\t%hu\t%hu\t%s\t%u\t%u\t%u\t%hu\t%hu\t%u\t%s\t%s\n",
               info.name.toLocal8Bit().constData(),
               info.versionCreated, info.versionNeeded, info.flags, info.method,
               info.dateTime.toString(Qt::ISODate).toLocal8Bit().constData(),
               info.crc, info.compressedSize, info.uncompressedSize, info.diskNumberStart,
               info.internalAttr, info.externalAttr,
               info.comment.toLocal8Bit().constData(), info.extra.constData());
               */
        if(!file.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", file.getZipError());
            return false;
        }
        name=file.getActualFileName();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }
        QString dirn = dir.absoluteFilePath(name);
        if (name.contains('/')) { // subdirectory support
            // there must be a more elegant way of doing this
            // but I couldn't find anything useful in QDir
            dirn.chop(dirn.length() - dirn.lastIndexOf("/"));
            QDir().mkpath(dirn);
        }
        out.setFileName(dir.absoluteFilePath(name));
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
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }
        if(!file.atEnd()) {
            qWarning("testRead(): read all but not EOF");
            return false;
        }
        file.close();
        if(file.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.close(): %d", file.getZipError());
            return false;
        }
    }
    zip.close();
    if(zip.getZipError()!=UNZ_OK) {
        qWarning("testRead(): zip.close(): %d", zip.getZipError());
        return false;
    }
    return true;
}


//clean the temporary folder
void ImportExport::clean(){
    QDir dir("/home/user/.phototheater/temp");
    QStringList files = dir.entryList();
    for (int i = 0; i < files.size(); i++) {
        QFile f(dir.absoluteFilePath(files.at(i)));
        f.remove();
    }

    if(dir.rmpath("/home/user/.phototheater/temp")){
        qDebug() << "temp dir successfully removed";
    }else{
        qDebug() << "couldn't remove temp dir...";
    }
}
