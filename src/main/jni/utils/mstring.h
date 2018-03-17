//
// Created by wjy50 on 2018/2/23.
//

#ifndef MMDVIEWER_MSTRING_H
#define MMDVIEWER_MSTRING_H

#include <stdio.h>

typedef enum M_STRING_ENCODING{
    UTF16=0,
    UTF8
}MStringEncoding;

class MString{
private:
    const char* mData;
    int mLength;
    int mHashCode;
public:
    MString(const char* data);
    MString(const char* data,int length);
    MString(MString& cpy);
    static MString* readString(FILE* file,MStringEncoding encoding);
    int hashCode();
    bool equals(MString& other);
    ~MString();
    const char* getData();
    int length();
    const char&operator[](int i);
};

#endif //MMDVIEWER_MSTRING_H
