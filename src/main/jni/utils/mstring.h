//
// Created by wjy50 on 2018/2/23.
//

#ifndef MMDVIEWER_MSTRING_H
#define MMDVIEWER_MSTRING_H

#include <fstream>

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

    void setData(const char *data, size_t length);

public:
    MString(const char *data = nullptr);

    MString(const char *data, size_t length);

    MString(const MString &other);

    MString(MString &&other) noexcept;

    void readString(std::ifstream &file, MStringEncoding fromEncoding, MStringEncoding toEncoding);

    void readStringAndTrim(std::ifstream &file, unsigned int maxSize, MStringEncoding fromEncoding,
                      MStringEncoding toEncoding);

    int hashCode();

    bool equals(const MString &other);

    ~MString();

    const char *getData() const;

    size_t length() const;

    void trim();

    size_t trimToSize();

    const char &operator[](int i) const;
};

class MStringBuilder
{
private:
    char *mBuffer;
    size_t mCapacity;
    size_t mSize;

    void ensure(size_t increment);

public:
    MStringBuilder();

    MStringBuilder(size_t initialCapacity);

    MStringBuilder &append(char c);

    MStringBuilder &append(const char *s);

    MStringBuilder &append(const char *s, size_t length);

    MStringBuilder &trim();

    MString build();

    const char *getData() const;

    size_t size() const;

    ~MStringBuilder();
};

#endif //MMDVIEWER_MSTRING_H
