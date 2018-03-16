//
// Created by wjy50 on 2018/2/10.
//

#include "texture.h"
#include "../png/png.h"
#include <string.h>

TextureLoadResult loadJpeg(const char* filePath,TextureImage* dest)
{
    FILE* file=fopen(filePath,"rb");
    if(file)
    {
        tjhandle handle;
        handle=tjInitDecompress();
        fseek(file,0,SEEK_END);
        unsigned long jpegSize=ftell(file);
        fseek(file,0,SEEK_SET);
        if(jpegSize > 0)
        {
            unsigned char* jpegBuffer=(unsigned char*)malloc(jpegSize);
            if(!jpegBuffer)
            {
                fclose(file);
                tjDestroy(handle);
                return MALLOC_FAILED;
            }
            fread(jpegBuffer, sizeof(char),jpegSize,file);
            fclose(file);
            if(jpegBuffer[0] != 0xff || jpegBuffer[1] != 0xd8 || jpegBuffer[2] != 0xff)
            {
                tjDestroy(handle);
                free(jpegBuffer);
                fclose(file);
                return INVALID_JPEG;
            }
            int jpegSubSample,jpegColorSpace;
            int r=tjDecompressHeader3(handle,jpegBuffer,jpegSize,&(dest->width),&(dest->height),&jpegSubSample,&jpegColorSpace);
            if(r == 0)
            {
                int pixelFormat=TJPF_RGB;
                int pitch=(dest->width)*tjPixelSize[pixelFormat];
                size_t destSize=(size_t)pitch*(dest->height);
                dest->data =(unsigned char*)malloc(destSize);
                if(!dest->data)
                {
                    tjDestroy(handle);
                    fclose(file);
                    free(jpegBuffer);
                    return MALLOC_FAILED;
                }
                r=tjDecompress(handle, jpegBuffer, jpegSize, dest->data, dest->width, pitch, dest->height, tjPixelSize[pixelFormat], 0);
                if(r == 0)
                {
                    dest->colorType=TEX_RGB;
                    free(jpegBuffer);
                    jpegBuffer=0;
                    tjDestroy(handle);
                    return LOAD_SUCCESS;
                }
                free(dest->data);
                dest->data =0;
            }
            free(jpegBuffer);
            jpegBuffer=0;
        }
        else fclose(file);
        tjDestroy(handle);
    }
    else return OPEN_FILE_FAILED;
    return UNKNOWN_ERROR;
}

TextureLoadResult loadPng(const char* filePath,TextureImage* dest)
{
    FILE* file=fopen(filePath,"rb");
    if(file)
    {
        png_const_bytep sig=(png_const_bytep)malloc(8);
        fread(sig, 1,8,file);
        if(png_sig_cmp(sig,0,8) != 0)
        {
            free(sig);
            fclose(file);
            return INVALID_PNG;
        }
        free(sig);
        fseek(file,0,SEEK_SET);
        png_infop info_ptr;
        png_structp png_ptr;
        png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
        info_ptr=png_create_info_struct(png_ptr);
        if(setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_read_struct(&png_ptr,&info_ptr,0);
            return UNKNOWN_ERROR;
        }
        if(png_ptr && info_ptr)
        {
            png_init_io(png_ptr,file);
            //png_set_sig_bytes(png_ptr,8);
            //png_read_info(png_ptr,info_ptr);
            png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_EXPAND,0);
            dest-> width=png_get_image_width(png_ptr,info_ptr);
            dest-> height=png_get_image_height(png_ptr,info_ptr);
            int color_type=png_get_color_type(png_ptr,info_ptr);
            size_t size=(size_t)(dest->width*dest->height*(color_type == PNG_COLOR_TYPE_RGBA ? 4 : 3));
            unsigned char* color=(unsigned char*)malloc(size);
            if(color)
            {
                png_bytep * row_pointers=png_get_rows(png_ptr,info_ptr);
                if(color_type == PNG_COLOR_TYPE_RGBA)
                {
                    dest->colorType= TEX_ARGB;
                    for (int i = 0; i < dest->height; ++i) {
                        int o=(i*dest->width)<<2;
                        for (int j = 0; j < dest->width; ++j) {
                            int offset=j<<2;
                            color[o+offset]=row_pointers[i][offset];
                            color[o+offset+1]=row_pointers[i][offset+1];
                            color[o+offset+2]=row_pointers[i][offset+2];
                            color[o+offset+3]=row_pointers[i][offset+3];
                        }
                    }
                }
                else if (color_type == PNG_COLOR_TYPE_RGB)
                {
                    dest->colorType=TEX_RGB;
                    for (int i = 0; i < dest->height; ++i) {
                        int o=(i*dest->width)*3;
                        for (int j = 0; j < dest->width; ++j) {
                            int offset=j*3;
                            color[o+offset]=row_pointers[i][offset];
                            color[o+offset+1]=row_pointers[i][offset+1];
                            color[o+offset+2]=row_pointers[i][offset+2];
                        }
                    }
                }
                else
                {
                    png_read_end(png_ptr,info_ptr);
                    png_destroy_read_struct(&png_ptr,&info_ptr,0);
                    fclose(file);
                    return NOT_SUPPORTED_PNG_COLOR_TYPE;
                }
                png_destroy_read_struct(&png_ptr,&info_ptr,0);
                dest->data=color;
                fclose(file);
                return LOAD_SUCCESS;
            }
            else
            {
                png_read_end(png_ptr,info_ptr);
                png_destroy_read_struct(&png_ptr,&info_ptr,0);
                fclose(file);
                return MALLOC_FAILED;
            }
        }
    }
    else return OPEN_FILE_FAILED;
    return UNKNOWN_ERROR;
}

