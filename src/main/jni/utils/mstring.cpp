//
// Created by wjy50 on 2018/2/23.
//

#include <string.h>
#include "mstring.h"
#include "../iconv/iconv.h"
#include "debugutils.h"

MStringBuilder::MStringBuilder()
{
    mBuffer = 0;
    mCapacity = 0;
    mSize = 0;
}

MStringBuilder::MStringBuilder(size_t initialCapacity)
{
    mBuffer = 0;
    mCapacity = 0;
    mSize = 0;
    ensure(initialCapacity);
}

void MStringBuilder::ensure(size_t increment)
{
    if (increment == 0)
        return;
    size_t cap=mCapacity;
    if (mCapacity == 0) mCapacity = 1;
    while (mCapacity < mSize + increment) mCapacity <<= 1;
    if (mCapacity != cap) {
        char * newArray = new char[mCapacity];
        if (cap != 0) {
            memcpy(newArray,mBuffer,mSize);
            delete[] mBuffer;
        }
        mBuffer = newArray;
    }
}

MStringBuilder& MStringBuilder::append(char c)
{
    ensure(sizeof(char));
    mBuffer[mSize++] = c;
    return *this;
}

MStringBuilder& MStringBuilder::append(const char *s)
{
    size_t inLength = strlen(s);
    ensure(inLength);
    memcpy(mBuffer, s, inLength);
    mSize += inLength;
    return *this;
}

MStringBuilder& MStringBuilder::append(const char *s, size_t length)
{
    ensure(length);
    memcpy(mBuffer, s, length);
    mSize += length;
    return *this;
}

MStringBuilder& MStringBuilder::trim()
{
    size_t begin = 0, end = mSize;
    for (size_t i = 0; i < mSize; ++i) {
        if(mBuffer[i] != ' ') {
            begin = i;
            break;
        }
    }
    for (size_t i = mSize - 1; i > begin; --i) {
        if (mBuffer[i] != ' ') {
            end = i + 1;
            break;
        }
    }
    size_t len = end - begin;
    if (len != mSize) {
        if (begin != 0) memmove(mBuffer, mBuffer + begin, len);
        mSize = len;
    }
    return *this;
}

MString* MStringBuilder::build()
{
    return new MString(mBuffer, mSize, true);
}

MStringBuilder::~MStringBuilder()
{
    if (mBuffer) delete[] mBuffer;
}

MString *MString::readString(FILE *file, MStringEncoding fromEncoding, MStringEncoding toEncoding)
{
    unsigned int size;
    fread(&size, sizeof(int), 1, file);
    char *s = new char[size + 1];
    s[size] = 0;
    fread(s, sizeof(char), size, file);
    if (size > 0) {
        if (fromEncoding != toEncoding) {
            MStringBuilder builder(size);
            LOG_PRINTF("%s -> %s", getEncodingName(fromEncoding), getEncodingName(toEncoding));
            iconv_t cd = iconv_open(getEncodingName(toEncoding), getEncodingName(fromEncoding));
            size_t inSize = size;
            size_t bufferSize = 256;
            size_t outSize = bufferSize;
            char *buffer = new char[bufferSize];
            char *inBuffer = s;
            char *outBuffer = buffer;
            while (inSize > 0) {
                iconv(cd, &inBuffer, &inSize, &outBuffer, &outSize);
                size_t converted = bufferSize - outSize;
                if (converted == 0) break;
                builder.append(buffer, converted);
                outBuffer = buffer;
                outSize = bufferSize;
            }
            iconv_close(cd);
            delete[] s;
            delete[] buffer;
            return builder.trim().build();
        } else {
            MString *string = new MString(s, size, false);
            string->trim();
            return string;
        }
    }
    return new MString("", true);
}

MString *MString::readStringAndTrim(FILE *file, unsigned int maxSize, MStringEncoding fromEncoding,
                                    MStringEncoding toEncoding)
{
    if (maxSize > 0) {
        char *s = new char[maxSize + 1];
        fread(s, sizeof(char), maxSize, file);
        LOG_PRINTLN(s);
        delete[] s;
        /*if (fromEncoding != toEncoding) {
            MStringBuilder builder(maxSize);
            iconv_t cd = iconv_open(getEncodingName(toEncoding), getEncodingName(fromEncoding));
            char *inBuffer = s;
            size_t inSize = maxSize;
        } else {
            MString *string = new MString(s, maxSize, false);
            string->trim();
            return string;
        }*/
    }
    return new MString("", true);
}

MString::MString(const char *data, bool copy)
{
    for (size_t i = 0;; ++i) {
        if (data[i] == 0) {
            mLength = i;
            break;
        }
    }
    if (copy) {
        char *temp = new char[mLength + 1];
        memcpy(temp, data, mLength);
        temp[mLength] = 0;
        mData = temp;
    } else mData = data;
    mHashCode = 0;
}

MString::MString(const char *data, size_t length, bool copy)
{
    if (copy) {
        char *temp = new char[length + 1];
        memcpy(temp, data, length);
        temp[length] = 0;
        mData = temp;
    } else {
        mData = data;
    }
    mLength = length;
    mHashCode = 0;
}

MString::MString(MString &cpy)
{
    mLength = cpy.mLength;
    mHashCode = cpy.mHashCode;
    char *data = new char[mLength + 1];
    data[mLength] = 0;
    memcpy(data, cpy.mData, mLength);
    mData = data;
}

int MString::hashCode()
{
    int h = mHashCode;
    if (mHashCode == 0 && mLength > 0) {
        for (int i = 0; i < mLength; ++i) {
            h = 31 * h + mData[i];
        }
        mHashCode = h;
    }
    return h;
}

bool MString::equals(MString &other)
{
    if (this == &other)return true;
    if (mLength == other.mLength) {
        for (size_t i = mLength + 1; i-- != 0;) {
            if (mData[i] != other.mData[i])return false;
        }
        return true;
    }
    return false;
}

MString::~MString()
{
    delete[] mData;
}

const char *MString::getData()
{
    return mData;
}

size_t MString::length()
{
    return mLength;
}

void MString::trim()
{
    size_t begin = 0, end = mLength;
    for (size_t i = 0; i < mLength; ++i) {
        if (mData[i] != ' ') {
            begin = i;
        }
    }
    for (size_t i = mLength-1; i > begin; --i) {
        if (mData[i] != ' ') {
            end = i+1;
        }
    }
    size_t len = end - begin;
    if (len != mLength) {
        char * temp = (char *) mData;
        if (begin != 0) memmove(temp, mData + begin, len);
        mLength = len;
        temp[mLength] = 0;
    }
}

size_t MString::trimToSize()
{
    size_t begin = 0, end = mLength;
    for (size_t i = 0; i < mLength; ++i) {
        if (mData[i] != ' ') {
            begin = i;
        }
    }
    for (size_t i = mLength-1; i > begin; --i) {
        if (mData[i] != ' ') {
            end = i+1;
        }
    }
    size_t len = end - begin;
    if (len != mLength) {
        char * temp = new char[len + 1];
        memcpy(temp, mData + begin, len);
        mLength = len;
        temp[mLength] = 0;
        delete [] mData;
        mData = temp;
    }
    return len;
}

const char &MString::operator[](int i)
{
    return mData[i];
}

const char *MString::getEncodingName(MStringEncoding encoding)
{
    return encodingNames[encoding];
}