//
// Created by wjy50 on 2018/2/7.
//

#ifndef MMDVIEWER_PMXREADER_H
#define MMDVIEWER_PMXREADER_H

#include "../../gl/objects/globject.h"
#include "pmxcommon.h"
#include "pmxobject.h"

typedef struct PMX_HEADER
{
    int magic;
} PMXHeader;

class PMXReader
{
private:
    std::string filePath;

    PMXInfo info;
    MStringEncoding encoding;

    void readInfo(std::ifstream &file);

    void readNameAndDescription(std::ifstream &file, PMXObject &object);

    void readVerticesAndIndices(std::ifstream &file, PMXObject &object);

    void readTextures(std::ifstream &file, PMXObject &object);

    void readMaterials(std::ifstream &file, PMXObject &object);

    void readBones(std::ifstream &file, PMXObject &object);

    void readMorphs(std::ifstream &file, PMXObject &object);
public:
    PMXReader(const char *filePath);

    PMXObject read();

    ~PMXReader() = default;
};

#endif //MMDVIEWER_PMXREADER_H
