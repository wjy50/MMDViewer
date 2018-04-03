//
// Created by wjy50 on 2018/2/23.
//

#ifndef MMDVIEWER_MSTRING_H
#define MMDVIEWER_MSTRING_H

#include <stdio.h>

static const char *encodingNames[] = {"utf-16le", "utf-8", "shift-jis"};

typedef enum M_STRING_ENCODING
{
    UTF_16_LE = 0,
    UTF_8,
    SHIFT_JIS
} MStringEncoding;

class MString
{
private:
    const char *mData;
    size_t mLength;
    int mHashCode;

    static const char *getEncodingName(MStringEncoding encoding);

public:
    MString(const char *data, bool copy);

    MString(const char *data, size_t length, bool copy);

    MString(MString &cpy);

    static MString *
    readString(FILE *file, MStringEncoding fromEncoding, MStringEncoding toEncoding);

    static MString *
    readStringAndTrim(FILE *file, unsigned int maxSize, MStringEncoding fromEncoding,
                      MStringEncoding toEncoding);

    int hashCode();

    bool equals(MString &other);

    ~MString();

    const char *getData();

    size_t length();

    void trim();

    size_t trimToSize();

    const char &operator[](int i);
};

class MStringBuilder
{
private:
    char * mBuffer;
    size_t mCapacity;
    size_t mSize;

    void ensure(size_t increment);

public:
    MStringBuilder();

    MStringBuilder(size_t initialCapacity);

    MStringBuilder & append(char c);

    MStringBuilder & append(const char * s);

    MStringBuilder & append(const char *s, size_t length);

    MStringBuilder & trim();

    MString * build();

    ~MStringBuilder();
};

#endif //MMDVIEWER_MSTRING_H
