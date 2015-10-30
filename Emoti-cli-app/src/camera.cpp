#include <time.h>

#include "camera.h"
#include <stdio.h>
#include <png.h>
#include <QObject>

Camera::Camera()
    :cam(new cv::VideoCapture())
{
}

bool Camera::initCamera(int _device)
{
    bool check = this->cam->open(_device);

    if (!check)
        fprintf(stderr, "Could not open device %d\n",_device);

    return check;
}

std::shared_ptr<cv::Mat> Camera::getImage()
{
    if (!this->cam->isOpened())
    {
        fprintf(stderr, "Camera is not accesible");
        return nullptr;
    }

    std::shared_ptr<cv::Mat> frame (new cv::Mat());
    cam->operator >> (*frame);

    return frame;

}


void my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  /* with libpng15 next line causes pointer deference error; use libpng12 */
  struct mem_encode* p=(struct mem_encode*)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
  size_t nsize = p->size + length;

  /* allocate or grow buffer */
  if(p->buffer)
  {
      p->buffer= ((char*) realloc(p->buffer, nsize));
  }
  else
  {
    p->buffer = (char*) malloc(nsize);
    p->allocd = nsize;
  }

  if(!p->buffer)
    png_error(png_ptr, "Allocate error");

  /* copy new bytes to end of buffer */
  memcpy(p->buffer + p->size, data, length);
  p->size += length;
}

void my_png_flush(png_structp png_ptr)
{
    struct mem_encode* p=(struct mem_encode*)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
    free(p->buffer);
}


inline void setRGB(png_byte *ptr, long int val)
{
    ptr[0] = val / 256 / 256 % 256;
    ptr[1] = val / 256 % 256;
    ptr[2] = val % 256;
}

std::shared_ptr<struct mem_encode*> mat2png(cv::Mat& _img)
{

    struct mem_encode *output  = (struct mem_encode *) malloc (sizeof(mem_encode));
    output->buffer = NULL;
    output->size = 0;
    output->allocd = 0;

    if (&_img == nullptr)
    {
        fprintf(stderr, "Could not read image;");
        return std::make_shared<struct mem_encode*> (output);
    }

    png_structp pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop infop;

    if (!pngp)
    {
        fprintf(stderr, "Error allocating psp");
        return std::make_shared<struct mem_encode*> (output);
    }

    /* Allocate/initialize the image information data.  REQUIRED */
    infop = png_create_info_struct(pngp);
    if (infop == NULL)
    {
        png_destroy_write_struct(&pngp, (png_infopp)NULL);
        return std::make_shared<struct mem_encode*> (output);
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(pngp))) {
            fprintf(stderr, "Error during png creation");
            png_destroy_write_struct(&pngp, &infop);
            return nullptr;
    }


    //Set chunck information
    png_set_IHDR(pngp, infop, _img.cols, _img.rows,
                    8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);



    //Set image comments with username
//    const char *name = qgetenv("USER").constData();
//    if (name == NULL)
//    {
//        name = qgetenv("USERNAME").data();
//    }

    png_text titletext;
    titletext.compression = PNG_TEXT_COMPRESSION_NONE;
    titletext.key = (char*) "Emoti";
    titletext.text = (char*) "Pruebas";
    titletext.text_length = strlen(titletext.text);
    titletext.itxt_length = 0;
    titletext.lang = NULL;
    titletext.lang_key = NULL;

    png_set_text(pngp, infop, &titletext, 1);


    //Set modification time for the image
    png_time timestamp;
    time_t rawtime;
    time (&rawtime);
    struct tm *timeinfo = localtime (&rawtime);

    timestamp.day = timeinfo->tm_mday;
    timestamp.hour = timeinfo->tm_hour;
    timestamp.minute = timeinfo->tm_min;
    timestamp.month = timeinfo->tm_mon;
    timestamp.second = timeinfo->tm_sec;
    timestamp.year = timeinfo->tm_year + 1900;

    png_set_tIME(pngp, infop, &timestamp);

    //Let's write into memory
    png_set_write_fn(pngp, &output, my_png_write_data, my_png_flush);

    png_write_info(pngp, infop);


    png_bytep row = (png_bytep) malloc(_img.cols * 3 * sizeof(png_byte));
    int x, y;
    for (y = 0 ; y < _img.rows; y++)
    {
        for (x = 0; x < _img.cols; x++)
        {
            setRGB(&(row[x*3]), _img.data[y*_img.cols + x]);
        }
        png_write_row(pngp, row);
    }

    png_write_end(pngp, NULL);

    //FREEDOM

    free(row);

    if (infop != NULL)
    {
        png_destroy_info_struct (pngp, &infop );
        png_free_data(pngp, infop, PNG_FREE_ALL, -1);
    }
    if (pngp != NULL) png_destroy_write_struct(&pngp, (png_infopp)NULL);




    return std::make_shared<struct mem_encode*> (output);
}