TextureLoadResult loadBmp(const char* filePath,TextureImage* dest)
{
    FILE* file=fopen(filePath,"rb");
    if (file)
    {
        unsigned short magic;
        fread(&magic, sizeof(unsigned short),1,file);
        if(magic == 0x4d42)
        {
            int size;
            int offset;
            fread(&size, sizeof(int),1,file);
            fseek(file,4,SEEK_CUR);
            fread(&offset, sizeof(int),1,file);
            int biSize;
            int width,height;
            short planes;
            short bitCount;
            int compression;
            int imageSize;
            int xPelsPerMeter;
            int yPelsPerMeter;
            int colorUsed;
            int importantColor;
            fread(&biSize, sizeof(int),1,file);
            fread(&width, sizeof(int),1,file);
            fread(&height, sizeof(int),1,file);
            fread(&planes, sizeof(short),1,file);
            fread(&bitCount,sizeof(short),1,file);
            if(bitCount != 24 && bitCount != 32)
            {
                fclose(file);
                return NOT_SUPPORTED_BMP_COLOR_TYPE;
            }
            fread(&compression, sizeof(int),1,file);
            if(compression != 0)
            {
                fclose(file);
                return NOT_SUPPORTED_BMP_COMPRESSION_TYPE;
            }
            fread(&imageSize, sizeof(int),1,file);
            fread(&xPelsPerMeter, sizeof(int),1,file);
            fread(&yPelsPerMeter, sizeof(int),1,file);
            fread(&colorUsed, sizeof(int),1,file);
            fread(&importantColor, sizeof(int),1,file);
            if(imageSize == 0)imageSize=width*height*bitCount>>3;
            long curPos=ftell(file);
            if(curPos == offset)
            {
                unsigned char* color=(unsigned char*)malloc(imageSize);
                if(!color)
                {
                    fclose(file);
                    return MALLOC_FAILED;
                }
                int byteCount=bitCount>>3;
                //int hByteCount=byteCount>>1;
                int pitch=width*byteCount;
                for (int i = 0; i < height; ++i) {
                    int o=(height-i-1)*pitch;
                    fread(color+o, sizeof(unsigned char),pitch,file);
                }
                fread(color, sizeof(unsigned char),imageSize,file);
                dest->width=width;
                dest->height=height;
                dest->data=color;
                dest->colorType=bitCount == 24 ? TEX_RGB : TEX_ARGB;
                fclose(file);
                for (int i = 0; i < imageSize; i+=byteCount) {
                    color[i+2]^=color[i];
                    color[i]=color[i+2]^color[i];
                    color[i+2]^=color[i];
                }
                return LOAD_SUCCESS;
            }
        }
        else
        {
            fclose(file);
            return INVALID_BMP;
        }
        fclose(file);
    }
    else return OPEN_FILE_FAILED;
    return UNKNOWN_ERROR;
}

