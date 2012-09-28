#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>
#include <QStack>
#include "CutOut.h"
#include "Scene.h"

class ImportExport
{
public:
    ImportExport(Scene * scene, bool save = true);

    unsigned int * getWalls();
    bool saveToFile(Scene * scene);
    void clean();
    CutOut * nextCutOut(bool *c);

protected:
    void addCutout(QDomNode cutOutTag);
    void addCutout(CutOut * cutOut);
    void addWall(int wall, unsigned int cutOut);
    void addCamera(Scene * s);

private:
    QDomDocument xmlDoc;
    bool unzip(QString file);
    QStack<CutOut *> cutOutStack;
    unsigned int walls[4];

};

#endif // IMPORTEXPORT_H
