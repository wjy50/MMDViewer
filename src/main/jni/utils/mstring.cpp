//
// Created by wjy50 on 2018/2/23.
//

#include <cstring>
#include "mstring.h"
#include "../iconv/iconv.h"
#include "UniquePointerExt.h"
#include "debugutils.h"

using namespace std;

MStringBuilder::MStringBuilder()
{
    mBuffer = nullptr;
    mCapacity = 0;
    mSize = 0;
}

MStringBuilder::MStringBuilder(size_t initialCapacity) : MStringBuilder()
{
    ensure(initialCapacity);
}

void MStringBuilder::ensure(size_t increment)
{
    if (increment == 0)
        return;
    size_t cap = mCapacity;
    if (mCapacity == 0) mCapacity = 1;
    while (mCapacity < mSize + increment) mCapacity <<= 1;
    if (mCapacity != cap) {
        auto newArray = make_unique_array<char[]>(mCapacity);
        if (cap != 0) {
            memcpy(newArray.get(), mBuffer, mSize);
            delete[] mBuffer;
        }
        mBuffer = newArray.release();
    }
}

MStringBuilder &MStringBuilder::append(char c)
{
    ensure(sizeof(char));
    mBuffer[mSize++] = c;
    return *this;
}

MStringBuilder &MStringBuilder::append(const char *s)
{
    size_t inLength = strlen(s);
    ensure(inLength);
    memcpy(mBuffer, s, inLength);
    mSize += inLength;
    return *this;
}

MStringBuilder &MStringBuilder::append(const char *s, size_t length)
{
    ensure(length);
    memcpy(mBuffer, s, length);
    mSize += length;
    return *this;
}

MStringBuilder &MStringBuilder::trim()
{
    size_t begin = 0, end = mSize;
    for (size_t i = 0; i < mSize; ++i) {
        if (mBuffer[i] != ' ') {
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

MString MStringBuilder::build()
{
    return MString(mBuffer, mSize);
}

const char* MStringBuilder::getData() const
{
    return mBuffer;
}

size_t MStringBuilder::size() const
{
    return mSize;
}

MStringBuilder::~MStringBuilder()
{
    delete[] mBuffer;
}

void MString::readString(ifstream &file, MStringEncoding fromEncoding, MStringEncoding toEncoding)
{
    unsigned int size;
    file.read(reinterpret_cast<char *>(&size), sizeof(int));
    auto s = make_unique_array<char[]>(size + 1);
    s[size] = 0;
    file.read(s.get(), size * sizeof(char));
    if (size > 0) {
        if (fromEncoding != toEncoding) {
            MStringBuilder builder(size);
            iconv_t cd = iconv_open(getEncodingName(toEncoding), getEncodingName(fromEncoding));
            size_t inSize = size;
            const size_t bufferSize = 256;
            size_t outSize = bufferSize;
            auto buffer = make_unique_array<char[]>(bufferSize);
            char *inBuffer = s.get();
            char *outBuffer = buffer.get();
            while (inSize > 0) {
                iconv(cd, &inBuffer, &inSize, &outBuffer, &outSize);
                size_t converted = bufferSize - outSize;
                if (converted == 0)
                    break;
                builder.append(buffer.get(), converted);
                outBuffer = buffer.get();
                outSize = bufferSize;
            }
            iconv_close(cd);
            setData(builder.trim().getData(), builder.size());
        } else {
            setData(s.get(), size);
            trim();
        }
    } else
        setData("", 0);
}

void MString::readStringAndTrim(ifstream &file, unsigned int maxSize, MStringEncoding fromEncoding,
                                    MStringEncoding toEncoding)
{
    if (maxSize > 0) {
        auto s = make_unique_array<char[]>(maxSize + 1);
        file.read(s.get(), maxSize * sizeof(char));
        s[maxSize] = 0;
        size_t size = strlen(s.get());
        if (fromEncoding != toEncoding) {
            MStringBuilder builder(maxSize);
            iconv_t cd = iconv_open(getEncodingName(toEncoding), getEncodingName(fromEncoding));
            char *inBuffer = s.get();
            size_t inSize = size;
            size_t bufferSize = 256;
            size_t outSize = bufferSize;
            auto buffer = make_unique_array<char[]>(bufferSize);
            char *outBuffer = buffer.get();
            while (inSize > 0) {
                iconv(cd, &inBuffer, &inSize, &outBuffer, &outSize);
                size_t converted = bufferSize - outSize;
                if (converted == 0) break;
                builder.append(buffer.get(), converted);
                outBuffer = buffer.get();
                outSize = bufferSize;
            }
            iconv_close(cd);
            setData(builder.trim().getData(), builder.size());
        } else {
            setData(s.get(), size);
            trim();
        }
    } else
        setData("", 0);
}

MString::MString(const char *data)
: mData(nullptr)
{
    if (!data || (mLength = strlen(data)) == 0)
        mLength = 0;
    setData(data, mLength);
}

MString::MString(const char *data, size_t length)
: mData(nullptr), mLength(0), mHashCode(0)
{
    setData(data, length);
}

MString::MString(const MString &other)
        : mData(nullptr)
{
    setData(other.mData, other.mLength);
    mHashCode = other.mHashCode;
}

MString::MString(MString &&other) noexcept
        : mData(other.mData), mLength(other.mLength), mHashCode(other.mHashCode)
{
    other.mData = nullptr;
}

void MString::setData(const char *data, size_t length)
{
    delete[] mData;
    auto temp = make_unique_array<char[]>(length + 1);
    memcpy(temp.get(), data, length);
    temp[length] = 0;
    mData = temp.release();
    mLength = length;
    mHashCode = 0;
}

int MString::hashCode()
{
    int h = mHashCode;
    if (mHashCode == 0 && mLength > 0) {
        for (size_t i = 0; i < mLength; ++i) {
            h = 31 * h + mData[i];
        }
        mHashCode = h;
    }
    return h;
}

bool MString::equals(const MString &other)
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

const char *MString::getData() const
{
    return mData;
}

size_t MString::length() const
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
    for (size_t i = mLength - 1; i > begin; --i) {
        if (mData[i] != ' ') {
            end = i + 1;
        }
    }
    size_t len = end - begin;
    if (len != mLength) {
        char *temp = const_cast<char *>(mData);
        if (begin != 0)
            memmove(temp, mData + begin, len);
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
    for (size_t i = mLength - 1; i > begin; --i) {
        if (mData[i] != ' ') {
            end = i + 1;
        }
    }
    size_t len = end - begin;
    if (len != mLength) {
        auto temp = make_unique_array<char[]>(len + 1);
        memcpy(temp.get(), mData + begin, len);
        mLength = len;
        temp[mLength] = 0;
        delete[] mData;
        mData = temp.release();
    }
    return len;
}

const char &MString::operator[](int i) const
{
    return mData[i];
}

const char *MString::getEncodingName(MStringEncoding encoding)
{
    return encodingNames[encoding];
}