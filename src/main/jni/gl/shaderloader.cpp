#include <fstream>
#include "shaderloader.h"

//
// Created by wjy50 on 2018/2/6.
//

using namespace std;

int loadShader(const char *fileName, int *ptrLength, unique_ptr<char[]> &result)
{
    ifstream file(fileName, ios::binary);
    if (file) {
        file.read(reinterpret_cast<char *>(ptrLength), sizeof(int));
        if (*ptrLength > 0) {
            auto temp = make_unique_array<char[]>((*ptrLength) + 1);
            temp[*ptrLength] = 0;
            file.read(temp.get(), *ptrLength * sizeof(char));
            result.swap(temp);
        }
        file.close();
        return 1;
    }
    return 0;
}