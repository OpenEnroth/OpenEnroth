#pragma once


/*  179 */
#pragma pack(push, 1)
struct stru314
{
  //----- (00489B60) --------------------------------------------------------
  stru314()
  {
    this->Normal.x = 0.0;
    this->Normal.y = 0.0;
    this->Normal.z = 0.0;

    this->field_10.x = 0.0;
    this->field_10.y = 0.0;
    this->field_10.z = 0.0;

    this->field_1C.x = 0.0;
    this->field_1C.y = 0.0;
    this->field_1C.z = 0.0;
    //this->vdestructor_ptr = &stru314_pvdtor;
  }
  
  //----- (00489B96) --------------------------------------------------------
  inline ~stru314()
  {}


  void (__fastcall ***vdestructor_ptr)(stru314 *, bool);
  Vec3_float_ Normal;
  Vec3_float_ field_10;
  Vec3_float_ field_1C;
  float dist;
};
#pragma pack(pop)
