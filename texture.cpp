#include "texture.h"
#include <iostream>
#include <fstream>
extern int g_Width;
extern int g_Height;

GLuint loadBMP_custom(const char * imagepath){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file)							    {printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0;}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);
	// Everything is in memory now, the file wan be closed
	fclose (file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

#define BMP_Header_Length 54
GLubyte* grab(const char* img_path, GLubyte* pPixelData)
{
    FILE*    pDummyFile;
    FILE*    pWritingFile;
    GLubyte  BMP_Header[BMP_Header_Length];
    GLint    i, j;
    GLint    PixelDataLength;

    // 计算像素数据的实际长度
    i = g_Width * 3;   // 得到每一行的像素数据长度
    while( i%4 != 0 )      // 补充数据，直到i是的倍数
        ++i;               // 本来还有更快的算法，
                           // 但这里仅追求直观，对速度没有太高要求
    PixelDataLength = i * g_Height;

    // 分配内存和打开文件
    if (pPixelData == 0)
    	pPixelData = new GLubyte[PixelDataLength];
    if( pPixelData == 0 )
        exit(0);

    // 读取像素
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glReadPixels(0, 0, g_Width, g_Height,
        GL_BGR_EXT, GL_UNSIGNED_BYTE, pPixelData);

    if (img_path) {
	    pDummyFile = fopen("leather.bmp", "rb");
	    if( pDummyFile == 0 )
	        exit(0);

	    pWritingFile = fopen(img_path, "wb");
	    if( pWritingFile == 0 )
	        exit(0);

	    // 把dummy.bmp的文件头复制为新文件的文件头
	    fread(BMP_Header, sizeof(BMP_Header), 1, pDummyFile);
	    fwrite(BMP_Header, sizeof(BMP_Header), 1, pWritingFile);
	    fseek(pWritingFile, 0x0012, SEEK_SET);
	    i = g_Width;
	    j = g_Height;
	    fwrite(&i, sizeof(i), 1, pWritingFile);
	    fwrite(&j, sizeof(j), 1, pWritingFile);

    // 写入像素数据
	    fseek(pWritingFile, 0, SEEK_END);
	    fwrite(pPixelData, PixelDataLength, 1, pWritingFile);

    // 释放内存和关闭文件
	    fclose(pDummyFile);
	    fclose(pWritingFile);
	}
    return pPixelData;
}
