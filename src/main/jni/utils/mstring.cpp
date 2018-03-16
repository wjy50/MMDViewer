//
// Created by wjy50 on 2018/2/23.
//

#include "mstring.h"

MString::MString(const char *data) {
    mData=data;
    for (int i = 0;; ++i) {
        if(data[i] == 0)
        {
            mLength=i;
            break;
        }
    }
    mHashCode=0;
}
MString::MString(const char *data, int length) {
    mData=data;
    mLength=length;
}
MString::MString(MString &cpy) {
    mLength=cpy.mLength;
    mHashCode=cpy.mHashCode;
    char * data=new char[mLength+1];
    data[mLength]=0;
    for (int i = 0; i < mLength; ++i) {
        data[i]=cpy.mData[i];
    }
    mData=data;
}
int MString::hashCode() {
    int h=mHashCode;
    if(mHashCode == 0 && mLength > 0)
    {
        for (int i = 0; i < mLength; ++i) {
            h=31*h+mData[i];
        }
        mHashCode=h;
    }
    return h;
}
bool MString::equals(MString &other) {
    if(this == &other)return true;
    if(mLength == other.mLength)
    {
        for (int i = mLength+1; i-- != 0;) {
            if(mData[i] != other.mData[i])return false;
        }
        return true;
    }
    return false;
}
MString::~MString() {
    delete [] mData;
}
const char* MString::getData() {
    return mData;
}
int MString::length() {
    return mLength;
}
const char& MString::operator[](int i) {
    return mData[i];
}