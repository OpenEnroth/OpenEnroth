#pragma once

#include <cassert>
#include <limits>
#define pi_double 3.14159265358979323846
/*  186 */
#pragma pack(push, 1)
struct stru193_math
{
  stru193_math();
  int Cos(int angle);
  unsigned int Atan2(int x, int y);
  int Sin(int angle);

  int pTanTable[520];
  int pCosTable[520];
  int pInvCosTable[520];
  static const unsigned int uIntegerPi = 1024;
  static const unsigned int uIntegerHalfPi = 512;
  static const unsigned int uIntegerDoublePi = 2048;
  static const unsigned int uDoublePiMask = 2047;
  static const unsigned int uPiMask = 1023;
  static const unsigned int uHalfPiMask = 511;
};
#pragma pack(pop)

__int64 fixpoint_mul(int, int);
__int64 fixpoint_dot(int x1, int x2, int y1, int y2, int z1, int z2);
__int64 fixpoint_div(int, int);
__int64 fixpoint_sub_unknown(int, int);
int fixpoint_from_float(float value);
int fixpoint_from_int(int lhv, int rhv);
int integer_sqrt(int val);
int __fastcall GetDiceResult(unsigned int uNumDice, unsigned int uDiceSides); // idb
inline int round_to_int(float x) { return (int)floor(x + 0.5f); }

template <typename FloatType>
inline int bankersRounding(const FloatType& value)
{
    assert("Method unsupported for this type" && false);
    return value;
}

template<> inline int bankersRounding<float>(const float& inValue)
{
  union Cast
  {
    double d;
    long l;
  };
  volatile Cast c;
  c.d = inValue + 6755399441055744.0;
  return c.l;
}

#pragma push_macro("max")
#undef max
template<> inline int bankersRounding<double>(const double& inValue)
{
  double maxValue = std::numeric_limits<int>::max();
  assert(maxValue - 6755399441055744.0 >= inValue);
  union Cast
  {
    double d;
    long l;
  };
  volatile Cast c;
  c.d = inValue + 6755399441055744.0;
  return c.l;
}
#pragma pop_macro("max")

extern struct stru193_math *stru_5C6E00;
