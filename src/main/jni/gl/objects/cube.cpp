#include "cube.h"

//
// Created by wjy50 on 2018/2/5.
//
Cube::Cube(float width, float height, float thickness) : GLObject()
{
    float hw = width / 2;
    float hh = height / 2;
    float ht = thickness / 2;

    float *coordinates = new float[72];
    float *normals = new float[72];

    coordinates[0] = coordinates[9] = -hw;
    coordinates[1] = coordinates[4] = hh;
    coordinates[2] = coordinates[5] = coordinates[8] = coordinates[11] = ht;
    coordinates[3] = coordinates[6] = hw;
    coordinates[7] = coordinates[10] = -hh;

    coordinates[12] = coordinates[15] = coordinates[18] = coordinates[21] = -hw;
    coordinates[13] = coordinates[16] = hh;
    coordinates[14] = coordinates[23] = -ht;
    coordinates[17] = coordinates[20] = ht;
    coordinates[19] = coordinates[22] = -hh;

    coordinates[24] = coordinates[33] = hw;
    coordinates[25] = coordinates[28] = hh;
    coordinates[26] = coordinates[29] = coordinates[32] = coordinates[35] = -ht;
    coordinates[27] = coordinates[30] = -hw;
    coordinates[31] = coordinates[34] = -hh;

    coordinates[36] = coordinates[39] = coordinates[42] = coordinates[45] = hw;
    coordinates[37] = coordinates[40] = hh;
    coordinates[38] = coordinates[47] = ht;
    coordinates[41] = coordinates[44] = -ht;
    coordinates[43] = coordinates[46] = -hh;

    coordinates[48] = coordinates[57] = -hw;
    coordinates[49] = coordinates[52] = coordinates[55] = coordinates[58] = hh;
    coordinates[50] = coordinates[53] = -ht;
    coordinates[51] = coordinates[54] = hw;
    coordinates[56] = coordinates[59] = ht;

    coordinates[60] = coordinates[69] = -hw;
    coordinates[61] = coordinates[64] = coordinates[67] = coordinates[70] = -hh;
    coordinates[62] = coordinates[65] = ht;
    coordinates[63] = coordinates[66] = hw;
    coordinates[68] = coordinates[71] = -ht;

    for (int i = 0; i < 4; ++i) {
        int offset = 3 * i;
        normals[offset] = normals[offset + 1] = 0;
        normals[offset + 2] = 1;

        offset += 12;
        normals[offset + 2] = normals[offset + 1] = 0;
        normals[offset] = -1;

        offset += 12;
        normals[offset] = normals[offset + 1] = 0;
        normals[offset + 2] = -1;

        offset += 12;
        normals[offset + 2] = normals[offset + 1] = 0;
        normals[offset] = 1;

        offset += 12;
        normals[offset] = normals[offset + 2] = 0;
        normals[offset + 1] = 1;

        offset += 12;
        normals[offset] = normals[offset + 2] = 0;
        normals[offset + 1] = -1;
    }

    setVertices(24, coordinates, normals);

    unsigned int *indices = new unsigned int[36];

    for (unsigned int i = 0; i < 6; ++i) {
        unsigned int offset1 = i * 4;
        int offset2 = i * 6;
        indices[offset2] = indices[offset2 + 3] = offset1;
        indices[offset2 + 1] = offset1 + 3;
        indices[offset2 + 2] = indices[offset2 + 4] = offset1 + 2;
        indices[offset2 + 5] = offset1 + 1;
    }

    setIndices(36, indices);

    genVertexBuffers();

    initShader();

    setAmbient(0, 0, .5f);

    setDiffuse(0, 0, 1, 1);

    setSpecular(0, 0, 1);

    setShininess(20);
}