// qsplit.h

#ifndef _QSPLIT_H_
#define _QSPLIT_H_ 1

#include "Surface.h"                
#include "BBox.h"
                
int qsplit(float* vertices, int size, float pivot_val, int axis)
{
   BBox bbox;
   double centroid;
   int ret_val = 0;

   for (int i = 0; i < size; i++)
   {
      bbox = BBox(vertices + i * 9);
      centroid = ((bbox.min())[axis] + (bbox.max())[axis]) / 2.0f;
      if (centroid < pivot_val)
      {
         Surface* temp = list[i];
         list[i]       = list[ret_val];
         list[ret_val] = temp;
         ret_val++;
      }
   }
   if (ret_val == 0 || ret_val == size) ret_val = size/2;

   return ret_val;
}
#endif // _QSPLIT_H_