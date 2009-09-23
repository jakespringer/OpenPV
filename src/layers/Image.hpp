/*
 * Image.hpp
 *
 *  Created on: Sep 8, 2009
 *      Author: rasmussn
 */

#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include "Image.hpp"
#include "../columns/HyPerCol.hpp"

namespace PV {

class Image {

protected:
   Image(HyPerCol * hc);

public:
   Image(const char * filename, HyPerCol * hc);
   virtual ~Image();

   virtual bool updateImage(float time, float dt);

   int read(const char * filename);
   int write(const char * filename);

   pvdata_t * getImageBuffer()    { return data; }

   int  toGrayScale();
   int  convolution();
   void setTau(int t)             { tau = t; }

   LayerLoc getImageLoc()         { return loc; }

protected:

   Communicator * comm;  // the communicator object for reading/writing files
   LayerLoc loc;         // size/location of image
   pvdata_t *  data;     // buffer containing image
   float tau;
};

}

#endif /* IMAGE_HPP_ */
