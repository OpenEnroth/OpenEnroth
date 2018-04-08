#pragma once

#include <cstdint>

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z);

#pragma pack(push, 1)
template <class T>
struct Vec2
{
  T x;
  T y;

  inline Vec2(T a = 0, T b = 0):
    x(a), y(b)
  {}
};
#pragma pack(pop)

#define Vec2_int_ Vec2<int32_t>
#define Vec2_float_ Vec2<float>

#pragma pack(push, 1)
template <class T>
struct Vec3: public Vec2<T>
{
  T z;

  inline Vec3(T a = 0, T b = 0, T c = 0):
    Vec2<T>(a, b), z(c)
  {}

  void Normalize_float();
  template <class U>
  inline uint32_t GetDistanceTo(Vec3<U> &o)
  {
    return int_get_vector_length(
      abs(this->x - o.x),
      abs(this->y - o.y),
      abs(this->z - o.z)
    );
  }

  static void Rotate(T sDepth, T sRotY, T sRotX, Vec3<T> v, T *outx, T *outy, T *outz);
  static void Normalize(T *x, T *y, T *z);
};
#pragma pack(pop)

#define Vec3_short_ Vec3<int16_t>
#define Vec3_int_ Vec3<int32_t>

#pragma pack(push, 1)
struct Vec3_float_
{
  void Normalize();
  
  //----- (0049B32D) --------------------------------------------------------
  static Vec3_float_ *Cross(Vec3_float_ *v1, Vec3_float_ *pOut, float x, float y, float z)
  {
    pOut->x = z * v1->y - y * v1->z;
    pOut->y = x * v1->z - z * v1->x;
    pOut->z = y * v1->x - x * v1->y;
    return pOut;
  }

  //----- (0049B02E) --------------------------------------------------------
  inline static float NegDot(Vec3_float_ *a1, Vec3_float_ *a2, float *a3)
  {
    return *a3 = -(a1->z * a2->z + a1->y * a2->y + a1->x * a2->x);
  }

  float x;
  float y;
  float z;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Vec4_int_
{
  int x;
  int y;
  int z;
  int w;
};
#pragma pack(pop)

/*   82 */
#pragma pack(push, 1)
struct Plane_int_
{
  Vec3_int_ vNormal;
  int dist;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BBox_short_
{
  int16_t x1;
  int16_t x2;
  int16_t y1;
  int16_t y2;
  int16_t z1;
  int16_t z2;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BBox_int_
{
  int x1;
  int x2;
  int y1;
  int y2;
  int z1;
  int z2;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Plane_float_
{
  struct Vec3_float_ vNormal;
  float dist;
};
#pragma pack(pop)

/*  196 */
#pragma pack(push, 1)
struct Matrix3x3_float_
{
  union
  {
    struct
    {
      float _11; float _12; float _13;
      float _21; float _22; float _23;
      float _31; float _32; float _33;
    };
    float v[3][3];
  };
};
#pragma pack(pop)
