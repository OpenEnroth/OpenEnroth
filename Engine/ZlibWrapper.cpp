
namespace zlib
{
#include "lib\zlib\zlib.h"
  int MemUnzip(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen)
  {
    //return uncompress((zlib::Bytef *)dest, (zlib::uLongf *)destLen, (zlib::Bytef *)source, sourceLen);
    return uncompress((Bytef *)dest, (uLongf *)destLen, (Bytef *)source, sourceLen);
    return 0;
  }

  int MemZip(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen)
  {
    //return compress((zlib::Bytef *)dest, (zlib::uLongf *)destLen, (zlib::Bytef *)source, sourceLen);
    return compress((Bytef *)dest, (uLongf *)destLen, (Bytef *)source, sourceLen);
    return 0;
  }
};