TextureLoadResult loadTga(const char* filePath,TextureImage* dest)
{
    FILE* file=fopen(filePath,"rb");
    if(file)
    {
        unsigned char picInfoLen;
        fread(&picInfoLen, sizeof(unsigned char),1,file);
        fseek(file,1,SEEK_CUR);
        unsigned char type;
        fread(&type, sizeof(unsigned char),1,file);
        fseek(file,5,SEEK_CUR);
        int picType=2;
        unsigned short lbx,lby;
        fread(&lbx, sizeof(unsigned short),1,file);
        fread(&lby, sizeof(unsigned short),1,file);
        unsigned short width,height;
        fread(&width, sizeof(unsigned short),1,file);
        fread(&height, sizeof(unsigned short),1,file);
        dest->width=width;
        dest->height=height;
        unsigned char bitsPerPix,screenStart;
        fread(&bitsPerPix, sizeof(unsigned char),1,file);
        fread(&screenStart, sizeof(unsigned char),1,file);
        screenStart=(unsigned char)(screenStart&0x10)>>4;
        if(picInfoLen > 0)fseek(file,picInfoLen,SEEK_CUR);
        int area=width*height;
        int byteCount=bitsPerPix>>3;
        int size=area*byteCount;
        unsigned char* color=(unsigned char*)malloc(size);
        if(!color)
        {
            fclose(file);
            return MALLOC_FAILED;
        }
        if(type == 2)
        {
            if(bitsPerPix == 32)
            {
                dest->colorType=TEX_ARGB;
            }
            else if(bitsPerPix == 24)
            {
                dest->colorType=TEX_RGB;
            }
            else
            {
                free(color);
                fclose(file);
                return NOT_SUPPORTED_TGA_COLOR_TYPE;
            }
            int pitch=width*byteCount;
            for (int i = 0; i < height; ++i) {
                int offset=(height-i-1)*pitch;
                fread(color+offset, sizeof(unsigned char),pitch,file);
            }
        }
        else if(type == 10)
        {
            int hi=0,wi=0;
            int offset=(height-hi-1)*width;
            if (bitsPerPix == 32)
            {
                dest->colorType=TEX_ARGB;
                unsigned int* colorInt=(unsigned int*)color;
                while (hi < height)
                {
                    unsigned char head;
                    fread(&head, sizeof(unsigned char),1,file);
                    unsigned char dataType=head>>7;
                    unsigned char len=(unsigned char)((head&0x7f)+1);
                    if(dataType == 0)
                    {
                        while (wi+len >= width)
                        {
                            int count=width-wi;
                            fread(colorInt+offset+wi, sizeof(unsigned int),count,file);
                            wi=0;
                            hi++;
                            offset=(height-hi-1)*width;
                            len-=count;
                        }
                        fread(colorInt+offset+wi, sizeof(unsigned int),len,file);
                        wi+=len;
                    }
                    else
                    {
                        unsigned int c;
                        fread(&c, sizeof(unsigned int),1,file);
                        while (wi+len >= width)
                        {
                            int count=width-wi;
                            for (int i = 0; i < count; ++i) {
                                colorInt[offset+wi+i]=c;
                            }
                            wi=0;
                            hi++;
                            offset=(height-hi-1)*width;
                            len-=count;
                        }
                        for (int i = 0; i < len; ++i) {
                            colorInt[offset+wi+i]=c;
                        }
                        wi+=len;
                    }
                }
            }
            else if(bitsPerPix == 24)
            {
                dest->colorType=TEX_RGB;
                while (hi < height)
                {
                    unsigned char head;
                    fread(&head, sizeof(unsigned char),1,file);
                    unsigned char dataType=head>>7;
                    unsigned char len=(unsigned char)((head&0x7f)+1);
                    if(dataType == 0)
                    {
                        while (wi+len >= width)
                        {
                            int count=width-wi;
                            fread(color+(offset+wi)*3, sizeof(unsigned char),count*3,file);
                            wi=0;
                            hi++;
                            offset=(height-hi-1)*width;
                            len-=count;
                        }
                        fread(color+(offset+wi)*3, sizeof(unsigned char),len*3,file);
                        wi+=len;
                    }
                    else
                    {
                        unsigned int c;
                        fread(&c, sizeof(unsigned char),3,file);
                        while (wi+len >= width)
                        {
                            int count=width-wi;
                            for (int i = 0; i < count; ++i) {
                                int o=(offset+wi+i)*3;
                                color[o]=(unsigned char)((c>>16)&0xff);
                                color[o+1]=(unsigned char)((c>>8)&0xff);
                                color[o+2]=(unsigned char)(c&0xff);
                            }
                            wi=0;
                            hi++;
                            offset=(height-hi-1)*width;
                            len-=count;
                        }
                        for (int i = 0; i < len; ++i) {
                            int o=(offset+wi+i)*3;
                            color[o]=(unsigned char)(c&0xff);
                            color[o+1]=(unsigned char)((c>>8)&0xff);
                            color[o+2]=(unsigned char)((c>>16)&0xff);
                        }
                        wi+=len;
                    }
                }
            }
            else
            {
                free(color);
                fclose(file);
                return NOT_SUPPORTED_TGA_COLOR_TYPE;
            }
        }
        else
        {
            free(color);
            fclose(file);
            return UNKNOWN_TGA_TYPE;
        }
        for (int i = 0; i < area; ++i) {
            int offset=i*byteCount;
            color[offset+2]^=color[offset];
            color[offset]=color[offset+2]^color[offset];
            color[offset+2]^=color[offset];
        }
        dest->data=color;
        fclose(file);
        return LOAD_SUCCESS;
    }
    else return OPEN_FILE_FAILED;
}

