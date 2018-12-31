#include <stdio.h>
#include <string.h>
#include "GL/freeglut.h"
#include "GL/gl.h"

/***************************************************************

    sudo apt-get install mesa-common-dev
    sudo apt-get install freeglut3-dev

     g++ secondWindow.cpp -lglut -lGLU -lGL
     ./a.out
*****************************************************************/

#pragma pack(2) /***** 2 byte aligned not 4 byte aligned *****/

typedef struct                       /**** BMP file header structure ****/
{
    unsigned short bfType;           /* Magic number for file */
    unsigned int   bfSize;           /* Size of file */
    unsigned short bfReserved1;      /* Reserved */
    unsigned short bfReserved2;      /* ... */
    unsigned int   bfOffBits;        /* Offset to bitmap data */
} BITMAPFILEHEADER;

#pragma pack()  /***** return to 4 byte aligned *****/

#  define BF_TYPE 0x4D42             /* "MB" */

typedef struct                       /**** BMP file info structure ****/
{
    unsigned int   biSize;           /* Size of info header */
    int            biWidth;          /* Width of image */
    int            biHeight;         /* Height of image */
    unsigned short biPlanes;         /* Number of color planes */
    unsigned short biBitCount;       /* Number of bits per pixel */
    unsigned int   biCompression;    /* Type of compression to use */
    unsigned int   biSizeImage;      /* Size of image data */
    int            biXPelsPerMeter;  /* X pixels per meter */
    int            biYPelsPerMeter;  /* Y pixels per meter */
    unsigned int   biClrUsed;        /* Number of colors used */
    unsigned int   biClrImportant;   /* Number of important colors */
} BITMAPINFOHEADER;


/*
 * Constants for the biCompression field...
 */

#  define BI_RGB       0             /* No compression - straight BGR data */
#  define BI_RLE8      1             /* 8-bit run-length compression */
#  define BI_RLE4      2             /* 4-bit run-length compression */
#  define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */

typedef struct                       /**** Colormap entry structure ****/
{
    unsigned char  rgbBlue;          /* Blue value */
    unsigned char  rgbGreen;         /* Green value */
    unsigned char  rgbRed;           /* Red value */
    unsigned char  rgbReserved;      /* Reserved */
} RGBQUAD;

class Bitmap {
public:
    Bitmap(const char* filename);
    ~Bitmap();
    RGBQUAD* pixels;
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;

    private:

};


void *ReadBitmap(char *filename, BITMAPINFOHEADER *ihptr)
{
    BITMAPFILEHEADER h;
    void *bits = NULL;
    FILE *file = fopen(filename, "rb");

    if(file != NULL)
    {
        printf("sizeof(BITMAPFILEHEADER) == %lu\n", sizeof(BITMAPFILEHEADER));
        size_t x = fread(&h, 1, sizeof(BITMAPFILEHEADER), file);
        printf("bytes read == %lu\n", x);
        printf("bfOffBits == %u\n", h.bfOffBits);


        printf("sizeof(BITMAPINFOHEADER) == %lu\n", sizeof(BITMAPINFOHEADER));
        size_t zz = fread(ihptr, 1, sizeof(BITMAPINFOHEADER), file);
        printf("bytes read == %lu\n", zz);
        printf("width == %d\n", ihptr->biWidth);
        printf("height == %d\n", ihptr->biHeight);
        printf("biPlanes == %u\n", ihptr->biPlanes);
        printf("biBitCount == %u\n", ihptr->biBitCount);
        printf("biCompression == %u\n", ihptr->biCompression);
        printf("biSizeImage = %u\n", ihptr->biSizeImage);

        int checksize = ihptr->biWidth * ihptr->biHeight * 3;
        printf("checksize = %d\n", checksize);

        int fseekstatus = fseek(file, h.bfOffBits, SEEK_SET);
        if (fseekstatus == 0)
           printf("good fseek\n");
        else
        {
           printf("bad fseek\n");
           fclose(file);
           return NULL;
        }

        bits = malloc(ihptr->biSizeImage);
        if (bits == NULL)
        {
           printf("bad malloc\n");
           fclose(file);
           return NULL;
        }
        else
           printf("good malloc\n");

        size_t bytesread = fread(bits, 1, ihptr->biSizeImage, file);
        
        fclose(file);

        if (bytesread == ihptr->biSizeImage)
        {
            printf("good fread\n");
            return bits;
        }
        else
        {
            printf("bad fread bytesread == %lu\n", bytesread);
            free(bits);
            return NULL;
        }
    }

    printf("failed to open file\n");
    return NULL;
}


void render()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBindTexture(GL_TEXTURE_2D, 13);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2f(-0.5f, -0.6975f);

        glTexCoord2f(0.0, 1.0);
        glVertex2f(-0.5f, 0.6975f);

        glTexCoord2f(1.0, 1.0);
        glVertex2f(0.5f, 0.6975f);

        glTexCoord2f(1.0, 0.0);
        glVertex2f(0.5f, -0.6975f);
    glEnd();

    glFlush();
}


void reshape(GLsizei width, GLsizei height)
{
    if (height == 0)
        height = 1;

    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (width >= height)
        gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
    else
        gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);

}


int main(int argc, char** argv)
{

    BITMAPINFOHEADER info;
    void *data = ReadBitmap("./B.bmp", &info);

    glutInit(&argc, argv);
    glutInitWindowSize(600,600);
    glutCreateWindow("Applying Textures");
    glutDisplayFunc(render);
    glutReshapeFunc(reshape);

    //******** load texture into video memory ************
    glBindTexture(GL_TEXTURE_2D, 13);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, info.biWidth, info.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glutMainLoop();    
    return 0;
}
