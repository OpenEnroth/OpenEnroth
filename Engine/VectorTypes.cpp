#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <utility>

#include "mm7_data.h"
#include "OurMath.h"

//----- (004621DA) --------------------------------------------------------
uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z)
{
  if (x < y)
  {
    std::swap(x, y);
  }
  if (x < z)
  {
    std::swap(x, z);
  }
  if (y < z)
  {
    std::swap(y, z);
  }

  return x + (11 * y >> 5) + (z >> 2);
}

//----- (0044C362) --------------------------------------------------------
template <class T>
void Vec3<T>::Normalize_float()
{
  double x = this->x;
  double y = this->y;
  double z = this->z;
  double s = sqrt(x * x + y * y + z * z);

  this->x = bankersRounding(x / s);
  this->y = bankersRounding(y / s);
  this->z = bankersRounding(z / s);
}

//----- (0043AA99) --------------------------------------------------------
template <class T>
void __fastcall Vec3<T>::Rotate(T sDepth, T sRotY, T sRotX, Vec3<T> v, T *outx, T *outy, T *outz)
{
 float cosf_x = cosf(3.14159265f * sRotX / 1024.0f),
       sinf_x = sinf(3.14159265f * sRotX / 1024.0f),
       cosf_y = cosf(3.14159265f * sRotY / 1024.0f),
       sinf_y = sinf(3.14159265f * sRotY / 1024.0f);

 *outx = v.x + ((unsigned __int64)(sinf_y * (signed __int64)((unsigned __int64)(cosf_x * (signed __int64)sDepth)>> 16)));
 *outy = v.y + ((unsigned __int64)(cosf_y * (signed __int64)((unsigned __int64)(cosf_x * (signed __int64)sDepth)>> 16)));
 *outz = v.z + ((unsigned __int64)(sinf_x * (signed __int64)sDepth) >> 16);
}

//----- (0043AB61) --------------------------------------------------------
template <class T>
void Vec3<T>::Normalize(T *x, T *y, T *z)
{
  extern int integer_sqrt(int val);
  *x *= 65536 / (integer_sqrt(*y * *y + *z * *z + *x * *x) | 1);
  *y *= 65536 / (integer_sqrt(*y * *y + *z * *z + *x * *x) | 1);
  *z *= 65536 / (integer_sqrt(*y * *y + *z * *z + *x * *x) | 1);
}

template Vec3<int32_t>;

//----- (004369DB) --------------------------------------------------------
void Vec3_float_::Normalize()
{
	this->x = (1.0 / sqrt(this->x * this->x + this->y * this->y + this->z * this->z)) * this->x;
	this->y = (1.0 / sqrt(this->x * this->x + this->y * this->y + this->z * this->z)) * this->y;
	this->z = (1.0 / sqrt(this->x * this->x + this->y * this->y + this->z * this->z)) * this->z;
}