int tryJpeg(const char* filePath,TextureImage* dest)
{
    TextureLoadResult result=loadJpeg(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_JPEG)return 0;
    result=loadPng(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_PNG)return 0;
    result=loadBmp(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_BMP)return 0;
    result=loadTga(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    return 0;
}

int tryPng(const char* filePath,TextureImage* dest)
{
    TextureLoadResult result=loadPng(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_PNG)return 0;
    result=loadBmp(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_BMP)return 0;
    result=loadJpeg(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_JPEG)return 0;
    result=loadTga(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    return 0;
}

int tryTga(const char* filePath,TextureImage* dest)
{
    TextureLoadResult result=loadTga(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result == MALLOC_FAILED)return 0;
    result=loadJpeg(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_JPEG)return 0;
    result=loadPng(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_PNG)return 0;
    result=loadBmp(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    return 0;
}

int tryBmp(const char* filePath,TextureImage* dest)
{
    TextureLoadResult result=loadBmp(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_BMP)return 0;
    result=loadJpeg(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_JPEG)return 0;
    result=loadPng(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    else if(result != INVALID_PNG)return 0;
    result=loadTga(filePath,dest);
    if(result == LOAD_SUCCESS)return 1;
    return 0;
}

int loadTexture(const char* filePath,TextureImage* dest)
{
    int len=strlen(filePath);
    int index=-1;
    for (int i = len-1; i >= 0; --i) {
        if(filePath[i] == '.')
        {
            index=i+1;
            break;
        }
    }
    int r=0;
    if(index > 0)//try according to postfix
    {
        int postfixLen=len-index;
        char* postfix=(char*)malloc(postfixLen+1);
        postfix[postfixLen]=0;
        for (int i = 0; i < postfixLen; ++i) {
            if(filePath[index+i] >= 'A' && filePath[index+i] <= 'Z')postfix[i]=(char)(filePath[index+i]-'A'+'a');
            else postfix[i]=filePath[index+i];
        }
        if(postfixLen == 4)r=tryJpeg(filePath,dest);//*.jpeg probably
        else if(postfixLen == 3)
        {
            r=-1;
            if(postfix[0] == 'p')
            {
                if(postfix[1] == 'n' && postfix[2] == 'g')r=tryPng(filePath,dest);
            }
            else if(postfix[0] == 't')
            {
                if(postfix[1] == 'g' && postfix[2] == 'a')r=tryTga(filePath,dest);
            }
            else if(postfix[0] == 'j')
            {
                if(postfix[1] == 'p' && postfix[2] == 'g')r=tryJpeg(filePath,dest);
            }
            else if(postfix[0] == 'b')
            {
                if(postfix[1] == 'm' && postfix[2] == 'p')r=tryBmp(filePath,dest);
            }
            else if(postfix[0] == 's' && postfix[1] == 'p' && (postfix[2] == 'a' || postfix[2] == 'h'))r=tryBmp(filePath,dest);
            if(r == -1)r=tryJpeg(filePath,dest);
        }
        else r=tryJpeg(filePath,dest);
        free(postfix);
    }
    else r=tryJpeg(filePath,dest);
    return r;
}