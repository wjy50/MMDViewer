//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_PMXCOMMON_H
#define MMDVIEWER_PMXCOMMON_H

#include <GLES3/gl3.h>
#include <stdexcept>

typedef struct PMX_INFO
{
    unsigned char encoding;
    unsigned char UVACount;
    unsigned char vertexSize;
    unsigned char texSize;
    unsigned char materialSize;
    unsigned char boneSize;
    unsigned char morphSize;
    unsigned char bodySize;
} PMXInfo;

typedef enum PMX_ERROR
{
    UNSUPPORTED_PMX_VERSION = 0,
    NOT_PMX_FILE
} PMXError;

class PMXException : public std::runtime_error
{
private:
    PMXError error;
public:
    PMXException(PMXError error);

    PMXError getError() const;

    const char * what() const noexcept override;
};

#define IS_NEGATIVE_ONE(x, size) ((x) ^ (0xffffffff << ((size) * 8))) == 0xffffffff

#endif //MMDVIEWER_PMXCOMMON_H
