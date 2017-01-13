#define _CRT_SECURE_NO_WARNINGS
/*


void reset_some_strus_flt_2Cs();
double get_shading_dist_mist();
void CreateAsyncMouse();
void CreateAsyncKeyboard();
double GetFogDensityByTime(struct OutdoorLocation *_this);
//void ShowMM7IntroVideo_and_LoadingScreen();


//  128
#pragma pack(push, 1)
struct stru11
{

  stru11();

  void CheckCPU();
  void RunCPUID();
  void RunCPUID_op1();
  void RunCPUID_op2();
  void RunCPUID_ext2_3_4();

  int cpuid_00000000_eax_numops;
  int cpuid_00000000_ebx_vendorstr1;
  int cpuid_00000000_edx_vendorstr2;
  int cpuid_00000000_ecx_vendorstr3;
  char pCPUString[16];
  int cpuid_00000001_eax;
  int cpuid_00000001_edx;
  int cpuid_00000002_eax;
  int cpuid_00000002_ebx;
  int cpuid_00000002_ecx;
  int cpuid_00000002_edx;
  int field_38;
  int field_3C;
  int field_40[8];
  int cpuid_80000000_edx;
  int cpuid_80000001_eax;
  int cpuid_80000001_edx;
  int cpuid_80000002_registers[4];
  int cpuid_80000003_registers[4];
  int cpuid_80000004_registers[4];
  int cpuid_80000005_ebx;
  int cpuid_80000005_ecx;
  int cpuid_80000005_edx;
  int cpuid_80000006_ecx;
  int field_AC;
  int uProcessorManufacturer;
  int cpuid_80000002_registers2[4];
  int cpuid_80000003_registers2[4];
  int cpuid_80000004_registers2[4];
  int field_E4;
};
#pragma pack(pop)



//  130
#pragma pack(push, 1)
struct stru12_MemoryBlock
{
  stru12_MemoryBlock(int a2);
  ~stru12_MemoryBlock();
  

  void *pAlignedBlock;
  void *pBlockBase;
};
#pragma pack(pop)

//  129
#pragma pack(push, 1)
struct stru12
{
   stru12(stru11 *pStru11);
  ~stru12();

  void _4898E6();

  void ( *vdestructor_ptr)(stru12 *);
  int field_4[16];
  int field_44;
  int field_48;
  int field_4C;
  int field_50;
  int field_54;
  int field_58;
  stru12_MemoryBlock *pMemBlocks[12];
  int field_8C;
};
#pragma pack(pop)




//----- (00438526) --------------------------------------------------------
stru11::stru11()
{
  stru11 *v1; // esi@1
  signed int v2; // eax@1
  char *v3; // edx@1

  v1 = this;
  v2 = 0;
  this->cpuid_00000000_eax_numops = 0;
  this->cpuid_00000001_eax = 0;
  this->cpuid_00000001_edx = 0;
  this->field_38 = 0;
  this->field_3C = 0;
  this->cpuid_80000000_edx = 0;
  this->cpuid_80000001_eax = 0;
  this->cpuid_80000001_edx = 0;
  this->cpuid_80000005_ebx = 0;
  this->cpuid_80000005_ecx = 0;
  this->cpuid_80000005_edx = 0;
  this->cpuid_80000006_ecx = 0;
  this->field_AC = 0;
  this->uProcessorManufacturer = 0;
  LOBYTE(this->cpuid_80000002_registers2[0]) = 0;
  v3 = (char *)&this->cpuid_00000002_eax;
  do
  {
    if ( v2 < 3 )
      *((int *)v3 - 9) = 0;
    if ( v2 < 4 )
      *(int *)v3 = 0;
    if ( v2 < 12 )
      *((int *)v3 + 17) = 0;
    if ( v2 < 13 )
      this->pCPUString[v2] = 0;
    if ( v2 < 30 )
      *((char *)this->field_40 + v2) = 0;
    *((char *)&this->cpuid_80000002_registers2[0] + v2++ + 1) = 0;
    v3 += 4;
  }
  while ( v2 < 48 );
  CheckCPU();
}

//----- (004385B5) --------------------------------------------------------
void stru11::CheckCPU()
{
  int v5; // [sp-4h] [bp-10h]@0
  signed int v6; // [sp+4h] [bp-8h]@1
  signed int v7; // [sp+8h] [bp-4h]@1

  _CF = 0;
  _OF = 0;
  _ZF = 1;
  _SF = 0;
  v7 = 0;
  __asm { pushf }
  v6 = 3;
  if ( v5 != (v5 ^ 0x40000) )
  {
    __asm { popf }
    v6 = 4;
    __asm { pushf }
    if ( v5 != (v5 ^ 0x200000) )
    {
      v7 = 1;
      v6 = 0;
    }
  }
  if ( v6 == 3 )
  {
    this->field_AC = 1;
  }
  else
  {
    if ( v6 == 4 )
    {
      this->field_AC = 2;
    }
    else
    {
      if ( v7 )
      {
        this->field_AC = 2;
        RunCPUID();
      }
      else
      {
        this->field_AC = 0;
      }
    }
  }
}

//----- (00438659) --------------------------------------------------------
void stru11::RunCPUID()
{
  stru11 *v6; // esi@1
  unsigned int uNumOps; // edi@1
  int uNumExtOps; // edi@16
  char pCyrixString[16]; // [sp+Ch] [bp-60h]@1
  char pCentaurString[16]; // [sp+1Ch] [bp-50h]@1
  char pAMDString[16]; // [sp+2Ch] [bp-40h]@1
  char pIntelString[16]; // [sp+3Ch] [bp-30h]@1
  char pCPUString[16]; // [sp+4Ch] [bp-20h]@1
  stru11 *thisa; // [sp+5Ch] [bp-10h]@1
  char *v35; // [sp+60h] [bp-Ch]@1
  int v36; // [sp+64h] [bp-8h]@1
  int v37; // [sp+68h] [bp-4h]@1

  thisa = this;
  *(int *)pIntelString = *(int *)"GenuineIntel";
  *(int *)&pIntelString[4] = *(int *)"ineIntel";
  *(int *)&pIntelString[8] = *(int *)"ntel";
  pIntelString[12] = aGenuineintel[12];
  *(int *)pAMDString = *(int *)"AuthenticAMD";
  *(int *)&pAMDString[4] = *(int *)"enticAMD";
  *(int *)&pAMDString[8] = *(int *)"cAMD";
  pAMDString[12] = aAuthenticamd[12];
  *(int *)pCyrixString = *(int *)"CyrixInstead";
  *(int *)&pCyrixString[4] = *(int *)"xInstead";
  *(int *)&pCyrixString[8] = *(int *)"tead";
  pCyrixString[12] = aCyrixinstead[12];
  *(int *)pCentaurString = *(int *)"CentaurHauls";
  *(int *)&pCentaurString[4] = *(int *)"aurHauls";
  *(int *)&pCentaurString[8] = *(int *)"auls";
  v37 = 0;
  v36 = 0;
  pCentaurString[12] = aCentaurhauls[12];
  v35 = pCPUString;
  pCPUString[12] = 0;
  _EAX = 0;
  __asm { cpuid }
  v37 = _EAX;
  *(int *)pCPUString = _EBX;
  *(int *)&pCPUString[4] = _EDX;
  *(int *)&pCPUString[8] = _ECX;
  v6 = thisa;
  uNumOps = _EAX;
  thisa->cpuid_00000000_ebx_vendorstr1 = _EBX;
  v6->cpuid_00000000_edx_vendorstr2 = *(int *)&pCPUString[4];
  v6->cpuid_00000000_ecx_vendorstr3 = *(int *)&pCPUString[8];
  v6->cpuid_00000000_eax_numops = _EAX;
  strcpy(v6->pCPUString, pCPUString);
  if ( strcmp(pIntelString, pCPUString) )
  {
    if ( strcmp(pAMDString, pCPUString) )
    {
      if ( strcmp(pCyrixString, pCPUString) )
      {
        if ( strcmp(pCentaurString, pCPUString) )
          v6->uProcessorManufacturer = 0;
        else
          v6->uProcessorManufacturer = 4;
      }
      else
      {
        v6->uProcessorManufacturer = 3;
      }
    }
    else
    {
      v6->uProcessorManufacturer = 2;
    }
  }
  else
  {
    v6->uProcessorManufacturer = 1;
  }
  if ( uNumOps >= 1 )
    RunCPUID_op1();
  if ( uNumOps >= 2 )
    RunCPUID_op2();
  _EAX = 0x80000000u;
  __asm { cpuid }
  if ( !(_EAX & 0x80000000) )
    _EAX = 0x80000000u;
  v36 = _EAX;
  v6->cpuid_80000000_edx = _EAX;
  uNumExtOps = _EAX ^ 0x80000000;
  if ( (_EAX ^ 0x80000000u) >= 1 )
  {
    _EAX = 0x80000001u;
    __asm { cpuid }
    thisa = (stru11 *)_EAX;
    v35 = (char *)_EDX;
    v6->cpuid_80000001_eax = _EAX;
    v6->cpuid_80000001_edx = (int)v35;
  }
  if ( (unsigned int)uNumExtOps >= 4 )
    RunCPUID_ext2_3_4();
  if ( (unsigned int)uNumExtOps >= 5 )
  {
    _EAX = 0x80000005u;
    __asm { cpuid }
    thisa = (stru11 *)_EBX;
    v35 = (char *)_ECX;
    v36 = _EDX;
    v6->cpuid_80000005_ebx = _EBX;
    v6->cpuid_80000005_ecx = (int)v35;
    v6->cpuid_80000005_edx = v36;
  }
  if ( (unsigned int)uNumExtOps >= 6 )
  {
    _EAX = 0x80000006u;
    __asm { cpuid }
    v6->cpuid_80000006_ecx = _ECX;
  }
}

//----- (00438821) --------------------------------------------------------
void stru11::RunCPUID_op1()
{
  stru11 *v1; // esi@1
  unsigned int v7; // ecx@1
  int v8; // eax@3
  int v9; // eax@10
  signed int v10; // eax@11
  int v11; // ecx@11
  int v12; // eax@15
  signed int v13; // eax@19
  int v14; // ecx@19
  signed int v15; // eax@25
  int v16; // ecx@25
  signed int v17; // eax@34
  int v18; // ecx@34

  v1 = this;
  _EAX = 1;
  __asm { cpuid }
  v7 = _EAX;
  v1->cpuid_00000001_edx = _EDX;
  v1->cpuid_00000001_eax = _EAX;
  if ( (_EAX & 0x3000) == 8192 )
  {
    LOBYTE(v1->cpuid_80000002_registers2[0]) = 1;
    v7 = 0;
  }
  v8 = (v7 >> 8) & 0xF;
  switch ( v8 )
  {
    case 4:
      v1->field_AC = 2;
      break;
    case 5:
      v1->field_AC = 15;
      break;
    case 6:
      v1->field_AC = 36;
      break;
    default:
      v1->field_AC = 49;
      break;
  }
  v9 = v1->uProcessorManufacturer;
  if ( v9 == 1 )
  {
    v10 = 0;
    v11 = v7 & 0x3FF0;
    while ( v11 != dword_4E4948[2 * v10] )
    {
      ++v10;
      if ( v10 >= 17 )
        return;
    }
    v12 = dword_4E494C[2 * v10];
    goto LABEL_39;
  }
  if ( v9 == 2 )
  {
    if ( (v7 & 0xF00) == dword_4E49D0[0] )
    {
      v12 = dword_4E49D4[0];
    }
    else
    {
      v13 = 1;
      v14 = v7 & 0x3FF0;
      while ( v14 != dword_4E49D0[2 * v13] )
      {
        ++v13;
        if ( v13 >= 9 )
          return;
      }
      v12 = dword_4E49D4[2 * v13];
    }
    goto LABEL_39;
  }
  if ( v9 != 3 )
  {
    if ( v9 != 4 )
      return;
    v17 = 0;
    v18 = v7 & 0x3FF0;
    while ( v18 != dword_4E4A40[2 * v17] )
    {
      ++v17;
      if ( v17 >= 2 )
        return;
    }
    v12 = dword_4E4A44[2 * v17];
    goto LABEL_39;
  }
  v15 = 1;
  v16 = v7 & 0x3FF0;
  while ( v16 != dword_4E4A18[2 * v15] )
  {
    ++v15;
    if ( v15 >= 5 )
      goto LABEL_30;
  }
  v1->field_AC = dword_4E4A1C[2 * v15];
LABEL_30:
  v12 = 24;
  if ( v1->field_AC == 24 )
  {
    if ( _EDX != 1 )
      v12 = (((_EDX != 261) - 1) & 0xA) + 15;
LABEL_39:
    v1->field_AC = v12;
  }
}

//----- (00438992) --------------------------------------------------------
void stru11::RunCPUID_op2()
{
  stru11 *v1; // edi@1
  int v7; // eax@1
  signed int v8; // ecx@2
  unsigned __int8 v9; // al@3
  signed int v10; // ecx@14
  unsigned __int8 v11; // al@15
  int v12; // [sp+Ch] [bp-14h]@1
  int v13; // [sp+10h] [bp-10h]@1
  int v14; // [sp+14h] [bp-Ch]@1
  int v15; // [sp+18h] [bp-8h]@1
  int *v16; // [sp+1Ch] [bp-4h]@1

  v1 = this;
  v16 = &v12;
  _EAX = 2;
  __asm { cpuid }
  v12 = _EAX;
  v13 = _EBX;
  v14 = _ECX;
  v15 = _EDX;
  v1->cpuid_00000002_eax = _EAX;
  v1->cpuid_00000002_ebx = v13;
  v1->cpuid_00000002_ecx = v14;
  v1->cpuid_00000002_edx = v15;
  v7 = v1->field_AC;
  if ( v7 == 40 )
  {
    v8 = 0;
    while ( 1 )
    {
      v9 = *((char *)&v12 + v8);
      if ( v9 == 64 )
      {
        v1->field_AC = 43;
        return;
      }
      if ( v9 >= 0x41u && v9 <= 0x43u )
      {
        v1->field_AC = 41;
        return;
      }
      if ( v9 >= 0x44u && v9 <= 0x45u )
        break;
      ++v8;
      if ( v8 >= 16 )
        return;
    }
    v1->field_AC = 42;
  }
  else
  {
    if ( v7 == 45 )
    {
      v10 = 0;
      while ( 1 )
      {
        v11 = *((char *)&v12 + v10);
        if ( v11 >= 0x40u && v11 <= 0x43u )
        {
          v1->field_AC = 46;
          return;
        }
        if ( v11 >= 0x44u && v11 <= 0x45u )
          break;
        ++v10;
        if ( v10 >= 16 )
          return;
      }
      v1->field_AC = 47;
    }
  }
}

//----- (00438A67) --------------------------------------------------------
void stru11::RunCPUID_ext2_3_4()
{
  stru11 *v1; // edi@1
  stru11 *v17; // eax@1
  int v18[3][4]; // [sp+Ch] [bp-38h]@1
  stru11 *v19; // [sp+3Ch] [bp-8h]@1

  v1 = this;
  v19 = this;
  _EAX = 0x80000002u;
  __asm { cpuid }
  v18[0][0] = _EAX;
  *(_QWORD *)&v18[0][1] = __PAIR__(_ECX, _EBX);
  v18[0][3] = _EDX;
  _EAX = 0x80000003u;
  __asm { cpuid }
  v18[1][0] = _EAX;
  *(_QWORD *)&v18[1][1] = __PAIR__(_ECX, _EBX);
  v18[1][3] = _EDX;
  _EAX = 0x80000004u;
  __asm { cpuid }
  v18[2][0] = _EAX;
  *(_QWORD *)&v18[2][1] = __PAIR__(_ECX, _EBX);
  v18[2][3] = _EDX;
  v17 = v19;
  memcpy(v1->cpuid_80000002_registers, v18, 0x30u);
  memcpy((char *)&v17->cpuid_80000002_registers2[0] + 1, v18, 0x30u);
}



//----- (0048958E) --------------------------------------------------------
stru12_MemoryBlock::stru12_MemoryBlock(int a2)
{
  stru12_MemoryBlock *v2; // esi@1
  void *v3; // eax@1
  void *v4; // ecx@1
  stru12_MemoryBlock *result; // eax@1

  v2 = this;
  v3 = operator new(8 * a2 + 16);
  v2->pBlockBase = v3;
  v4 = (char *)v3 + PID_TYPE(-(signed int)v3);
  result = v2;
  v2->pAlignedBlock = v4;
}

//----- (004895B7) --------------------------------------------------------
stru12::stru12(stru11 *pStru11)
{
  stru12 *v2; // esi@1

  v2 = this;
  this->field_44 = 0x41000000u;
  this->field_4C = 0x46000000u;
  //this->vdestructor_ptr = stru12::_4898E6;
  this->field_48 = 0;
  this->field_50 = 0;
  this->field_54 = 0x33D6BF95u;
  this->field_58 = 0;
  
  for (int i = 0; i < 12; ++i)
    v2->pMemBlocks[i] = new stru12_MemoryBlock(640);
}

//----- (00489810) --------------------------------------------------------
stru12::~stru12()
{
  for (int i = 0; i < 12; ++i)
    delete pMemBlocks[i];
}

//----- (004898BF) --------------------------------------------------------
stru12_MemoryBlock::~stru12_MemoryBlock()
{
  stru12_MemoryBlock *v2; // esi@1
  void *v3; // ST00_4@1

  v2 = this;
  v3 = this->pBlockBase;
  this->pAlignedBlock = 0;
  free(v3);
  v2->pBlockBase = 0;
  //if ( a2 & 1 )
  //  free(v2);
}

//----- (004898E6) --------------------------------------------------------
void stru12::_4898E6()
{
  void *v1; // eax@1
  void *v2; // edx@1
  char *v3; // edi@2
  double v4; // st7@2
  char *v5; // ebx@2
  double v6; // st6@3
  double v7; // st5@3
  double v8; // st4@3
  int v9; // esi@3
  double v10; // st3@3
  float v11; // ST34_4@5
  double v12; // st6@10
  double v13; // ST0C_8@10
  char *v14; // [sp+14h] [bp-44h]@2
  char *v15; // [sp+18h] [bp-40h]@2
  char *v16; // [sp+1Ch] [bp-3Ch]@2
  char *v17; // [sp+20h] [bp-38h]@2
  char *v18; // [sp+24h] [bp-34h]@2
  float v19; // [sp+30h] [bp-28h]@3
  float v20; // [sp+38h] [bp-20h]@3
  char *v21; // [sp+3Ch] [bp-1Ch]@2
  int v22; // [sp+40h] [bp-18h]@1
  char *v23; // [sp+44h] [bp-14h]@2
  char *v24; // [sp+48h] [bp-10h]@2
  int v25; // [sp+4Ch] [bp-Ch]@2
  float v26; // [sp+50h] [bp-8h]@3
  float v27; // [sp+54h] [bp-4h]@3

  __debugbreak();
  v22 = 0;
  v1 = this->pMemBlocks[1]->pAlignedBlock;
  v2 = this->pMemBlocks[6]->pAlignedBlock;
  if ( this->field_8C > 0 )
  {
    v24 = (char *)((char *)pMemBlocks[0]->pAlignedBlock - v1);
    v23 = (char *)((char *)pMemBlocks[2]->pAlignedBlock - v1);
    v18 = (char *)((char *)pMemBlocks[7]->pAlignedBlock - v2);
    v17 = (char *)((char *)pMemBlocks[8]->pAlignedBlock - v2);
    v25 = (int)v1 - (int)v2;
    v16 = (char *)((char *)pMemBlocks[9]->pAlignedBlock - v2);
    v3 = (char *)((char *)pMemBlocks[4]->pAlignedBlock - v2);
    v15 = (char *)((char *)pMemBlocks[10]->pAlignedBlock - v2);
    v4 = 1.0;
    v5 = (char *)((char *)pMemBlocks[5]->pAlignedBlock - v2);
    v21 = (char *)((char *)pMemBlocks[3]->pAlignedBlock - v2);
    v14 = (char *)((char *)pMemBlocks[11]->pAlignedBlock - v2);
    do
    {
      v26 = *(float *)&v24[(int)((char *)v2 + v25)] - (double)pIndoorCamera->pos.x;
      v27 = *(float *)((char *)v2 + v25) - (double)pIndoorCamera->pos.y;
      v6 = *(float *)&v23[(int)((char *)v2 + v25)] - (double)pIndoorCamera->pos.z;
      v7 = pIndoorCamera->fRotationYCosine;
      v8 = pIndoorCamera->fRotationYSine;
      v20 = pIndoorCamera->fRotationXCosine;
      v19 = pIndoorCamera->fRotationXSine;
      v9 = (int)((char *)v2 + (int)v21);
      v10 = v27 * pIndoorCamera->fRotationYSine + pIndoorCamera->fRotationYCosine * v26;
      if ( pIndoorCamera->sRotationX )
      {
        v11 = v10;
        *(float *)v9 = v11 * pIndoorCamera->fRotationXCosine + pIndoorCamera->fRotationXSine * v6;
        *(float *)((char *)v2 + (int)v3) = v7 * v27 - v8 * v26;
        *(float *)((char *)v2 + (int)v5) = v20 * v6 - v11 * v19;
      }
      else
      {
        *(float *)v9 = v10;
        *(float *)((char *)v2 + (int)v3) = v7 * v27 - v8 * v26;
        *(float *)((char *)v2 + (int)v5) = v6;
      }
      if ( *(float *)v9 >= 8.0 )
      {
        if ( (double)pODMRenderParams->shading_dist_mist >= *(float *)v9 )
        {
          *(int *)v2 = 0;
          v12 = v4 / (*(float *)v9 + 0.0000001) * (double)pODMRenderParams->int_fov_rad;
          *(float *)((char *)v2 + (int)v18) = (double)pViewport->uScreenCenterX
                                               - v12 * *(float *)((char *)v2 + (int)v3);
          *(float *)((char *)v2 + (int)v17) = (double)pViewport->uScreenCenterY
                                               - v12 * *(float *)((char *)v2 + (int)v5);
          *(float *)((char *)v2 + (int)v16) = v4
                                               - v4 / (*(float *)v9 * 1000.0 / (double)pODMRenderParams->shading_dist_mist);
          *(float *)((char *)v2 + (int)v15) = v4 / (*(float *)v9 + 0.0000001);
          v13 = *(float *)v9 + 6.7553994e15;
          v4 = 1.0;
          *(int *)((char *)v2 + (int)v14) = LODWORD(v13);
        }
        else
        {
          *(int *)v2 = 2;
        }
      }
      else
      {
        *(int *)v2 = 1;
      }
      ++v22;
      v2 = (char *)v2 + 4;
    }
    while ( v22 < this->field_8C );
  }
  this->field_8C = 0;
}


//----- (0048276F) --------------------------------------------------------
void Polygon::_48276F_sr()
{
  unsigned int v1; // ebx@1
  float v2; // edx@2
  double v3; // st7@2
  char *v4; // ecx@3
  float v5; // eax@5
  float v6; // eax@7
  float v7; // eax@9
  float v8; // ecx@13
  int i; // eax@16
  int v10; // edx@20
  RenderVertexSoft *v11; // ecx@22
  RenderVertexSoft *v12; // edx@22
  RenderVertexSoft *v13; // esi@22
  int v14; // ebx@26
  RenderVertexSoft *v15; // ebx@27
  double v16; // st6@28
  double v17; // st5@28
  double v18; // st4@28
  int v19; // [sp+4h] [bp-2Ch]@20
  int v20; // [sp+8h] [bp-28h]@22
  int v21; // [sp+Ch] [bp-24h]@22
  Polygon *v22; // [sp+10h] [bp-20h]@1
  float v23; // [sp+14h] [bp-1Ch]@11
  float v24; // [sp+18h] [bp-18h]@7
  float v25; // [sp+1Ch] [bp-14h]@5
  float v26; // [sp+20h] [bp-10h]@2
  float v27; // [sp+24h] [bp-Ch]@2
  float v28; // [sp+28h] [bp-8h]@2
  float v29; // [sp+2Ch] [bp-4h]@9

  v1 = this->uNumVertices;
  v22 = this;
  if ( (signed int)v1 >= 3 )
  {
    LODWORD(v2) = 0;
    v26 = 10000.0;
    v28 = 10000.0;
    v3 = -10000.0;
    v27 = -10000.0;
    if ( (signed int)v1 > 0 )
    {
      v4 = (char *)&sr_508690[0].vWorldViewProjY;
      do
      {
        if ( *((float *)v4 - 1) < (double)v26 )
        {
          LODWORD(v5) = *((int *)v4 - 1);
          v25 = v2;
          v26 = v5;
        }
        if ( *((float *)v4 - 1) > (double)v27 )
        {
          LODWORD(v6) = *((int *)v4 - 1);
          v24 = v2;
          v27 = v6;
        }
        if ( *(float *)v4 < (double)v28 )
        {
          LODWORD(v7) = *(int *)v4;
          v29 = v2;
          v28 = v7;
        }
        if ( v3 < *(float *)v4 )
        {
          v3 = *(float *)v4;
          v23 = v2;
        }
        ++LODWORD(v2);
        v4 += 48;
      }
      while ( SLODWORD(v2) < (signed int)v1 );
    }
    v8 = v29;
    if ( LODWORD(v29) == LODWORD(v25) || LODWORD(v29) == LODWORD(v24) )
      v8 = v23;
    v29 = 0.0;
    for ( i = 0; i < (signed int)v1; ++i )
    {
      if ( i == LODWORD(v25) || i == LODWORD(v24) || i == LODWORD(v8) )
      {
        v10 = LODWORD(v29)++;
        *(&v19 + v10) = i;
      }
    }
    v11 = &sr_508690[v19];
    v12 = &sr_508690[v20];
    v13 = &sr_508690[v21];
    if ( LODWORD(v29) != 3 )
    {
      v11 = sr_508690;
      v13 = (RenderVertexSoft *)((char *)sr_508690 + 16 * (3 * v1 - 3));
      v12 = &sr_508690[1];
      v28 = sr_508690[1].vWorldPosition.x - sr_508690[0].vWorldPosition.x;
      v27 = sr_508690[1].vWorldPosition.y - sr_508690[0].vWorldPosition.y;
      v29 = sr_508690[1].vWorldPosition.z - sr_508690[0].vWorldPosition.z;
      v26 = v13->vWorldPosition.x - sr_508690[0].vWorldPosition.x;
      v25 = v13->vWorldPosition.y - sr_508690[0].vWorldPosition.y;
      v24 = v13->vWorldPosition.z - sr_508690[0].vWorldPosition.z;
      if ( v24 * v27 - v25 * v29 == 0.0 )
      {
        if ( v26 * v29 - v24 * v28 == 0.0 )
        {
          if ( v25 * v28 - v26 * v27 == 0.0 )
          {
            v14 = v1 - 2;
            LODWORD(v26) = v14;
            if ( v14 >= 2 )
            {
              v15 = &sr_508690[v14];
              do
              {
                v16 = v15->vWorldPosition.x - sr_508690[0].vWorldPosition.x;
                v17 = v15->vWorldPosition.y - sr_508690[0].vWorldPosition.y;
                v18 = v15->vWorldPosition.z - sr_508690[0].vWorldPosition.z;
                v13 = v15;
                if ( v27 * v18 - v17 * v29 != 0.0 )
                  break;
                if ( v16 * v29 - v18 * v28 != 0.0 )
                  break;
                if ( v28 * v17 - v16 * v27 != 0.0 )
                  break;
                --LODWORD(v26);
                --v15;
              }
              while ( SLODWORD(v26) >= 2 );
            }
          }
        }
      }
    }
    sr_sub_4829B9(v11, v12, v13, v22, 1);
  }
}


//----- (004829B9) --------------------------------------------------------
Polygon *sr_sub_4829B9(RenderVertexSoft *a1, RenderVertexSoft *a2, RenderVertexSoft *a3, Polygon *a4, int a5)
{
  double v5; // st7@1
  RenderVertexSoft *v6; // esi@1
  double v7; // st6@1
  Polygon *result; // eax@3
  double v9; // st6@3
  double v10; // st5@3
  float v11; // ST0C_4@3
  float v12; // ST04_4@3
  double v13; // st4@3
  float v14; // [sp+8h] [bp-Ch]@1
  float v15; // [sp+10h] [bp-4h]@1
  float v16; // [sp+1Ch] [bp+8h]@1
  float v17; // [sp+1Ch] [bp+8h]@3

  v5 = a2->vWorldViewProjX - a1->vWorldViewProjX;
  v6 = a3;
  v16 = a3->vWorldViewProjY - a1->vWorldViewProjY;
  v15 = a2->vWorldViewProjY - a1->vWorldViewProjY;
  v14 = v6->vWorldViewProjX - a1->vWorldViewProjX;
  v7 = v16 * v5 - v14 * v15;
  if ( v7 == 0.0 )
    v7 = 0.0000001;
  result = a4;
  v9 = 1.0 / v7;
  v10 = 1.0 / a1->vWorldViewPosition.x;
  v11 = 1.0 / a2->vWorldViewPosition.x - v10;
  v12 = 1.0 / v6->vWorldViewPosition.x - v10;
  v13 = (v11 * v16 - v12 * v15) * v9;
  v17 = (v11 * v14 - v12 * v5) * -v9;
  a4->field_C = a1->vWorldViewProjX;
  a4->field_10 = a1->vWorldViewProjY;
  a4->field_0 = v10;
  a4->field_8 = v17;
  a4->field_4 = v13;
  return result;
}

//----- (00481DB2) --------------------------------------------------------
char sr_sub_481DB2(RenderVertexSoft *a1, signed int a2, Polygon *a3)
{
  int v3; // eax@2
  int v4; // esi@2
  signed int v5; // esi@2
  char *v6; // edi@3
  double v7; // ST14_8@4
  double v8; // ST0C_8@4
  char result; // al@5

  if ( a2 >= 3 )
  {
    v3 = a3->uTileBitmapID;
    v4 = a3->uTileBitmapID;
    a3->ptr_38 = &stru_8019C8;
    a3->pTexture = (Texture_MM7 *)(v4 != -1 ? (int)&pBitmaps_LOD->pTextures[v3] : 0);
    v5 = 0;
    if ( (signed int)a3->uNumVertices > 0 )
    {
      v6 = (char *)&sr_508690[0].vWorldViewProjY;
      do
      {
        v7 = *((float *)v6 - 1) + 6.7553994e15;
        dword_50B638[v5] = LODWORD(v7);
        v8 = *(float *)v6 + 6.7553994e15;
        v6 += 48;
        dword_50B570[v5++] = LODWORD(v8);
      }
      while ( v5 < (signed int)a3->uNumVertices );
    }
    result = sr_sub_486B4E_push_outdoor_edges(a1, dword_50B638.data(), dword_50B570.data(), a3);
  }
  return result;
}

//----- (00486B4E) --------------------------------------------------------
char sr_sub_486B4E_push_outdoor_edges(RenderVertexSoft *a1, int *a2, int *a3, Polygon *a4)//maybe DrawPolygonSW
{
  Polygon *v4; // esi@1
  RenderVertexSoft *v5; // edi@1
  char v6; // zf@1
  unsigned int v7; // eax@3
  int v8; // ebx@3
  int *v9; // ecx@7
  int v10; // ebx@8
  int v11; // eax@10
  double v12; // st7@14
  int *v13; // edx@14
  double v14; // st6@14
  double v15; // st7@16
  int v16; // edi@16
  double v17; // st7@16
  double v18; // st7@16
  int v19; // edi@18
  double v20; // st7@18
  double v21; // st7@18
  Edge *i; // edx@20
  double v23; // st7@28
  Edge *v24; // eax@28
  std::string v26; // [sp-18h] [bp-98h]@2
  const char *v27; // [sp-8h] [bp-88h]@2
  int v28; // [sp-4h] [bp-84h]@2
  double v29; // [sp+Ch] [bp-74h]@28
  double v30; // [sp+14h] [bp-6Ch]@28
  double v31; // [sp+1Ch] [bp-64h]@20
  double v32; // [sp+24h] [bp-5Ch]@16
  double v33; // [sp+2Ch] [bp-54h]@14
  unsigned int v34; // [sp+34h] [bp-4Ch]@2
  unsigned __int64 v35; // [sp+38h] [bp-48h]@28
  int v36; // [sp+40h] [bp-40h]@28
  int v37; // [sp+44h] [bp-3Ch]@20
  float v38; // [sp+48h] [bp-38h]@18
  int v39; // [sp+4Ch] [bp-34h]@16
  int v40; // [sp+50h] [bp-30h]@14
  int v41; // [sp+54h] [bp-2Ch]@3
  RenderVertexSoft *v42; // [sp+58h] [bp-28h]@1
  int v43; // [sp+5Ch] [bp-24h]@14
  int v44; // [sp+60h] [bp-20h]@6
  int v45; // [sp+64h] [bp-1Ch]@6
  unsigned int v46; // [sp+68h] [bp-18h]@7
  int *v47; // [sp+6Ch] [bp-14h]@1
  int v48; // [sp+70h] [bp-10h]@7
  float *v49; // [sp+74h] [bp-Ch]@7
  float v50; // [sp+78h] [bp-8h]@10
  float v51; // [sp+7Ch] [bp-4h]@14

  v4 = a4;
  v5 = a1;
  v47 = a2;
  v6 = (HIBYTE(a4->flags) & 0x40) == 0;
  v42 = a1;
  if ( !v6 )
  {
          MessageBoxW(nullptr, L"The Texture_MM7 Frame Table is not a supported feature.", L"E:\\WORK\\MSDEV\\MM7\\MM7\\Code\\Odspan.cpp:162", 0);
  }
  LOBYTE(v7) = v4->field_108;
  v8 = v4->uNumVertices;
  v4->field_108 = 0;
  BYTE3(a4) = v7;
  v41 = v8;
  if ( v8 > 0 )
  {
    if ( (signed int)pODMRenderParams->uNumSurfs < 1999 )
    {
      v7 = pODMRenderParams->uNumEdges;
      if ( (signed int)pODMRenderParams->uNumEdges < 5979 )
      {
        v4->uEdgeList1Size = 0;
        v4->uEdgeList2Size = 0;
        v45 = -1;
        v34 = v7;
        v28 = v8;
        v27 = (const char *)v5;
        v44 = 10000;
        pEngine->pLightmapBuilder->_45CB89(v5, v8);
        if ( v8 > 0 )
        {
          v9 = a3;
          v48 = 1;
          v49 = &v5->flt_2C;
          v46 = (char *)v47 - (char *)a3;
          do
          {
            v10 = v48;
            ++pODMRenderParams->uNumEdges;
            if ( v48 >= v41 )
              v10 = 0;
            v11 = *v9;
            LODWORD(v50) = *v9;
            if ( SLODWORD(v50) > v45 )
              v45 = v11;
            if ( v11 < v44 )
              v44 = v11;
            v12 = (double)SLODWORD(v50);
            v13 = &a3[v10];
            v51 = v12;
            v14 = (double)*v13;
            v50 = v14;
            *(float *)&v40 = v14 - v12;
            v33 = *(float *)&v40 + 6.7553994e15;
            v43 = LODWORD(v33);
            if ( LODWORD(v33) )
            {
              if ( SLODWORD(v33) >= 0 )
              {
                v19 = (int)((char *)v9 + v46);
                ptr_80C978_Edges->field_8 = 1;
                LODWORD(v38) = (int)&v47[v10];
                v40 = *(unsigned int *)LODWORD(v38) - *(int *)v19;
                v6 = BYTE3(a4) == 0;
                v20 = (double)v40;
                v40 = *v13 - *v9;
                v21 = v20 / (double)v40;
                ptr_80C978_Edges->field_4 = v21;
                ptr_80C978_Edges->field_0 = (v51 - (double)*v9) * v21 + (double)*(signed int *)v19;
                if ( !v6 )
                {
                  ptr_80C978_Edges->field_1C = *v49;
                  ptr_80C978_Edges->field_20 = v42[v10].flt_2C;
                  ptr_80C978_Edges->field_24 = (double)*(signed int *)v19;
                  ptr_80C978_Edges->field_28 = (double)(signed int)*(unsigned int *)LODWORD(v38);
                  ptr_80C978_Edges->field_2C = (double)*v9;
                  ptr_80C978_Edges->field_30 = (double)*v13;
                  v4->pEdgeList1[v4->uEdgeList1Size++] = ptr_80C978_Edges;
                }
              }
              else
              {
                v32 = v51 + 6.7553994e15;
                v39 = LODWORD(v32);
                v51 = v50;
                v15 = (double)SLODWORD(v32);
                ptr_80C978_Edges->field_8 = 0;
                v16 = (int)&v47[v10];
                v50 = v15;
                v40 = *(int *)((char *)v9 + v46) - *(int *)v16;
                v6 = BYTE3(a4) == 0;
                v17 = (double)v40;
                v40 = *v9 - *v13;
                v18 = v17 / (double)v40;
                ptr_80C978_Edges->field_4 = v18;
                ptr_80C978_Edges->field_0 = (v51 - (double)*v13) * v18 + (double)*(signed int *)v16;
                if ( !v6 )
                {
                  ptr_80C978_Edges->field_1C = v42[v10].flt_2C;
                  ptr_80C978_Edges->field_20 = *v49;
                  ptr_80C978_Edges->field_24 = (double)*(signed int *)v16;
                  ptr_80C978_Edges->field_28 = (double)*(int *)((char *)v9 + v46);
                  ptr_80C978_Edges->field_2C = (double)*v13;
                  ptr_80C978_Edges->field_30 = (double)*v9;
                  v4->pEdgeList2[v4->uEdgeList2Size++] = ptr_80C978_Edges;
                }
              }
              v31 = v51 + 6.7553994e15;
              v37 = LODWORD(v31);
              for ( i = &pNewEdges[LODWORD(v31)]; i->pNext->field_0 < (double)ptr_80C978_Edges->field_0; i = i->pNext )
                ;
              if ( i->field_0 != ptr_80C978_Edges->field_0 || i->field_8 || ptr_80C978_Edges->field_8 != 1 )
              {
                ptr_80C978_Edges->pNext = i->pNext;
                i->pNext = ptr_80C978_Edges;
              }
              else
              {
                ptr_80C978_Edges->pNext = i;
                i->pPrev->pNext = ptr_80C978_Edges;
              }
              v23 = v50 - 1.0;
              *(float *)&v40 = v23;
              v38 = v23;
              v30 = v38 + 6.7553994e15;
              v36 = LODWORD(v30);
              ptr_80C978_Edges->ptr_18 = ptr_80CA10[LODWORD(v30)];
              v29 = *(float *)&v40 + 6.7553994e15;
              v35 = __PAIR__(v40, LODWORD(v29));
              v24 = ptr_80C978_Edges;
              ptr_80CA10[LODWORD(v29)] = ptr_80C978_Edges;
              v24->pSurf = ptr_80C97C_Surfs;
              if ( ptr_80C978_Edges < &pEdges[5999] )
                ++ptr_80C978_Edges;
            }
            ++v48;
            v49 += 12;
            ++v9;
          }
          while ( v48 - 1 < v41 );
        }
        LOBYTE(v7) = v34;
        if ( pODMRenderParams->uNumEdges != v34 )
        {
          v4->ptr_48 = 0;
          ptr_80C97C_Surfs->field_22 = 0;
          ptr_80C97C_Surfs->pParent = v4;
          ptr_80C97C_Surfs->field_4 = v4->field_4;
          ptr_80C97C_Surfs->field_8 = v4->field_8;
          ptr_80C97C_Surfs->field_0 = v4->field_0;
          ptr_80C97C_Surfs->field_C = v4->field_C;
          ptr_80C97C_Surfs->field_10 = v4->field_10;
          LOBYTE(v7) = (char)pSurfs + 28;
          if ( ptr_80C97C_Surfs < &pSurfs[1999] )
          {
            ++ptr_80C97C_Surfs;
            ++pODMRenderParams->uNumSurfs;
          }
          if ( BYTE3(a4) )
            v4->field_108 = 1;
        }
      }
    }
  }
  return v7;
}

//----- (00482A94) --------------------------------------------------------
int sr_sub_482A94(Span *_this)
{
  stru315 *v1; // ebp@0
  Span *v2; // edi@1
  Polygon *v3; // esi@1
  int v4; // ecx@1
  stru149 *v5; // eax@1
  stru149 *v6; // eax@1
  int v7; // edx@1
  int v8; // eax@1
  int v9; // ecx@1
  int v10; // edx@1
  int v11; // ebx@1
  int v12; // eax@1
  signed int v13; // ebx@1
  int v14; // ebx@2
  signed __int64 v15; // qtt@3
  stru149 *v16; // eax@3
  signed int v17; // ebx@3
  Texture_MM7 *v18; // eax@14
  unsigned __int16 *v19; // eax@15
  stru149 *v20; // eax@21
  signed int v21; // eax@21
  int v22; // eax@21
  int v23; // ecx@21
  Texture_MM7 *v24; // edx@21
  signed int v25; // eax@21
  signed int v27; // [sp-4h] [bp-A4h]@8
  int v28; // [sp+Ch] [bp-94h]@1
  int v29; // [sp+10h] [bp-90h]@1
  stru316 a2; // [sp+14h] [bp-8Ch]@21
  stru315 a1; // [sp+3Ch] [bp-64h]@1
  int v32; // [sp+80h] [bp-20h]@1
  int v33; // [sp+84h] [bp-1Ch]@1
  int v34; // [sp+88h] [bp-18h]@1
  int v35; // [sp+8Ch] [bp-14h]@1
  int v36; // [sp+90h] [bp-10h]@1
  int v37; // [sp+94h] [bp-Ch]@1
  int v38; // [sp+98h] [bp-8h]@1
  int X; // [sp+9Ch] [bp-4h]@1

  v2 = _this;
  v3 = _this->pParent;
  v4 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - _this->field_A);
  v5 = v3->ptr_38;
  v38 = v4;
  v37 = v5->field_14;
  v38 = (unsigned __int64)(v37 * (signed __int64)v4) >> 16;
  v6 = v3->ptr_38;
  v7 = v38 + v6->field_C;
  v37 = v6->field_20;
  v33 = v7;
  v8 = ((unsigned __int64)(v37 * (signed __int64)v4) >> 16) + v3->ptr_38->field_18;
  v38 = v4;
  v34 = v8;
  v37 = v3->v_18.z;
  v38 = (unsigned __int64)(v37 * (signed __int64)v4) >> 16;
  v9 = v3->v_18.x;
  v28 = v3->sTextureDeltaU << 16;
  v35 = v3->sTextureDeltaV << 16;
  v10 = v2->field_8;
  v29 = pIndoorCameraD3D->int_sine_y;
  v32 = pIndoorCameraD3D->int_cosine_y;
  a1.field_28 = v2->field_C;
  v11 = v3->field_24;
  v37 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterX - v10);
  v12 = v3->v_18.y;
  v13 = -v11;
  v36 = v13;
  X = ((unsigned __int64)(v37 * (signed __int64)v12) >> 16) + v38 + v9;
  if ( !X || (v14 = abs(v13 >> 14), v14 > abs(X)) )
    return 0;
  LODWORD(v15) = v36 << 16;
  HIDWORD(v15) = v36 >> 16;
  v38 = v15 / X;
  v16 = v3->ptr_38;
  X = v37;
  v36 = v16->field_10;
  X = v37;
  v36 = v33 + ((unsigned __int64)(v36 * (signed __int64)v37) >> 16);
  v33 = v3->ptr_38->field_1C;
  X = (unsigned __int64)(v33 * (signed __int64)v37) >> 16;
  v34 += (unsigned __int64)(v33 * (signed __int64)v37) >> 16;
  v17 = 2;
  a1.pTextureLOD = (unsigned __int16 *)v3->pTexture->pLevelOfDetail2;
  if ( v38 >= mipmapping_building_mm1 << 16 )
  {
    if ( v38 >= mipmapping_building_mm2 << 16 )
    {
      if ( v38 >= mipmapping_building_mm3 << 16 )
      {
        if ( bUseLoResSprites )
          goto LABEL_12;
        v27 = 3;
      }
      else
      {
        v27 = 2;
      }
      v17 = v27;
      goto LABEL_12;
    }
    v17 = 1;
  }
  else
  {
    v17 = 0;
  }
LABEL_12:
  if ( v17 < (signed int)v3->ptr_48 )
    v17 = (signed int)v3->ptr_48;
  v18 = v3->pTexture;
  if ( v17 )
  {
    if ( v17 == 1 )
    {
      v19 = (unsigned __int16 *)v18->pLevelOfDetail1;
    }
    else
    {
      if ( v17 == 2 )
        v19 = (unsigned __int16 *)v18->pLevelOfDetail2;
      else
        v19 = (unsigned __int16 *)v18->pLevelOfDetail3;
    }
  }
  else
  {
    v19 = (unsigned __int16 *)v18->pLevelOfDetail0_prolly_alpha_mask;
  }
  a1.pTextureLOD = v19;
  X = (unsigned __int64)(v36 * (signed __int64)v38) >> 16;
  v20 = v3->ptr_38;
  X = v38;
  a1.field_30 = v28 + ((unsigned __int64)(v36 * (signed __int64)v38) >> 16) - v20->field_24;
  X = (unsigned __int64)(v34 * (signed __int64)v38) >> 16;
  v21 = X - v3->ptr_38->field_28 - v35;
  a1.field_30 >>= v17 + bUseLoResSprites;
  a1.field_2C = v21 >> (v17 + bUseLoResSprites);
  v35 = pODMRenderParams->int_fov_rad_inv;
  v37 = (signed int)((unsigned __int64)(pODMRenderParams->int_fov_rad_inv * (signed __int64)v38) >> 16) >> (v17 + bUseLoResSprites);
  a1.field_4 = (unsigned __int64)(v37 * (signed __int64)v29) >> 16;
  X = (unsigned __int64)(v37 * (signed __int64)v32) >> 16;
  a1.field_0 = -(signed int)((unsigned __int64)(v37 * (signed __int64)v32) >> 16);
  v22 = v2->field_A;
  a1.pColorBuffer = &render->pTargetSurface[v2->field_8 + render->uTargetSurfacePitch * v2->field_A];
  HIWORD(v23) = HIWORD(v38);
  a1.pDepthBuffer = (unsigned int *)&render->pActiveZBuffer[v2->field_8 + 640 * v22];
  LOWORD(v23) = 0;
  a1.field_24 = v23 | v3->field_50;
  v24 = v3->pTexture;
  v32 = (signed int)v24->uTextureWidth >> v17;
  v25 = (signed int)v24->uTextureHeight >> v17;
  a1.field_10 = v17 - v24->uWidthLn2 + 16;
  a1.field_C = v32 - 1;
  a1.field_8 = (v25 << 16) - 65536;
  a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&a2, v2, v38, v3, pODMRenderParams->building_gamme, 1u, 1);
  if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
    sr_sub_485975(&a1, (stru315 *)&a2);
  else
    sr_sub_4D6FB0(v1);
  return 1;
}

//----- (004789DE) --------------------------------------------------------
BSPModel *Render::DrawBuildingsSW()
{
  BSPModel *result; // eax@1
  BSPModel *v1; // ebx@2
  int v2; // eax@4
  ODMFace *v3; // edi@4
  int v4; // ecx@6
  int v5; // eax@6
  int v6; // ecx@6
  int v7; // ecx@6
  double v8; // st7@7
  char *v9; // ebx@7
  int v10; // eax@9
  double v11; // st7@9
  Polygon *v12; // ebx@19
  double v13; // st7@21
  double v14; // ST4C_8@23
  int v15; // eax@23
  int v16; // eax@27
  int v17; // ecx@29
  unsigned __int8 v18; // sf@29
  unsigned __int8 v19; // of@29
  int v20; // ecx@30
  int v21; // eax@31
  int v22; // eax@31
  int v23; // eax@35
  int v24; // eax@39
  Texture_MM7 *v25; // eax@43
  int v26; // esi@43
  signed int v27; // ecx@43
  double v28; // st6@43
  double v29; // st5@43
  unsigned short *v30; // edx@44
  int v31; // eax@44
  double v32; // st4@45
  int v33; // ecx@45
  char v34; // zf@45
  BSPVertexBuffer *v35; // eax@50
  unsigned int v36; // eax@53
  signed int v37; // esi@53
  int v38; // eax@54
  signed int v39; // edx@55
  char *v40; // ecx@56
  double v41; // ST1C_8@57
  double v42; // ST24_8@57
  signed int v43; // eax@60
  signed int v44; // esi@60
  int v45; // eax@61
  signed int v46; // edx@62
  char *v47; // ecx@63
  double v48; // ST34_8@64
  double v49; // ST44_8@64
  int v50; // eax@66
  signed int v51; // ecx@67
  char *v52; // edx@68
  double v53; // ST3C_8@69
  double v54; // ST2C_8@69
  float v55; // [sp+44h] [bp-7Ch]@50
  float v56; // [sp+48h] [bp-78h]@50
  float v57; // [sp+4Ch] [bp-74h]@50
  float v58; // [sp+50h] [bp-70h]@64
  int v59; // [sp+54h] [bp-6Ch]@64
  int v60; // [sp+58h] [bp-68h]@69
  float v61; // [sp+5Ch] [bp-64h]@64
  float v62; // [sp+60h] [bp-60h]@69
  float v63; // [sp+64h] [bp-5Ch]@57
  float v64; // [sp+68h] [bp-58h]@57
  int v65; // [sp+6Ch] [bp-54h]@64
  int v66; // [sp+70h] [bp-50h]@1
  int v67; // [sp+74h] [bp-4Ch]@6
  int v68; // [sp+78h] [bp-48h]@4
  int v69; // [sp+7Ch] [bp-44h]@57
  int v70; // [sp+80h] [bp-40h]@69
  int v71; // [sp+84h] [bp-3Ch]@23
  float v72; // [sp+88h] [bp-38h]@69
  int v73; // [sp+8Ch] [bp-34h]@57
  int v74; // [sp+90h] [bp-30h]@6
  int v75; // [sp+94h] [bp-2Ch]@6
  ODMFace *v76; // [sp+98h] [bp-28h]@4
  int v77; // [sp+9Ch] [bp-24h]@6
  BSPModel *v78; // [sp+A0h] [bp-20h]@2
  int v79; // [sp+A4h] [bp-1Ch]@4
  int a1; // [sp+A8h] [bp-18h]@1
  unsigned short *v81; // [sp+ACh] [bp-14h]@7
  int v82; // [sp+B0h] [bp-10h]@6
  int v83; // [sp+B4h] [bp-Ch]@6
  signed int v84; // [sp+B8h] [bp-8h]@7
  int a2; // [sp+BCh] [bp-4h]@1

  a2 = 0;
  a1 = 0;
  result = (BSPModel *)pOutdoor->pBModels;
  v66 = pOutdoor->uNumBModels;
  if ( (signed int)pOutdoor->uNumBModels > 0 )
  {
    v1 = pOutdoor->pBModels;
    v78 = pOutdoor->pBModels;
    while ( 1 )
    {
      if ( IsBModelVisible(a1, &a2) )
      {
		v1->field_40 |= 1u;
        v2 = v1->uNumFaces;
        v3 = v1->pFaces;
        v74 = 0;
        v76 = v3;
        v68 = v2;
        if ( v2 > 0 )
          break;
      }
LABEL_73:
      ++a1;
      ++v1;
      result = (BSPModel *)a1;
      v78 = v1;
      if ( a1 >= v66 )
        return result;
    }
    while ( 1 )
    {
	  if ( !v3->Invisible() )
      {
        v4 = 3 * v3->pVertexIDs[0];
        v5 = (int)((char *)v1 - 8);
        v67 = (int)((char *)v1 - 8);
        v6 = *(_DWORD *)(*(&v1->uNumConvexFaces - 2) + 4 * v4 + 8) + 4;
        v82 = 0;
        v83 = v6;
        v74 = 0;
        v7 = v3->uNumVertices;
        v75 = 0;
        v77 = v7;
        if ( v7 > 0 )
        {
          v8 = (double)v83;
          v9 = (char *)&array_73D150[0].vWorldViewPosition;
		  v81 = v3->pVertexIDs;
          v83 = v7;
          *(float *)&v84 = v8;
          while ( 1 )
          {
            v10 = *(int *)(v5 + 4) + 12 * *(short *)v81;
            *((float *)v9 - 3) = (double)*(signed int *)v10 + 4.0;
            *((float *)v9 - 2) = (double)*(signed int *)(v10 + 4) + 4.0;
            v11 = (double)*(signed int *)(v10 + 8) + 4.0;
            *((float *)v9 - 1) = v11;
            if ( *(float *)&v84 == v11 )
              ++v82;
            pIndoorCameraD3D->ViewTransform((RenderVertexSoft *)(v9 - 12), 1u);
            if ( *(float *)v9 < 8.0 || (double)pODMRenderParams->shading_dist_mist < *(float *)v9 )
            {
              if ( *(float *)v9 >= 8.0 )
                v75 = 1;
              else
                v74 = 1;
            }
            else
            {
              pIndoorCameraD3D->Project((RenderVertexSoft *)(v9 - 12), 1u, 0);
            }
            v81 += 2;
            v9 += 48;
            --v83;
            if ( !v83 )
              break;
            v5 = v67;
          }
          v7 = v77;
        }
        v12 = &array_77EC08[pODMRenderParams->uNumPolygons];
        *(int *)&v12->flags = 0;
        if ( v82 == v7 )
          *(int *)&v12->flags = 65536;
        v12->pODMFace = v3;
        v12->uNumVertices = v7;
        v12->field_59 = 5;
        v84 = v3->pFacePlane.vNormal.x;
        v83 = (unsigned __int64)(v84 * (signed __int64)-pOutdoor->vSunlight.x) >> 16;
        v82 = -pOutdoor->vSunlight.y;
        v84 = v3->pFacePlane.vNormal.y;
        v82 = (unsigned __int64)(v84 * (signed __int64)-pOutdoor->vSunlight.y) >> 16;
        v81 = (unsigned __int16 *)-pOutdoor->vSunlight.z;
        v84 = v3->pFacePlane.vNormal.z;
        v81 = (unsigned __int16 *)((unsigned __int64)(v84 * (signed __int64)-pOutdoor->vSunlight.z) >> 16);
        v84 = v83 + v82 + ((unsigned __int64)(v84 * (signed __int64)-pOutdoor->vSunlight.z) >> 16);
        v13 = (double)v84 * 0.000015258789;
        if ( v13 < 0.0 )
          v13 = 0.0;
        *(float *)&v84 = v13 * 31.0;
        v14 = *(float *)&v84 + 6.7553994e15;
        v71 = LODWORD(v14);
        v15 = (int)&v12->dimming_level;
        v12->dimming_level = 31 - LOBYTE(v14);
        if ( (char)(31 - LOBYTE(v14)) < 0 )
          *(char *)v15 = 0;
        if ( *(char *)v15 > 31 )
          *(char *)v15 = 31;
        v16 = v3->uTextureID;
        if ( BYTE1(v3->uAttributes) & 0x40 )
          v16 = pTextureFrameTable->GetFrameTexture(v16, pEventTimer->uTotalGameTimeElapsed);
        v17 = v16;
        result = (BSPModel *)&pBitmaps_LOD->pTextures[v16];
        v19 = __OFSUB__(pODMRenderParams->uNumPolygons, 1999);
        v18 = pODMRenderParams->uNumPolygons - 1999 < 0;
        v12->pTexture = (Texture_MM7 *)(v17 != -1 ? (int)result : 0);
        if ( !(v18 ^ v19) )
          return result;
        ++pODMRenderParams->uNumPolygons;
        ++pODMRenderParams->field_44;
        if ( !ODMFace::IsBackfaceCulled(array_73D150, v12) )
        {
LABEL_71:
          --pODMRenderParams->uNumPolygons;
          --pODMRenderParams->field_44;
          goto LABEL_72;
        }
        LOBYTE(v20) = v79;
        v21 = a1;
        v3->bVisible = 1;
        v12->uBModelFaceID = v20;
        LOWORD(v20) = (unsigned __int8)v20;
        v12->uBModelID = v21;
        v22 = 8 * (v20 | (v21 << 6));
        LOBYTE(v22) = v22 | 6;
        v12->field_50 = v22;
        if ( v3->uAttributes & 0x10 )
          *(int *)&v12->flags |= 2u;
        if ( BYTE2(v3->uAttributes) & 0x40 )
          HIBYTE(v12->flags) |= 4u;
        v23 = v3->uAttributes;
        if ( v23 & 4 )
        {
          HIBYTE(v12->flags) |= 4u;
        }
        else
        {
          if ( v23 & 0x20 )
            HIBYTE(v12->flags) |= 8u;
        }
        v24 = v3->uAttributes;
        if ( BYTE1(v24) & 8 )
        {
          HIBYTE(v12->flags) |= 0x20u;
        }
        else
        {
          if ( v24 & 0x40 )
            HIBYTE(v12->flags) |= 0x10u;
        }
        v25 = v12->pTexture;
        v26 = v77;
        v27 = v25->uTextureWidth;
        v84 = v25->uTextureHeight;
        v28 = 1.0 / (double)v27;
        v29 = 1.0 / (double)v84;
        if ( v77 > 0 )
        {
		  v30 = v3->pTextureVIDs;
          v31 = 0;
          v83 = v77;
          do
          {
            v32 = 1.0 / (*(float *)(v31 * 48 + 7590236) + 0.0000001);
            memcpy(&VertexRenderList[v31], &array_73D150[v31], sizeof(VertexRenderList[v31]));
            ++v31;
            array_50A2B0[v31 + 49]._rhw = v32;
            v84 = v12->sTextureDeltaU + *(short *)(v30 - 40);
            array_50A2B0[v31 + 49].u = (double)v84 * v28;
            v33 = v12->sTextureDeltaV + *(short *)v30;
            v30 += 2;
            v34 = v83-- == 1;
            v84 = v33;
            array_50A2B0[v31 + 49].v = (double)v33 * v29;
          }
          while ( !v34 );
          v3 = v76;
          v26 = v77;
        }
        
        static stru154 static_sub_004789DE_stru_73C818; // idb

        pEngine->pLightmapBuilder->ApplyLights_OutdoorFace((ODMFace *)v3);
        if ( pLightsData.uNumLightsApplied <= 0 )
        {
          v12->field_108 = 0;
        }
        else
        {
          v35 = (BSPVertexBuffer *)(v78 - 2);
          v12->field_108 = 1;

          static_sub_004789DE_stru_73C818.GetFacePlaneAndClassify((ODMFace *)v3, v35);
          v55 = static_sub_004789DE_stru_73C818.face_plane.vNormal.x;
          v56 = static_sub_004789DE_stru_73C818.face_plane.vNormal.y;
          v57 = static_sub_004789DE_stru_73C818.face_plane.vNormal.z;
          pEngine->pLightmapBuilder->_45CA88(&Lights, VertexRenderList, v26, (Vec3_float_ *)&v55);
        }
        if ( v74 )
        {
          v36 = sr_4250FE(v26);
          v37 = v36;
          if ( !v36 || (ODMRenderParams::Project(v36), v38 = sr_4254D2(v37), (v12->uNumVertices = v38) == 0) )
            goto LABEL_71;
          v12->_48276F_sr();
          v39 = 0;
          if ( (signed int)v12->uNumVertices > 0 )
          {
            v40 = (char *)&sr_508690[0].vWorldViewProjY;
            do
            {
              LODWORD(v64) = *((int *)v40 - 1);
              v41 = v64 + 6.7553994e15;
              v69 = LODWORD(v41);
              dword_50B638[v39] = LODWORD(v41);
              LODWORD(v63) = *(int *)v40;
              v42 = v63 + 6.7553994e15;
              v73 = LODWORD(v42);
              v40 += 48;
              dword_50B570[v39++] = LODWORD(v42);
            }
            while ( v39 < (signed int)v12->uNumVertices );
          }
        }
        else
        {
          if ( v75 )
          {
            v43 = sr_4252E8(v26);
            v44 = v43;
            if ( !v43 || (ODMRenderParams::Project(v43), v45 = sr_4254D2(v44), (v12->uNumVertices = v45) == 0) )
              goto LABEL_71;
            v12->_48276F_sr();
            v46 = 0;
            if ( (signed int)v12->uNumVertices > 0 )
            {
              v47 = (char *)&sr_508690[0].vWorldViewProjY;
              do
              {
                LODWORD(v61) = *((int *)v47 - 1);
                v48 = v61 + 6.7553994e15;
                v65 = LODWORD(v48);
                dword_50B638[v46] = LODWORD(v48);
                LODWORD(v58) = *(int *)v47;
                v49 = v58 + 6.7553994e15;
                v59 = LODWORD(v49);
                v47 += 48;
                dword_50B570[v46++] = LODWORD(v49);
              }
              while ( v46 < (signed int)v12->uNumVertices );
            }
          }
          else
          {
            v50 = sr_4254D2(v26);
            v12->uNumVertices = v50;
            if ( !v50 )
              goto LABEL_71;
            sr_sub_4829B9(
              &array_73D150[v3->uGradientVertex1],
              &array_73D150[v3->uGradientVertex2],
              &array_73D150[v3->uGradientVertex3],
              v12,
              0);
            v51 = 0;
            if ( (signed int)v12->uNumVertices > 0 )
            {
              v52 = (char *)&sr_508690[0].vWorldViewProjY;
              do
              {
                LODWORD(v62) = *((int *)v52 - 1);
                v53 = v62 + 6.7553994e15;
                v60 = LODWORD(v53);
                dword_50B638[v51] = LODWORD(v53);
                LODWORD(v72) = *(int *)v52;
                v54 = v72 + 6.7553994e15;
                v70 = LODWORD(v54);
                v52 += 48;
                dword_50B570[v51++] = LODWORD(v54);
              }
              while ( v51 < (signed int)v12->uNumVertices );
            }
          }
        }
        sr_sub_486B4E_push_outdoor_edges(sr_508690, dword_50B638.data(), dword_50B570.data(), v12);
      }
LABEL_72:
      ++v79;
      v1 = v78;
      ++v3;
      v76 = v3;
      if ( v79 >= v68 )
        goto LABEL_73;
    }
  }
  return result;
}

//----- (0047BC6F) --------------------------------------------------------
unsigned __int16 *sr_GetBillboardPalette(RenderBillboard *a1, int a2, signed int a3, int a4)
{
  int v4; // ebx@1
  int v6; // edx@4
  int v7; // ecx@5
  signed int v8; // eax@6
  signed __int64 v9; // qtt@11
  signed int v10; // eax@12
  int v11; // esi@17
  signed __int64 v12; // qtt@19
  double v13; // ST10_8@19
  int v14; // edi@25
  signed int v15; // edx@26
  signed __int64 v16; // qtt@29
  signed int v17; // eax@30
  int v18; // [sp+10h] [bp-8h]@1
  float a3a; // [sp+20h] [bp+8h]@19

  v4 = a2;
  v18 = a2;
  if ( pParty->armageddon_timer )
    return PaletteManager::Get(a2);
  if ( !pWeather->bNight )
  {
    if (day_attrib & DAY_ATTRIB_FOG)
    {
      v14 = day_fogrange_1 << 16;
      if ( a3 >= day_fogrange_1 << 16 )
      {
        if ( a3 <= day_fogrange_2 << 16 )
        {
          LODWORD(v16) = (a3 - v14) << 16;
          HIDWORD(v16) = (a3 - v14) >> 16;
          v15 = (unsigned __int64)(27 * v16 / ((day_fogrange_2 - day_fogrange_1) << 16)) >> 16;
        }
        else
        {
          v15 = 27;
        }
      }
      else
      {
        v15 = 0;
      }
      v17 = _43F55F_get_billboard_light_level(a1, v15);
      if ( v17 > 27 )
        v17 = 27;
      if ( !a3 )
        v17 = 27;
      v7 = v18;
      v6 = v17;
      return (unsigned __int16 *)PaletteManager::Get_Mist_or_Red_LUT(v7, v6, 1);
    }
    v11 = a4;
    if ( a4 < 0 )
      v11 = 0;
    LODWORD(v12) = a3 << 16;
    HIDWORD(v12) = a3 >> 16;
    a3a = (double)(signed int)(((unsigned __int64)(11 * v12 / (pODMRenderParams->shading_dist_shade << 16)) >> 16) + 20)
        * pOutdoor->fFogDensity;
    v13 = a3a + 6.7553994e15;
    v10 = _43F55F_get_billboard_light_level(a1, LODWORD(v13) + v11);
    if ( v10 > 27 )
      v10 = 27;
    if ( v10 < a4 )
      v10 = a4;
    if ( v10 > pOutdoor->max_terrain_dimming_level )
      v10 = pOutdoor->max_terrain_dimming_level;
    return PaletteManager::Get_Dark_or_Red_LUT(v4, v10, 1);
  }
  v6 = 0;
  if (pWeather->bNight)
  {
    v8 = 67108864;
    if ( (signed __int64)pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uExpireTime > 0 )
      v8 = pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower << 26;
    if ( a3 <= v8 )
    {
      if ( a3 > 0 )
      {
        LODWORD(v9) = a3 << 16;
        HIDWORD(v9) = a3 >> 16;
        v6 = (unsigned __int64)(27 * v9 / v8) >> 16;
      }
    }
    else
    {
      v6 = 27;
    }
    v10 = _43F55F_get_billboard_light_level(a1, v6);
    if ( v10 > 27 || !a3 )
      v10 = 27;
    return PaletteManager::Get_Dark_or_Red_LUT(v4, v10, 1);
  }
  v7 = 0;
  return (unsigned __int16 *)PaletteManager::Get_Mist_or_Red_LUT(v7, v6, 1);
}
//----- (00487389) --------------------------------------------------------
void Render::ExecOutdoorDrawSW()
{
  unsigned __int16 *v0; // ebx@1
  unsigned int v1; // esi@1
  Polygon *v2; // eax@1
  Span *v3; // edi@6
  Polygon *v4; // esi@9
  Texture_MM7 *v5; // ebp@10
  int v6; // esi@16
  unsigned int v7; // edx@16
  char *v8; // ecx@17
  unsigned int v9; // edi@22
  int v10; // eax@26
  int v11; // eax@27
  unsigned int v12; // ebp@32
  Span *v13; // esi@33
  int v14; // ecx@37
  int v15; // eax@40
  Texture_MM7 *v16; // ebp@51
  //unsigned int v17; // eax@51
  int v18; // eax@54
  char v19; // al@56
  unsigned int v20; // eax@57
  int v21; // ecx@57
  unsigned int v22; // eax@57
  stru149 *v23; // eax@65
  int v24; // eax@67
  ODMFace *v25; // eax@78
  signed int v26; // edx@79
  signed int v27; // ecx@79
  unsigned int v28; // eax@106
  unsigned int v29; // ebp@117
  Span *v30; // esi@118
  unsigned int v31; // ST04_4@124
  int v33; // [sp+18h] [bp-44h]@80
  signed int v34; // [sp+1Ch] [bp-40h]@3
  signed int v35; // [sp+20h] [bp-3Ch]@79
  Span **v36; // [sp+24h] [bp-38h]@4
  unsigned int v37; // [sp+28h] [bp-34h]@1
  unsigned __int16 *a1; // [sp+2Ch] [bp-30h]@1
  int a1a; // [sp+2Ch] [bp-30h]@26
  unsigned int a1b; // [sp+2Ch] [bp-30h]@116
  char v41; // [sp+30h] [bp-2Ch]@57

  v0 = render->pTargetSurface;
  v1 = pODMRenderParams->uNumPolygons;
  a1 = render->pTargetSurface;
  v2 = array_77EC08.data();
  v37 = pODMRenderParams->uNumPolygons;
  if ( !(pParty->uFlags & 2) )
  {
    v1 = uNumElementsIn80AA28;
    v2 = ptr_80AA28[0];
    v37 = uNumElementsIn80AA28;
  }
  v34 = 0;
  if ( (signed int)v1 > 0 )
  {
    v36 = &v2->prolly_head;
    do
    {
      if ( pParty->uFlags & 2 )
        v3 = *v36;
      else
        v3 = ptr_80AA28[v34]->prolly_head;
      if ( v3 )
      {
        v4 = v3->pParent;
        if ( v4 )
        {
          v5 = v4->pTexture;
          if ( v5 )
          {
            if ( v3->field_8 >= (signed int)pViewport->uViewportTL_X || v3->field_C != pViewport->uViewportTL_X )
            {
              LOBYTE(v4->field_32) |= 2u;
              v14 = *(int *)&v4->flags;
              if ( v14 & 0x10 && v4->field_59 != 5 )
              {
                dword_80AA20 = (v4->terrain_grid_z - 64) << 25;
                dword_80AA1C = dword_80AA20 + 0x1FF0000;
                dword_80AA14 = (v4->terrain_grid_x << 25) + 0x7FFF0000;
                dword_80AA18 = dword_80AA14 - 0x1FF0000;
                byte_80AA10 = ((unsigned int)pOutdoor->ActuallyGetSomeOtherTileInfo(
                                               v4->terrain_grid_z,
                                               v4->terrain_grid_x - 1) >> 9) & 1;
                if ( *(int *)&v4->flags & 2 || (v15 = *(int *)&v4->flags, BYTE1(v15) & 1) )
                {
                  if ( *(int *)&v4->flags & 2 )
                  {
                    while ( 1 )
                    {
                      if ( pODMRenderParams->outdoor_no_wavy_water )
                        sr_sub_48408A_prolly_odm_water_no_waves(v3);
                      else
                        sr_sub_485407_prolly_odm_water_wavy(v3);
                      v3->field_E = LOWORD(sr_6BE060[1]);
                      if ( v4->prolly_tail == v3 )
                        break;
                      v3 = v3->pNext;
                    }
                  }
                  else
                  {
                    while ( 1 )
                    {
                      v16 = v4->pTexture;
                      v4->pTexture = pBitmaps_LOD->LoadTexturePtr("wtrtyl");
                      if ( pODMRenderParams->outdoor_no_wavy_water )
                        sr_sub_48408A_prolly_odm_water_no_waves(v3);
                      else
                        sr_sub_485407_prolly_odm_water_wavy(v3);
                      v18 = v4->terrain_grid_z - 64;
                      v4->pTexture = v16;
                      dword_80AA20 = v18 << 25;
                      dword_80AA1C = (v18 << 25) + 0x1FF0000;
                      dword_80AA14 = (v4->terrain_grid_x << 25) + 0x7FFF0000;
                      dword_80AA18 = dword_80AA14 - 0x1FF0000;
                      byte_80AA10 = ((unsigned int)pOutdoor->ActuallyGetSomeOtherTileInfo(
                                                     v4->terrain_grid_z,
                                                     v4->terrain_grid_x - 1) >> 9) & 1;
                      sr_sub_484442(v3);
                      v3->field_E = LOWORD(sr_6BE060[1]);
                      if ( v4->prolly_tail == v3 )
                        break;
                      v3 = v3->pNext;
                    }
                  }
                }
                else
                {
                  while ( sr_sub_48408A_prolly_odm_water_no_waves(v3) )
                  {
                    v3->field_E = LOWORD(sr_6BE060[1]);
                    if ( v4->prolly_tail == v3 )
                      break;
                    v3 = v3->pNext;
                  }
                }
              }
              else
              {
                v19 = v4->field_59;
                if ( v19 != 5 )
                {
                  if ( v14 & 2 )
                  {
                    while ( 1 )
                    {
                      v28 = pBitmaps_LOD->LoadTexture("wtrtyla");
                      v4->pTexture = (Texture_MM7 *)(v28 != -1 ? &pBitmaps_LOD->pTextures[v28] : 0);
                      if ( !sr_sub_4847EB(v3) )
                        break;
                      v4->pTexture = v5;
                      if ( !sr_sub_484442(v3) )
                        break;
                      v3->field_E = LOWORD(sr_6BE060[1]);
                      if ( v4->prolly_tail == v3 )
                        break;
                      v3 = v3->pNext;
                      v5 = v4->pTexture;
                    }
                  }
                  else
                  {
                    if ( v19 == 1 )
                    {
                      dword_80AA20 = (v4->terrain_grid_z - 64) << 25;
                      dword_80AA1C = dword_80AA20 + 33488896;
                      dword_80AA14 = (v4->terrain_grid_x << 25) + 0x7FFF0000;
                      dword_80AA18 = dword_80AA14 - 33488896;
                      byte_80AA10 = ((unsigned int)pOutdoor->ActuallyGetSomeOtherTileInfo(
                                                     v4->terrain_grid_z,
                                                     v4->terrain_grid_x - 1) >> 9) & 1;
                      while ( 1 )
                      {
                        if ( !sr_sub_4847EB(v3) )
                          sr_sub_48585C_mb_DrawSpan(v3, &render->pTargetSurface[v3->field_8 + 640 * v3->field_A], 0);
                        v3->field_E = LOWORD(sr_6BE060[1]);
                        if ( v4->prolly_tail == v3 )
                          break;
                        v3 = v3->pNext;
                      }
                    }
                  }
                  goto LABEL_14;
                }
                v4->ptr_38 = (stru149 *)&v41;
                v4->_479295();
                v20 = GetTickCount();
                v21 = *(int *)&v4->flags;
                v22 = v20 >> 3;
                if ( BYTE1(v21) & 4 )
                {
                  v4->sTextureDeltaV -= v22 & v4->pTexture->uHeightMinus1;
                }
                else
                {
                  if ( BYTE1(v21) & 8 )
                    v4->sTextureDeltaV += v22 & v4->pTexture->uHeightMinus1;
                }
                if ( BYTE1(v21) & 0x10 )
                {
                  v4->sTextureDeltaU -= v22 & v4->pTexture->uWidthMinus1;
                }
                else
                {
                  if ( BYTE1(v21) & 0x20 )
                    v4->sTextureDeltaU += v22 & v4->pTexture->uWidthMinus1;
                }
                v23 = v4->ptr_38;
                v4->field_52 = 32;
                v4->field_5A = 5;
                if ( (double)abs(v23->field_C) > 52428.8 )
                {
                  v4->field_52 = 8;
                  v4->field_5A = 3;
                }
                v24 = *(int *)&v4->flags;
                if ( !(v24 & 0x10000) )
                {
                  if ( !(v24 & 2) )
                  {
                    v25 = v4->pODMFace;
                    if ( v25->uPolygonType == 1 )
                    {
                      v26 = v25->pTextureUIDs[0];
                      dword_80AA1C = v26;
                      dword_80AA20 = v26;
                      v27 = v25->pTextureVIDs[0];
                      dword_80AA14 = v27;
                      dword_80AA18 = v27;
                      v35 = 1;
                      if ( v25->uNumVertices > 1u )
                      {
                        v33 = (int)&v25->pTextureVIDs[1];
                        do
                        {
                          if ( dword_80AA20 > *(short *)(v33 - 40) )
                            dword_80AA20 = *(short *)(v33 - 40);
                          if ( v26 < *(short *)(v33 - 40) )
                          {
                            v26 = *(short *)(v33 - 40);
                            dword_80AA1C = *(short *)(v33 - 40);
                          }
                          if ( dword_80AA18 > *(short *)v33 )
                            dword_80AA18 = *(short *)v33;
                          if ( v27 < *(short *)v33 )
                          {
                            v27 = *(short *)v33;
                            dword_80AA14 = *(short *)v33;
                          }
                          ++v35;
                          v33 += 2;
                        }
                        while ( v35 < v25->uNumVertices );
                        v0 = a1;
                      }
                      dword_80AA20 = (dword_80AA20 + v4->sTextureDeltaU) << 16;
                      dword_80AA1C = ((v26 + v4->sTextureDeltaU) << 16) - 65536;
                      dword_80AA18 = (dword_80AA18 + v4->sTextureDeltaV) << 16;
                      dword_80AA14 = ((v27 + v4->sTextureDeltaV) << 16) - 65536;
                    }
                    while ( 1 )
                    {
                      if ( !sr_sub_482E07(v3, v0) )
                        sr_sub_48585C_mb_DrawSpan(v3, &v0[v3->field_8 + 640 * v3->field_A], 0);
                      v3->field_E = LOWORD(sr_6BE060[1]);
                      if ( v4->prolly_tail == v3 )
                        break;
                      v3 = v3->pNext;
                    }
                    goto LABEL_14;
                  }
                  while ( 1 )
                  {
LABEL_74:
                    if ( !sr_sub_4839BD(v3, v0) )
                      sr_sub_48585C_mb_DrawSpan(v3, &v0[v3->field_8 + 640 * v3->field_A], 0);
                    v3->field_E = LOWORD(sr_6BE060[1]);
                    if ( v4->prolly_tail == v3 )
                      break;
                    v3 = v3->pNext;
                  }
                  goto LABEL_14;
                }
                if ( v24 & 2 )
                  goto LABEL_74;
                while ( 1 )
                {
                  if ( !sr_sub_482A94(v3) )
                    sr_sub_48585C_mb_DrawSpan(v3, &v0[v3->field_8 + 640 * v3->field_A], 0);
                  v3->field_E = LOWORD(sr_6BE060[1]);
                  if ( v4->prolly_tail == v3 )
                    break;
                  v3 = v3->pNext;
                }
              }
            }
            else
            {
              v3->field_E = LOWORD(sr_6BE060[1]);
            }
          }
        }
      }
LABEL_14:
      ++v34;
      LOWORD(v2) = v34;
      v36 += 67;
    }
    while ( v34 < (signed int)v37 );
  }
  if ( pParty->uFlags & 2 )
  {
    v6 = pODMRenderParams->uNumPolygons;
    v7 = 0;
    uNumElementsIn80AA28 = 0;
    if ( pODMRenderParams->uNumPolygons > 0 )
    {
      v8 = (char *)&array_77EC08[0].flags;
      do
      {
        v2 = *(Polygon **)v8;
        if ( (unsigned int)v2 & 0x20000 )
        {
          ++v7;
          *(int *)v8 = (unsigned int)v2 & 0xFFFDFFFF;
          LOWORD(v2) = (short)v8 - 48;
          *(&uNumElementsIn80AA28 + v7) = (unsigned int)(v8 - 48);
        }
        v8 += 268;
        --v6;
      }
      while ( v6 );
      uNumElementsIn80AA28 = v7;
    }
  }
  v9 = pODMRenderParams->uNumSpans;
  sr_6BE060[0] = pODMRenderParams->uNumSpans;
  if (pODMRenderParams->uNumPolygons >= 1999)
    return;

  array_77EC08[1999].Create_48607B(&stru_8019C8);
  array_77EC08[1999].ptr_38->_48694B_frustum_sky();

  if (pOutdoor->sMainTile_BitmapID == -1)
  {
    array_77EC08[1999].pTexture = nullptr;
    return;
  }
  else
    array_77EC08[1999].pTexture = pBitmaps_LOD->GetTexture(pOutdoor->sMainTile_BitmapID);
  array_77EC08[1999].dimming_level = 23 - (-20 * pOutdoor->vSunlight.z >> 16);
  if ( array_77EC08[1999].dimming_level > 20 )
    array_77EC08[1999].dimming_level = 20;
  v10 = stru_5C6E00->Sin(pIndoorCamera->sRotationX);
  array_77EC08[1999].v_18.y = 0;
  array_77EC08[1999].v_18.x = v10;
  array_77EC08[1999].v_18.z = stru_5C6E00->Cos(pIndoorCamera->sRotationX);
  array_77EC08[1999].field_24 = 2048 - (pIndoorCamera->pos.z << 16);
  a1a = (signed __int64)((double)(pIndoorCamera->pos.z * pODMRenderParams->int_fov_rad)
                       / ((double)pODMRenderParams->int_fov_rad + 8192.0)
                       + (double)pViewport->uScreenCenterY);
  cos((double)pIndoorCamera->sRotationX * 0.0030664064);
  sin((double)pIndoorCamera->sRotationX * 0.0030664064);
  array_77EC08[1999].Create_48607B(&stru_8019C8);
  array_77EC08[1999].ptr_38->_48694B_frustum_sky();

  if (pOutdoor->sSky_TextureID == -1)
  {
    array_77EC08[1999].pTexture = nullptr;
    return;
  }
  else
    array_77EC08[1999].pTexture = pBitmaps_LOD->GetTexture(pOutdoor->sSky_TextureID);
  array_77EC08[1999].dimming_level = 0;
  v11 = stru_5C6E00->Sin(pIndoorCamera->sRotationX + 16);
  array_77EC08[1999].v_18.y = 0;
  array_77EC08[1999].v_18.x = -v11;
  array_77EC08[1999].v_18.z = -stru_5C6E00->Cos(pIndoorCamera->sRotationX + 16);
  LOWORD(v2) = 224 * LOWORD(pMiscTimer->uTotalGameTimeElapsed);
  array_77EC08[1999].field_24 = 0x2000000u;
  array_77EC08[1999].sTextureDeltaU = 224 * pMiscTimer->uTotalGameTimeElapsed;
  array_77EC08[1999].sTextureDeltaV = 224 * pMiscTimer->uTotalGameTimeElapsed;
  if ( day_attrib & DAY_ATTRIB_FOG
    && (LOWORD(v2) = LOWORD(pParty->uCurrentHour), pParty->uCurrentHour >= 5)
    && pParty->uCurrentHour < 0x15
    || bUnderwater )
  {
    v2 = (Polygon *)*(short *)PaletteManager::Get_Mist_or_Red_LUT(array_77EC08[1999].pTexture->palette_id2, 31, 1);
    a1b = (unsigned int)v2;
    if ( (signed int)v9 <= 0 )
      return;
    v29 = v9;
    while ( 1 )
    {
      v30 = &pSpans[v29 - 1];
      v2 = (Polygon *)v30->field_E;
      if ( v2 != (Polygon *)sr_6BE060[1] )
      {
        LOWORD(v2) = LOWORD(pViewport->uViewportTL_X);
        if ( v30->field_8 >= (signed int)pViewport->uViewportTL_X )
          goto LABEL_124;
        if ( v30->field_C == pViewport->uViewportTL_X )
        {
          v30->field_E = LOWORD(sr_6BE060[1]);
        }
        else
        {
          v30->field_8 = LOWORD(pViewport->uViewportTL_X);
          v30->field_C -= LOWORD(pViewport->uViewportTL_X);
          if ( v30->field_C >= 0 )
          {
LABEL_124:
            v31 = v30->field_C;
            v30->pParent = &array_77EC08[1999];
            fill_pixels_fast(
              a1b,
              &render->pTargetSurface[v30->field_8 + render->uTargetSurfacePitch * v30->field_A],
              v31);
            j_memset32(-65536, &render->pActiveZBuffer[v30->field_8 + 640 * v30->field_A], v30->field_C);
            goto LABEL_125;
          }
          LOWORD(v2) = LOWORD(sr_6BE060[1]);
          v30->field_E = LOWORD(sr_6BE060[1]);
        }
      }
LABEL_125:
      --v29;
      --v9;
      if ( !v9 )
        return;
    }
  }
  if ( (signed int)v9 > 0 )
  {
    v12 = v9;
    do
    {
      v13 = &pSpans[v12 - 1];
      v2 = (Polygon *)v13->field_E;
      if ( v2 != (Polygon *)sr_6BE060[1] )
      {
        LOWORD(v2) = LOWORD(pViewport->uViewportTL_X);
        if ( v13->field_8 >= (signed int)pViewport->uViewportTL_X )
          goto LABEL_109;
        if ( v13->field_C != pViewport->uViewportTL_X )
        {
          v13->field_8 = LOWORD(pViewport->uViewportTL_X);
          v13->field_C -= LOWORD(pViewport->uViewportTL_X);
          if ( v13->field_C >= 0 )
          {
LABEL_109:
            if ( pODMRenderParams->bNoSky )
            {
              const_1_0();
            }
            else
            {
              v13->pParent = &array_77EC08[1999];
              if ( !Render::DrawSkySW(v13, &array_77EC08[1999], a1a) )
                j_memset32(-65536, &render->pActiveZBuffer[v13->field_8 + 640 * v13->field_A], v13->field_C);
            }
          }
          LOWORD(v2) = LOWORD(sr_6BE060[1]);
          v13->field_E = LOWORD(sr_6BE060[1]);
          goto LABEL_114;
        }
        v13->field_E = LOWORD(sr_6BE060[1]);
      }
LABEL_114:
      --v12;
      --v9;
    }
    while ( v9 );
  }
}

//----- (00485044) --------------------------------------------------------
int Render::DrawSkySW(Span *a1, Polygon *a2, int a3)
{
  Polygon *v3; // esi@1
  Span *v4; // edi@1
  float v5; // ST2C_4@1
  signed int result; // eax@2
  int v7; // ST40_4@3
  stru149 *v8; // eax@3
  int v9; // ebx@3
  int v10; // ecx@3
  int v11; // edx@3
  int v12; // eax@3
  int v13; // ST28_4@5
  int v14; // eax@5
  signed __int64 v15; // qtt@11
  int v16; // ST28_4@11
  int v17; // eax@11
  signed int v18; // ecx@11
  int v19; // ST40_4@11
  int v20; // ST3C_4@11
  int v21; // ST30_4@11
  void *v22; // eax@11
  Texture_MM7 *v23; // esi@11
  int v24; // ecx@11
  unsigned int v25; // esi@11
  int v26; // edi@11
  unsigned __int16 *v27; // eax@11
  int *v28; // ebx@12
  int v29; // edx@13
  unsigned __int16 v30; // cx@13
  int v31; // edx@14
  unsigned __int16 v32; // cx@14
  unsigned __int8 v33; // sf@15
  unsigned __int8 v34; // of@15
  double v35; // [sp+14h] [bp-38h]@1
  int v36; // [sp+18h] [bp-34h]@3
  signed int v37; // [sp+18h] [bp-34h]@11
  int v38; // [sp+1Ch] [bp-30h]@3
  signed int v39; // [sp+1Ch] [bp-30h]@11
  int v40; // [sp+20h] [bp-2Ch]@3
  void *v41; // [sp+20h] [bp-2Ch]@11
  int v42; // [sp+24h] [bp-28h]@3
  unsigned __int8 *v43; // [sp+24h] [bp-28h]@11
  int v44; // [sp+28h] [bp-24h]@11
  int v45; // [sp+2Ch] [bp-20h]@3
  signed int v46; // [sp+30h] [bp-1Ch]@3
  __int16 v47; // [sp+30h] [bp-1Ch]@11
  signed int v48; // [sp+34h] [bp-18h]@3
  int v49; // [sp+34h] [bp-18h]@11
  int v50; // [sp+38h] [bp-14h]@3
  unsigned __int16 *v51; // [sp+38h] [bp-14h]@11
  int v52; // [sp+3Ch] [bp-10h]@4
  int a1a; // [sp+40h] [bp-Ch]@3
  int v54; // [sp+44h] [bp-8h]@3
  int v55; // [sp+48h] [bp-4h]@3
  int v56; // [sp+54h] [bp+8h]@11
  int *v57; // [sp+54h] [bp+8h]@11

  v3 = a2;
  v4 = a1;
  v5 = (double)(pViewport->uViewportBR_X - pViewport->uViewportTL_X) * 0.5 / tan(0.6457717418670654) + 0.5;
  v35 = v5 + 6.7553994e15;
  if ( LODWORD(v35) )
  {
    v55 = 65536 / SLODWORD(v35);
    v7 = 65536 / SLODWORD(v35) * (a3 - v4->field_A);
    v8 = v3->ptr_38;
    v42 = ((unsigned __int64)(v3->ptr_38->field_14 * (signed __int64)v7) >> 16) + v8->field_C;
    v40 = ((unsigned __int64)(v8->field_20 * (signed __int64)v7) >> 16) + v3->ptr_38->field_18;
    v38 = pIndoorCameraD3D->int_sine_y;
    HIDWORD(v35) = pIndoorCameraD3D->int_cosine_y;
    v45 = v4->field_C;
    v9 = ((unsigned __int64)(v3->v_18.z * (signed __int64)v7) >> 16) + v3->v_18.x;
    v10 = 65536 / SLODWORD(v35) * (pViewport->uScreenCenterX - v4->field_8);
    v48 = 0;
    v50 = 65536 / SLODWORD(v35) * (pViewport->uScreenCenterX - v4->field_8);
    v46 = -v3->field_24;
    v11 = v4->field_A - 1;
    v54 = v11;
    v12 = 65536 / SLODWORD(v35) * (a3 - v11);
    a1a = 65536 / SLODWORD(v35) * (a3 - v11);
    while ( 1 )
    {
      v52 = v9;
      if ( v9 )
      {
        v13 = abs(v46 >> 14);
        v14 = abs(v9);
        v11 = v54;
        v10 = v50;
        if ( v13 <= v14 )
          break;
        v12 = a1a;
      }
      if ( v11 <= (signed int)pViewport->uViewportTL_Y )
        break;
      v9 = ((unsigned __int64)(v3->v_18.z * (signed __int64)v12) >> 16) + v3->v_18.x;
      --v54;
      a1a += v55;
      v12 = a1a;
      v11 = v54;
      v48 = 1;
    }
    if ( v48 )
      v52 = ((unsigned __int64)(v3->v_18.z * (signed __int64)(v55 * (a3 + (signed int)v4->field_A - 2 * v11))) >> 16)
          + v3->v_18.x;
    LODWORD(v15) = v46 << 16;
    HIDWORD(v15) = v46 >> 16;
    v16 = v42 + ((unsigned __int64)(v3->ptr_38->field_10 * (signed __int64)v10) >> 16);
    v17 = v40 + ((unsigned __int64)(v3->ptr_38->field_1C * (signed __int64)v10) >> 16);
    v18 = v15 / v52;
    v43 = v3->pTexture->pLevelOfDetail0_prolly_alpha_mask;
    v19 = v3->sTextureDeltaU + ((signed int)((unsigned __int64)(v16 * v15 / v52) >> 16) >> 3);
    v56 = v15 / v52;
    v20 = v3->sTextureDeltaV + ((signed int)((unsigned __int64)(v17 * v15 / v52) >> 16) >> 3);
    v21 = (unsigned __int64)(v55 * (signed __int64)v56) >> 16;
    v39 = (signed int)((unsigned __int64)(v21 * (signed __int64)v38) >> 16) >> 3;
    v37 = (signed int)((unsigned __int64)(v21 * (signed __int64)v36) >> 16) >> 3;
    v22 = sr_sub_47C178(v18, v3, 0, 1);
    v23 = v3->pTexture;
    v41 = v22;
    v47 = 16 - v23->uWidthLn2;
    v44 = v23->uTextureWidth - 1;
    v49 = (v23->uTextureHeight << 16) - 65536;
    v24 = v4->field_8;
    v51 = &render->pTargetSurface[v24 + render->uTargetSurfacePitch * v4->field_A];
    v57 = &render->pActiveZBuffer[v24 + 640 * v4->field_A];
    v25 = v19;
    v26 = v20;
    v27 = v51;
    if ( !(v45 & 1) )
      goto LABEL_15;
    --v45;
    v27 = v51 - 1;
    v28 = v57;
    ++v57;
    while ( 1 )
    {
      *v28 = -65536;
      v31 = v44 & (v25 >> 16);
      v27 += 2;
      v25 += v39;
      v32 = *((short *)v41 + *(&v43[v31] + ((v49 & (unsigned int)v26) >> v47)));
      v26 += v37;
      *(v27 - 1) = v32;
LABEL_15:
      v34 = __OFSUB__(v45, 2);
      v33 = v45 - 2 < 0;
      v45 -= 2;
      if ( v33 ^ v34 )
        break;
      v29 = v44 & (v25 >> 16);
      v25 += v39;
      v30 = *((short *)v41 + *(&v43[v29] + ((v49 & (unsigned int)v26) >> v47)));
      v26 += v37;
      v28 = v57;
      *v27 = v30;
      v57 += 2;
      v28[1] = -65536;
    }
    result = 1;
  }
  else
  {
    result = 0;
  }
  return result;
}

//----- (00482E07) --------------------------------------------------------
signed int sr_sub_482E07(Span *ecx0, unsigned __int16 *pRenderTarget)
{
  stru315 *v2; // ebp@0
  Polygon *v3; // esi@1
  int v4; // edi@1
  int v5; // edi@1
  stru149 *v6; // eax@1
  stru149 *v7; // eax@1
  int v8; // edx@1
  int v9; // eax@1
  int v10; // edi@1
  int v11; // eax@1
  unsigned __int64 v12; // qax@1
  int v13; // eax@1
  signed __int64 v14; // qtt@3
  int v15; // ebx@4
  signed __int64 v16; // qtt@5
  int v17; // eax@5
  unsigned __int16 *v18; // eax@7
  Texture_MM7 *v19; // eax@8
  Texture_MM7 *v20; // eax@10
  Texture_MM7 *v21; // eax@12
  Texture_MM7 *v22; // eax@14
  int v23; // ecx@17
  Texture_MM7 *v24; // ebx@17
  signed int v25; // edx@17
  signed int v26; // eax@17
  char v27; // bl@17
  stru149 *v28; // eax@18
  stru149 *v29; // eax@18
  int v30; // eax@18
  int v31; // eax@18
  unsigned int v32; // edx@18
  int v33; // edi@21
  signed __int64 v34; // qtt@22
  signed int v35; // ecx@22
  int v36; // eax@24
  stru149 *v37; // eax@24
  int v38; // edi@24
  int v39; // eax@24
  int v40; // edi@35
  signed __int64 v41; // qtt@36
  int v42; // edx@36
  stru149 *v43; // eax@36
  int v44; // edi@36
  int v45; // eax@36
  Span *v46; // edi@44
  stru149 *v47; // eax@44
  stru149 *v48; // eax@44
  int v49; // eax@44
  int v50; // ecx@44
  unsigned int v51; // edx@44
  int v52; // edi@46
  signed __int64 v53; // qtt@47
  unsigned int v54; // ecx@47
  int v55; // eax@49
  stru149 *v56; // eax@49
  int v57; // edi@49
  int v58; // eax@49
  int v59; // edi@60
  signed __int64 v60; // qtt@61
  int v61; // edx@61
  stru149 *v62; // eax@61
  int v63; // edi@61
  int v64; // eax@61
  int v66; // [sp+Ch] [bp-B8h]@1
  int v67; // [sp+10h] [bp-B4h]@1
  int v68; // [sp+14h] [bp-B0h]@1
  int v69; // [sp+18h] [bp-ACh]@1
  int v70; // [sp+1Ch] [bp-A8h]@5
  int v71; // [sp+20h] [bp-A4h]@1
  signed int v72; // [sp+24h] [bp-A0h]@1
  stru316 a2; // [sp+28h] [bp-9Ch]@18
  int v74; // [sp+50h] [bp-74h]@17
  stru315 a1; // [sp+54h] [bp-70h]@5
  Span *v76; // [sp+98h] [bp-2Ch]@1
  unsigned __int16 *v77; // [sp+9Ch] [bp-28h]@1
  int v78; // [sp+A0h] [bp-24h]@17
  int v79; // [sp+A4h] [bp-20h]@3
  int v80; // [sp+A8h] [bp-1Ch]@3
  int v81; // [sp+ACh] [bp-18h]@1
  int X; // [sp+B0h] [bp-14h]@2
  int v83; // [sp+B4h] [bp-10h]@1
  int v84; // [sp+B8h] [bp-Ch]@1
  int v85; // [sp+BCh] [bp-8h]@1
  int v86; // [sp+C0h] [bp-4h]@18

  v85 = ecx0->field_C;
  v3 = ecx0->pParent;
  v4 = pViewport->uScreenCenterY - ecx0->field_A;
  v77 = pRenderTarget;
  v5 = pODMRenderParams->int_fov_rad_inv * v4;
  v6 = v3->ptr_38;
  v76 = ecx0;
  v81 = v5;
  v83 = v6->field_14;
  v81 = (unsigned __int64)(v83 * (signed __int64)v5) >> 16;
  v7 = v3->ptr_38;
  v8 = v81 + v7->field_C;
  v83 = v7->field_20;
  v67 = v8;
  v9 = ((unsigned __int64)(v83 * (signed __int64)v5) >> 16) + v3->ptr_38->field_18;
  v81 = v5;
  v71 = v9;
  v83 = v3->v_18.z;
  v81 = (unsigned __int64)(v83 * (signed __int64)v5) >> 16;
  v68 = v3->sTextureDeltaU << 16;
  v69 = v3->sTextureDeltaV << 16;
  v72 = -v3->field_24;
  v10 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterX - ecx0->field_8);
  v11 = v3->v_18.y;
  v66 = v81 + v3->v_18.x;
  v83 = v10;
  v12 = v10 * (signed __int64)v11;
  v81 = v12 >> 16;
  v13 = v66 + (v12 >> 16);
  v84 = v13;
  if ( !v13
    || (X = v72 >> 15, v83 = abs(v13), abs(v72 >> 15) >= v83)
    || (LODWORD(v14) = v72 << 16,
        HIDWORD(v14) = v72 >> 16,
        v79 = v14 / v84,
        v80 = v10 - v85 * pODMRenderParams->int_fov_rad_inv,
        v81 = (unsigned __int64)(v80 * (signed __int64)v3->v_18.y) >> 16,
        (v84 = v66 + v81) == 0)
    || (v15 = abs(v66 + v81), abs(X) >= v15) )
    return 0;
  LODWORD(v16) = v72 << 16;
  HIDWORD(v16) = v72 >> 16;
  v70 = v16 / v84;
  v17 = v16 / v84;
  a1.field_24 = v79;
  if ( v79 >= v17 )
    a1.field_24 = v17;
  v18 = (unsigned __int16 *)v3->pTexture->pLevelOfDetail2;
  a1.pTextureLOD = (unsigned __int16 *)v3->pTexture->pLevelOfDetail2;
  v84 = 2;
  if ( a1.field_24 >= mipmapping_building_mm1 << 16 )
  {
    if ( a1.field_24 >= mipmapping_building_mm2 << 16 )
    {
      if ( a1.field_24 >= mipmapping_building_mm3 << 16 )
      {
        if ( bUseLoResSprites )
          goto LABEL_16;
        v22 = v3->pTexture;
        v84 = 3;
        v18 = (unsigned __int16 *)v22->pLevelOfDetail3;
      }
      else
      {
        v21 = v3->pTexture;
        v84 = 2;
        v18 = (unsigned __int16 *)v21->pLevelOfDetail2;
      }
    }
    else
    {
      v20 = v3->pTexture;
      v84 = 1;
      v18 = (unsigned __int16 *)v20->pLevelOfDetail1;
    }
  }
  else
  {
    v19 = v3->pTexture;
    v84 = 0;
    v18 = (unsigned __int16 *)v19->pLevelOfDetail0_prolly_alpha_mask;
  }
  a1.pTextureLOD = v18;
LABEL_16:
  if ( v18 )
  {
    v23 = v3->field_5A;
    v83 = v3->field_52;
    v24 = v3->pTexture;
    v74 = v23;
    v78 = v85 >> v23;
    v25 = v24->uTextureWidth;
    v81 = v85 - (v85 >> v23 << v23);
    v26 = (signed int)v24->uTextureHeight >> v84;
    a1.field_10 = v84 - v24->uWidthLn2 + 16;
    v27 = v84 + bUseLoResSprites;
    a1.field_8 = (v26 << 16) - 65536;
    a1.field_C = (v25 >> v84) - 1;
    if ( v79 >= v70 )
    {
      v46 = v76;
      a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&a2, v76, v70, v3, pODMRenderParams->building_gamme, 0, 1);
      v47 = v3->ptr_38;
      v79 = v80;
      v86 = v47->field_10;
      v79 = v80;
      v84 = v67 + ((unsigned __int64)(v86 * (signed __int64)v80) >> 16);
      v86 = v3->ptr_38->field_1C;
      v86 = v71 + ((unsigned __int64)(v86 * (signed __int64)v80) >> 16);
      v79 = (unsigned __int64)(v84 * (signed __int64)v70) >> 16;
      v48 = v3->ptr_38;
      v79 = v70;
      a1.field_30 = v68 + ((unsigned __int64)(v84 * (signed __int64)v70) >> 16) - v48->field_24;
      v79 = (unsigned __int64)(v86 * (signed __int64)v70) >> 16;
      v49 = ((unsigned __int64)(v86 * (signed __int64)v70) >> 16) - v3->ptr_38->field_28;
      a1.field_30 >>= v27;
      a1.field_2C = (v69 + v49) >> v27;
      a1.field_14 = dword_80AA20 >> v27;
      a1.field_18 = dword_80AA1C >> v27;
      a1.field_1C = dword_80AA18 >> v27;
      v50 = v46->field_8;
      a1.field_20 = dword_80AA14 >> v27;
      v51 = v50 + render->uTargetSurfacePitch * v46->field_A;
      a1.pDepthBuffer = (unsigned int *)(&render->pActiveZBuffer[640 * v46->field_A - 1] + v85 + v50);
      a1.pColorBuffer = &v77[v85 - 1] + v51;
      v80 += pODMRenderParams->int_fov_rad_inv << v74;
      if ( v78 > 0 )
      {
        do
        {
          v77 = (unsigned __int16 *)v3->v_18.y;
          v86 = (unsigned __int64)((signed int)v77 * (signed __int64)v80) >> 16;
          v84 = v66 + ((unsigned __int64)((signed int)v77 * (signed __int64)v80) >> 16);
          if ( v66 + (unsigned int)((unsigned __int64)((signed int)v77 * (signed __int64)v80) >> 16)
            && (v52 = abs((signed __int64)v66 + ((__int64)((signed int)v77 * (signed __int64)v80) >> 16)), abs((signed __int64)X) < v52) )
          {
            LODWORD(v53) = v72 << 16;
            HIDWORD(v53) = v72 >> 16;
            v86 = v53 / v84;
            v54 = v53 / v84;
            v84 = v53 / v84;
          }
          else
          {
            v84 = 0x40000000u;
            v54 = 0x40000000u;
          }
          HIWORD(v55) = HIWORD(v54);
          LOWORD(v55) = 0;
          a1.field_24 = v3->field_50 | v55;
          v77 = (unsigned __int16 *)v3->ptr_38->field_10;
          v77 = (unsigned __int16 *)(v67 + ((unsigned __int64)((signed int)v77 * (signed __int64)v80) >> 16));
          v86 = (unsigned __int64)((signed int)v77 * (signed __int64)(signed int)v54) >> 16;
          v56 = v3->ptr_38;
          v57 = ((unsigned __int64)((signed int)v77 * (signed __int64)(signed int)v54) >> 16) - v56->field_24;
          v77 = (unsigned __int16 *)v56->field_1C;
          v77 = (unsigned __int16 *)(v71 + ((unsigned __int64)((signed int)v77 * (signed __int64)v80) >> 16));
          v86 = (unsigned __int64)((signed int)v77 * (signed __int64)(signed int)v54) >> 16;
          v58 = (((v69 + v86 - v3->ptr_38->field_28) >> v27) - a1.field_2C) >> v74;
          a1.field_4 = (((v68 + v57) >> v27) - a1.field_30) >> v74;
          a1.field_0 = v58;
          a1.field_28 = v83;
          if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_485BAE(&a1, &a2);
            else
              sr_sub_485AFF(&a1, &a2);
          }
          else
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_4D71F8(&a1);
            else
              sr_sub_4D714C(&a1);
          }
          a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(
                                                      &a2,
                                                      v76,
                                                      v84,
                                                      v3,
                                                      pODMRenderParams->building_gamme,
                                                      0,
                                                      0);
          v80 += pODMRenderParams->int_fov_rad_inv << v74;
          --v78;
        }
        while ( v78 );
      }
      if ( !v81 )
        return 1;
      v83 = v3->v_18.y;
      v86 = (unsigned __int64)(v83 * (signed __int64)v80) >> 16;
      v84 = v66 + ((unsigned __int64)(v83 * (signed __int64)v80) >> 16);
      if ( v66 + (unsigned int)((unsigned __int64)(v83 * (signed __int64)v80) >> 16) )
      {
        v59 = abs((signed __int64)v66 + ((__int64)(v83 * (signed __int64)v80) >> 16));
        if ( abs(X) < v59 )
        {
          LODWORD(v60) = v72 << 16;
          HIDWORD(v60) = v72 >> 16;
          v86 = v60 / v84;
          HIWORD(v61) = (unsigned int)(v60 / v84) >> 16;
          LOWORD(v61) = 0;
          a1.field_24 = v61 | v3->field_50;
          v83 = v3->ptr_38->field_10;
          v83 = v67 + ((unsigned __int64)(v83 * (signed __int64)v80) >> 16);
          v86 = (unsigned __int64)(v83 * v60 / v84) >> 16;
          v62 = v3->ptr_38;
          v63 = ((unsigned __int64)(v83 * v60 / v84) >> 16) - v62->field_24;
          v83 = v62->field_1C;
          v83 = v71 + ((unsigned __int64)(v83 * (signed __int64)v80) >> 16);
          v86 = (unsigned __int64)(v83 * v60 / v84) >> 16;
          v64 = (((signed int)(v69 + ((unsigned __int64)(v83 * v60 / v84) >> 16) - v3->ptr_38->field_28) >> v27)
               - a1.field_2C) >> v74;
          a1.field_4 = (((v68 + v63) >> v27) - a1.field_30) >> v74;
          a1.field_0 = v64;
          a1.field_28 = v81;
          if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_485BAE(&a1, &a2);
            else
              sr_sub_485AFF(&a1, &a2);
          }
          else
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_4D71F8(&a1);
            else
              sr_sub_4D714C(&a1);
          }
          return 1;
        }
      }
    }
    else
    {
      a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&a2, v76, v79, v3, pODMRenderParams->building_gamme, 1u, 1);
      v28 = v3->ptr_38;
      v85 = v10;
      v86 = v28->field_10;
      v85 = v10;
      v84 = v67 + ((unsigned __int64)(v86 * (signed __int64)v10) >> 16);
      v86 = v3->ptr_38->field_1C;
      v86 = v71 + ((unsigned __int64)(v86 * (signed __int64)v10) >> 16);
      v85 = (unsigned __int64)(v84 * (signed __int64)v79) >> 16;
      v29 = v3->ptr_38;
      v85 = v79;
      a1.field_30 = v68 + ((unsigned __int64)(v84 * (signed __int64)v79) >> 16) - v29->field_24;
      v85 = (unsigned __int64)(v86 * (signed __int64)v79) >> 16;
      v30 = ((unsigned __int64)(v86 * (signed __int64)v79) >> 16) - v3->ptr_38->field_28;
      a1.field_30 >>= v27;
      a1.field_2C = (v69 + v30) >> v27;
      a1.field_14 = dword_80AA20 >> v27;
      a1.field_18 = dword_80AA1C >> v27;
      a1.field_1C = dword_80AA18 >> v27;
      a1.field_20 = dword_80AA14 >> v27;
      v31 = v76->field_A;
      v32 = render->uTargetSurfacePitch * v76->field_A;
      v86 = v76->field_8;
      a1.pColorBuffer = &v77[v86 + v32];
      a1.pDepthBuffer = (unsigned int *)&render->pActiveZBuffer[v86 + 640 * v31];
      v80 = v10 - (pODMRenderParams->int_fov_rad_inv << v74);
      if ( v78 > 0 )
      {
        v86 = v78;
        do
        {
          v78 = v3->v_18.y;
          v85 = (unsigned __int64)(v78 * (signed __int64)v80) >> 16;
          v84 = v66 + ((unsigned __int64)(v78 * (signed __int64)v80) >> 16);
          if ( v66 + (unsigned int)((unsigned __int64)(v78 * (signed __int64)v80) >> 16)
            && (v33 = abs((signed __int64)v66 + ((__int64)(v78 * (signed __int64)v80) >> 16)), abs((signed __int64)X) < v33) )
          {
            LODWORD(v34) = v72 << 16;
            HIDWORD(v34) = v72 >> 16;
            v85 = v34 / v84;
            v35 = v34 / v84;
            v84 = v34 / v84;
          }
          else
          {
            v84 = 1073741824;
            v35 = 1073741824;
          }
          HIWORD(v36) = HIWORD(v35);
          LOWORD(v36) = 0;
          a1.field_24 = v3->field_50 | v36;
          v78 = v3->ptr_38->field_10;
          v78 = v67 + ((unsigned __int64)(v78 * (signed __int64)v80) >> 16);
          v85 = (unsigned __int64)(v78 * (signed __int64)v35) >> 16;
          v37 = v3->ptr_38;
          v38 = ((unsigned __int64)(v78 * (signed __int64)v35) >> 16) - v37->field_24;
          v78 = v37->field_1C;
          v78 = v71 + ((unsigned __int64)(v78 * (signed __int64)v80) >> 16);
          v85 = (unsigned __int64)(v78 * (signed __int64)v35) >> 16;
          v39 = (((v69 + v85 - v3->ptr_38->field_28) >> v27) - a1.field_2C) >> v74;
          a1.field_4 = (((v68 + v38) >> v27) - a1.field_30) >> v74;
          a1.field_0 = v39;
          a1.field_28 = v83;
          if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_485A24(&a1, (stru315 *)&a2);
            else
              sr_sub_485975(&a1, (stru315 *)&a2);
          }
          else
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_4D705A(v2);
            else
              sr_sub_4D6FB0(v2);
          }
          a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(
                                                      &a2,
                                                      v76,
                                                      v84,
                                                      v3,
                                                      pODMRenderParams->building_gamme,
                                                      1u,
                                                      0);
          v80 -= pODMRenderParams->int_fov_rad_inv << v74;
          --v86;
        }
        while ( v86 );
      }
      if ( !v81 )
        return 1;
      v83 = v3->v_18.y;
      v76 = (Span *)((unsigned __int64)(v83 * (signed __int64)v80) >> 16);
      v84 = v66 + ((unsigned __int64)(v83 * (signed __int64)v80) >> 16);
      if ( v66 + (unsigned int)((unsigned __int64)(v83 * (signed __int64)v80) >> 16) )
      {
        v40 = abs((signed __int64)v66 + ((__int64)(v83 * (signed __int64)v80) >> 16));
        if ( abs(X) < v40 )
        {
          LODWORD(v41) = v72 << 16;
          HIDWORD(v41) = v72 >> 16;
          X = v41 / v84;
          HIWORD(v42) = (unsigned int)(v41 / v84) >> 16;
          LOWORD(v42) = 0;
          a1.field_24 = v42 | v3->field_50;
          v83 = v3->ptr_38->field_10;
          v83 = v67 + ((unsigned __int64)(v83 * (signed __int64)v80) >> 16);
          X = (unsigned __int64)(v83 * v41 / v84) >> 16;
          v43 = v3->ptr_38;
          v44 = ((unsigned __int64)(v83 * v41 / v84) >> 16) - v43->field_24;
          v83 = v43->field_1C;
          v83 = v71 + ((unsigned __int64)(v83 * (signed __int64)v80) >> 16);
          X = (unsigned __int64)(v83 * v41 / v84) >> 16;
          v45 = (((signed int)(v69 + ((unsigned __int64)(v83 * v41 / v84) >> 16) - v3->ptr_38->field_28) >> v27)
               - a1.field_2C) >> v74;
          a1.field_4 = (((v68 + v44) >> v27) - a1.field_30) >> v74;
          a1.field_0 = v45;
          a1.field_28 = v81;
          if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_485A24(&a1, (stru315 *)&a2);
            else
              sr_sub_485975(&a1, (stru315 *)&a2);
          }
          else
          {
            if ( v3->pODMFace->uPolygonType == 1 )
              sr_sub_4D705A(v2);
            else
              sr_sub_4D6FB0(v2);
          }
          return 1;
        }
      }
    }
  }
  return 0;
}

//----- (00485AFF) --------------------------------------------------------
stru315 *sr_sub_485AFF(stru315 *a1, stru316 *a2)
{
  stru315 *result; // eax@1
  int i; // ecx@1
  int v4; // esi@2
  int v5; // esi@2
  int v6; // ecx@2
  int v7; // esi@3
  int v8; // edi@3
  int v9; // ebx@3
  int v10; // ecx@8
  unsigned __int16 *v11; // esi@10
  unsigned int *v12; // ecx@12
  int v13; // esi@12
  int v14; // [sp+0h] [bp-4h]@2

  result = a1;
  for ( i = a1->field_28; i; i = result->field_28 )
  {
    v4 = result->field_8 & result->field_2C;
    result->field_28 = i - 1;
    v5 = (result->field_C & HIWORD(result->field_30)) + (v4 >> result->field_10);
    v6 = *((char *)result->pTextureLOD + v5);
    v14 = *((char *)result->pTextureLOD + v5);
    if ( a2->field_20 )
    {
      v7 = HIWORD(a2->field_14);
      v8 = a2->field_C;
      v9 = v7;
      if ( v7 >= v8 )
        v9 = a2->field_C;
      if ( a2->field_8 - v9 <= 0 )
      {
        v10 = 0;
      }
      else
      {
        if ( v7 >= v8 )
          v7 = a2->field_C;
        v10 = a2->field_8 - v7;
      }
      v11 = a2->field_24_palette;
      v6 = v14 + (v10 << 8);
    }
    else
    {
      v11 = result->field_34_palette;
    }
    *result->pColorBuffer = v11[v6];
    v12 = result->pDepthBuffer;
    v13 = result->field_24;
    --result->pColorBuffer;
    *v12 = v13;
    --result->pDepthBuffer;
    a2->field_14 -= a2->field_18;
    result->field_30 += result->field_4;
    result->field_2C += result->field_0;
  }
  --result->field_28;
  return result;
}

//----- (0047F4D3) --------------------------------------------------------
void _47F4D3_initialize_terrain_bezier_stuff(int band1, int band2, int band3)
{
  int v3; // edi@1
  //stru220 *v4; // esi@1
  double v5; // ST2C_8@3
  double v6; // st7@3
  //double v7; // [sp+18h] [bp-28h]@3
  //double v8; // [sp+20h] [bp-20h]@2
  int v9; // [sp+34h] [bp-Ch]@1
  int v10; // [sp+38h] [bp-8h]@1
  //signed int band3a; // [sp+48h] [bp+8h]@2

  v9 = band2 * 512;
  pODMRenderParams->outdoor_grid_band_3 = band3;
  v10 = band1 * 512;
  v3 = band3 * 512;
  pODMRenderParams->uPickDepth = band3 * 512;
  //v4 = terrain_76E5C8;                             // v4:  0 -> 65536
  for (uint i = 0; i < 16384; ++i)
  //do
  {
    auto v4 = terrain_76E5C8[i];
    //band3a = 256;
    //v8 = (double)(signed int)((char *)v4 + 256 - (int)terrain_76E5C8);
    for (uint j = 0; j < 128; ++j)                  // band3a: 0 -> 128
    {
      v5 = pow(j * 512 + 256, 2.0);
      v6 = pow(i * 4 + 256, 2.0);
      //*((float *)&v5 + 1) = sqrt(v6 + v5);
      int v7 = floorf(sqrtf(v5 + v6) + 0.5f);//*((float *)&v5 + 1) + 6.7553994e15;
      if (v7 >= v10)
      {
        if (v7 >= v9)
          v4.field_0 = ((v7 >= v3) - 1) & 2;
        else
          v4.field_0 = 2;
      }
      else
      {
        v4.field_0 = 1;
      }
      //band3a += 512;
      v4.distance = v7;
      //++v4;
    }
    //while ( band3a < 65792 );
  }
  //while ( (signed int)v4 < (signed int)arary_77E5C8 );
}

//----- (00481EFA) --------------------------------------------------------
bool _481EFA_clip_terrain_poly(RenderVertexSoft *a1, RenderVertexSoft *a2, RenderVertexSoft *a3, RenderVertexSoft *a4, int a5)
{
  RenderVertexSoft *v5; // esi@1
  RenderVertexSoft *v6; // edx@1
  bool v7; // edi@2
  bool v8; // ecx@5
  bool v9; // esi@8
  bool v10; // eax@11
  double v11; // st7@14
  signed int v12; // esi@15
  signed int v13; // edx@18
  signed int v14; // ecx@21
  signed int v15; // eax@24
  RenderVertexSoft *v17; // [sp+Ch] [bp-8h]@1

  v5 = a2;
  v6 = a1;
  v17 = v5;
  v7 = a1->vWorldViewPosition.x < 8.0;
  v8 = v5->vWorldViewPosition.x < 8.0;
  v9 = a3->vWorldViewPosition.x < 8.0;
  v10 = a4->vWorldViewPosition.x < 8.0;
  return !(v8 & v9 & v10 & v7)
      && ((v11 = (double)pODMRenderParams->shading_dist_mist, v11 > v6->vWorldViewPosition.x) ? (v12 = 0) : (v12 = 1),
          v11 > v17->vWorldViewPosition.x ? (v13 = 0) : (v13 = 1),
          v11 > a3->vWorldViewPosition.x ? (v14 = 0) : (v14 = 1),
          v11 > a4->vWorldViewPosition.x ? (v15 = 0) : (v15 = 1),
          !(v13 & v14 & v15 & v12));
}
//----- (00481212) --------------------------------------------------------
void Render::DrawTerrainSW(int a1, int a2, int a3, int a4)
{
  int v3; // esi@1
  int v4; // ecx@1
  int v5; // ST10_4@1
  int v6; // edi@1
  int v7; // ebx@2
  int v8; // eax@2
  int v9; // eax@3
  RenderVertexSoft *v10; // edi@3
  RenderVertexSoft *v11; // ebx@4
  RenderVertexSoft *v12; // ecx@4
  float v13; // eax@6
  int v14; // eax@6
  double v15; // st7@6
  double v16; // st7@6
  Polygon *v17; // ebx@12
  unsigned __int16 v18; // ax@12
  int v19; // eax@13
  float v20; // ecx@13
  stru220 *v21; // eax@13
  int v22; // eax@13
  signed int v23; // eax@13
  Vec3_float_ *v24; // eax@15
  double v25; // st7@18
  double v26; // st5@24
  double v27; // st5@24
  double v28; // st5@24
  double v29; // st5@24
  Game *v30; // eax@25
  RenderVertexSoft *v31; // edi@29
  double v32; // st7@31
  int v33; // esi@35
  RenderVertexSoft *v34; // edx@36
  RenderVertexSoft *v35; // ecx@36
  signed int v36; // eax@39
  signed int v37; // esi@40
  Polygon *v38; // ebx@43
  unsigned __int16 v39; // ax@43
  int v40; // eax@44
  float v41; // ecx@44
  stru220 *v42; // eax@44
  int v43; // eax@44
  int v44; // eax@44
  signed int v45; // eax@44
  Vec3_float_ *v46; // eax@46
  double v47; // st7@49
  double v48; // st5@57
  double v49; // st5@57
  double v50; // st5@57
  Game *v51; // eax@58
  RenderVertexSoft *v52; // edi@62
  double v53; // st7@63
  int v54; // esi@66
  signed int v55; // eax@69
  signed int v56; // esi@71
  unsigned __int16 v57; // ax@75
  stru220 *v58; // eax@76
  signed int v59; // eax@76
  Vec3_float_ *v60; // eax@77
  double v61; // st7@80
  double v62; // st5@88
  double v63; // st5@88
  double v64; // st5@88
  Game *v65; // eax@89
  double v66; // st7@94
  RenderVertexSoft *v67; // [sp-Ch] [bp-C4h]@36
  Polygon *v68; // [sp-8h] [bp-C0h]@36
  int v69; // [sp-4h] [bp-BCh]@36
  float v70; // [sp+Ch] [bp-ACh]@88
  double v71; // [sp+10h] [bp-A8h]@6
  double v72; // [sp+18h] [bp-A0h]@82
  double v73; // [sp+20h] [bp-98h]@6
  double v74; // [sp+28h] [bp-90h]@51
  double v75; // [sp+30h] [bp-88h]@6
  double v76; // [sp+38h] [bp-80h]@20
  double v77; // [sp+40h] [bp-78h]@6
  float v78; // [sp+48h] [bp-70h]@57
  float v79; // [sp+4Ch] [bp-6Ch]@6
  float v80; // [sp+50h] [bp-68h]@6
  int v81; // [sp+54h] [bp-64h]@1
  int v82; // [sp+58h] [bp-60h]@6
  int v83; // [sp+5Ch] [bp-5Ch]@82
  int v84; // [sp+60h] [bp-58h]@20
  int v85; // [sp+64h] [bp-54h]@1
  int v86; // [sp+68h] [bp-50h]@1
  int v87; // [sp+6Ch] [bp-4Ch]@51
  float v88; // [sp+70h] [bp-48h]@51
  int v89; // [sp+74h] [bp-44h]@6
  int v90; // [sp+78h] [bp-40h]@1
  float a3a; // [sp+7Ch] [bp-3Ch]@24
  stru220 *v92; // [sp+80h] [bp-38h]@2
  float v93; // [sp+84h] [bp-34h]@44
  int v94; // [sp+88h] [bp-30h]@6
  float v95; // [sp+8Ch] [bp-2Ch]@1
  float v96; // [sp+90h] [bp-28h]@1
  float v97; // [sp+94h] [bp-24h]@1
  int X; // [sp+98h] [bp-20h]@1
  float v99; // [sp+9Ch] [bp-1Ch]@6
  int v100; // [sp+A0h] [bp-18h]@6
  unsigned __int64 v101; // [sp+A4h] [bp-14h]@6
  RenderVertexSoft *v102; // [sp+ACh] [bp-Ch]@6
  RenderVertexSoft *v103; // [sp+B0h] [bp-8h]@6
  RenderVertexSoft *v104; // [sp+B4h] [bp-4h]@3
  float pNormalc; // [sp+C0h] [bp+8h]@6
  float pNormald; // [sp+C0h] [bp+8h]@6
  Vec3_float_ *pNormal; // [sp+C0h] [bp+8h]@17
  Vec3_float_ *pNormala; // [sp+C0h] [bp+8h]@48
  Vec3_float_ *pNormalb; // [sp+C0h] [bp+8h]@77

  v3 = a1;
  //v4 = *(short *)(a3 + 4);
  //v5 = *(short *)(a3 + 6);
  v85 = a2;
  v86 = a3; //v4;
  X = abs(a4); //v5
  v6 = 0;
  v90 = 0;
  v81 = v3 - 1;
  v95 = (double)pOutdoor->vSunlight.x / 65536.0;
  v96 = (double)pOutdoor->vSunlight.y / 65536.0;
  v97 = (double)pOutdoor->vSunlight.z / 65536.0;
  if ( v3 - 1 > 0 )
  {
    while ( 1 )
    {
      v7 = abs(X);
      v8 = abs(v86);
      --X;
      v92 = &terrain_76E5C8[(v7 << 7) + v8];
      if ( !v92->field_0
        || ((v9 = v6, v10 = &pVerticesSR_806210[v6], v104 = v10, !v85) ? (v11 = &pVerticesSR_801A10[v9],
                                                                          v12 = &pVerticesSR_806210[v9 + 1]) : (v11 = &pVerticesSR_806210[v9 + 1], v12 = &pVerticesSR_801A10[v9]),
            ((v103 = &pVerticesSR_801A10[v9 + 1],
              v13 = v10->vWorldPosition.x,
              v102 = v12,
              v80 = v13,
              v73 = v13 + 6.7553994e15,
              v101 = __PAIR__((unsigned int)v11, LODWORD(v73)),
              v79 = v10->vWorldPosition.y,
              v75 = v79 + 6.7553994e15,
              v100 = LODWORD(v75),
              pNormalc = (v12->vWorldPosition.x + v10->vWorldPosition.x) * 0.5,
              v71 = pNormalc + 6.7553994e15,
              v89 = LODWORD(v71),
              v14 = WorldPosToGridCellX(COERCE_UNSIGNED_INT64(pNormalc + 6.7553994e15)),
              v15 = v11->vWorldPosition.y + v10->vWorldPosition.y,
              v94 = v14,
              pNormald = v15 * 0.5,
              v16 = pNormald + 6.7553994e15,
              v77 = v16,
              v82 = LODWORD(v77),
              LODWORD(v99) = WorldPosToGridCellZ(LODWORD(v16)),
              WorldPosToGridCellX(v101),
              WorldPosToGridCellZ(v100),
              !byte_4D864C)
          || !(pEngine->uFlags & 0x80))
         && !_481EFA_clip_terrain_poly(v10, v11, v102, v103, 1)) )
        goto LABEL_105;
      if ( v10->vWorldPosition.z != v11->vWorldPosition.z
        || v11->vWorldPosition.z != v103->vWorldPosition.z
        || v103->vWorldPosition.z != v102->vWorldPosition.z )
        break;
      v17 = &array_77EC08[pODMRenderParams->uNumPolygons];
      v18 = pOutdoor->GetTileTexture(v101, v100);
      v17->uTileBitmapID = v18;
      if ( v18 != -1 )
      {
        v19 = pOutdoor->GetSomeOtherTileInfo(v101, v100);
        LOWORD(v19) = v19 | 0x8010;
        v20 = v99;
        *(int *)&v17->flags = v19;
        v21 = v92;
        v17->field_59 = 1;
        v17->terrain_grid_x = LOBYTE(v20);
        v17->field_34 = v21->distance;
        v22 = v94;
        v17->terrain_grid_z = v94;
        v23 = pTerrainNormalIndices[2 * (LODWORD(v20) + (v22 << 7)) + 1];
        if ( v23 < 0 || v23 > (signed int)(uNumTerrainNormals - 1) )
          v24 = 0;
        else
          v24 = &pTerrainNormals[v23];
        pNormal = v24;
        if ( v24 )
        {
          v25 = -(v97 * v24->z + v96 * v24->y + v95 * v24->x);
          if ( v25 < 0.0 )
            v25 = 0.0;
          v99 = v25 * 31.0;
          v76 = v99 + 6.7553994e15;
          v84 = LODWORD(v76);
          v17->dimming_level = 31 - LOBYTE(v76);
        }
        else
        {
          v17->dimming_level = 0;
        }
        if ( pODMRenderParams->uNumPolygons >= 1999 )
          return;
        ++pODMRenderParams->uNumPolygons;
        if ( !_481FC9_terrain(v10, (RenderVertexSoft *)HIDWORD(v101), v102, v17) )
          goto LABEL_104;
        v26 = 1.0 / (v104->vWorldViewPosition.x + 0.0000001);
        memcpy(VertexRenderList, v104, 0x30u);
        VertexRenderList[0]._rhw = v26;
        VertexRenderList[0].u = 0.0;
        VertexRenderList[0].v = 0.0;
        v27 = *(float *)(HIDWORD(v101) + 12) + 0.0000001;
        memcpy(&VertexRenderList[1], (void *)HIDWORD(v101), sizeof(VertexRenderList[1]));
        VertexRenderList[1]._rhw = 1.0 / v27;
        VertexRenderList[1].u = 0.0;
        VertexRenderList[1].v = 1.0;
        v28 = v103->vWorldViewPosition.x + 0.0000001;
        memcpy(&VertexRenderList[2], v103, sizeof(VertexRenderList[2]));
        VertexRenderList[2]._rhw = 1.0 / v28;
        VertexRenderList[2].u = 1.0;
        VertexRenderList[2].v = 1.0;
        v29 = v102->vWorldViewPosition.x + 0.0000001;
        memcpy(&VertexRenderList[3], v102, sizeof(VertexRenderList[3]));
        VertexRenderList[3]._rhw = 1.0 / v29;
        VertexRenderList[3].u = 1.0;
        VertexRenderList[3].v = 0.0;
        pEngine->pLightmapBuilder->StackLights_TerrainFace(pNormal, &a3a, VertexRenderList, 4u, 1);
        if ( pLightsData.uNumLightsApplied <= 0 )
        {
          v17->field_108 = 0;
        }
        else
        {
          v30 = pEngine;
          v17->field_108 = 1;
          pEngine->pLightmapBuilder->_45CA88(&Lights, VertexRenderList, 4, pNormal);
        }
        if ( v104->vWorldViewPosition.x < 8.0
          || *(float *)(HIDWORD(v101) + 12) < 8.0
          || (v31 = v102, v102->vWorldViewPosition.x < 8.0)
          || v103->vWorldViewPosition.x < 8.0 )
        {
          v36 = sr_4250FE(4u);
        }
        else
        {
          v32 = (double)pODMRenderParams->shading_dist_mist;
          if ( v32 >= v104->vWorldViewPosition.x
            && v32 >= *(float *)(HIDWORD(v101) + 12)
            && v32 >= v102->vWorldViewPosition.x
            && v32 >= v103->vWorldViewPosition.x )
          {
            v33 = sr_4254D2(4);
            v17->uNumVertices = v33;
            if ( !v33 )
              goto LABEL_104;
            v34 = (RenderVertexSoft *)HIDWORD(v101);
            v35 = v104;
            v69 = 0;
            v68 = v17;
            v67 = v31;
            goto LABEL_37;
          }
          v36 = sr_4252E8(4u);
        }
        v37 = v36;
        if ( !v36 )
          goto LABEL_104;
LABEL_41:
        ODMRenderParams::Project(v37);
LABEL_102:
        v33 = sr_4254D2(v37);
        v17->uNumVertices = v33;
        if ( !v33 )
          goto LABEL_104;
        v17->_48276F_sr();
        goto LABEL_38;
      }
LABEL_105:
      v6 = v90++ + 1;
      if ( v90 >= v81 )
        return;
    }
    v38 = &array_77EC08[pODMRenderParams->uNumPolygons];
    v39 = pOutdoor->GetTileTexture(v101, v100);
    v38->uTileBitmapID = v39;
    if ( v39 == -1 )
      goto LABEL_105;
    v40 = pOutdoor->GetSomeOtherTileInfo(v101, v100);
    BYTE1(v40) |= 0x80u;
    v41 = v99;
    *(int *)&v38->flags = v40;
    v42 = v92;
    v38->field_59 = 1;
    v38->terrain_grid_x = LOBYTE(v41);
    v38->field_34 = v42->distance;
    v43 = v94;
    v38->terrain_grid_z = v94;
    v44 = 2 * (LODWORD(v41) + (v43 << 7));
    LODWORD(v93) = v44 * 2;
    v45 = pTerrainNormalIndices[v44 + 1];
    if ( v45 < 0 || v45 > (signed int)(uNumTerrainNormals - 1) )
      v46 = 0;
    else
      v46 = &pTerrainNormals[v45];
    pNormala = v46;
    if ( v46 )
    {
      v47 = -(v97 * v46->z + v96 * v46->y + v95 * v46->x);
      if ( v47 < 0.0 )
        v47 = 0.0;
      v88 = v47 * 31.0;
      v74 = v88 + 6.7553994e15;
      v87 = LODWORD(v74);
      v38->dimming_level = 31 - LOBYTE(v74);
    }
    else
    {
      v38->dimming_level = 0;
    }
    if ( v38->dimming_level < 0 )
      v38->dimming_level = 0;
    if ( pODMRenderParams->uNumPolygons >= 1999 )
      return;
    ++pODMRenderParams->uNumPolygons;
    if ( !_481FC9_terrain((RenderVertexSoft *)HIDWORD(v101), v103, v104, v38) )
      goto LABEL_74;
    v48 = 1.0 / (v104->vWorldViewPosition.x + 0.0000001);
    memcpy(VertexRenderList, v104, 0x30u);
    VertexRenderList[0]._rhw = v48;
    VertexRenderList[0].u = 0.0;
    VertexRenderList[0].v = 0.0;
    v49 = *(float *)(HIDWORD(v101) + 12) + 0.0000001;
    memcpy(&VertexRenderList[1], (void *)HIDWORD(v101), sizeof(VertexRenderList[1]));
    VertexRenderList[1]._rhw = 1.0 / v49;
    VertexRenderList[1].u = 0.0;
    VertexRenderList[1].v = 1.0;
    v50 = v103->vWorldViewPosition.x + 0.0000001;
    memcpy(&VertexRenderList[2], v103, sizeof(VertexRenderList[2]));
    VertexRenderList[2]._rhw = 1.0 / v50;
    VertexRenderList[2].u = 1.0;
    VertexRenderList[2].v = 1.0;
    pEngine->pLightmapBuilder->StackLights_TerrainFace(pNormala, &v78, VertexRenderList, 3u, 0);
    if ( pLightsData.uNumLightsApplied <= 0 )
    {
      v38->field_108 = 0;
    }
    else
    {
      v51 = pEngine;
      v38->field_108 = 1;
      pEngine->pLightmapBuilder->_45CA88(&Lights, VertexRenderList, 3, pNormala);
    }
    if ( v104->vWorldViewPosition.x < 8.0
      || *(float *)(HIDWORD(v101) + 12) < 8.0
      || (v52 = v103, v103->vWorldViewPosition.x < 8.0) )
    {
      v55 = sr_4250FE(3u);
    }
    else
    {
      v53 = (double)pODMRenderParams->shading_dist_mist;
      if ( v53 >= v104->vWorldViewPosition.x
        && v53 >= *(float *)(HIDWORD(v101) + 12)
        && v53 >= v103->vWorldViewPosition.x )
      {
        v54 = sr_4254D2(3);
        v38->uNumVertices = v54;
        if ( v54 )
        {
          sr_sub_4829B9((RenderVertexSoft *)HIDWORD(v101), v52, v104, v38, 0);
LABEL_68:
          sr_sub_481DB2(sr_508690, v54, v38);
LABEL_75:
          v17 = &array_77EC08[pODMRenderParams->uNumPolygons];
          v57 = pOutdoor->GetTileTexture(v101, v100);
          v17->uTileBitmapID = v57;
          if ( v57 == -1 )
            goto LABEL_105;
          *(int *)&v17->flags = pOutdoor->GetSomeOtherTileInfo(v101, v100);
          v58 = v92;
          v17->field_59 = 1;
          v17->field_34 = v58->distance;
          v17->terrain_grid_z = v94;
          v17->terrain_grid_x = LOBYTE(v99);
          v59 = *(unsigned __int16 *)((char *)pTerrainNormalIndices.data() + LODWORD(v93));
          if ( v59 > (signed int)(uNumTerrainNormals - 1) )
          {
            pNormalb = 0;
            v60 = 0;
          }
          else
          {
            v60 = &pTerrainNormals[v59];
            pNormalb = v60;
          }
          if ( v60 )
          {
            v61 = -(v97 * v60->z + v96 * v60->y + v95 * v60->x);
            if ( v61 < 0.0 )
              v61 = 0.0;
            v93 = v61 * 31.0;
            v72 = v93 + 6.7553994e15;
            v83 = LODWORD(v72);
            v17->dimming_level = 31 - LOBYTE(v72);
          }
          else
          {
            v17->dimming_level = 0;
          }
          if ( v17->dimming_level < 0 )
            v17->dimming_level = 0;
          if ( pODMRenderParams->uNumPolygons >= 1999 )
            return;
          ++pODMRenderParams->uNumPolygons;
          if ( !_481FC9_terrain(v102, v104, v103, v17) )
          {
LABEL_104:
            --pODMRenderParams->uNumPolygons;
            goto LABEL_105;
          }
          v62 = 1.0 / (v104->vWorldViewPosition.x + 0.0000001);
          memcpy(VertexRenderList, v104, 0x30u);
          VertexRenderList[0]._rhw = v62;
          VertexRenderList[0].u = 0.0;
          VertexRenderList[0].v = 0.0;
          v63 = v103->vWorldViewPosition.x + 0.0000001;
          memcpy(&VertexRenderList[1], v103, sizeof(VertexRenderList[1]));
          VertexRenderList[1]._rhw = 1.0 / v63;
          VertexRenderList[1].u = 1.0;
          VertexRenderList[1].v = 1.0;
          v64 = v102->vWorldViewPosition.x + 0.0000001;
          memcpy(&VertexRenderList[2], v102, sizeof(VertexRenderList[2]));
          VertexRenderList[2]._rhw = 1.0 / v64;
          VertexRenderList[2].u = 1.0;
          VertexRenderList[2].v = 0.0;
          pEngine->pLightmapBuilder->StackLights_TerrainFace(pNormalb, &v70, VertexRenderList, 3u, 1);
          if ( pLightsData.uNumLightsApplied <= 0 )
          {
            v17->field_108 = 0;
          }
          else
          {
            v65 = pEngine;
            v17->field_108 = 1;
            pEngine->pLightmapBuilder->_45CA88(&Lights, VertexRenderList, 3, pNormalb);
          }
          if ( v104->vWorldViewPosition.x < 8.0 || v103->vWorldViewPosition.x < 8.0 || v102->vWorldViewPosition.x < 8.0 )
          {
            v37 = sr_4250FE(3u);
            ODMRenderParams::Project(v37);
            if ( !v37 )
              goto LABEL_104;
            goto LABEL_102;
          }
          v66 = (double)pODMRenderParams->shading_dist_mist;
          if ( v66 < v104->vWorldViewPosition.x || v66 < v103->vWorldViewPosition.x || v66 < v102->vWorldViewPosition.x )
          {
            v37 = sr_4252E8(3u);
            if ( !v37 )
              goto LABEL_105;
            goto LABEL_41;
          }
          v33 = sr_4254D2(3);
          v17->uNumVertices = v33;
          if ( !v33 )
            goto LABEL_104;
          v34 = v104;
          v35 = v102;
          v69 = 0;
          v68 = v17;
          v67 = v103;
LABEL_37:
          sr_sub_4829B9(v35, v34, v67, v68, v69);
LABEL_38:
          sr_sub_481DB2(sr_508690, v33, v17);
          goto LABEL_105;
        }
        goto LABEL_74;
      }
      v55 = sr_4252E8(3u);
    }
    v56 = v55;
    if ( v55 )
    {
      ODMRenderParams::Project(v55);
      v54 = sr_4254D2(v56);
      v38->uNumVertices = v54;
      if ( v54 )
      {
        v38->_48276F_sr();
        goto LABEL_68;
      }
    }
LABEL_74:
    --pODMRenderParams->uNumPolygons;
    goto LABEL_75;
  }
}

//----- (00481FC9) --------------------------------------------------------
int _481FC9_terrain(RenderVertexSoft *a1, RenderVertexSoft *a2, RenderVertexSoft *a3, struct Polygon *a4)//Rotate camera
{
  float arg_0, arg_4, var_4, var_8, var_c, var_10, var_14;

  if (a1->vWorldPosition.y == a2->vWorldPosition.y && a2->vWorldPosition.y == a3->vWorldPosition.y)
    a4->flags |= 0x10;
  var_c = a1->vWorldViewPosition.x - a2->vWorldViewPosition.x;
  var_14 = a1->vWorldViewPosition.y - a2->vWorldViewPosition.y;
  arg_0 = a1->vWorldViewPosition.z - a2->vWorldViewPosition.z;
  var_10 = a3->vWorldViewPosition.x - a2->vWorldViewPosition.x;
  var_8 = a3->vWorldViewPosition.y - a2->vWorldViewPosition.y;
  var_4 = a3->vWorldViewPosition.z - a2->vWorldViewPosition.z;
  arg_4 = a3->vWorldPosition.y - a2->vWorldPosition.y;
  if (((a3->vWorldPosition.y - a2->vWorldPosition.y) * (a1->vWorldPosition.x - a2->vWorldPosition.x) - (a3->vWorldPosition.x - a2->vWorldPosition.x) * (a1->vWorldPosition.y - a2->vWorldPosition.y)) * (pIndoorCamera->pos.y - a2->vWorldPosition.z) +
      ((a3->vWorldPosition.z - a2->vWorldPosition.z) * (a1->vWorldPosition.y - a2->vWorldPosition.y) - (a3->vWorldPosition.y - a2->vWorldPosition.y) * (a1->vWorldPosition.z - a2->vWorldPosition.z)) * (pIndoorCamera->pos.x - a2->vWorldPosition.x) +
      ((a3->vWorldPosition.x - a2->vWorldPosition.x) * (a1->vWorldPosition.z - a2->vWorldPosition.z) - (a3->vWorldPosition.z - a2->vWorldPosition.z) * (a1->vWorldPosition.x - a2->vWorldPosition.x)) * (pIndoorCamera->pos.z - a2->vWorldPosition.y) < 0)
  {
   arg_4 = var_4 * var_14 - var_8 * arg_0;
   a4->v_18.x = floorf(arg_4 + 0.5f);
   arg_4 = var_10 * arg_0 - var_4 * var_c;
   a4->v_18.y = floorf(arg_4 + 0.5f);
   arg_4 = var_8 * var_c - var_10 * var_14;
   a4->v_18.z = floorf(arg_4 + 0.5f);
   a4->_normalize_v_18();
   arg_4 = -(a4->v_18.x * a2->vWorldViewPosition.x) - a4->v_18.y * a2->vWorldViewPosition.y - a4->v_18.z * a2->vWorldViewPosition.z;
   a4->field_24 = floorf(arg_4 + 0.5f);
   return true;
  }
  else
    return false;
}
//----- (00438250) --------------------------------------------------------
void IndoorCameraD3D::sr_Reset_list_0037C()
{
  this->list_0037C_size = 0;
}
//----- (00438240) --------------------------------------------------------
void IndoorCameraD3D::sr_438240_draw_lits()
{
  IndoorCameraD3D *v1; // ebp@1
  int v2; // edi@2
  int v3; // ebx@3
  int v4; // esi@4
  double v5; // st7@6
  float v6; // ST00_4@8
  IndoorCameraD3D *v7; // esi@1
  float v8; // [sp+4h] [bp-28h]@6
  float v9; // [sp+8h] [bp-24h]@6
  unsigned int v10; // [sp+Ch] [bp-20h]@6
  char v11; // [sp+10h] [bp-1Ch]@6
  float v12; // [sp+14h] [bp-18h]@6
  int v13; // [sp+28h] [bp-4h]@1

  v7 = this;
  sr_438141_draw_list_0037C();
  v13 = 0;
  v1 = v7;
  if ( v7->list_E0380_size > 0 )
  {
    v2 = (int)&v7->list_E0380[0].field_C04;
    do
    {
      v3 = 0;
      if ( *(int *)v2 > 0 )
      {
        v4 = v2 - 3048;
        do
        {
          if ( *(char *)(v2 - 3076) & 1 )
          {
            LODWORD(v12) = 1;
            v11 = 1;
            v10 = *(int *)(v2 + 4);
            v9 = 0.0;
            v8 = *(float *)(v4 + 4);
            v5 = *(float *)v4;
          }
          else
          {
            LODWORD(v12) = 0;
            v11 = 1;
            v10 = *(int *)(v2 + 4);
            v9 = *(float *)(v4 - 16);
            v8 = *(float *)(v4 - 20);
            v5 = *(float *)(v4 - 24);
          }
          v6 = v5;
          sr_437D4A_draw_some_vertices(v6, v8, v9, v10, v11, v12);
          ++v3;
          v4 += 48;
        }
        while ( v3 < *(int *)v2 );
      }
      ++v13;
      v2 += 3088;
    }
    while ( v13 < v1->list_E0380_size );
  }
}

//----- (00437D4A) --------------------------------------------------------
void IndoorCameraD3D::sr_437D4A_draw_some_vertices(float x, float y, float z, unsigned int a5, char a6, float a7)
{
  IndoorCameraD3D *v7; // edx@1
  char *v8; // eax@1
  signed int v9; // ecx@1
  signed int v10; // edx@10
  char *v11; // ecx@10
  signed int uNumD3DVertices; // edi@16
  char *v13; // ecx@24
  unsigned int v14; // edx@25
  unsigned int v15; // eax@25
  int v16; // ebx@25
  int v17; // eax@26
  int v18; // edx@26
  IDirect3DDevice3 *v19; // ST24_4@27
  HRESULT v20; // eax@27
  RenderVertexSoft vert[20]; // [sp+0h] [bp-680h]@1
  RenderVertexD3D3 pD3DVertices[5]; // [sp+3C0h] [bp-2C0h]@15
  RenderVertexSoft a1; // [sp+640h] [bp-40h]@1
  //double v24; // [sp+670h] [bp-10h]@25
  //double v25; // [sp+678h] [bp-8h]@1

  v7 = this;
  a1.flt_2C = 0.0;
  //HIDWORD(v25) = (int)this;
  v8 = (char *)&vert[0].flt_2C;
  v9 = 20;
  do
  {
    *(float *)v8 = 0.0;
    v8 += 48;
    --v9;
  }
  while ( v9 );
  if ( LOBYTE(a7) )
  {
    vert[0].vWorldViewProjX = x;
    vert[0].vWorldViewProjY = y;
    goto LABEL_15;
  }
  a1.vWorldPosition.x = x;
  a1.vWorldPosition.y = y;
  a1.vWorldPosition.z = z;
  if ( render->pRenderD3D )
  {
    LODWORD(a7) = 1;
    if ( v7->CalcPortalShape(
           &a1,
           (unsigned int *)&a7,
           vert,
           v7->std__vector_000034_prolly_frustrum,
           4,
           1,
           0) == 1
      && SLODWORD(a7) < 1 )
      return;
  }
  else
  {
    vert[0].vWorldPosition.x = x;
    vert[0].vWorldPosition.y = y;
    vert[0].vWorldPosition.z = z;
  }
  ViewTransform(vert, 1u);
  v10 = 0;
  v11 = (char *)&vert[0].vWorldViewPosition;
  do
  {
    if ( *(float *)v11 >= 8.0 )
      break;
    ++v10;
    v11 += 48;
  }
  while ( v10 < 1 );
  if ( v10 < 1 )
  {
    Project(vert, 1u, 0);
LABEL_15:
    pD3DVertices[0].pos.x = vert[0].vWorldViewProjX;
    pD3DVertices[0].pos.y = vert[0].vWorldViewProjY;
    pD3DVertices[0].specular = 0;
    pD3DVertices[0].diffuse = a5;
    if ( a6 )
    {
      pD3DVertices[0].pos.z = 0.000099999997;
      pD3DVertices[1].pos.x = vert[0].vWorldViewProjX;
      pD3DVertices[2].pos.x = vert[0].vWorldViewProjX;
      pD3DVertices[0].rhw = 0.001;
      uNumD3DVertices = 5;
      pD3DVertices[0].texcoord.x = 0.0;
      pD3DVertices[1].diffuse = a5;
      pD3DVertices[1].specular = 0;
      pD3DVertices[0].texcoord.y = 0.0;
      pD3DVertices[2].diffuse = a5;
      pD3DVertices[2].specular = 0;
      pD3DVertices[3].pos.y = vert[0].vWorldViewProjY;
      pD3DVertices[3].diffuse = a5;
      pD3DVertices[3].specular = 0;
      pD3DVertices[4].pos.y = vert[0].vWorldViewProjY;
      pD3DVertices[1].pos.y = vert[0].vWorldViewProjY - 1.0;
      pD3DVertices[4].diffuse = a5;
      pD3DVertices[4].specular = 0;
      pD3DVertices[1].pos.z = 0.000099999997;
      pD3DVertices[1].rhw = 0.001;
      pD3DVertices[1].texcoord.x = 0.0;
      pD3DVertices[1].texcoord.y = 0.0;
      pD3DVertices[2].pos.y = vert[0].vWorldViewProjY + 1.0;
      pD3DVertices[2].pos.z = 0.000099999997;
      pD3DVertices[2].rhw = 0.001;
      pD3DVertices[2].texcoord.x = 0.0;
      pD3DVertices[2].texcoord.y = 0.0;
      pD3DVertices[3].pos.x = vert[0].vWorldViewProjX - 1.0;
      pD3DVertices[3].pos.z = 0.000099999997;
      pD3DVertices[3].rhw = 0.001;
      pD3DVertices[3].texcoord.x = 0.0;
      pD3DVertices[3].texcoord.y = 0.0;
      pD3DVertices[4].pos.x = vert[0].vWorldViewProjX + 1.0;
      pD3DVertices[4].pos.z = 0.000099999997;
      pD3DVertices[4].rhw = 0.001;
      pD3DVertices[4].texcoord.x = 0.0;
      pD3DVertices[4].texcoord.y = 0.0;
    }
    else
    {
      uNumD3DVertices = 1;
      __debugbreak(); // make things right
      *(unsigned int *)(&pD3DVertices[0].pos.z) = 0x38D1B717u;
      pD3DVertices[0].texcoord.x = 0.0;
      *(unsigned int *)(&pD3DVertices[0].rhw) = 0x3A83126Fu;
      pD3DVertices[0].texcoord.y = 0.0;
    }
    if ( render->pRenderD3D )
    {
      __debugbreak(); // decompilation bug
      //v19 = render->pRenderD3D->pDevice;
      ErrD3D(render->pRenderD3D->pDevice->SetTexture(0, nullptr));
      ErrD3D(render->pRenderD3D->pDevice->DrawPrimitive(D3DPT_POINTLIST,
              D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
              pD3DVertices,
              uNumD3DVertices,
              16));
    }
    else
    {
      if ( (double)(signed int)pViewport->uViewportTL_X <= vert[0].vWorldViewProjX - 1.0
        && (double)(signed int)pViewport->uViewportBR_X > vert[0].vWorldViewProjX + 2.0
        && (double)(signed int)pViewport->uViewportTL_Y <= vert[0].vWorldViewProjY - 1.0
        && (double)(signed int)pViewport->uViewportBR_Y > vert[0].vWorldViewProjY + 2.0
        && uNumD3DVertices > 0 )
      {
        v13 = (char *)&pD3DVertices[0].diffuse;
        do
        {
          LODWORD(a7) = *((int *)v13 - 3);
          //v25 = a7 + 6.7553994e15;
          auto _v25 = floorf(a7 + 0.5f);

          //v24 = *((float *)v13 - 4) + 6.7553994e15;
          auto _v24 = floorf(*((float *)v13 - 4) + 0.5f);

          v14 = *(int *)v13;
          v15 = *(int *)v13;
          v16 = (*(int *)v13 >> 3) & 0x1F;
          if ( render->uTargetGBits == 5 )
          {
            v17 = (v15 >> 6) & 0x3E0;
            v18 = (v14 >> 9) & 0x7C00;
          }
          else
          {
            v17 = (v15 >> 5) & 0x7E0;
            v18 = (v14 >> 8) & 0xF800;
          }
          v13 += 32;
          --uNumD3DVertices;
          render->pTargetSurface[(uint)_v24 + render->uTargetSurfacePitch * (uint)_v25] = v18 | v16 | (unsigned __int16)v17;
        }
        while ( uNumD3DVertices );
      }
    }
  }
}

//----- (0047BEB1) --------------------------------------------------------
int sr_sub_47BEB1(signed int a1, Polygon *a2, int terrain_gamma, int a4, int *a5, int *a6, int a7, int a8)
{
  Polygon *v8; // ebx@1
  signed int v9; // edx@1
  int v10; // eax@5
  signed int v11; // eax@9
  signed int v12; // eax@15
  signed __int64 v13; // qtt@21
  int *v14; // ecx@30
  int *v15; // edi@30
  int v16; // ebx@32
  signed __int64 v17; // qtt@37
  double v18; // ST10_8@37
  double v19; // ST10_8@38
  int v20; // edi@39
  int result; // eax@46
  int *v22; // eax@48
  signed int v23; // edx@51
  signed __int64 v24; // qtt@51
  int v25; // ecx@51
  signed int v26; // [sp+14h] [bp-4h]@1
  float v27; // [sp+28h] [bp+10h]@37
  float v28; // [sp+28h] [bp+10h]@38
  int v29; // [sp+2Ch] [bp+14h]@37

  v8 = a2;
  v9 = a1;
  v26 = a1;
  if ( pParty->armageddon_timer )
  {
    *a5 = -1;
    *a6 = -1;
    *(char *)a7 = 1;
LABEL_46:
    result = a8;
    *(char *)a8 = 0;
    return result;
  }
  if ( a1 < 0 )
  {
    v26 = v8->field_34 << 16;
    v9 = v8->field_34 << 16;
  }
  v10 = pWeather->bNight;
  if ( bUnderwater == 1 )
    v10 = 0;
  if ( !v10 )
  {
    if ( !(day_attrib & DAY_ATTRIB_FOG) && !bUnderwater )
    {
      v14 = a5;
      v15 = a6;
      if ( !v9 )
      {
        *a5 = 31;
        *a6 = -1;
      }
      v16 = v8->dimming_level - terrain_gamma;
      if ( v16 >= 0 )
      {
        if ( v16 > 27 )
          v16 = 27;
      }
      else
      {
        v16 = 0;
      }
      *a6 = 27;
      if ( a4 )
      {
        v28 = pOutdoor->fFogDensity * 27.0;
        v19 = v28 + 6.7553994e15;
        v29 = LODWORD(v19);
      }
      else
      {
        LODWORD(v17) = v9 << 16;
        HIDWORD(v17) = v9 >> 16;
        v27 = (double)(signed int)(((unsigned __int64)(v17 / (pODMRenderParams->shading_dist_shade << 16) * (31 - *a6)) >> 16)
                                 + *a6)
            * pOutdoor->fFogDensity;
        v18 = v27 + 6.7553994e15;
        v29 = LODWORD(v18);
      }
      *v14 = v16 + v29;
      v20 = *v15;
      if ( v16 + v29 > v20 )
        *v14 = v20;
      if ( *v14 < v16 )
        *v14 = v16;
      if ( *v14 > pOutdoor->max_terrain_dimming_level )
        *v14 = pOutdoor->max_terrain_dimming_level;
      goto LABEL_45;
    }
    if ( v9 >= day_fogrange_1 << 16 )
    {
      if ( v9 <= day_fogrange_2 << 16 )
      {
        v23 = v9 - (day_fogrange_1 << 16);
        LODWORD(v24) = v23 << 16;
        HIDWORD(v24) = v23 >> 16;
        v25 = (unsigned __int64)(27 * v24 / ((day_fogrange_2 - day_fogrange_1) << 16)) >> 16;
        v22 = a5;
        *a5 = v25;
        if ( v25 > 27 )
          goto LABEL_54;
        v9 = v26;
      }
      else
      {
        v22 = a5;
        *a5 = 27;
      }
    }
    else
    {
      v22 = a5;
      *a5 = 0;
    }
    if ( v9 )
    {
LABEL_55:
      if ( a4 )
        *v22 = 31;
      *a6 = 31;
      *(char *)a7 = 0;
      goto LABEL_59;
    }
LABEL_54:
    *v22 = 27;
    goto LABEL_55;
  }
  if ( v10 == 1 )
  {
    v11 = 1;
    if ( byte_4D864C && BYTE1(pEngine->uFlags) & 0x10 )
    {
      v12 = 0;
    }
    else
    {
      if ( (signed __int64)pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uExpireTime > 0 )
      {
        if ( (signed __int64)pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uExpireTime <= 0 )
          v11 = 0;
        else
          v11 = pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;
      }
      v12 = v11 << 26;
    }
    if ( a4 )
      goto LABEL_24;
    if ( v9 <= v12 )
    {
      if ( v9 > 0 )
      {
        LODWORD(v13) = v9 << 16;
        HIDWORD(v13) = v9 >> 16;
        v9 = v26;
        *a5 = (unsigned __int64)(27 * v13 / v12) >> 16;
      }
      if ( *a5 > 27 )
        goto LABEL_24;
    }
    else
    {
      *a5 = 27;
    }
    if ( v9 )
    {
LABEL_25:
      if ( v8->field_32 & 4 )
        *a5 = 27;
      *a6 = 27;
LABEL_45:
      *(char *)a7 = 0;
      goto LABEL_46;
    }
LABEL_24:
    *a5 = 27;
    goto LABEL_25;
  }
  *a5 = -1;
  *a6 = -1;
  *(char *)a7 = 1;
LABEL_59:
  result = a8;
  *(char *)a8 = 1;
  return result;
}
//----- (0047C24C) --------------------------------------------------------
unsigned __int16 *sr_sub_47C24C_get_palette(BLVFace *a1, int a2, int a3, char a4)
{
  int v4; // esi@1
  int v5; // eax@2
  int a3a; // [sp+4h] [bp-4h]@1

  v4 = a2;
  a3a = 1;
  if ( a4 )
  {
    v5 = pEngine->_44ED0A(a1, &a3a, 31);
    if ( v5 != -1 )
      a3 = v5;
  }
  return PaletteManager::Get_Dark_or_Red_LUT(v4, a3, a3a);
}
//----- (00438141) --------------------------------------------------------
void IndoorCameraD3D::sr_438141_draw_list_0037C()
{
  IndoorCameraD3D *v1; // edi@1
  signed int v2; // ebx@1
  char *v3; // esi@2
  double v4; // st7@4
  float v5; // ST00_4@6
  float v6; // [sp+4h] [bp-20h]@4
  float v7; // [sp+8h] [bp-1Ch]@4
  unsigned int v8; // [sp+Ch] [bp-18h]@4
  char v9; // [sp+10h] [bp-14h]@4
  float v10; // [sp+14h] [bp-10h]@4

  v1 = this;
  v2 = 0;
  if ( (signed int)this->list_0037C_size > 0 )
  {
    v3 = (char *)&this->list_0037C[0].field_20;
    do
    {
      if ( *(v3 - 32) & 1 )
      {
        LODWORD(v10) = 1;
        v9 = 1;
        v8 = *((int *)v3 + 5);
        v7 = 0.0;
        v6 = *(float *)v3;
        v4 = *((float *)v3 - 1);
      }
      else
      {
        LODWORD(v10) = 0;
        v9 = 1;
        v8 = *((int *)v3 + 5);
        v7 = *((float *)v3 - 5);
        v6 = *((float *)v3 - 6);
        v4 = *((float *)v3 - 7);
      }
      v5 = v4;
      sr_437D4A_draw_some_vertices(v5, v6, v7, v8, v9, v10);
      ++v2;
      v3 += 56;
    }
    while ( v2 < (signed int)v1->list_0037C_size );
  }
}
//----- (00485BAE) --------------------------------------------------------
stru315 *sr_sub_485BAE(stru315 *a1, stru316 *a2)
{
  stru315 *result; // eax@1
  int i; // ecx@1
  int v4; // ecx@2
  int v5; // ecx@4
  int v6; // ecx@6
  int v7; // ecx@8
  int v8; // esi@10
  int v9; // ecx@10
  int v10; // esi@11
  int v11; // edi@11
  int v12; // ebx@11
  int v13; // ecx@16
  unsigned __int16 *v14; // esi@18
  unsigned int *v15; // ecx@20
  int v16; // esi@20
  int v17; // [sp+0h] [bp-4h]@10

  result = a1;
  for ( i = a1->field_28; i; i = result->field_28 )
  {
    result->field_28 = i - 1;
    v4 = result->field_18;
    if ( result->field_30 > v4 )
      result->field_30 = v4;
    v5 = result->field_20;
    if ( result->field_2C > v5 )
      result->field_2C = v5;
    v6 = result->field_14;
    if ( result->field_30 < v6 )
      result->field_30 = v6;
    v7 = result->field_1C;
    if ( result->field_2C < v7 )
      result->field_2C = v7;
    v8 = (result->field_C & HIWORD(result->field_30)) + ((result->field_2C & result->field_8) >> result->field_10);
    v9 = *((char *)result->pTextureLOD + v8);
    v17 = *((char *)result->pTextureLOD + v8);
    if ( a2->field_20 )
    {
      v10 = HIWORD(a2->field_14);
      v11 = a2->field_C;
      v12 = v10;
      if ( v10 >= v11 )
        v12 = a2->field_C;
      if ( a2->field_8 - v12 <= 0 )
      {
        v13 = 0;
      }
      else
      {
        if ( v10 >= v11 )
          v10 = a2->field_C;
        v13 = a2->field_8 - v10;
      }
      v14 = a2->field_24_palette;
      v9 = v17 + (v13 << 8);
    }
    else
    {
      v14 = result->field_34_palette;
    }
    *result->pColorBuffer = v14[v9];
    v15 = result->pDepthBuffer;
    v16 = result->field_24;
    --result->pColorBuffer;
    *v15 = v16;
    --result->pDepthBuffer;
    a2->field_14 -= a2->field_18;
    result->field_30 += result->field_4;
    result->field_2C += result->field_0;
  }
  --result->field_28;
  return result;
}

//----- (00485C89) --------------------------------------------------------
stru315 *sr_sub_485C89(stru315 *a1, stru316 *a2)
{
  stru315 *result; // eax@1
  int i; // ecx@1
  int v4; // esi@2
  int v5; // esi@2
  int v6; // ecx@2
  int v7; // esi@4
  int v8; // edi@4
  int v9; // ecx@9
  unsigned __int16 *v10; // esi@11
  unsigned int *v11; // ecx@14
  int v12; // esi@14
  int v13; // [sp+0h] [bp-4h]@2

  result = a1;
  for ( i = a1->field_28; i; i = result->field_28 )
  {
    v4 = result->field_8 & result->field_2C;
    result->field_28 = i - 1;
    v5 = (result->field_C & HIWORD(result->field_30)) + (v4 >> result->field_10);
    v6 = *((char *)result->pTextureLOD + v5);
    v13 = *((char *)result->pTextureLOD + v5);
    if ( *((char *)result->pTextureLOD + v5) )
    {
      if ( a2->field_20 )
      {
        v7 = HIWORD(a2->field_10);
        v8 = v7;
        if ( v7 >= a2->field_C )
          v8 = a2->field_C;
        if ( a2->field_8 - v8 <= 0 )
        {
          v9 = 0;
        }
        else
        {
          if ( v7 >= a2->field_C )
            v7 = a2->field_C;
          v9 = a2->field_8 - v7;
        }
        v10 = a2->field_24_palette;
        v6 = v13 + (v9 << 8);
      }
      else
      {
        v10 = result->field_34_palette;
      }
      *result->pColorBuffer = v10[v6];
    }
    v11 = result->pDepthBuffer;
    v12 = result->field_24;
    ++result->pColorBuffer;
    *v11 = v12;
    ++result->pDepthBuffer;
    a2->field_10 += a2->field_18;
    result->field_30 += result->field_4;
    result->field_2C += result->field_0;
  }
  --result->field_28;
  return result;
}

//----- (00485D3E) --------------------------------------------------------
stru315 *sr_sub_485D3E(stru315 *a1, stru316 *a2)
{
  stru315 *result; // eax@1
  int i; // ecx@1
  int v4; // ecx@2
  int v5; // ecx@4
  int v6; // ecx@6
  int v7; // ecx@8
  int v8; // esi@10
  int v9; // ecx@10
  int v10; // esi@12
  int v11; // edi@12
  int v12; // ecx@17
  unsigned __int16 *v13; // esi@19
  unsigned int *v14; // ecx@22
  int v15; // esi@22
  int v16; // [sp+0h] [bp-4h]@10

  result = a1;
  for ( i = a1->field_28; i; i = result->field_28 )
  {
    result->field_28 = i - 1;
    v4 = result->field_18;
    if ( result->field_30 > v4 )
      result->field_30 = v4;
    v5 = result->field_20;
    if ( result->field_2C > v5 )
      result->field_2C = v5;
    v6 = result->field_14;
    if ( result->field_30 < v6 )
      result->field_30 = v6;
    v7 = result->field_1C;
    if ( result->field_2C < v7 )
      result->field_2C = v7;
    v8 = (result->field_C & HIWORD(result->field_30)) + ((result->field_2C & result->field_8) >> result->field_10);
    v9 = *((char *)result->pTextureLOD + v8);
    v16 = *((char *)result->pTextureLOD + v8);
    if ( *((char *)result->pTextureLOD + v8) )
    {
      if ( a2->field_20 )
      {
        v10 = HIWORD(a2->field_10);
        v11 = v10;
        if ( v10 >= a2->field_C )
          v11 = a2->field_C;
        if ( a2->field_8 - v11 <= 0 )
        {
          v12 = 0;
        }
        else
        {
          if ( v10 >= a2->field_C )
            v10 = a2->field_C;
          v12 = a2->field_8 - v10;
        }
        v13 = a2->field_24_palette;
        v9 = v16 + (v12 << 8);
      }
      else
      {
        v13 = result->field_34_palette;
      }
      *result->pColorBuffer = v13[v9];
    }
    v14 = result->pDepthBuffer;
    v15 = result->field_24;
    ++result->pColorBuffer;
    *v14 = v15;
    ++result->pDepthBuffer;
    a2->field_10 += a2->field_18;
    result->field_30 += result->field_4;
    result->field_2C += result->field_0;
  }
  --result->field_28;
  return result;
}

//----- (0047C178) --------------------------------------------------------
void *sr_sub_47C178(signed int a1, Polygon *a2, int terrain_gamma, int a4)
{
  Polygon *v4; // esi@1
  void *result; // eax@2
  int v6; // [sp+4h] [bp-8h]@3
  int v7; // [sp+8h] [bp-4h]@3

  v4 = a2;
  if ( pParty->armageddon_timer )
  {
    result = PaletteManager::Get(a2->pTexture->palette_id2);
  }
  else
  {
    sr_sub_47BEB1(a1, a2, terrain_gamma, a4, &v6, &v7, (int)((char *)&terrain_gamma + 3), (int)((char *)&a4 + 3));
    result = sr_sub_47C1CA(v4, SBYTE3(a4), v6, v7);
  }
  return result;
}

//----- (0047C1CA) --------------------------------------------------------
void *sr_sub_47C1CA(Polygon *a1, char a2, int a3, signed int a4)
{
  Polygon *v4; // esi@1
  void *result; // eax@2
  int v6; // edx@3
  int v7; // ecx@8
  int a3a; // [sp+4h] [bp-8h]@1
  char v9; // [sp+8h] [bp-4h]@1

  v9 = a2;
  v4 = a1;
  a3a = 1;
  if ( pParty->armageddon_timer )
  {
    result = PaletteManager::Get(a1->pTexture->palette_id2);
  }
  else
  {
    v6 = pEngine->_44EC23(a1, &a3a, a4);
    if ( v6 == -1 )
      v6 = a3;
    if ( v9 == 1 )
    {
      if ( v6 != -1 || a4 != -1 )
      {
        v7 = v4->pTexture->palette_id2;
      }
      else
      {
        v6 = 0;
        v7 = 0;
      }
      result = PaletteManager::Get_Mist_or_Red_LUT(v7, v6, a3a);
    }
    else
    {
      result = PaletteManager::Get_Dark_or_Red_LUT(v4->pTexture->palette_id2, v6, a3a);
    }
  }
  return result;
}

//----- (0047C28C) --------------------------------------------------------
char *sr_sub_47C28C_get_palette(Polygon *a1, char a2, signed int a3, signed int a4)
{
  Polygon *v4; // esi@1
  char *result; // eax@2
  signed int v6; // eax@3
  int v7; // ecx@8
  int a2a; // [sp+4h] [bp-8h]@1
  char v9; // [sp+8h] [bp-4h]@1

  v9 = a2;
  v4 = a1;
  a2a = 1;
  if ( pParty->armageddon_timer )
  {
    result = (char *)pPaletteManager->field_199600_palettes[a1->pTexture->palette_id2];
  }
  else
  {
    v6 = pEngine->_44EC23(a1, &a2a, a4);
    if ( v6 != -1 )
      a3 = v6;
    if ( v9 == 1 )
    {
      if ( a3 != -1 || a4 != -1 )
        v7 = v4->pTexture->palette_id2;
      else
        v7 = 0;
      result = (char *)PaletteManager::_47C30E_get_palette(v7, a2a);
    }
    else
    {
      result = (char *)PaletteManager::_47C33F_get_palette(v4->pTexture->palette_id2, a2a);
    }
  }
  return result;
}

//----- (004AFF79) --------------------------------------------------------
void IndoorLocation::ExecDraw_sw(unsigned int uFaceID)
{
  unsigned int v1; // ebx@1
  BLVFace *v2; // esi@3
  unsigned int v3; // eax@3
  Texture_MM7 *v4; // eax@8
  Texture_MM7 *v5; // edi@8
  int v6; // eax@9
  int v7; // eax@9
  int v8; // ecx@17
  int v9; // ebx@17
  int v10; // eax@17
  int v11; // esi@17
  unsigned int v12; // eax@17
  int j; // ecx@19
  int v14; // edx@20
  int v15; // eax@20
  int v16; // edx@20
  int i; // ebx@22
  int v18; // ecx@23
  int v19; // eax@23
  int v20; // eax@23
  int v21; // eax@24
  unsigned __int8 *v22; // ecx@24
  int v23; // ebx@24
  int v24; // esi@25
  int v25; // eax@28
  unsigned __int16 *v26; // eax@28
  unsigned int v27; // eax@29
  int v28; // eax@30
  char *v29; // esi@31
  int v30; // eax@33
  int v31; // eax@33
  int v32; // eax@35
  int v33; // edx@35
  signed int v34; // ebx@35
  int v35; // eax@35
  int v36; // ebx@35
  signed int v37; // ebx@35
  signed int v38; // edi@35
  unsigned int v39; // edi@36
  int v40; // edx@40
  int v41; // ecx@40
  signed int v42; // edx@40
  int v43; // edx@42
  int v44; // eax@42
  unsigned __int16 *v45; // eax@43
  int *v46; // esi@44
  unsigned __int16 *v47; // edi@44
  unsigned int v48; // edx@44
  int v49; // ebx@44
  char v50; // cl@44
  char v51; // ch@44
  unsigned int v52; // ebx@46
  int v53; // edx@46
  unsigned int v54; // ebx@46
  int v55; // edx@46
  unsigned int v56; // ebx@47
  int v57; // edx@47
  int v58; // ebx@47
  int v59; // edx@47
  unsigned __int16 *v60; // eax@50
  int *v61; // esi@51
  unsigned __int16 *v62; // edi@51
  unsigned int v63; // edx@51
  int v64; // ebx@51
  char v65; // cl@51
  char v66; // ch@51
  unsigned int v67; // ebx@53
  int v68; // edx@53
  unsigned int v69; // ebx@53
  int v70; // edx@53
  unsigned int v71; // ebx@54
  int v72; // edx@54
  int v73; // ebx@54
  int v74; // edx@54
  unsigned __int16 *v75; // eax@58
  int *v76; // esi@59
  int v77; // edi@59
  unsigned int v78; // edx@59
  int v79; // ebx@59
  char v80; // cl@59
  char v81; // ch@59
  int v82; // ebx@61
  int v83; // edx@61
  unsigned int v84; // ebx@62
  int v85; // edx@62
  unsigned __int16 *v86; // eax@65
  int *v87; // esi@66
  int v88; // edi@66
  unsigned int v89; // edx@66
  int v90; // ebx@66
  char v91; // cl@66
  char v92; // ch@66
  int v93; // ebx@68
  int v94; // edx@68
  unsigned __int16 v95; // bx@69
  int v96; // edx@69
  unsigned __int8 *v97; // [sp+Ch] [bp-9Ch]@24
  unsigned __int8 *v98; // [sp+10h] [bp-98h]@24
  unsigned __int8 *v99; // [sp+14h] [bp-94h]@24
  unsigned __int8 *v100; // [sp+18h] [bp-90h]@24
  int v101; // [sp+1Ch] [bp-8Ch]@40
  int v102; // [sp+20h] [bp-88h]@31
  BLVFace *v103; // [sp+24h] [bp-84h]@3
  unsigned __int16 *v104; // [sp+28h] [bp-80h]@24
  int v105; // [sp+2Ch] [bp-7Ch]@30
  int v106; // [sp+30h] [bp-78h]@24
  int v107; // [sp+34h] [bp-74h]@9
  Texture_MM7 *v108; // [sp+38h] [bp-70h]@8
  int v109; // [sp+3Ch] [bp-6Ch]@9
  unsigned int v110; // [sp+40h] [bp-68h]@24
  unsigned int v111; // [sp+44h] [bp-64h]@1
  int *k; // [sp+48h] [bp-60h]@31
  int v113; // [sp+4Ch] [bp-5Ch]@35
  int v114; // [sp+50h] [bp-58h]@35
  int v115; // [sp+54h] [bp-54h]@42
  unsigned __int8 *v116; // [sp+58h] [bp-50h]@35
  int v117; // [sp+5Ch] [bp-4Ch]@33
  int a1; // [sp+60h] [bp-48h]@27
  int v119; // [sp+64h] [bp-44h]@17
  int v120; // [sp+68h] [bp-40h]@23
  unsigned int v121; // [sp+6Ch] [bp-3Ch]@40
  unsigned int v122; // [sp+70h] [bp-38h]@35
  int v123; // [sp+74h] [bp-34h]@30
  int v124; // [sp+78h] [bp-30h]@17
  int v125; // [sp+7Ch] [bp-2Ch]@35
  unsigned int v126; // [sp+80h] [bp-28h]@9
  int v127; // [sp+84h] [bp-24h]@17
  int v128; // [sp+88h] [bp-20h]@9
  int *pZPixel; // [sp+8Ch] [bp-1Ch]@28
  int a2; // [sp+90h] [bp-18h]@16
  unsigned int v131; // [sp+94h] [bp-14h]@17
  unsigned __int16 *pColorPixel; // [sp+98h] [bp-10h]@28
  int v133; // [sp+9Ch] [bp-Ch]@17
  int v134; // [sp+A0h] [bp-8h]@17
  int v135; // [sp+A4h] [bp-4h]@24

  v1 = uFaceID;
  v111 = render->uTargetSurfacePitch;
  if ( (uFaceID & 0x80000000u) == 0 )
  {
    if ( (signed int)uFaceID < (signed int)pIndoor->uNumFaces )
    {
      v2 = &pIndoor->pFaces[uFaceID];
      v103 = v2;
      v3 = v2->uAttributes;
      if ( !(BYTE1(v3) & 0x20) )
      {
        if ( v3 & 0x400000 )
        {
          sr_4ADD1D(uFaceID);
          return;
        }
        if ( !(v3 & 0x10) || (sr_4AD504(uFaceID), render->pRenderD3D) )
        {
          v4 = v2->GetTexture();
          ++pBLVRenderParams->uNumFacesRenderedThisFrame;
          v5 = v4;
          v108 = v4;
          if ( v4 )
          {
            v6 = v4->palette_id2;
            LOBYTE(v2->uAttributes) |= 0x80u;
            v109 = v6;
            sr_4AE5F1(v1);
            v126 = pLightsData.pDeltaUV[0];
            v128 = pLightsData.pDeltaUV[1];
            v107 = bUseLoResSprites;
            v7 = GetPortalScreenCoord(v1);
            if ( v7 )
            {
              if ( PortalFrustrum(v7, &stru_F8A590, pBLVRenderParams->field_7C, v1) )
              {
                if ( v2->uPolygonType == 1 )
                {
                  for ( i = 0; i < Lights.uNumLightsApplied; Lights._blv_lights_ys[v18] = v20 )
                  {
                    v18 = i;
                    v120 = pLightsData._blv_lights_xs[i];
                    v134 = (unsigned __int64)(v120 * (signed __int64)-pLightsData.plane_4.vNormal.y) >> 16;
                    v133 = pLightsData.plane_4.vNormal.x;
                    v120 = pLightsData._blv_lights_ys[i];
                    v133 = (unsigned __int64)(v120 * (signed __int64)pLightsData.plane_4.vNormal.x) >> 16;
                    v19 = v128;
                    Lights._blv_lights_xs[i] = v126
                                                  + v134
                                                  + ((unsigned __int64)(v120
                                                                      * (signed __int64)Lights.plane_4.vNormal.x) >> 16);
                    v20 = v19 - Lights._blv_lights_zs[i++];
                  }
                }
                else
                {
                  if ( v2->uPolygonType != 3 )
                  {
                    if ( v2->uPolygonType == 4 )
                    {
LABEL_16:
                      a2 = 0;
                      if ( Lights.uNumLightsApplied > 0 )
                      {
                        do
                        {
                          v8 = a2;
                          v9 = Lights._blv_lights_xs[a2];
                          v131 = Lights._blv_lights_ys[a2];
                          v10 = Lights._blv_lights_zs[a2];
                          v11 = (signed int)(v10 * Lights.plane_4.vNormal.z
                                           + Lights.plane_4.dist
                                           + v9 * Lights.plane_4.vNormal.x
                                           + v131 * Lights.plane_4.vNormal.y) >> 16;
                          v119 = v9
                               - ((unsigned __int64)(Lights.plane_4.vNormal.x
                                                   * (signed __int64)((signed int)(v10 * Lights.plane_4.vNormal.z
                                                                                 + Lights.plane_4.dist
                                                                                 + v9 * Lights.plane_4.vNormal.x
                                                                                 + v131 * Lights.plane_4.vNormal.y) >> 16)) >> 16);
                          v131 -= (unsigned __int64)(Lights.plane_4.vNormal.y * (signed __int64)v11) >> 16;
                          v127 = v10 - ((unsigned __int64)(Lights.plane_4.vNormal.z * (signed __int64)v11) >> 16);
                          Lights._blv_lights_xs[a2] = ((unsigned __int64)(v119
                                                                             * (signed __int64)Lights.vec_14.x) >> 16)
                                                         + ((unsigned __int64)((signed int)v131
                                                                             * (signed __int64)Lights.vec_14.y) >> 16);
                          v124 = (unsigned __int64)(v119 * (signed __int64)Lights.vec_20.x) >> 16;
                          v134 = (unsigned __int64)((signed int)v131 * (signed __int64)Lights.vec_20.y) >> 16;
                          v133 = (unsigned __int64)(v127 * (signed __int64)Lights.vec_20.z) >> 16;
                          v12 = v126;
                          Lights._blv_lights_ys[v8] = v124
                                                         + ((unsigned __int64)((signed int)v131
                                                                             * (signed __int64)Lights.vec_20.y) >> 16)
                                                         + ((unsigned __int64)(v127
                                                                             * (signed __int64)Lights.vec_20.z) >> 16);
                          Lights._blv_lights_xs[v8] += v12;
                          Lights._blv_lights_ys[v8] += v128;
                          ++a2;
                        }
                        while ( a2 < Lights.uNumLightsApplied );
                        v2 = v103;
                      }
                      goto LABEL_24;
                    }
                    if ( v2->uPolygonType != 5 )
                    {
                      if ( v2->uPolygonType != 6 )
                        goto LABEL_24;
                      goto LABEL_16;
                    }
                  }
                  for ( j = 0; j < Lights.uNumLightsApplied; *(int *)v15 = v16 )
                  {
                    v14 = v128;
                    Lights._blv_lights_xs[j] += v126;
                    v15 = 4 * j + 16297672;
                    v16 = v14 - Lights._blv_lights_ys[j++];
                  }
                }
LABEL_24:
                v135 = 1;
                pEngine->_44ED0A(v2, &v135, 31);
                v104 = sr_sub_47C24C_get_palette(v2, v109, 0, 1);
                v134 = Lights.field_44;
                v106 = Lights.field_48;
                v21 = stru_F8A590._viewport_space_y;
                a2 = stru_F8A590._viewport_space_y;
                v110 = v111 * stru_F8A590._viewport_space_y;
                v97 = v5->pLevelOfDetail0_prolly_alpha_mask;
                v98 = v5->pLevelOfDetail1;
                v99 = v5->pLevelOfDetail2;
                v22 = v5->pLevelOfDetail3;
                v23 = 640 * stru_F8A590._viewport_space_y;
                v119 = 640 * stru_F8A590._viewport_space_y;
                v100 = v22;
                if ( stru_F8A590._viewport_space_y <= stru_F8A590._viewport_space_w )
                {
                  v24 = 2 * stru_F8A590._viewport_space_y;
                  v120 = 2 * stru_F8A590._viewport_space_y;
                  while ( 1 )
                  {
                    a1 = *(__int16 *)((char *)stru_F8A590.viewport_left_side + v24);
                    sr_4AE313(a1, v21, &stru_F81018.field_0);
                    if ( LOBYTE(viewparams->field_20) )
                    {
                      v27 = v111 * (v24 - pBLVRenderParams->uViewportY);
                      pZPixel = &pBLVRenderParams->pTargetZBuffer[2
                                                               * (*(__int16 *)((char *)stru_F8A590.viewport_left_side + v24)
                                                                + 320 * (v24 - pBLVRenderParams->uViewportY))
                                                               - pBLVRenderParams->uViewportX];
                      pColorPixel = &pBLVRenderParams->pRenderTarget[v27
                                                                  + 2 * *(__int16 *)((char *)stru_F8A590.viewport_left_side + v24)
                                                                  - pBLVRenderParams->uViewportX];
                      v26 = &pBLVRenderParams->pRenderTarget[v27
                                                          + 2 * *(__int16 *)((char *)stru_F8A590.viewport_right_side + v24)
                                                          - pBLVRenderParams->uViewportX];
                      v23 = v119;
                    }
                    else
                    {
                      v25 = *(__int16 *)((char *)stru_F8A590.viewport_left_side + v24);
                      pZPixel = &pBLVRenderParams->pTargetZBuffer[v25 + v23];
                      pColorPixel = &pBLVRenderParams->pRenderTarget[v25 + v110];
                      v26 = &pBLVRenderParams->pRenderTarget[v110 + *(__int16 *)((char *)stru_F8A590.viewport_right_side + v24)];
                    }
                    v131 = (unsigned int)v26;
                    HIWORD(v28) = HIWORD(stru_F81018.field_0.field_0);
                    LOWORD(v28) = 0;
                    v105 = Lights.field_0 | v28;
                    v123 = sr_4AE491(SHIWORD(stru_F81018.field_0.field_4), SHIWORD(stru_F81018.field_0.field_8));
                    if ( (unsigned int)pColorPixel < v131 )
                    {
                      v102 = v107 + v106;
                      v29 = (char *)&stru_F81018.field_34.field_8;
                      a1 += v134;
                      for ( k = &stru_F81018.field_34.field_8; ; v29 = (char *)k )
                      {
                        sr_4AE313(a1, a2, (stru337_stru0 *)(v29 - 8));
                        v30 = *((int *)v29 - 2);
                        LOWORD(v30) = 0;
                        v117 = v105;
                        v31 = Lights.field_0 | v30;
                        if ( v105 <= (unsigned int)v31 )
                          v117 = v31;
                        v105 = v31;
                        v32 = *((int *)v29 - 14);
                        v122 = *((int *)v29 - 14) >> v107;
                        v33 = *((int *)v29 - 13);
                        v125 = *((int *)v29 - 13) >> v107;
                        v34 = *((int *)v29 - 1) - v32;
                        v113 = (*(int *)v29 - v33) >> v102;
                        v35 = *((int *)v29 - 11);
                        v114 = v34 >> v102;
                        v116 = (&v97)[4 * v35];
                        v36 = v35 + 16 - v5->uWidthLn2;
                        v133 = v35 + 16;
                        v127 = v35 + v36;
                        v37 = v5->uWidthMinus1 >> v35;
                        v38 = v5->uHeightMinus1 >> v35 << (v35 + 16);
                        v128 = v37;
                        v126 = v38;
                        v39 = (unsigned int)(LOBYTE(viewparams->field_20) ? &pColorPixel[2 * v134] : &pColorPixel[v134]);
                        if ( v39 > v131 )
                          v39 = v131;
                        v40 = *((short *)v29 + 1);
                        v41 = *((short *)v29 - 1);
                        v121 = v39;
                        v42 = sr_4AE491(v41, v40);
                        v101 = v42;
                        v124 = (signed int)(v39 - (int)pColorPixel) >> 1;
                        if ( v123 >> 16 == v42 >> 16 || v135 & 2 )
                        {
                          v123 = (int)sr_sub_47C24C_get_palette(v103, v109, v123 >> 16, 1);
                          if ( LOBYTE(viewparams->field_20) )
                          {
                            v86 = pColorPixel;
                            if ( (unsigned int)pColorPixel < v121 )
                            {
                              v87 = pZPixel;
                              v88 = v123;
                              v89 = v122;
                              v90 = v125;
                              v91 = v133;
                              v92 = v127;
                              if ( v124 & 2 )
                              {
                                *pZPixel = v117;
                                v87 -= 2;
                                v86 = pColorPixel + 2;
                                goto LABEL_69;
                              }
                              do
                              {
                                v86 += 4;
                                v93 = *(&v116[v128 & (v89 >> v91)] + ((v126 & v90) >> v92));
                                v94 = v117;
                                LOWORD(v93) = *(short *)(v88 + 2 * v93);
                                *v87 = v117;
                                v87[1] = v94;
                                v87[640] = v94;
                                v87[641] = v94;
                                *(v86 - 4) = v93;
                                *(v86 - 3) = v93;
                                v86[636] = v93;
                                v86[637] = v93;
                                v87[2] = v94;
                                v87[3] = v94;
                                v87[642] = v94;
                                v87[643] = v94;
                                v122 += v114;
                                v125 += v113;
                                v89 = v122;
                                v90 = v125;
LABEL_69:
                                v87 += 4;
                                v95 = *(short *)(v88 + 2 * *(&v116[v128 & (v89 >> v91)] + ((v126 & v90) >> v92)));
                                v96 = v114;
                                *(v86 - 2) = v95;
                                *(v86 - 1) = v95;
                                v86[638] = v95;
                                v86[639] = v95;
                                v122 += v96;
                                v125 += v113;
                                v89 = v122;
                                v90 = v125;
                              }
                              while ( (unsigned int)v86 < v121 );
                              pColorPixel = v86;
                              pZPixel = v87;
                            }
                          }
                          else
                          {
                            v75 = pColorPixel;
                            if ( (unsigned int)pColorPixel < v121 )
                            {
                              v76 = pZPixel;
                              v77 = v123;
                              v78 = v122;
                              v79 = v125;
                              v80 = v133;
                              v81 = v127;
                              if ( v124 & 1 )
                              {
                                *pZPixel = v117;
                                --v76;
                                v75 = pColorPixel + 1;
                                goto LABEL_62;
                              }
                              do
                              {
                                v75 += 2;
                                v82 = *(&v116[v128 & (v78 >> v80)] + ((v126 & v79) >> v81));
                                v83 = v117;
                                LOWORD(v82) = *(short *)(v77 + 2 * v82);
                                *v76 = v117;
                                *(v75 - 2) = v82;
                                v76[1] = v83;
                                v122 += v114;
                                v125 += v113;
                                v78 = v122;
                                v79 = v125;
LABEL_62:
                                v84 = (unsigned int)(&v116[v128 & (v78 >> v80)] + ((v126 & v79) >> v81));
                                v76 += 2;
                                v85 = v114;
                                *(v75 - 1) = *(short *)(v77 + 2 * *(char *)v84);
                                v122 += v85;
                                v125 += v113;
                                v78 = v122;
                                v79 = v125;
                              }
                              while ( (unsigned int)v75 < v121 );
                              pColorPixel = v75;
                              pZPixel = v76;
                            }
                          }
                        }
                        else
                        {
                          v43 = (v42 - v123) >> v106;
                          v44 = v123 - v43;
                          v123 = v43;
                          v115 = v44;
                          if ( LOBYTE(viewparams->field_20) )
                          {
                            v60 = pColorPixel;
                            if ( (unsigned int)pColorPixel < v121 )
                            {
                              v61 = pZPixel;
                              v62 = v104;
                              v63 = v122;
                              v64 = v125;
                              v65 = v133;
                              v66 = v127;
                              if ( v124 & 2 )
                              {
                                *pZPixel = v117;
                                v61 += 2;
                                v60 = pColorPixel + 2;
                                goto LABEL_54;
                              }
                              do
                              {
                                v67 = (v128 & (v63 >> v65)) + ((v126 & v64) >> v66);
                                v68 = v123 + v115;
                                v60 += 4;
                                v115 = v68;
                                v69 = ((v68 & 0xFFFF0000u) >> 8) + v116[v67];
                                v70 = v117;
                                LOWORD(v69) = v62[v69];
                                *v61 = v117;
                                v61[1] = v70;
                                v61[640] = v70;
                                v61[641] = v70;
                                *(v60 - 4) = v69;
                                *(v60 - 3) = v69;
                                v60[636] = v69;
                                v60[637] = v69;
                                v61[2] = v70;
                                v61[3] = v70;
                                v61[642] = v70;
                                v61[643] = v70;
                                v122 += v114;
                                v125 += v113;
                                v63 = v122;
                                v64 = v125;
                                v61 += 4;
LABEL_54:
                                v71 = (v128 & (v63 >> v65)) + ((v126 & v64) >> v66);
                                v72 = v123 + v115;
                                v73 = v116[v71];
                                v115 = v72;
                                LOWORD(v73) = v62[((v72 & 0xFFFF0000u) >> 8) + v73];
                                v74 = v114;
                                *(v60 - 2) = v73;
                                *(v60 - 1) = v73;
                                v60[638] = v73;
                                v60[639] = v73;
                                v122 += v74;
                                v125 += v113;
                                v63 = v122;
                                v64 = v125;
                              }
                              while ( (unsigned int)v60 < v121 );
                              pColorPixel = v60;
                              pZPixel = v61;
                            }
                          }
                          else
                          {
                            v45 = pColorPixel;
                            if ( (unsigned int)pColorPixel < v121 )
                            {
                              v46 = pZPixel;
                              v47 = v104;
                              v48 = v122;
                              v49 = v125;
                              v50 = v133;
                              v51 = v127;
                              if ( v124 & 1 )
                              {
                                *pZPixel = v117;
                                ++v46;
                                v45 = pColorPixel + 1;
                                goto LABEL_47;
                              }
                              do
                              {
                                v52 = (v128 & (v48 >> v50)) + ((v126 & v49) >> v51);
                                v53 = v123 + v115;
                                v45 += 2;
                                v115 = v53;
                                v54 = ((v53 & 0xFFFF0000u) >> 8) + v116[v52];
                                v55 = v117;
                                LOWORD(v54) = v47[v54];
                                *v46 = v117;
                                *(v45 - 2) = v54;
                                v46[1] = v55;
                                v122 += v114;
                                v125 += v113;
                                v48 = v122;
                                v49 = v125;
                                v46 += 2;
LABEL_47:
                                v56 = (v128 & (v48 >> v50)) + ((v126 & v49) >> v51);
                                v57 = v123 + v115;
                                v58 = v116[v56];
                                v115 = v57;
                                LOWORD(v58) = v47[((v57 & 0xFFFF0000u) >> 8) + v58];
                                v59 = v114;
                                *(v45 - 1) = v58;
                                v122 += v59;
                                v125 += v113;
                                v48 = v122;
                                v49 = v125;
                              }
                              while ( (unsigned int)v45 < v121 );
                              pColorPixel = v45;
                              pZPixel = v46;
                            }
                          }
                        }
                        k += 13;
                        v5 = v108;
                        v123 = v101;
                        a1 += v134;
                        if ( (unsigned int)pColorPixel >= v131 )
                          break;
                      }
                      v23 = v119;
                      v24 = v120;
                    }
                    ++a2;
                    v110 += v111;
                    v23 += 640;
                    v24 += 2;
                    v120 = v24;
                    v119 = v23;
                    if ( a2 > stru_F8A590._viewport_space_w )
                      break;
                    v21 = a2;
                  }
                }
                return;
              }
            }
          }
        }
      }
    }
  }
}
//----- (00485E1F) --------------------------------------------------------
void *sr_sub_485E1F(stru316 *a1, Span *a2, int a3, Polygon *a4, int a5, unsigned __int8 a6, char a7)
{
  stru316 *v7; // esi@1
  signed int *v8; // edi@1
  signed int *v9; // ebx@1
  char v10; // zf@1
  int v11; // eax@1
  Span *v12; // ecx@3
  double v13; // ST24_8@3
  double v14; // ST24_8@3
  int v15; // eax@3
  signed int v16; // ST14_4@4
  char v17; // dl@4
  signed int v18; // ST10_4@4
  void *v19; // eax@4
  signed int v20; // ST14_4@5
  char v21; // dl@5
  int v22; // ST10_4@5
  signed int v23; // ST14_4@6
  char v24; // dl@6
  Span *v26; // [sp+10h] [bp-8h]@1
  char v27; // [sp+16h] [bp-2h]@1
  char v28; // [sp+17h] [bp-1h]@1
  float v29; // [sp+30h] [bp+18h]@3
  float v30; // [sp+30h] [bp+18h]@3

  v7 = a1;
  v26 = a2;
  v8 = &a1->field_C;
  v9 = &a1->field_8;
  v10 = a1->field_0 == 0;
  v27 = a1->field_4 != 0;
  v28 = !v10;
  sr_sub_47BEB1(a3, a4, a5, 0, &a1->field_8, &a1->field_C, (int)&v27, (int)&v28);
  v7->field_24_palette = (unsigned __int16 *)sr_sub_47C28C_get_palette(a4, v28, *v9, *v8);
  v11 = a4->field_108;
  v7->field_20 = v11;
  if ( v11 )
  {
    if ( a7 )
    {
      v12 = v26;
      v29 = v26->field_10 * 31.0;
      v13 = v29 + 6.7553994e15;
      v7->field_10 = LODWORD(v13) << 16;
      v30 = v12->field_14 * 31.0;
      v14 = v30 + 6.7553994e15;
      v15 = v7->field_10;
      v7->field_14 = LODWORD(v14) << 16;
      v7->field_18 = -((v15 - (LODWORD(v14) << 16)) / v12->field_C);
    }
    v16 = *v8;
    v17 = v28;
    v18 = *v9;
    v7->field_1C = a6;
    v19 = sr_sub_47C28C_get_palette(a4, v17, v18, v16);
  }
  else
  {
    v20 = *v8;
    v21 = v28;
    v7->field_10 = 0;
    v7->field_14 = 0;
    v22 = *v9;
    v7->field_18 = 0;
    v7->field_1C = 0;
    v19 = sr_sub_47C1CA(a4, v21, v22, v20);
  }
  v23 = *v8;
  v24 = v28;
  v7->field_24_palette = (unsigned __int16 *)v19;
  return sr_sub_47C1CA(a4, v24, *v9, v23);
}

//----- (00486F92) --------------------------------------------------------
void  sr_sub_486F92_MessWithEdgesAndSpans()
{
  Span *v0; // ebx@1
  int v1; // eax@2
  Edge *v2; // ecx@3
  Edge *v3; // edx@3
  Edge *v4; // esi@6
  Edge *v5; // eax@7
  Edge *v6; // ecx@8
  Surf *v7; // esi@11
  double v8; // st7@13
  Surf *v9; // edi@13
  double v10; // st6@13
  double v11; // st7@14
  signed __int64 v12; // qax@14
  Polygon *v13; // eax@15
  Span *v14; // ecx@17
  double v15; // st7@28
  signed __int64 v16; // qax@28
  Polygon *v17; // eax@29
  Span *v18; // ecx@31
  Edge *i; // eax@40
  Edge *v20; // ecx@43
  Edge *v21; // esi@44
  double v22; // st7@45
  Edge *v23; // edx@48
  unsigned int v24; // [sp+10h] [bp-10h]@2
  float v25; // [sp+14h] [bp-Ch]@3
  Edge *v26; // [sp+18h] [bp-8h]@8
  unsigned int v27; // [sp+1Ch] [bp-4h]@1

  v0 = pSpans;
  stru_80C9D8.pSurf = &stru_80C980;
  stru_80C9D8.field_0 = (double)(signed int)pViewport->uViewportTL_X;
  stru_80C9A4.pSurf = &stru_80C980;
  stru_80C980.pPrev = &stru_80C980;
  stru_80C980.pNext = &stru_80C980;
  stru_80C9A4.field_0 = (double)(signed int)pViewport->uViewportBR_X;
  stru_80C980.field_8 = 0.0;
  stru_80C980.field_4 = 0.0;
  stru_80C9D8.pNext = &stru_80C9A4;
  stru_80C9D8.pPrev = 0;
  stru_80C9D8.field_8 = 1;
  stru_80C9A4.pNext = 0;
  stru_80C9A4.pPrev = &stru_80C9D8;
  stru_80C9A4.field_8 = 0;
  LODWORD(stru_80C980.field_0) = 0xC97423F0u;
  v27 = pViewport->uViewportTL_Y;
  if ( (signed int)pViewport->uViewportTL_Y > (signed int)pViewport->uViewportBR_Y )
  {
LABEL_51:
    v0->field_8 = -1;
  }
  else
  {
    v1 = 52 * pViewport->uViewportTL_Y;
    v24 = 52 * pViewport->uViewportTL_Y;
    while ( 1 )
    {
      v2 = *(Edge **)((char *)&pNewEdges->pNext + v1);
      v3 = &stru_80C9D8;
      v25 = (double)(signed int)v27;
      if ( v2 != &defaultEdge )
      {
        do
        {
          while ( 1 )
          {
            v4 = v3->pNext;
            if ( v2->field_0 <= (double)v4->field_0 )
              break;
            v3 = v3->pNext;
          }
          v5 = v2->pNext;
          v2->pNext = v4;
          v2->pPrev = v3;
          v3->pNext->pPrev = v2;
          v3->pNext = v2;
          v3 = v2;
          v2 = v5;
        }
        while ( v5 != &defaultEdge );
      }
      v6 = stru_80C9D8.pNext;
      stru_80C980.field_20 = 0;
      stru_80C980.field_22 = 1;
      v26 = stru_80C9D8.pNext;
      if ( stru_80C9D8.pNext )
      {
        while ( 1 )
        {
          v7 = v6->pSurf;
          if ( v6->field_8 )
          {
            ++v7->field_22;
            if ( v7->field_22 == 1 )
            {
              v8 = v6->field_0 + 2.0;
              v9 = stru_80C980.pNext;
              v10 = (v8 - v7->field_C) * v7->field_4 + (v25 - v7->field_10) * v7->field_8 + v7->field_0;
              if ( v10 <= (v8 - stru_80C980.pNext->field_C) * stru_80C980.pNext->field_4
                        + (v25 - stru_80C980.pNext->field_10) * stru_80C980.pNext->field_8
                        + stru_80C980.pNext->field_0 )
              {
                do
                  v9 = v9->pNext;
                while ( v10 <= (v8 - v9->field_C) * v9->field_4 + (v25 - v9->field_10) * v9->field_8 + v9->field_0 );
                v7->pNext = v9;
                v7->pPrev = v9->pPrev;
                v9->pPrev->pNext = v7;
                v9->pPrev = v7;
              }
              else
              {
                v11 = v6->field_0 + 0.5;
                v12 = (signed __int64)(v11 - (double)stru_80C980.pNext->field_20);
                v0->field_C = v12;
                if ( (signed __int16)v12 > 0 )
                {
                  v0->field_A = v27;
                  v0->field_8 = v9->field_20;
                  v13 = v9->pParent;
                  v0->pParent = v13;
                  if ( v13 )
                  {
                    if ( v13->prolly_head )
                    {
                      v14 = v13->prolly_tail;
                      if ( !v14 )
                        return;
                      v14->pNext = v0;
                    }
                    else
                    {
                      v13->prolly_head = v0;
                    }
                    v13->prolly_tail = v0;
                  }
                  if ( (signed int)pODMRenderParams->uNumSpans >= 12499 )
                    return;
                  ++v0;
                  ++pODMRenderParams->uNumSpans;
                }
                v6 = v26;
                v7->field_20 = (signed __int64)v11;
                v7->pNext = v9;
                v9->pPrev = v7;
                stru_80C980.pNext = v7;
                v7->pPrev = &stru_80C980;
              }
            }
          }
          else
          {
            --v7->field_22;
            if ( !v7->field_22 )
            {
              if ( stru_80C980.pNext == v7 )
              {
                v15 = v6->field_0 + 0.5;
                v16 = (signed __int64)(v15 - (double)v7->field_20);
                v0->field_C = v16;
                if ( (signed __int16)v16 > 0 )
                {
                  v0->field_A = v27;
                  v0->field_8 = v7->field_20;
                  v17 = v7->pParent;
                  v0->pParent = v17;
                  if ( v17 )
                  {
                    if ( v17->prolly_head )
                    {
                      v18 = v17->prolly_tail;
                      if ( !v18 )
                        return;
                      v18->pNext = v0;
                    }
                    else
                    {
                      v17->prolly_head = v0;
                    }
                    v17->prolly_tail = v0;
                  }
                  if ( (signed int)pODMRenderParams->uNumSpans >= 12499 )
                    return;
                  ++v0;
                  ++pODMRenderParams->uNumSpans;
                }
                v7->pNext->field_20 = (signed __int64)v15;
                v6 = v26;
              }
              v7->pNext->pPrev = v7->pPrev;
              v7->pPrev->pNext = v7->pNext;
            }
          }
          v26 = v6->pNext;
          if ( !v26 )
            break;
          v6 = v6->pNext;
        }
      }
      for ( i = ptr_80CA10[v27]; i; i = i->ptr_18 )
      {
        i->pPrev->pNext = i->pNext;
        i->pNext->pPrev = i->pPrev;
      }
      v20 = stru_80C9D8.pNext;
      if ( stru_80C9D8.pNext != &stru_80C9A4 )
        break;
LABEL_50:
      ++v27;
      v1 = v24 + 52;
      v24 += 52;
      if ( (signed int)v27 > (signed int)pViewport->uViewportBR_Y )
        goto LABEL_51;
    }
    while ( 1 )
    {
      v21 = v20->pNext;
      if ( !v21 )
        break;
      v22 = v20->field_4 + v20->field_0;
      v20->field_0 = v22;
      if ( v22 < stru_80C9D8.field_0 )
        v20->field_0 = stru_80C9D8.field_0 + 0.0000001;
      while ( 1 )
      {
        v23 = v20->pPrev;
        if ( v20->field_0 >= (double)v23->field_0 )
          break;
        v23->pNext = v20->pNext;
        v20->pNext->pPrev = v23;
        v23->pPrev->pNext = v20;
        v20->pPrev = v23->pPrev;
        v20->pNext = v23;
        v23->pPrev = v20;
      }
      v20 = v21;
      if ( v21 == &stru_80C9A4 )
        goto LABEL_50;
    }
  }
}

//----- (00487355) --------------------------------------------------------
bool ODMRenderParams::ApplyLightmapsSW()
{
  int v0; // esi@1
  Polygon *v1; // edi@2
  bool result; // eax@3

  v0 = 0;
  if ( pODMRenderParams->uNumPolygons > 0 )
  {
    v1 = array_77EC08.data();
    do
    {
      result = pEngine->pLightmapBuilder->_45D3C7_sw(v1);
      ++v0;
      ++v1;
    }
    while ( v0 < pODMRenderParams->uNumPolygons );
  }
  return result;
}

//----- (00479332) --------------------------------------------------------
int Render::OnOutdoorRedrawSW()
{
  signed int result; // eax@1
  unsigned int v1; // edi@1
  char *v2; // esi@2
  char v3; // cl@3
  double v4; // st7@4
  float v5; // ST34_4@6
  double v6; // ST24_8@6
  int v7; // eax@6
  unsigned int v8; // eax@11
  signed int v9; // eax@13
  Vec3_float_ *v10; // eax@14
  double v11; // st7@17
  float v12; // ST34_4@19
  double v13; // ST1C_8@19
  float v14; // [sp+4h] [bp-34h]@1
  float v15; // [sp+8h] [bp-30h]@1
  float v16; // [sp+Ch] [bp-2Ch]@1

  v14 = (double)pOutdoor->vSunlight.x / 65536.0;
  result = 0;
  v1 = (unsigned int)&array_77EC08[pODMRenderParams->uNumPolygons];
  v15 = (double)pOutdoor->vSunlight.y / 65536.0;
  v16 = (double)pOutdoor->vSunlight.z / 65536.0;
  if ( v1 > (unsigned int)array_77EC08.data() )
  {
    v2 = (char *)&array_77EC08[0].pODMFace;
    while ( 1 )
    {
      v3 = v2[5];
      if ( v3 == 5 )
        break;
      if ( v3 == 1 )
      {
        v9 = pTerrainNormalIndices[((*((int *)v2 - 9) >> 15) & 1)
                                 + 2 * ((unsigned __int8)v2[9] + ((unsigned __int8)v2[8] << 7))];
        if ( v9 > (signed int)(uNumTerrainNormals - 1) )
          v10 = 0;
        else
          v10 = &pTerrainNormals[v9];
        if ( v10 )
        {
          v11 = -(v16 * v10->z + v15 * v10->y + v14 * v10->x);
          if ( v11 < 0.0 )
            v11 = 0.0;
          v12 = v11 * 31.0;
          v13 = v12 + 6.7553994e15;
          v2[4] = 31 - LOBYTE(v13);
        }
        else
        {
          v2[4] = 0;
        }
        if ( v2[4] < 0 )
          v2[4] = 0;
        goto LABEL_23;
      }
LABEL_24:
      v2 += 268;
      if ( (unsigned int)(v2 - 84) >= v1 )
        return result;
    }
    v4 = (double)(signed int)(((unsigned __int64)(**(int **)v2 * (signed __int64)-pOutdoor->vSunlight.x) >> 16)
                            + ((unsigned __int64)(*(int *)(*(int *)v2 + 4) * (signed __int64)-pOutdoor->vSunlight.y) >> 16)
                            + ((unsigned __int64)(*(int *)(*(int *)v2 + 8) * (signed __int64)-pOutdoor->vSunlight.z) >> 16))
       * 0.000015258789;
    if ( v4 < 0.0 )
      v4 = 0.0;
    v5 = v4 * 31.0;
    v6 = v5 + 6.7553994e15;
    v7 = (int)(v2 + 4);
    v2[4] = 31 - LOBYTE(v6);
    if ( (char)(31 - LOBYTE(v6)) < 0 )
      *(char *)v7 = 0;
    if ( *(char *)v7 > 31 )
      *(char *)v7 = 31;
    if ( *(char *)(*(int *)v2 + 29) & 0x40 )
    {
      v8 = pTextureFrameTable->GetFrameTexture(
             *((short *)v2 - 4),
             pEventTimer->uTotalGameTimeElapsed);
      *((int *)v2 - 6) = v8 != -1 ? (int)&pBitmaps_LOD->pTextures[v8] : 0;
    }
LABEL_23:
    result = 1;
    goto LABEL_24;
  }
  return result;
}
//----- (00485A24) --------------------------------------------------------
stru315 *sr_sub_485A24(stru315 *a1, stru315 *a2)
{
  stru315 *result; // eax@1
  int i; // ecx@1
  int v4; // ecx@2
  int v5; // ecx@4
  int v6; // ecx@6
  int v7; // ecx@8
  int v8; // esi@10
  int v9; // ecx@10
  int v10; // esi@11
  int v11; // edi@11
  int v12; // ebx@11
  int v13; // ecx@16
  unsigned __int16 *v14; // esi@18
  unsigned int *v15; // ecx@20
  int v16; // esi@20
  int v17; // [sp+0h] [bp-4h]@10

  result = a1;
  for ( i = a1->field_28; i; i = result->field_28 )
  {
    result->field_28 = i - 1;
    v4 = result->field_18;
    if ( result->field_30 > v4 )
      result->field_30 = v4;
    v5 = result->field_20;
    if ( result->field_2C > v5 )
      result->field_2C = v5;
    v6 = result->field_14;
    if ( result->field_30 < v6 )
      result->field_30 = v6;
    v7 = result->field_1C;
    if ( result->field_2C < v7 )
      result->field_2C = v7;
    v8 = (result->field_C & HIWORD(result->field_30)) + ((result->field_2C & result->field_8) >> result->field_10);
    v9 = *((char *)result->pTextureLOD + v8);
    v17 = *((char *)result->pTextureLOD + v8);
    if ( a2->field_20 )
    {
      v10 = HIWORD(a2->field_10);
      v11 = a2->field_C;
      v12 = v10;
      if ( v10 >= v11 )
        v12 = a2->field_C;
      if ( a2->field_8 - v12 <= 0 )
      {
        v13 = 0;
      }
      else
      {
        if ( v10 >= v11 )
          v10 = a2->field_C;
        v13 = a2->field_8 - v10;
      }
      v14 = (unsigned __int16 *)a2->field_24;
      v9 = v17 + (v13 << 8);
    }
    else
    {
      v14 = result->field_34_palette;
    }
    *result->pColorBuffer = v14[v9];
    v15 = result->pDepthBuffer;
    v16 = result->field_24;
    ++result->pColorBuffer;
    *v15 = v16;
    ++result->pDepthBuffer;
    a2->field_10 += a2->field_18;
    result->field_30 += result->field_4;
    result->field_2C += result->field_0;
  }
  --result->field_28;
  return result;
}
//----- (00485975) --------------------------------------------------------
stru315 *sr_sub_485975(stru315 *a1, stru315 *a2)
{
  stru315 *result; // eax@1
  int i; // ecx@1
  int v4; // esi@2
  int v5; // esi@2
  int v6; // ecx@2
  int v7; // esi@3
  int v8; // edi@3
  int v9; // ebx@3
  int v10; // ecx@8
  unsigned __int16 *v11; // esi@10
  unsigned int *v12; // ecx@12
  int v13; // esi@12
  int v14; // [sp+0h] [bp-4h]@2

  result = a1;
  for ( i = a1->field_28; i; i = result->field_28 )
  {
    v4 = result->field_8 & result->field_2C;
    result->field_28 = i - 1;
    v5 = (result->field_C & HIWORD(result->field_30)) + (v4 >> result->field_10);
    v6 = *((char *)result->pTextureLOD + v5);
    v14 = *((char *)result->pTextureLOD + v5);
    if ( a2->field_20 )
    {
      v7 = HIWORD(a2->field_10);
      v8 = a2->field_C;
      v9 = v7;
      if ( v7 >= v8 )
        v9 = a2->field_C;
      if ( a2->field_8 - v9 <= 0 )
      {
        v10 = 0;
      }
      else
      {
        if ( v7 >= v8 )
          v7 = a2->field_C;
        v10 = a2->field_8 - v7;
      }
      v11 = (unsigned __int16 *)a2->field_24;
      v6 = v14 + (v10 << 8);
    }
    else
    {
      v11 = result->field_34_palette;
    }
    *result->pColorBuffer = v11[v6];
    v12 = result->pDepthBuffer;
    v13 = result->field_24;
    ++result->pColorBuffer;
    *v12 = v13;
    ++result->pDepthBuffer;
    a2->field_10 += a2->field_18;
    result->field_30 += result->field_4;
    result->field_2C += result->field_0;
  }
  --result->field_28;
  return result;
}
//----- (004839BD) --------------------------------------------------------
signed int sr_sub_4839BD(Span *ecx0, unsigned __int16 *pTargetSurface)
{
  stru315 *v2; // ebp@0
  int v3; // eax@1
  int v4; // edi@1
  Polygon *v5; // esi@1
  unsigned int v6; // ebx@1
  int v7; // ecx@1
  int v8; // ebx@1
  int v9; // ecx@1
  int v10; // eax@1
  stru149 *v11; // eax@1
  int v12; // edx@1
  int v13; // eax@1
  int v14; // edi@1
  int v15; // ecx@1
  int v16; // eax@1
  signed int v17; // edi@1
  int v18; // edi@2
  signed __int64 v19; // qtt@3
  int v20; // edi@3
  unsigned __int16 *v21; // eax@3
  Texture_MM7 *v22; // eax@4
  Texture_MM7 *v23; // eax@6
  Texture_MM7 *v24; // eax@8
  Texture_MM7 *v25; // eax@10
  stru149 *v26; // eax@13
  int v27; // edi@13
  signed int v28; // edx@13
  Texture_MM7 *v29; // ebx@13
  int v30; // edi@13
  signed int v31; // edx@13
  signed int v32; // eax@13
  signed int v33; // eax@13
  int v34; // ebx@13
  int v35; // eax@15
  int v36; // ebx@15
  int v37; // eax@16
  signed __int64 v38; // qtt@17
  int v39; // ecx@17
  int v40; // eax@19
  stru149 *v41; // eax@21
  int v42; // ebx@21
  int v43; // ebx@21
  int v44; // eax@21
  char v45; // zf@25
  int v46; // eax@28
  int v47; // eax@28
  int v48; // ebx@28
  int v49; // eax@29
  signed __int64 v50; // qtt@30
  int v51; // ecx@30
  int v52; // eax@30
  int v53; // edx@31
  stru149 *v54; // eax@33
  int v55; // ebx@33
  signed int v56; // ebx@33
  int v57; // eax@33
  unsigned __int64 v58; // qax@33
  int v60; // [sp+Ch] [bp-BCh]@1
  Span *v61; // [sp+10h] [bp-B8h]@1
  int v62; // [sp+14h] [bp-B4h]@2
  int v63; // [sp+18h] [bp-B0h]@1
  stru315 a1; // [sp+1Ch] [bp-ACh]@1
  stru316 a2; // [sp+60h] [bp-68h]@13
  int v66; // [sp+88h] [bp-40h]@13
  int v67; // [sp+8Ch] [bp-3Ch]@1
  int v68; // [sp+90h] [bp-38h]@13
  int v69; // [sp+94h] [bp-34h]@3
  int v70; // [sp+98h] [bp-30h]@1
  int v71; // [sp+9Ch] [bp-2Ch]@1
  int v72; // [sp+A0h] [bp-28h]@1
  int v73; // [sp+A4h] [bp-24h]@13
  int v74; // [sp+A8h] [bp-20h]@1
  int v75; // [sp+ACh] [bp-1Ch]@3
  int v76; // [sp+B0h] [bp-18h]@1
  int v77; // [sp+B4h] [bp-14h]@1
  int X; // [sp+B8h] [bp-10h]@1
  int v79; // [sp+BCh] [bp-Ch]@21
  int v80; // [sp+C0h] [bp-8h]@13
  unsigned int v81; // [sp+C4h] [bp-4h]@1

  v3 = ecx0->field_A;
  v4 = ecx0->field_8;
  v5 = ecx0->pParent;
  v6 = v4 + render->uTargetSurfacePitch * ecx0->field_A;
  v61 = ecx0;
  v7 = ecx0->field_C;
  a1.pColorBuffer = &pTargetSurface[v6];
  v74 = v7;
  a1.pDepthBuffer = (unsigned int *)&render->pActiveZBuffer[v4 + 640 * v3];
  v8 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterX - v4);
  v9 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - v3);
  v10 = v5->ptr_38->field_14;
  v76 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterX - v4);
  v72 = v10;
  v81 = (unsigned __int64)(v10 * (signed __int64)v9) >> 16;
  v11 = v5->ptr_38;
  v12 = v81 + v11->field_C;
  v72 = v11->field_20;
  v67 = v12;
  v13 = ((unsigned __int64)(v72 * (signed __int64)v9) >> 16) + v5->ptr_38->field_18;
  v81 = v9;
  v70 = v13;
  v72 = v5->v_18.z;
  v81 = (unsigned __int64)(v72 * (signed __int64)v9) >> 16;
  v14 = v5->field_24;
  v15 = v81 + v5->v_18.x;
  v71 = v5->sTextureDeltaU << 16;
  v63 = v5->sTextureDeltaV << 16;
  v16 = v5->v_18.y;
  v17 = -v14;
  v60 = v15;
  v77 = v17;
  v81 = (unsigned __int64)(v76 * (signed __int64)v16) >> 16;
  X = v81 + v15;
  if ( !(v81 + v15) || (v62 = v17 >> 14, v18 = abs(v17 >> 14), v18 > abs(X)) )
    return 0;
  LODWORD(v19) = v77 << 16;
  HIDWORD(v19) = v77 >> 16;
  v69 = v19 / X;
  v20 = v19 / X;
  v21 = (unsigned __int16 *)v5->pTexture->pLevelOfDetail2;
  a1.pTextureLOD = (unsigned __int16 *)v5->pTexture->pLevelOfDetail2;
  v75 = 2;
  if ( v20 >= mipmapping_building_mm1 << 16 )
  {
    if ( v20 >= mipmapping_building_mm2 << 16 )
    {
      if ( v20 >= mipmapping_building_mm3 << 16 )
      {
        if ( bUseLoResSprites )
          goto LABEL_12;
        v25 = v5->pTexture;
        v75 = 3;
        v21 = (unsigned __int16 *)v25->pLevelOfDetail3;
      }
      else
      {
        v24 = v5->pTexture;
        v75 = 2;
        v21 = (unsigned __int16 *)v24->pLevelOfDetail2;
      }
    }
    else
    {
      v23 = v5->pTexture;
      v75 = 1;
      v21 = (unsigned __int16 *)v23->pLevelOfDetail1;
    }
  }
  else
  {
    v22 = v5->pTexture;
    v75 = 0;
    v21 = (unsigned __int16 *)v22->pLevelOfDetail0_prolly_alpha_mask;
  }
  a1.pTextureLOD = v21;
LABEL_12:
  if ( v21 )
  {
    a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&a2, v61, v20, v5, pODMRenderParams->building_gamme, 1u, 1);
    a1.field_28 = 16;
    v66 = v74 >> 4;
    v81 = v8;
    v26 = v5->ptr_38;
    v72 = v74 - 16 * (v74 >> 4);
    v76 = v26->field_10;
    v81 = v8;
    v74 = v67 + ((unsigned __int64)(v76 * (signed __int64)v8) >> 16);
    v76 = v5->ptr_38->field_1C;
    v76 = v70 + ((unsigned __int64)(v76 * (signed __int64)v8) >> 16);
    v81 = (unsigned __int64)(v74 * (signed __int64)v20) >> 16;
    v27 = ((unsigned __int64)(v74 * (signed __int64)v20) >> 16) - v5->ptr_38->field_24;
    v81 = (unsigned __int64)(v76 * (signed __int64)v69) >> 16;
    v28 = v63 + ((unsigned __int64)(v76 * (signed __int64)v69) >> 16) - v5->ptr_38->field_28;
    v76 = v75 + bUseLoResSprites;
    v73 = v8 - 16 * pODMRenderParams->int_fov_rad_inv;
    v29 = v5->pTexture;
    v30 = (v71 + v27) >> v76;
    v31 = v28 >> v76;
    v32 = (signed int)v29->uTextureWidth >> v75;
    v74 = (signed __int16)v75;
    v68 = v31;
    v80 = v32;
    v33 = (signed int)v29->uTextureHeight >> v75;
    v34 = v75 - v29->uWidthLn2;
    a1.field_8 = (v33 << 16) - 65536;
    v81 = 2 * pMiscTimer->uTotalGameTimeElapsed;
    a1.field_10 = v34 + 16;
    a1.field_C = v80 - 1;
    if ( v66 > 0 )
    {
      v74 = v66;
      v66 = 12 - v75;
      do
      {
        v80 = v5->v_18.y;
        v35 = (unsigned __int64)(v80 * (signed __int64)v73) >> 16;
        v36 = v35 + v60;
        X = v35 + v60;
        if ( v35 + v60 && (v80 = abs(v62), v37 = abs(v36), v80 <= v37) )
        {
          LODWORD(v38) = v77 << 16;
          HIDWORD(v38) = v77 >> 16;
          v80 = v38 / X;
          v39 = v38 / X;
          X = v38 / X;
        }
        else
        {
          X = 0x40000000u;
          v39 = 0x40000000u;
        }
        HIWORD(v40) = HIWORD(v39);
        if ( v39 <= v69 )
          HIWORD(v40) = HIWORD(v69);
        LOWORD(v40) = 0;
        a1.field_24 = v5->field_50 | v40;
        v79 = v5->ptr_38->field_10;
        v79 = v67 + ((unsigned __int64)(v79 * (signed __int64)v73) >> 16);
        v80 = (unsigned __int64)(v79 * (signed __int64)v39) >> 16;
        v41 = v5->ptr_38;
        v42 = ((unsigned __int64)(v79 * (signed __int64)v39) >> 16) - v41->field_24;
        v79 = v41->field_1C;
        v79 = v70 + ((unsigned __int64)(v79 * (signed __int64)v73) >> 16);
        v80 = (unsigned __int64)(v79 * (signed __int64)v39) >> 16;
        v43 = (v71 + v42) >> v76;
        v79 = (signed int)(v63 + ((unsigned __int64)(v79 * (signed __int64)v39) >> 16) - v5->ptr_38->field_28) >> v76;
        a1.field_4 = (v43 - v30) >> 4;
        a1.field_0 = (v79 - v68) >> 4;
        a1.field_30 = v30 + 4 * stru_5C6E00->Cos(v81 + (v68 >> v66));
        v44 = stru_5C6E00->Sin(v81 + (v30 >> v66));
        a1.field_2C = v68 + 4 * v44;
        if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
          sr_sub_485975(&a1, (stru315 *)&a2);
        else
          sr_sub_4D6FB0(v2);
        a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&a2, v61, X, v5, pODMRenderParams->building_gamme, 1u, 0);
        v73 -= 16 * pODMRenderParams->int_fov_rad_inv;
        v45 = v74-- == 1;
        a1.field_28 = 16;
        v30 = v43;
        v68 = v79;
      }
      while ( !v45 );
      v31 = v79;
    }
    if ( !v72 )
      return 1;
    v66 = 12 - v75;
    a1.field_30 = v30 + 4 * stru_5C6E00->Cos(v81 + (v31 >> (12 - v75)));
    v46 = stru_5C6E00->Sin(v81 + (v30 >> v66));
    a1.field_2C = v68 + 4 * v46;
    v79 = v5->v_18.y;
    v80 = (unsigned __int64)(v79 * (signed __int64)v73) >> 16;
    v47 = (unsigned __int64)(v79 * (signed __int64)v73) >> 16;
    v48 = v47 + v60;
    X = v47 + v60;
    if ( v47 + v60 )
    {
      v79 = abs(v62);
      v49 = abs(v48);
      if ( v79 <= v49 )
      {
        LODWORD(v50) = v77 << 16;
        HIDWORD(v50) = v77 >> 16;
        v77 = v50 / X;
        v51 = v50 / X;
        HIWORD(v52) = HIWORD(v69);
        if ( v51 <= v69 )
        {
          LOWORD(v52) = 0;
          a1.field_24 = v52 | v5->field_50;
        }
        else
        {
          HIWORD(v53) = HIWORD(v51);
          LOWORD(v53) = 0;
          a1.field_24 = v53 | v5->field_50;
        }
        v79 = v5->ptr_38->field_10;
        v77 = (unsigned __int64)(v79 * (signed __int64)v73) >> 16;
        v79 = v67 + ((unsigned __int64)(v79 * (signed __int64)v73) >> 16);
        v67 = (unsigned __int64)(v79 * (signed __int64)v51) >> 16;
        v54 = v5->ptr_38;
        v55 = v67 - v54->field_24;
        v79 = v54->field_1C;
        v56 = v71 + v55;
        v71 = (unsigned __int64)(v79 * (signed __int64)v73) >> 16;
        v57 = v70 + ((unsigned __int64)(v79 * (signed __int64)v73) >> 16);
        v79 = v57;
        v58 = v57 * (signed __int64)v51;
        v70 = v58 >> 16;
        LODWORD(v58) = (signed int)(v63 + (v58 >> 16) - v5->ptr_38->field_28) >> v76;
        a1.field_4 = ((v56 >> v76) - v30) >> 4;
        a1.field_0 = ((signed int)v58 - v68) >> 4;
        a1.field_28 = v72;
        if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
          sr_sub_485975(&a1, (stru315 *)&a2);
        else
          sr_sub_4D6FB0(v2);
        return 1;
      }
    }
  }
  return 0;
}

//----- (0048408A) --------------------------------------------------------
signed int sr_sub_48408A_prolly_odm_water_no_waves(Span *_this)
{
  stru315 *v1; // ebp@0
  Span *v2; // edi@1
  Polygon *v3; // esi@1
  int v4; // ecx@1
  stru149 *v5; // eax@1
  stru149 *v6; // eax@1
  int v7; // edx@1
  int v8; // eax@1
  int v9; // ebx@1
  int v10; // eax@1
  int v11; // ecx@1
  int v12; // eax@1
  int v13; // ecx@1
  int v14; // eax@1
  signed int v15; // ecx@1
  int v16; // ebx@1
  signed __int64 v17; // qtt@3
  stru149 *v18; // eax@3
  int v19; // ebx@3
  Texture_MM7 *v20; // eax@4
  unsigned __int16 *v21; // eax@4
  Texture_MM7 *v22; // eax@6
  Texture_MM7 *v23; // ecx@8
  Texture_MM7 *v24; // eax@10
  stru149 *v25; // eax@12
  signed int v26; // eax@12
  int v27; // ecx@12
  int v28; // eax@14
  int v29; // edx@14
  int v30; // ecx@14
  Texture_MM7 *v31; // esi@14
  int v32; // edx@14
  int v33; // eax@14
  int v35; // [sp+Ch] [bp-8Ch]@1
  int v36; // [sp+10h] [bp-88h]@1
  stru316 a2; // [sp+14h] [bp-84h]@14
  stru315 a1; // [sp+3Ch] [bp-5Ch]@1
  int v39; // [sp+80h] [bp-18h]@1
  int v40; // [sp+84h] [bp-14h]@1
  int v41; // [sp+88h] [bp-10h]@1
  int v42; // [sp+8Ch] [bp-Ch]@1
  int v43; // [sp+90h] [bp-8h]@2
  int v44; // [sp+94h] [bp-4h]@1

  v2 = _this;
  v3 = _this->pParent;
  v4 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - _this->field_A);
  v5 = v3->ptr_38;
  v44 = v4;
  v42 = v5->field_14;
  v44 = (unsigned __int64)(v42 * (signed __int64)v4) >> 16;
  v6 = v3->ptr_38;
  v7 = v44 + v6->field_C;
  v42 = v6->field_20;
  v39 = v7;
  v8 = ((unsigned __int64)(v42 * (signed __int64)v4) >> 16) + v3->ptr_38->field_18;
  v44 = v4;
  v41 = v8;
  v42 = v3->v_18.z;
  v44 = (unsigned __int64)(v42 * (signed __int64)v4) >> 16;
  v9 = v44 + v3->v_18.x;
  v35 = stru_5C6E00->Sin(pIndoorCamera->sRotationY);
  v10 = stru_5C6E00->Cos(pIndoorCamera->sRotationY);
  v11 = v2->field_8;
  v36 = v10;
  a1.field_28 = v2->field_C;
  v12 = pViewport->uScreenCenterX - v11;
  v13 = v3->field_24;
  v42 = pODMRenderParams->int_fov_rad_inv * v12;
  v14 = v3->v_18.y;
  v15 = -v13;
  v40 = v15;
  v16 = ((unsigned __int64)(v42 * (signed __int64)v14) >> 16) + v9;
  v44 = v16;
  if ( !v16 || (v43 = abs(v15 >> 14), v43 > abs(v16)) )
    return 0;
  LODWORD(v17) = v40 << 16;
  HIDWORD(v17) = v40 >> 16;
  v43 = v17 / v44;
  v18 = v3->ptr_38;
  v44 = v42;
  v40 = v18->field_10;
  v44 = v42;
  v40 = v39 + ((unsigned __int64)(v40 * (signed __int64)v42) >> 16);
  v39 = v3->ptr_38->field_1C;
  v44 = (unsigned __int64)(v39 * (signed __int64)v42) >> 16;
  v19 = v43;
  v41 += (unsigned __int64)(v39 * (signed __int64)v42) >> 16;
  a1.pTextureLOD = (unsigned __int16 *)v3->pTexture->pLevelOfDetail2;
  v44 = 2;
  if ( v43 < mipmapping_terrain_mm1 << 16 )
  {
    v20 = v3->pTexture;
    v44 = 0;
    v21 = (unsigned __int16 *)v20->pLevelOfDetail0_prolly_alpha_mask;
LABEL_11:
    a1.pTextureLOD = v21;
    goto LABEL_12;
  }
  if ( v43 < mipmapping_terrain_mm2 << 16 )
  {
    v22 = v3->pTexture;
    v44 = 1;
    v21 = (unsigned __int16 *)v22->pLevelOfDetail1;
    goto LABEL_11;
  }
  if ( v43 >= mipmapping_terrain_mm3 << 16 )
  {
    if ( !bUseLoResSprites )
    {
      v24 = v3->pTexture;
      v44 = 3;
      v21 = (unsigned __int16 *)v24->pLevelOfDetail3;
      goto LABEL_11;
    }
  }
  else
  {
    v23 = v3->pTexture;
    v44 = 2;
    a1.pTextureLOD = (unsigned __int16 *)v23->pLevelOfDetail2;
  }
LABEL_12:
  v43 = (unsigned __int64)(v40 * (signed __int64)v43) >> 16;
  v25 = v3->ptr_38;
  v43 = v19;
  a1.field_30 = ((unsigned __int64)(v40 * (signed __int64)v19) >> 16) - v25->field_24;
  v43 = (unsigned __int64)(v41 * (signed __int64)v19) >> 16;
  v26 = ((unsigned __int64)(v41 * (signed __int64)v19) >> 16) - v3->ptr_38->field_28;
  v27 = bUseLoResSprites + v44 + 2;
  a1.field_30 >>= v27;
  v41 = bUseLoResSprites + v44 + 2;
  a1.field_2C = v26 >> v27;
  if ( byte_80AA10 )
  {
    a1.field_14 = dword_80AA20 >> v27;
    a1.field_18 = dword_80AA1C >> v27;
    a1.field_1C = dword_80AA18 >> v27;
    a1.field_20 = dword_80AA14 >> v27;
  }
  v41 = pODMRenderParams->int_fov_rad_inv;
  v42 = (signed int)((unsigned __int64)(pODMRenderParams->int_fov_rad_inv * (signed __int64)v19) >> 16) >> v27;
  a1.field_4 = (unsigned __int64)(v42 * (signed __int64)v35) >> 16;
  v43 = (unsigned __int64)(v42 * (signed __int64)v36) >> 16;
  a1.field_0 = (unsigned __int64)(v42 * (signed __int64)v36) >> 16;
  a1.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&a2, v2, v19, v3, pODMRenderParams->terrain_gamma, 1u, 1);
  LOWORD(v19) = 0;
  v28 = v2->field_A;
  v29 = v2->field_A;
  a1.field_24 = v19;
  v30 = v2->field_8;
  a1.pColorBuffer = &render->pTargetSurface[v30 + render->uTargetSurfacePitch * v29];
  a1.pDepthBuffer = (unsigned int *)&render->pActiveZBuffer[v30 + 640 * v28];
  v31 = v3->pTexture;
  v32 = ((signed int)v31->uTextureWidth >> v44) - 1;
  v33 = ((signed int)v31->uTextureHeight >> v44 << 16) - 65536;
  a1.field_10 = v44 - v31->uWidthLn2 + 16;
  a1.field_C = v32;
  a1.field_8 = v33;
  if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
  {
    if ( byte_80AA10 )
      sr_sub_485A24(&a1, (stru315 *)&a2);
    else
      sr_sub_485975(&a1, (stru315 *)&a2);
  }
  else
  {
    if ( byte_80AA10 )
      sr_sub_4D705A(v1);
    else
      sr_sub_4D6FB0(v1);
  }
  return 1;
}

//----- (00484442) --------------------------------------------------------
signed int sr_sub_484442(Span *_this)
{
  int v1; // ebp@0
  Span *v2; // edi@1
  Polygon *v3; // esi@1
  int v4; // ecx@1
  stru149 *v5; // eax@1
  stru149 *v6; // eax@1
  int v7; // edx@1
  int v8; // eax@1
  int v9; // ebx@1
  int v10; // eax@1
  int v11; // ecx@1
  int v12; // eax@1
  int v13; // ecx@1
  signed int v14; // ecx@1
  int v15; // ebx@1
  signed __int64 v16; // qtt@3
  int v17; // ecx@3
  int v18; // ebx@3
  int v19; // eax@3
  signed int v20; // ebx@3
  unsigned __int16 *v21; // eax@4
  stru149 *v22; // eax@12
  signed int v23; // eax@12
  int v24; // ecx@12
  int v25; // ecx@14
  unsigned int *v26; // eax@14
  Texture_MM7 *v27; // esi@14
  signed int v28; // edi@14
  signed int v29; // eax@14
  signed int v31; // [sp+Ch] [bp-90h]@1
  int v32; // [sp+10h] [bp-8Ch]@1
  int v33; // [sp+14h] [bp-88h]@1
  stru316 v34; // [sp+18h] [bp-84h]@14
  stru315 v35; // [sp+40h] [bp-5Ch]@1
  int v36; // [sp+84h] [bp-18h]@1
  int v37; // [sp+88h] [bp-14h]@1
  int v38; // [sp+8Ch] [bp-10h]@1
  int v39; // [sp+90h] [bp-Ch]@1
  int v40; // [sp+94h] [bp-8h]@1
  int v41; // [sp+98h] [bp-4h]@1

  v2 = _this;
  v3 = _this->pParent;
  v4 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - _this->field_A);
  v5 = v3->ptr_38;
  v40 = v4;
  v39 = v5->field_14;
  v40 = (unsigned __int64)(v39 * (signed __int64)v4) >> 16;
  v6 = v3->ptr_38;
  v7 = v40 + v6->field_C;
  v39 = v6->field_20;
  v36 = v7;
  v8 = ((unsigned __int64)(v39 * (signed __int64)v4) >> 16) + v3->ptr_38->field_18;
  v40 = v4;
  v38 = v8;
  v39 = v3->v_18.z;
  v40 = (unsigned __int64)(v39 * (signed __int64)v4) >> 16;
  v9 = v40 + v3->v_18.x;
  v32 = stru_5C6E00->Sin(pIndoorCamera->sRotationY);
  v10 = stru_5C6E00->Cos(pIndoorCamera->sRotationY);
  v11 = v2->field_8;
  v33 = v10;
  v35.field_28 = v2->field_C;
  v12 = pViewport->uScreenCenterX - v11;
  v13 = v3->field_24;
  v41 = pODMRenderParams->int_fov_rad_inv * v12;
  v14 = -v13;
  v31 = v14;
  v15 = ((unsigned __int64)(pODMRenderParams->int_fov_rad_inv * v12 * (signed __int64)v3->v_18.y) >> 16) + v9;
  v37 = v15;
  if ( !v15 || (v39 = abs(v14 >> 14), v39 > abs(v15)) )
    return 0;
  LODWORD(v16) = v31 << 16;
  HIDWORD(v16) = v31 >> 16;
  v40 = v16 / v37;
  v17 = v16 / v37;
  v18 = v41;
  HIWORD(v19) = (unsigned int)(v16 / v37) >> 16;
  LOWORD(v19) = 0;
  v35.field_24 = v19;
  v37 = v36 + ((unsigned __int64)(v3->ptr_38->field_10 * (signed __int64)v41) >> 16);
  v36 = v3->ptr_38->field_1C;
  v41 = (unsigned __int64)(v36 * (signed __int64)v41) >> 16;
  v38 += (unsigned __int64)(v36 * (signed __int64)v18) >> 16;
  v20 = 2;
  v35.pTextureLOD = (unsigned __int16 *)v3->pTexture->pLevelOfDetail2;
  if ( v17 < mipmapping_terrain_mm1 << 16 )
  {
    v20 = 0;
    v21 = (unsigned __int16 *)v3->pTexture->pLevelOfDetail0_prolly_alpha_mask;
LABEL_11:
    v35.pTextureLOD = v21;
    goto LABEL_12;
  }
  if ( v17 < mipmapping_terrain_mm2 << 16 )
  {
    v20 = 1;
    v21 = (unsigned __int16 *)v3->pTexture->pLevelOfDetail1;
    goto LABEL_11;
  }
  if ( v17 < mipmapping_terrain_mm3 << 16 )
  {
    v21 = (unsigned __int16 *)v3->pTexture->pLevelOfDetail2;
    goto LABEL_11;
  }
  if ( !bUseLoResSprites )
  {
    v20 = 3;
    v21 = (unsigned __int16 *)v3->pTexture->pLevelOfDetail3;
    goto LABEL_11;
  }
LABEL_12:
  v41 = (unsigned __int64)(v37 * (signed __int64)v17) >> 16;
  v22 = v3->ptr_38;
  v41 = v17;
  v35.field_30 = ((unsigned __int64)(v37 * (signed __int64)v17) >> 16) - v22->field_24;
  v23 = ((unsigned __int64)(v38 * (signed __int64)v17) >> 16) - v3->ptr_38->field_28;
  v24 = bUseLoResSprites + v20 + 2;
  v35.field_30 >>= v24;
  v38 = bUseLoResSprites + v20 + 2;
  v35.field_2C = v23 >> v24;
  if ( byte_80AA10 )
  {
    v35.field_14 = dword_80AA20 >> v24;
    v35.field_18 = dword_80AA1C >> v24;
    v35.field_1C = dword_80AA18 >> v24;
    v35.field_20 = dword_80AA14 >> v24;
  }
  v38 = pODMRenderParams->int_fov_rad_inv;
  v39 = (signed int)((unsigned __int64)(pODMRenderParams->int_fov_rad_inv * (signed __int64)v40) >> 16) >> v24;
  v35.field_4 = (unsigned __int64)(v39 * (signed __int64)v32) >> 16;
  v41 = (unsigned __int64)(v39 * (signed __int64)v33) >> 16;
  v35.field_0 = (unsigned __int64)(v39 * (signed __int64)v33) >> 16;
  v35.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&v34, v2, v40, v3, pODMRenderParams->terrain_gamma, 1u, 1);
  v25 = v2->field_8;
  v26 = (unsigned int *)&render->pActiveZBuffer[v25 + 640 * v2->field_A];
  v35.pColorBuffer = &render->pTargetSurface[v25 + render->uTargetSurfacePitch * v2->field_A];
  v35.pDepthBuffer = v26;
  v27 = v3->pTexture;
  v28 = (signed int)v27->uTextureWidth >> v20;
  v29 = (signed int)v27->uTextureHeight >> v20;
  v35.field_10 = v20 - v27->uWidthLn2 + 16;
  v35.field_C = v28 - 1;
  v35.field_8 = (v29 << 16) - 65536;
  if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
  {
    if ( byte_80AA10 )
      sr_sub_485D3E(&v35, &v34);
    else
      sr_sub_485C89(&v35, &v34);
  }
  else
  {
    if ( byte_80AA10 )
      sr_sub_4D72EC(v1);
    else
      sr_sub_4D73DF(v1);
  }
  return 1;
}

//----- (004847EB) --------------------------------------------------------
int sr_sub_4847EB(Span *_this)
{
  stru315 *v1; // ebp@0
  int v2; // ebx@1
  int v3; // edx@1
  Polygon *v4; // esi@1
  int v5; // eax@1
  signed int v6; // eax@3
  int v7; // edi@3
  stru149 *v8; // eax@3
  stru149 *v9; // eax@3
  int v10; // edx@3
  int v11; // eax@3
  signed int v12; // ebx@3
  int v13; // ebx@4
  signed __int64 v14; // qtt@5
  int v15; // eax@5
  signed int v16; // ebx@5
  unsigned __int16 *v17; // ecx@6
  stru149 *v18; // eax@14
  stru149 *v19; // eax@14
  signed int v20; // eax@14
  int v21; // ecx@14
  Texture_MM7 *v22; // edx@16
  signed int v23; // eax@16
  int v24; // ebx@16
  int v25; // edi@17
  int v26; // ebx@17
  int v27; // eax@17
  int v28; // edi@17
  int v29; // eax@18
  signed __int64 v30; // qtt@19
  int v31; // ecx@19
  int v32; // eax@19
  stru149 *v33; // eax@21
  signed int v34; // edi@21
  int v35; // ebx@21
  signed int v36; // edi@21
  signed __int64 v37; // qtt@21
  signed int v38; // ebx@21
  int v39; // ebx@29
  int v40; // eax@32
  unsigned __int64 v41; // qax@32
  int v42; // edi@32
  int v43; // eax@33
  signed __int64 v44; // qtt@34
  int v45; // ecx@34
  int v46; // eax@34
  stru149 *v47; // eax@36
  signed int v48; // edi@36
  int v49; // eax@36
  int v50; // eax@47
  unsigned __int64 v51; // qax@47
  int v52; // edi@47
  int v53; // eax@48
  signed __int64 v54; // qtt@49
  int v55; // ecx@49
  int v56; // eax@49
  stru149 *v57; // eax@51
  signed int v58; // edi@51
  int v59; // eax@51
  Span *v61; // [sp+Ch] [bp-B0h]@1
  int v62; // [sp+10h] [bp-ACh]@3
  int v63; // [sp+14h] [bp-A8h]@3
  int v64; // [sp+18h] [bp-A4h]@3
  int v65; // [sp+1Ch] [bp-A0h]@3
  signed int v66; // [sp+20h] [bp-9Ch]@3
  signed int v67; // [sp+24h] [bp-98h]@3
  int v68; // [sp+28h] [bp-94h]@1
  char v69; // [sp+2Ch] [bp-90h]@1
  stru315 sr; // [sp+30h] [bp-8Ch]@1
  stru316 sr2; // [sp+74h] [bp-48h]@14
  int v72; // [sp+9Ch] [bp-20h]@5
  int v73; // [sp+A0h] [bp-1Ch]@1
  int v74; // [sp+A4h] [bp-18h]@3
  int v75; // [sp+A8h] [bp-14h]@21
  int v76; // [sp+ACh] [bp-10h]@1
  int v77; // [sp+B0h] [bp-Ch]@14
  int v78; // [sp+B4h] [bp-8h]@3
  int v79; // [sp+B8h] [bp-4h]@3

  v2 = _this->field_A;
  v3 = _this->field_8;
  v4 = _this->pParent;
  v73 = 0;
  v61 = _this;
  sr.pDepthBuffer = (unsigned int *)&render->pActiveZBuffer[v3 + 640 * v2];
  v76 = v3;
  v5 = _this->field_C;
  sr.pColorBuffer = &render->pTargetSurface[v3 + render->uTargetSurfacePitch * v2];
  v68 = texmapping_terrain_subdivsize;
  v69 = texmapping_terrain_subdivpow2;
  if ( v5 >= texmapping_terrain_subdivsize )
    v73 = texmapping_terrain_subdivsize - (v3 & (texmapping_terrain_subdivsize - 1));
  v6 = v5 - v73;
  v67 = v6 >> texmapping_terrain_subdivpow2;
  v62 = v6 - (v6 >> texmapping_terrain_subdivpow2 << texmapping_terrain_subdivpow2);
  v7 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterX - v76);
  v8 = v4->ptr_38;
  v79 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterX - v76);
  v78 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - v2);
  v76 = v8->field_14;
  v78 = (unsigned __int64)(v76 * (signed __int64)(pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - v2))) >> 16;
  v9 = v4->ptr_38;
  v10 = v78 + v9->field_C;
  v76 = v9->field_20;
  v65 = v10;
  v11 = ((unsigned __int64)(v76 * (signed __int64)(pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - v2))) >> 16)
      + v4->ptr_38->field_18;
  v78 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - v2);
  v64 = v11;
  v76 = v4->v_18.z;
  v78 = (unsigned __int64)(v76 * (signed __int64)(pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - v2))) >> 16;
  v12 = -v4->field_24;
  v63 = v78 + v4->v_18.x;
  v66 = -v4->field_24;
  v78 = (unsigned __int64)(v79 * (signed __int64)v4->v_18.y) >> 16;
  v74 = v78 + v63;
  if ( !(v78 + v63) || (v76 = v12 >> 14, v13 = abs(v12 >> 14), v13 > abs(v74)) )
    return 0;
  LODWORD(v14) = v66 << 16;
  HIDWORD(v14) = v66 >> 16;
  v72 = v14 / v74;
  sr.pTextureLOD = (unsigned __int16 *)v4->pTexture->pLevelOfDetail2;
  v15 = v14 / v74;
  v16 = 2;
  if ( v15 >= mipmapping_terrain_mm1 << 16 )
  {
    if ( v15 >= mipmapping_terrain_mm2 << 16 )
    {
      if ( v15 >= mipmapping_terrain_mm3 << 16 )
      {
        if ( bUseLoResSprites )
          goto LABEL_14;
        v16 = 3;
        v17 = (unsigned __int16 *)v4->pTexture->pLevelOfDetail3;
      }
      else
      {
        v17 = (unsigned __int16 *)v4->pTexture->pLevelOfDetail2;
      }
    }
    else
    {
      v16 = 1;
      v17 = (unsigned __int16 *)v4->pTexture->pLevelOfDetail1;
    }
  }
  else
  {
    v16 = 0;
    v17 = (unsigned __int16 *)v4->pTexture->pLevelOfDetail0_prolly_alpha_mask;
  }
  sr.pTextureLOD = v17;
LABEL_14:
  sr.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&sr2, v61, v15, v4, pODMRenderParams->terrain_gamma, 1u, 1);
  v18 = v4->ptr_38;
  v78 = v7;
  v79 = v18->field_10;
  v78 = v7;
  v77 = v65 + ((unsigned __int64)(v79 * (signed __int64)v7) >> 16);
  v79 = v4->ptr_38->field_1C;
  v79 = v64 + ((unsigned __int64)(v79 * (signed __int64)v7) >> 16);
  v78 = (unsigned __int64)(v77 * (signed __int64)v72) >> 16;
  v19 = v4->ptr_38;
  v78 = v72;
  sr.field_30 = ((unsigned __int64)(v77 * (signed __int64)v72) >> 16) - v19->field_24;
  v78 = (unsigned __int64)(v79 * (signed __int64)v72) >> 16;
  v20 = ((unsigned __int64)(v79 * (signed __int64)v72) >> 16) - v4->ptr_38->field_28;
  v21 = bUseLoResSprites + v16 + 2;
  sr.field_30 >>= v21;
  v78 = bUseLoResSprites + v16 + 2;
  sr.field_2C = v20 >> v21;
  if ( byte_80AA10 )
  {
    sr.field_14 = dword_80AA20 >> v21;
    sr.field_18 = dword_80AA1C >> v21;
    sr.field_1C = dword_80AA18 >> v21;
    sr.field_20 = dword_80AA14 >> v21;
  }
  v22 = v4->pTexture;
  v79 = (signed int)v22->uTextureWidth >> v16;
  v23 = (signed int)v22->uTextureHeight >> v16;
  v24 = v16 - v22->uWidthLn2 + 16;
  sr.field_8 = (v23 << 16) - 65536;
  sr.field_10 = v24;
  sr.field_C = v79 - 1;
  if ( v73 )
  {
    v25 = v7 - v73 * pODMRenderParams->int_fov_rad_inv;
    v26 = v25;
    v77 = v4->v_18.y;
    v79 = v25;
    v27 = (unsigned __int64)(v77 * (signed __int64)v25) >> 16;
    v28 = v27 + v63;
    v74 = v27 + v63;
    if ( !(v27 + v63) || (v77 = abs(v76), v29 = abs(v28), v77 > v29) )
      return 0;
    LODWORD(v30) = v66 << 16;
    HIDWORD(v30) = v66 >> 16;
    v77 = v30 / v74;
    v31 = v30 / v74;
    HIWORD(v32) = HIWORD(v72);
    if ( v72 <= v31 )
      HIWORD(v32) = HIWORD(v31);
    LOWORD(v32) = 0;
    v77 = v26;
    sr.field_24 = v32;
    v74 = v4->ptr_38->field_10;
    v74 = v65 + ((unsigned __int64)(v74 * (signed __int64)v26) >> 16);
    v77 = (unsigned __int64)(v74 * (signed __int64)v31) >> 16;
    v33 = v4->ptr_38;
    v77 = v26;
    v34 = ((unsigned __int64)(v74 * (signed __int64)v31) >> 16) - v33->field_24;
    v74 = v33->field_1C;
    v74 = v64 + ((unsigned __int64)(v74 * (signed __int64)v26) >> 16);
    v77 = (unsigned __int64)(v74 * (signed __int64)v31) >> 16;
    v35 = (v77 - v4->ptr_38->field_28) >> v78;
    v74 = v73 << 16;
    v36 = (v34 >> v78) - sr.field_30;
    LODWORD(v37) = v36 << 16;
    HIDWORD(v37) = v36 >> 16;
    v38 = v35 - sr.field_2C;
    sr.field_4 = v37 / (v73 << 16);
    v75 = v73 << 16;
    LODWORD(v37) = v38 << 16;
    HIDWORD(v37) = v38 >> 16;
    v77 = v37 / (v73 << 16);
    sr.field_0 = v37 / (v73 << 16);
    sr.field_28 = v73;
    if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
    {
      if ( byte_80AA10 )
        sr_sub_485A24(&sr, (stru315 *)&sr2);
      else
        sr_sub_485975(&sr, (stru315 *)&sr2);
    }
    else
    {
      if ( byte_80AA10 )
        sr_sub_4D705A(v1);
      else
        sr_sub_4D6FB0(v1);
    }
    v39 = v79 - (pODMRenderParams->int_fov_rad_inv << v69);
  }
  else
  {
    v79 = v7 - (pODMRenderParams->int_fov_rad_inv << v69);
    v39 = v7 - (pODMRenderParams->int_fov_rad_inv << v69);
  }
  sr.field_28 = v68;
  while ( v67 > 0 )
  {
    v40 = v4->v_18.y;
    --v67;
    v75 = v40;
    v41 = v40 * (signed __int64)v39;
    v79 = v41 >> 16;
    LODWORD(v41) = v41 >> 16;
    v42 = v41 + v63;
    v74 = v41 + v63;
    if ( !((int)v41 + v63) || (v75 = abs(v76), v43 = abs(v42), v75 > v43) )
      return 0;
    LODWORD(v44) = v66 << 16;
    HIDWORD(v44) = v66 >> 16;
    v73 = v44 / v74;
    v45 = v44 / v74;
    HIWORD(v46) = HIWORD(v72);
    if ( v72 <= v45 )
      HIWORD(v46) = HIWORD(v45);
    LOWORD(v46) = 0;
    v79 = v39;
    sr.field_24 = v46;
    v75 = v4->ptr_38->field_10;
    v75 = v65 + ((unsigned __int64)(v75 * (signed __int64)v39) >> 16);
    v79 = (unsigned __int64)(v75 * (signed __int64)v45) >> 16;
    v47 = v4->ptr_38;
    v79 = v39;
    v48 = ((unsigned __int64)(v75 * (signed __int64)v45) >> 16) - v47->field_24;
    v75 = v47->field_1C;
    v75 = v64 + ((unsigned __int64)(v75 * (signed __int64)v39) >> 16);
    v79 = (unsigned __int64)(v75 * (signed __int64)v45) >> 16;
    v49 = (((v79 - v4->ptr_38->field_28) >> v78) - sr.field_2C) >> v69;
    sr.field_4 = ((v48 >> v78) - sr.field_30) >> v69;
    sr.field_0 = v49;
    if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
    {
      if ( byte_80AA10 )
        sr_sub_485A24(&sr, (stru315 *)&sr2);
      else
        sr_sub_485975(&sr, (stru315 *)&sr2);
    }
    else
    {
      if ( byte_80AA10 )
        sr_sub_4D705A(v1);
      else
        sr_sub_4D6FB0(v1);
    }
    sr.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&sr2, v61, v73, v4, pODMRenderParams->terrain_gamma, 1u, 0);
    sr.field_28 = v68;
    v39 -= pODMRenderParams->int_fov_rad_inv << v69;
  }
  if ( !v62 )
    return 1;
  v50 = v4->v_18.y;
  v75 = v50;
  v51 = v50 * (signed __int64)v39;
  v79 = v51 >> 16;
  LODWORD(v51) = v51 >> 16;
  v52 = v51 + v63;
  v74 = v51 + v63;
  if ( (int)v51 + v63 )
  {
    v75 = abs(v76);
    v53 = abs(v52);
    if ( v75 <= v53 )
    {
      LODWORD(v54) = v66 << 16;
      HIDWORD(v54) = v66 >> 16;
      v76 = v54 / v74;
      v55 = v54 / v74;
      HIWORD(v56) = HIWORD(v72);
      if ( v72 <= v55 )
        HIWORD(v56) = HIWORD(v55);
      LOWORD(v56) = 0;
      v76 = v39;
      sr.field_24 = v56;
      v75 = v4->ptr_38->field_10;
      v75 = v65 + ((unsigned __int64)(v75 * (signed __int64)v39) >> 16);
      v76 = (unsigned __int64)(v75 * (signed __int64)v55) >> 16;
      v57 = v4->ptr_38;
      v76 = v39;
      v58 = ((unsigned __int64)(v75 * (signed __int64)v55) >> 16) - v57->field_24;
      v75 = v57->field_1C;
      v75 = v64 + ((unsigned __int64)(v75 * (signed __int64)v39) >> 16);
      v76 = (unsigned __int64)(v75 * (signed __int64)v55) >> 16;
      v59 = (((v76 - v4->ptr_38->field_28) >> v78) - sr.field_2C) >> v69;
      sr.field_4 = ((v58 >> v78) - sr.field_30) >> v69;
      sr.field_0 = v59;
      sr.field_28 = v62;
      if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
      {
        if ( byte_80AA10 )
          sr_sub_485A24(&sr, (stru315 *)&sr2);
        else
          sr_sub_485975(&sr, (stru315 *)&sr2);
      }
      else
      {
        if ( byte_80AA10 )
          sr_sub_4D705A(v1);
        else
          sr_sub_4D6FB0(v1);
      }
      return 1;
    }
  }
  return 0;
}

//----- (00485407) --------------------------------------------------------
signed int sr_sub_485407_prolly_odm_water_wavy(Span *a1)
{
  stru315 *v1; // ebp@0
  int v2; // eax@1
  int v3; // edx@1
  unsigned int v4; // edi@1
  Polygon *v5; // esi@1
  int v6; // ecx@1
  int v7; // ebx@1
  int v8; // ecx@1
  stru149 *v9; // eax@1
  stru149 *v10; // eax@1
  int v11; // edx@1
  int v12; // eax@1
  int v13; // eax@1
  int v14; // ecx@1
  signed int v15; // edi@1
  int v16; // edi@2
  signed __int64 v17; // qtt@3
  int v18; // edi@3
  int v19; // eax@3
  unsigned __int8 *pLOD; // eax@3
  Texture_MM7 *v21; // eax@4
  Texture_MM7 *v22; // eax@6
  Texture_MM7 *v23; // eax@8
  Texture_MM7 *v24; // eax@10
  stru149 *v25; // eax@13
  stru149 *v26; // eax@13
  signed int v27; // ebx@13
  stru149 *v28; // eax@13
  Texture_MM7 *v29; // esi@13
  signed int v30; // ebx@13
  signed int v31; // edi@13
  signed int v32; // edx@13
  signed int v33; // eax@13
  int v34; // esi@13
  int v35; // eax@13
  unsigned __int64 v36; // qax@13
  int v37; // eax@13
  unsigned __int64 v38; // qax@13
  char v39; // cl@14
  stru316 a2; // [sp+Ch] [bp-90h]@13
  stru315 a1a; // [sp+34h] [bp-68h]@1
  Span *v43; // [sp+78h] [bp-24h]@1
  int v44; // [sp+7Ch] [bp-20h]@1
  int v45; // [sp+80h] [bp-1Ch]@1
  int v46; // [sp+84h] [bp-18h]@1
  Span *v47; // [sp+88h] [bp-14h]@1
  int v48; // [sp+8Ch] [bp-10h]@1
  int v49; // [sp+90h] [bp-Ch]@3
  int i; // [sp+94h] [bp-8h]@3
  int X; // [sp+98h] [bp-4h]@1

  v2 = a1->field_A;
  v3 = a1->field_8;
  v4 = v3 + render->uTargetSurfacePitch * a1->field_A;
  v5 = a1->pParent;
  v43 = a1;
  v6 = a1->field_C;
  a1a.pColorBuffer = &render->pTargetSurface[v4];
  v46 = v6;
  v7 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterX - v3);
  v8 = pODMRenderParams->int_fov_rad_inv * (pViewport->uScreenCenterY - v2);
  a1a.pDepthBuffer = (unsigned int *)&render->pActiveZBuffer[v3 + 640 * v2];
  v9 = v5->ptr_38;
  v47 = (Span *)v7;
  X = v8;
  v48 = v9->field_14;
  X = (unsigned __int64)(v48 * (signed __int64)v8) >> 16;
  v10 = v5->ptr_38;
  v11 = X + v10->field_C;
  v48 = v10->field_20;
  v44 = v11;
  v12 = ((unsigned __int64)(v48 * (signed __int64)v8) >> 16) + v5->ptr_38->field_18;
  X = v8;
  v45 = v12;
  v48 = v5->v_18.z;
  X = (unsigned __int64)(v48 * (signed __int64)v8) >> 16;
  v13 = v5->v_18.y;
  v14 = X + v5->v_18.x;
  v15 = -v5->field_24;
  v48 = -v5->field_24;
  X = ((unsigned __int64)(v7 * (signed __int64)v13) >> 16) + v14;
  if ( !X || (v16 = abs(v15 >> 14), v16 > abs(X)) )
    return 0;
  LODWORD(v17) = v48 << 16;
  HIDWORD(v17) = v48 >> 16;
  i = v17 / X;
  v18 = v17 / X;
  HIWORD(v19) = (unsigned int)(v17 / X) >> 16;
  LOWORD(v19) = 0;
  a1a.field_24 = v19;
  pLOD = v5->pTexture->pLevelOfDetail2;
  a1a.pTextureLOD = (unsigned __int16 *)v5->pTexture->pLevelOfDetail2;
  v49 = 2;
  if ( v18 >= mipmapping_terrain_mm1 << 16 )
  {
    if ( v18 >= mipmapping_terrain_mm2 << 16 )
    {
      if ( v18 >= mipmapping_terrain_mm3 << 16 )
      {
        if ( bUseLoResSprites )
          goto LABEL_12;
        v24 = v5->pTexture;
        v49 = 3;
        pLOD = v24->pLevelOfDetail3;
      }
      else
      {
        v23 = v5->pTexture;
        v49 = 2;
        pLOD = v23->pLevelOfDetail2;
      }
    }
    else
    {
      v22 = v5->pTexture;
      v49 = 1;
      pLOD = v22->pLevelOfDetail1;
    }
  }
  else
  {
    v21 = v5->pTexture;
    v49 = 0;
    pLOD = v21->pLevelOfDetail0_prolly_alpha_mask;
  }
  a1a.pTextureLOD = (unsigned __int16 *)pLOD;
LABEL_12:
  if ( !pLOD )
    return 0;
  a1a.field_34_palette = (unsigned __int16 *)sr_sub_485E1F(&a2, v43, v18, v5, pODMRenderParams->terrain_gamma, 1u, 1);
  a1a.field_28 = 16;
  v43 = (Span *)(v46 >> 4);
  X = v7;
  v25 = v5->ptr_38;
  v48 = v46 - 16 * (v46 >> 4);
  v46 = v25->field_10;
  X = v7;
  v46 = v44 + ((unsigned __int64)(v46 * (signed __int64)v7) >> 16);
  v44 = v5->ptr_38->field_1C;
  v45 += (unsigned __int64)(v44 * (signed __int64)v7) >> 16;
  X = (unsigned __int64)(v46 * (signed __int64)v18) >> 16;
  v26 = v5->ptr_38;
  X = v18;
  v27 = ((unsigned __int64)(v46 * (signed __int64)v18) >> 16) - v26->field_24;
  X = (unsigned __int64)(v45 * (signed __int64)v18) >> 16;
  v28 = v5->ptr_38;
  v29 = v5->pTexture;
  v44 = bUseLoResSprites + v49 + 2;
  v30 = v27 >> v44;
  v31 = (signed int)(((unsigned __int64)(v45 * (signed __int64)v18) >> 16) - v28->field_28) >> v44;
  v32 = (signed int)v29->uTextureWidth >> v49;
  v33 = (signed int)v29->uTextureHeight >> v49;
  v34 = v49 - v29->uWidthLn2;
  a1a.field_8 = (v33 << 16) - 65536;
  X = 2 * pMiscTimer->uTotalGameTimeElapsed;
  v45 = pODMRenderParams->int_fov_rad_inv;
  a1a.field_10 = v34 + 16;
  a1a.field_C = v32 - 1;
  i = (unsigned __int64)(pODMRenderParams->int_fov_rad_inv * (signed __int64)i) >> 16;
  v47 = (Span *)(i >> v44);
  v35 = stru_5C6E00->Sin(pIndoorCamera->sRotationY);
  v36 = (signed int)v47 * (signed __int64)v35;
  i = v36 >> 16;
  a1a.field_4 = v36 >> 16;
  v37 = stru_5C6E00->Cos(pIndoorCamera->sRotationY);
  v38 = (signed int)v47 * (signed __int64)v37;
  i = v38 >> 16;
  a1a.field_0 = v38 >> 16;
  if ( (signed int)v43 > 0 )
  {
    v47 = v43;
    v39 = 12 - v49;
    for ( i = 12 - v49; ; v39 = i )
    {
      a1a.field_30 = v30 + 4 * stru_5C6E00->Cos(X + (v31 >> v39));
      a1a.field_2C = v31 + 4 * stru_5C6E00->Sin(X + (v30 >> i));
      if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
        sr_sub_485975(&a1a, (stru315 *)&a2);
      else
        sr_sub_4D6FB0(v1);
      a1a.field_28 = 16;
      v30 += 16 * a1a.field_4;
      v31 += 16 * a1a.field_0;
      v47 = (Span *)((char *)v47 - 1);
      if ( !v47 )
        break;
    }
  }
  if ( v48 )
  {
    i = 12 - v49;
    a1a.field_30 = v30 + 4 * stru_5C6E00->Cos(X + (v31 >> (12 - v49)));
    a1a.field_2C = v31 + 4 * stru_5C6E00->Sin(X + (v30 >> i));
    a1a.field_28 = v48;
    if ( byte_4D864C && BYTE1(pEngine->uFlags) & 8 )
      sr_sub_485975(&a1a, (stru315 *)&a2);
    else
      sr_sub_4D6FB0(v1);
  }
  return 1;
}

//----- (0048585C) --------------------------------------------------------
signed int sr_sub_48585C_mb_DrawSpan(Span *a1, unsigned __int16 *pRenderTarget, int a4)
{
  Span *v3; // esi@1
  Polygon *v4; // edi@1
  int v5; // ebx@2
  signed __int64 v6; // qtt@3
  int v7; // ebx@3
  signed int v8; // ecx@3
  void *v9; // eax@5
  int v10; // ecx@5
  unsigned __int16 *pPixels; // [sp+Ch] [bp-14h]@1
  signed int v13; // [sp+10h] [bp-10h]@1
  int X; // [sp+18h] [bp-8h]@1

  v3 = a1;
  pPixels = pRenderTarget;
  v4 = a1->pParent;
  v13 = -v4->field_24;
  X = ((unsigned __int64)(pODMRenderParams->int_fov_rad_inv
                        * (pViewport->uScreenCenterX - (signed int)a1->field_8)
                        * (signed __int64)v4->v_18.y) >> 16)
    + ((unsigned __int64)(v4->v_18.z
                        * (signed __int64)(pODMRenderParams->int_fov_rad_inv
                                         * (pViewport->uScreenCenterY - (signed int)a1->field_A))) >> 16)
    + v4->v_18.x;
  if ( X && (v5 = abs(-v4->field_24 >> 14), v5 <= abs(X)) )
  {
    LODWORD(v6) = v13 << 16;
    HIDWORD(v6) = v13 >> 16;
    v7 = v6 / X;
    v8 = v6 / X;
  }
  else
  {
    v7 = pODMRenderParams->shading_dist_mist << 16;
    v8 = pODMRenderParams->shading_dist_mist << 16;
  }
  v9 = sr_sub_47C178(v8, v4, pODMRenderParams->terrain_gamma, a4);
  fill_pixels_fast(*((short *)v9 + v4->pTexture->uDecompressedSize), pPixels, v3->field_C);
  HIWORD(v10) = HIWORD(v7);
  LOWORD(v10) = 0;
  j_memset32(v10, &render->pActiveZBuffer[v3->field_8 + 640 * v3->field_A], v3->field_C);
  return 1;
}
//----- (004252E8) --------------------------------------------------------
signed int sr_4252E8(unsigned int uVertexID)
{
  unsigned int v1; // edx@1
  double v2; // st7@1
  char *v3; // edi@5
  char *v4; // esi@5
  char *v5; // ecx@5
  int v6; // ebx@6
  double v7; // st6@11
  double v8; // st5@11
  double v9; // st6@12
  unsigned __int8 v10; // c2@16
  unsigned __int8 v11; // c3@16
  void *v12; // edi@23
  double v13; // st6@23
  char *v14; // ecx@23
  char v15; // zf@24
  signed int result; // eax@25
  unsigned int v17; // [sp+8h] [bp-28h]@5
  bool v18; // [sp+Ch] [bp-24h]@2
  bool v19; // [sp+10h] [bp-20h]@7
  char *v20; // [sp+14h] [bp-1Ch]@5
  char *v21; // [sp+18h] [bp-18h]@5
  signed int v22; // [sp+1Ch] [bp-14h]@1
  RenderVertexSoft *v23; // [sp+20h] [bp-10h]@5
  char *v24; // [sp+24h] [bp-Ch]@5
  char *v25; // [sp+28h] [bp-8h]@5
  char *v26; // [sp+2Ch] [bp-4h]@5

  v1 = uVertexID;
  v2 = (double)pODMRenderParams->shading_dist_mist;
  memcpy(&VertexRenderList[uVertexID], VertexRenderList, sizeof(VertexRenderList[uVertexID]));
  v22 = 0;
  v18 = VertexRenderList[0].vWorldViewPosition.x <= v2;
  if ( (signed int)(uVertexID + 1) <= 1 )
    return 0;
  v3 = (char *)&array_507D30[0].vWorldViewPosition.z;
  v4 = (char *)&array_507D30[0].vWorldViewPosition.y;
  v25 = (char *)&array_507D30[0]._rhw;
  v23 = array_507D30;
  v20 = (char *)&array_507D30[0].vWorldViewPosition.z;
  v21 = (char *)&array_507D30[0].vWorldViewPosition.y;
  v24 = (char *)&array_507D30[0].vWorldViewPosition;
  v26 = (char *)&array_507D30[0].flt_2C;
  v5 = (char *)&VertexRenderList[0].vWorldViewPosition;
  v17 = v1;
  do
  {
    v6 = (int)(v5 + 48);
    v19 = v2 >= *((float *)v5 + 12);
    if ( v18 != v19 )
    {
      if ( v19 )
      {
        v7 = (v2 - *(float *)v5) / (*(float *)v6 - *(float *)v5);
        *(float *)v4 = (*((float *)v5 + 13) - *((float *)v5 + 1)) * v7 + *((float *)v5 + 1);
        *(float *)v3 = (*((float *)v5 + 14) - *((float *)v5 + 2)) * v7 + *((float *)v5 + 2);
        v8 = (*((float *)v5 + 20) - *((float *)v5 + 8)) * v7 + *((float *)v5 + 8);
      }
      else
      {
        v9 = (v2 - *(float *)v6) / (*(float *)v5 - *(float *)v6);
        *(float *)v4 = (*((float *)v5 + 1) - *((float *)v5 + 13)) * v9 + *((float *)v5 + 13);
        *(float *)v3 = (*((float *)v5 + 2) - *((float *)v5 + 14)) * v9 + *((float *)v5 + 14);
        v8 = (*((float *)v5 + 8) - *((float *)v5 + 20)) * v9 + *((float *)v5 + 20);
      }
      *(float *)v26 = v8;
      *(float *)v24 = v2;
      *(float *)v25 = 1.0 / v2;
      if ( v18 )
      {
        if ( v2 == *(float *)v5 && *(float *)v4 == *((float *)v5 + 1) )
        {
          v10 = 0;
          v11 = *(float *)v3 == *((float *)v5 + 2);
          goto LABEL_20;
        }
      }
      else
      {
        if ( v2 == *(float *)v6 && *(float *)v4 == *((float *)v5 + 13) )
        {
          v10 = 0;
          v11 = *(float *)v3 == *((float *)v5 + 14);
LABEL_20:
          if ( v11 | v10 )
            goto LABEL_22;
          goto LABEL_21;
        }
      }
LABEL_21:
      ++v23;
      v24 += 48;
      v26 += 48;
      v4 += 48;
      v3 += 48;
      ++v22;
      v25 += 48;
      v21 = v4;
      v20 = v3;
    }
LABEL_22:
    if ( v19 )
    {
      v12 = v23;
      v21 += 48;
      v20 += 48;
      v13 = 1.0 / (*(float *)v6 + 0.0000001);
      ++v22;
      v26 += 48;
      v24 += 48;
      ++v23;
      memcpy(v12, v5 + 36, 0x30u);
      v14 = v25;
      v25 += 48;
      v4 = v21;
      v3 = v20;
      *(float *)v14 = v13;
    }
    v15 = v17-- == 1;
    v18 = v19;
    v5 = (char *)v6;
  }
  while ( !v15 );
  result = v22;
  if ( v22 < 3 )
    return 0;
  return result;
}

//----- (004250FE) --------------------------------------------------------
signed int sr_4250FE(unsigned int uVertexID)
{
  unsigned int v1; // edx@1
  char *v2; // edi@5
  char *v3; // esi@5
  char *v4; // ecx@5
  int v5; // ebx@6
  double v6; // st6@11
  double v7; // st5@11
  double v8; // st6@12
  unsigned __int8 v9; // c2@16
  unsigned __int8 v10; // c3@16
  void *v11; // edi@23
  double v12; // st6@23
  char *v13; // ecx@23
  char v14; // zf@24
  signed int result; // eax@25
  unsigned int v16; // [sp+8h] [bp-28h]@5
  bool v17; // [sp+Ch] [bp-24h]@2
  bool v18; // [sp+10h] [bp-20h]@7
  char *v19; // [sp+14h] [bp-1Ch]@5
  char *v20; // [sp+18h] [bp-18h]@5
  signed int v21; // [sp+1Ch] [bp-14h]@1
  RenderVertexSoft *v22; // [sp+20h] [bp-10h]@5
  char *v23; // [sp+24h] [bp-Ch]@5
  char *v24; // [sp+28h] [bp-8h]@5
  char *v25; // [sp+2Ch] [bp-4h]@5

  v1 = uVertexID;
  memcpy(&VertexRenderList[uVertexID], VertexRenderList, sizeof(VertexRenderList[uVertexID]));
  v21 = 0;
  v17 = VertexRenderList[0].vWorldViewPosition.x >= 8.0;
  if ( (signed int)(uVertexID + 1) <= 1 )
    return 0;
  v2 = (char *)&array_507D30[0].vWorldViewPosition.z;
  v3 = (char *)&array_507D30[0].vWorldViewPosition.y;
  v24 = (char *)&array_507D30[0]._rhw;
  v22 = array_507D30;
  v19 = (char *)&array_507D30[0].vWorldViewPosition.z;
  v20 = (char *)&array_507D30[0].vWorldViewPosition.y;
  v23 = (char *)&array_507D30[0].vWorldViewPosition;
  v25 = (char *)&array_507D30[0].flt_2C;
  v4 = (char *)&VertexRenderList[0].vWorldViewPosition;
  v16 = v1;
  do
  {
    v5 = (int)(v4 + 48);
    v18 = *((float *)v4 + 12) >= 8.0;
    if ( v17 != v18 )
    {
      if ( v18 )
      {
        v6 = (8.0 - *(float *)v4) / (*(float *)v5 - *(float *)v4);
        *(float *)v3 = (*((float *)v4 + 13) - *((float *)v4 + 1)) * v6 + *((float *)v4 + 1);
        *(float *)v2 = (*((float *)v4 + 14) - *((float *)v4 + 2)) * v6 + *((float *)v4 + 2);
        v7 = (*((float *)v4 + 20) - *((float *)v4 + 8)) * v6 + *((float *)v4 + 8);
      }
      else
      {
        v8 = (8.0 - *(float *)v5) / (*(float *)v4 - *(float *)v5);
        *(float *)v3 = (*((float *)v4 + 1) - *((float *)v4 + 13)) * v8 + *((float *)v4 + 13);
        *(float *)v2 = (*((float *)v4 + 2) - *((float *)v4 + 14)) * v8 + *((float *)v4 + 14);
        v7 = (*((float *)v4 + 8) - *((float *)v4 + 20)) * v8 + *((float *)v4 + 20);
      }
      *(float *)v25 = v7;
      *(float *)v23 = 8.0;
      *(int *)v24 = 0x3E000000u;
      if ( v17 )
      {
        if ( 8.0 == *(float *)v4 && *(float *)v3 == *((float *)v4 + 1) )
        {
          v9 = 0;
          v10 = *(float *)v2 == *((float *)v4 + 2);
          goto LABEL_20;
        }
      }
      else
      {
        if ( 8.0 == *(float *)v5 && *(float *)v3 == *((float *)v4 + 13) )
        {
          v9 = 0;
          v10 = *(float *)v2 == *((float *)v4 + 14);
LABEL_20:
          if ( v10 | v9 )
            goto LABEL_22;
          goto LABEL_21;
        }
      }
LABEL_21:
      ++v22;
      v23 += 48;
      v25 += 48;
      v3 += 48;
      v2 += 48;
      ++v21;
      v24 += 48;
      v20 = v3;
      v19 = v2;
    }
LABEL_22:
    if ( v18 )
    {
      v11 = v22;
      v20 += 48;
      v19 += 48;
      v12 = 1.0 / (*(float *)v5 + 0.0000001);
      ++v21;
      v25 += 48;
      v23 += 48;
      ++v22;
      memcpy(v11, v4 + 36, 0x30u);
      v13 = v24;
      v24 += 48;
      v3 = v20;
      v2 = v19;
      *(float *)v13 = v12;
    }
    v14 = v16-- == 1;
    v17 = v18;
    v4 = (char *)v5;
  }
  while ( !v14 );
  result = v21;
  if ( v21 < 3 )
    return 0;
  return result;
}


//----- (004254D2) --------------------------------------------------------
int sr_4254D2(signed int a1)
{
  signed int v1; // ebx@2
  int v2; // ecx@2
  int result; // eax@11
  double v4; // ST74_8@12
  bool v5; // edi@12
  char *v6; // esi@13
  char *v7; // ebx@13
  double v8; // st7@15
  double v9; // st7@16
  double v10; // st6@16
  double v11; // st7@17
  float v12; // eax@18
  double v13; // ST4C_8@19
  double v14; // ST10_8@19
  double v15; // ST28_8@20
  double v16; // ST34_8@20
  char v17; // zf@20
  double v18; // ST18_8@21
  double v19; // ST3C_8@21
  double v20; // ST44_8@22
  double v21; // ST54_8@22
  void *v22; // edi@26
  double v23; // ST54_8@29
  bool v24; // esi@29
  char *v25; // edi@30
  char *v26; // ebx@30
  bool v27; // ecx@32
  double v28; // st7@33
  double v29; // st7@34
  double v30; // st6@34
  double v31; // st7@35
  double v32; // ST44_8@37
  double v33; // ST3C_8@37
  double v34; // ST18_8@38
  double v35; // ST34_8@38
  char v36; // zf@38
  double v37; // ST28_8@39
  double v38; // ST10_8@39
  double v39; // ST20_8@40
  double v40; // ST5C_8@40
  void *v41; // edi@44
  double v42; // ST5C_8@47
  bool v43; // edi@47
  char *v44; // esi@48
  char *v45; // ebx@48
  bool v46; // ecx@49
  double v47; // st6@50
  double v48; // st6@51
  double v49; // st5@51
  double v50; // st6@52
  double v51; // ST20_8@54
  double v52; // ST54_8@54
  double v53; // ST44_8@55
  double v54; // ST3C_8@55
  char v55; // zf@55
  double v56; // ST18_8@56
  double v57; // ST34_8@56
  double v58; // ST28_8@57
  double v59; // ST10_8@57
  void *v60; // edi@61
  double v61; // ST5C_8@64
  bool v62; // edi@64
  char *v63; // esi@65
  char *v64; // ebx@65
  bool v65; // ecx@66
  double v66; // st6@67
  double v67; // st6@68
  double v68; // st5@68
  double v69; // st6@69
  double v70; // ST20_8@71
  double v71; // ST54_8@71
  double v72; // ST44_8@72
  double v73; // ST3C_8@72
  char v74; // zf@72
  double v75; // ST18_8@73
  double v76; // ST34_8@73
  double v77; // ST28_8@74
  double v78; // ST10_8@74
  void *v79; // edi@78
  int v80; // ebx@81
  char *v81; // ecx@82
  signed int v82; // esi@82
  float v83; // ST78_4@83
  double v84; // ST5C_8@83
  float v85; // eax@84
  double v86; // ST20_8@85
  double v87; // ST54_8@88
  float v88; // eax@89
  double v89; // ST44_8@90
  unsigned __int8 v90; // sf@94
  unsigned __int8 v91; // of@94
  int v92; // edx@97
  int v93; // esi@97
  int v94; // edi@97
  int v95; // eax@97
  int v96; // eax@104
  double v97; // [sp+4Ch] [bp-8Ch]@32
  double v98; // [sp+54h] [bp-84h]@49
  double v99; // [sp+54h] [bp-84h]@66
  float v100; // [sp+5Ch] [bp-7Ch]@1
  float v101; // [sp+60h] [bp-78h]@1
  float v102; // [sp+64h] [bp-74h]@1
  double v103; // [sp+6Ch] [bp-6Ch]@14
  float v104; // [sp+A8h] [bp-30h]@1
  signed int v105; // [sp+A8h] [bp-30h]@96
  bool v106; // [sp+ACh] [bp-2Ch]@14
  bool v107; // [sp+ACh] [bp-2Ch]@32
  bool v108; // [sp+ACh] [bp-2Ch]@49
  bool v109; // [sp+ACh] [bp-2Ch]@66
  signed int v110; // [sp+B0h] [bp-28h]@12
  char *v111; // [sp+B8h] [bp-20h]@30
  signed int v112; // [sp+B8h] [bp-20h]@48
  signed int v113; // [sp+B8h] [bp-20h]@65
  char *v114; // [sp+BCh] [bp-1Ch]@13
  signed int v115; // [sp+BCh] [bp-1Ch]@29
  signed int v116; // [sp+BCh] [bp-1Ch]@64
  signed int v117; // [sp+C0h] [bp-18h]@1
  char *v118; // [sp+C0h] [bp-18h]@30
  char *v119; // [sp+C0h] [bp-18h]@48
  char *v120; // [sp+C0h] [bp-18h]@65
  char *v121; // [sp+C4h] [bp-14h]@13
  char *v122; // [sp+C4h] [bp-14h]@30
  signed int v123; // [sp+C4h] [bp-14h]@47
  char *v124; // [sp+C8h] [bp-10h]@30
  char *v125; // [sp+C8h] [bp-10h]@48
  char *v126; // [sp+C8h] [bp-10h]@65
  signed int v127; // [sp+CCh] [bp-Ch]@2
  char *v128; // [sp+CCh] [bp-Ch]@13
  char *v129; // [sp+CCh] [bp-Ch]@30
  RenderVertexSoft *v130; // [sp+CCh] [bp-Ch]@48
  RenderVertexSoft *v131; // [sp+CCh] [bp-Ch]@65
  RenderVertexSoft *v132; // [sp+D0h] [bp-8h]@13
  char *v133; // [sp+D0h] [bp-8h]@30
  char *v134; // [sp+D0h] [bp-8h]@48
  char *v135; // [sp+D0h] [bp-8h]@65
  signed int v136; // [sp+D4h] [bp-4h]@1
  char *v137; // [sp+D4h] [bp-4h]@13
  RenderVertexSoft *v138; // [sp+D4h] [bp-4h]@30
  char *v139; // [sp+D4h] [bp-4h]@48
  char *v140; // [sp+D4h] [bp-4h]@65

  v101 = (double)(signed int)pViewport->uViewportTL_X;
  v100 = (double)(signed int)pViewport->uViewportBR_X;
  v117 = a1;
  v136 = 1;
  v104 = (double)(signed int)pViewport->uViewportTL_Y;
  v102 = (double)(pViewport->uViewportBR_Y + 1);
  if ( a1 <= 0 )
    return v117;
  v127 = a1;
  v1 = a1;
  memcpy(sr_508690, VertexRenderList, 4 * ((unsigned int)(48 * a1) >> 2));
  v2 = 0;
  do
  {
    if ( VertexRenderList[v2].vWorldViewProjX < (double)v101 || VertexRenderList[v2].vWorldViewProjX > (double)v100 )
      v136 = 0;
    if ( *(float *)(v2 * 48 + 5286956) < (double)v104 || *(float *)(v2 * 48 + 5286956) > (double)v102 )
      v136 = 0;
    ++v2;
    --v127;
  }
  while ( v127 );
  if ( v136 )
    return v117;
  v110 = 0;
  memcpy(&VertexRenderList[v1], VertexRenderList, sizeof(VertexRenderList[v1]));
  v4 = VertexRenderList[0].vWorldViewProjX + 6.7553994e15;
  v5 = SLODWORD(v4) >= (signed int)pViewport->uViewportTL_X;
  if ( v117 < 1 )
    goto LABEL_112;
  v6 = (char *)&array_50A2B0[0].vWorldViewProjY;
  v132 = array_50A2B0;
  v137 = (char *)&array_50A2B0[0].vWorldViewProjX;
  v128 = (char *)&array_50A2B0[0].vWorldViewProjY;
  v121 = (char *)&array_50A2B0[0].flt_2C;
  v114 = (char *)&array_50A2B0[0]._rhw;
  v7 = (char *)&VertexRenderList[0].flt_2C;
  do
  {
    v103 = *((float *)v7 + 7) + 6.7553994e15;
    v106 = SLODWORD(v103) >= (signed int)pViewport->uViewportTL_X;
    if ( !(v5 ^ v106) )
      goto LABEL_25;
    v8 = v101;
    if ( SLODWORD(v103) >= (signed int)pViewport->uViewportTL_X )
    {
      v9 = (v8 - *((float *)v7 - 5)) / (*((float *)v7 + 7) - *((float *)v7 - 5));
      *(float *)v6 = (*((float *)v7 + 8) - *((float *)v7 - 4)) * v9 + *((float *)v7 - 4);
      *(float *)v114 = (*((float *)v7 + 9) - *((float *)v7 - 3)) * v9 + *((float *)v7 - 3);
      v10 = (*((float *)v7 + 12) - *(float *)v7) * v9 + *(float *)v7;
    }
    else
    {
      v11 = (v8 - *((float *)v7 + 7)) / (*((float *)v7 - 5) - *((float *)v7 + 7));
      *(float *)v6 = (*((float *)v7 - 4) - *((float *)v7 + 8)) * v11 + *((float *)v7 + 8);
      *(float *)v114 = (*((float *)v7 - 3) - *((float *)v7 + 9)) * v11 + *((float *)v7 + 9);
      v10 = (*(float *)v7 - *((float *)v7 + 12)) * v11 + *((float *)v7 + 12);
    }
    *(float *)v121 = v10;
    *(int *)v137 = LODWORD(v101);
    LODWORD(v12) = *(int *)v6;
    if ( v5 )
    {
      v13 = v12 + 6.7553994e15;
      v14 = *((float *)v7 - 4) + 6.7553994e15;
      if ( LODWORD(v13) == LODWORD(v14) )
      {
        v15 = *(float *)v137 + 6.7553994e15;
        v16 = *((float *)v7 - 5) + 6.7553994e15;
        v17 = LODWORD(v15) == LODWORD(v16);
        goto LABEL_23;
      }
    }
    else
    {
      v18 = v12 + 6.7553994e15;
      v19 = *((float *)v7 + 8) + 6.7553994e15;
      if ( LODWORD(v18) == LODWORD(v19) )
      {
        v20 = *(float *)v137 + 6.7553994e15;
        v21 = *((float *)v7 + 7) + 6.7553994e15;
        v17 = LODWORD(v20) == LODWORD(v21);
LABEL_23:
        if ( v17 )
          goto LABEL_25;
        goto LABEL_24;
      }
    }
LABEL_24:
    v121 += 48;
    v114 += 48;
    v6 += 48;
    ++v110;
    ++v132;
    v128 = v6;
    v137 += 48;
LABEL_25:
    if ( v106 )
    {
      v22 = v132;
      v128 += 48;
      ++v110;
      v114 += 48;
      v121 += 48;
      v137 += 48;
      ++v132;
      memcpy(v22, v7 + 4, 0x30u);
      v6 = v128;
    }
    v5 = v106;
    v7 += 48;
    --v117;
  }
  while ( v117 );
  if ( v110 < 3
    || (v115 = 0,
        memcpy(&array_50A2B0[v110], array_50A2B0, sizeof(array_50A2B0[v110])),
        v23 = array_50A2B0[0].vWorldViewProjX + 6.7553994e15,
        v24 = SLODWORD(v23) <= (signed int)pViewport->uViewportBR_X,
        v110 < 1) )
    goto LABEL_112;
  v25 = (char *)&array_509950[0]._rhw;
  v138 = array_509950;
  v129 = (char *)&array_50A2B0[0].vWorldViewProjX;
  v118 = (char *)&array_50A2B0[0].vWorldViewProjY;
  v133 = (char *)&array_509950[0].vWorldViewProjX;
  v122 = (char *)&array_509950[0].vWorldViewProjY;
  v124 = (char *)&array_509950[0].flt_2C;
  v111 = (char *)&array_509950[0]._rhw;
  v26 = (char *)&array_50A2B0[0].flt_2C;
  while ( 2 )
  {
    v97 = *((float *)v26 + 7) + 6.7553994e15;
    v27 = SLODWORD(v97) <= (signed int)pViewport->uViewportBR_X;
    v107 = SLODWORD(v97) <= (signed int)pViewport->uViewportBR_X;
    if ( v24 != v27 )
    {
      v28 = v100;
      if ( SLODWORD(v97) <= (signed int)pViewport->uViewportBR_X )
      {
        v29 = (v28 - *((float *)v26 - 5)) / (*((float *)v26 + 7) - *((float *)v26 - 5));
        *(float *)v122 = (*((float *)v26 + 8) - *((float *)v26 - 4)) * v29 + *((float *)v26 - 4);
        *(float *)v25 = (*((float *)v26 + 9) - *((float *)v26 - 3)) * v29 + *((float *)v26 - 3);
        v30 = (*((float *)v26 + 12) - *(float *)v26) * v29 + *(float *)v26;
      }
      else
      {
        v31 = (v28 - *((float *)v26 + 7)) / (*((float *)v26 - 5) - *((float *)v26 + 7));
        *(float *)v122 = (*((float *)v26 - 4) - *((float *)v26 + 8)) * v31 + *((float *)v26 + 8);
        *(float *)v25 = (*((float *)v26 - 3) - *((float *)v26 + 9)) * v31 + *((float *)v26 + 9);
        v30 = (*(float *)v26 - *((float *)v26 + 12)) * v31 + *((float *)v26 + 12);
      }
      *(float *)v124 = v30;
      *(int *)v133 = LODWORD(v100);
      if ( v24 )
      {
        v32 = *(float *)v122 + 6.7553994e15;
        v33 = *((float *)v26 - 4) + 6.7553994e15;
        if ( LODWORD(v32) == LODWORD(v33) )
        {
          v34 = *(float *)v133 + 6.7553994e15;
          v35 = *((float *)v26 - 5) + 6.7553994e15;
          v36 = LODWORD(v34) == LODWORD(v35);
          goto LABEL_41;
        }
        goto LABEL_42;
      }
      v37 = *(float *)v118 + 6.7553994e15;
      v38 = *((float *)v26 + 8) + 6.7553994e15;
      if ( LODWORD(v37) != LODWORD(v38) )
        goto LABEL_42;
      v39 = *(float *)v129 + 6.7553994e15;
      v40 = *((float *)v26 + 7) + 6.7553994e15;
      v36 = LODWORD(v39) == LODWORD(v40);
LABEL_41:
      if ( !v36 )
      {
LABEL_42:
        v129 += 48;
        v118 += 48;
        v133 += 48;
        v122 += 48;
        v124 += 48;
        v111 += 48;
        ++v115;
        ++v138;
      }
    }
    if ( v27 )
    {
      v41 = v138;
      ++v115;
      v111 += 48;
      v124 += 48;
      v122 += 48;
      v133 += 48;
      v118 += 48;
      v129 += 48;
      ++v138;
      memcpy(v41, v26 + 4, 0x30u);
    }
    v24 = v107;
    v26 += 48;
    --v110;
    if ( v110 )
    {
      v25 = v111;
      continue;
    }
    break;
  }
  if ( v115 < 3
    || (v123 = 0,
        memcpy(&array_509950[v115], array_509950, sizeof(array_509950[v115])),
        v42 = array_509950[0].vWorldViewProjY + 6.7553994e15,
        v43 = SLODWORD(v42) >= (signed int)pViewport->uViewportTL_Y,
        v115 < 1) )
    goto LABEL_112;
  v44 = (char *)&array_508FF0[0].vWorldViewProjX;
  v130 = array_508FF0;
  v119 = (char *)&array_508FF0[0].vWorldViewProjX;
  v139 = (char *)&array_508FF0[0].vWorldViewProjY;
  v125 = (char *)&array_508FF0[0].flt_2C;
  v134 = (char *)&array_508FF0[0]._rhw;
  v45 = (char *)&array_509950[0].flt_2C;
  v112 = v115;
  while ( 2 )
  {
    v98 = *((float *)v45 + 8) + 6.7553994e15;
    v46 = SLODWORD(v98) >= (signed int)pViewport->uViewportTL_Y;
    v108 = SLODWORD(v98) >= (signed int)pViewport->uViewportTL_Y;
    if ( v43 != v46 )
    {
      v47 = v104;
      if ( SLODWORD(v98) >= (signed int)pViewport->uViewportTL_Y )
      {
        v48 = (v47 - *((float *)v45 - 4)) / (*((float *)v45 + 8) - *((float *)v45 - 4));
        *(float *)v44 = (*((float *)v45 + 7) - *((float *)v45 - 5)) * v48 + *((float *)v45 - 5);
        *(float *)v134 = (*((float *)v45 + 9) - *((float *)v45 - 3)) * v48 + *((float *)v45 - 3);
        v49 = (*((float *)v45 + 12) - *(float *)v45) * v48 + *(float *)v45;
      }
      else
      {
        v50 = (v47 - *((float *)v45 + 8)) / (*((float *)v45 - 4) - *((float *)v45 + 8));
        *(float *)v44 = (*((float *)v45 - 5) - *((float *)v45 + 7)) * v50 + *((float *)v45 + 7);
        *(float *)v134 = (*((float *)v45 - 3) - *((float *)v45 + 9)) * v50 + *((float *)v45 + 9);
        v49 = (*(float *)v45 - *((float *)v45 + 12)) * v50 + *((float *)v45 + 12);
      }
      *(float *)v125 = v49;
      *(float *)v139 = v104;
      if ( v43 )
      {
        v51 = v104 + 6.7553994e15;
        v52 = *((float *)v45 - 4) + 6.7553994e15;
        if ( LODWORD(v51) == LODWORD(v52) )
        {
          v53 = *(float *)v44 + 6.7553994e15;
          v54 = *((float *)v45 - 5) + 6.7553994e15;
          v55 = LODWORD(v53) == LODWORD(v54);
          goto LABEL_58;
        }
        goto LABEL_59;
      }
      v56 = v104 + 6.7553994e15;
      v57 = *((float *)v45 + 8) + 6.7553994e15;
      if ( LODWORD(v56) != LODWORD(v57) )
        goto LABEL_59;
      v58 = *(float *)v44 + 6.7553994e15;
      v59 = *((float *)v45 + 7) + 6.7553994e15;
      v55 = LODWORD(v58) == LODWORD(v59);
LABEL_58:
      if ( !v55 )
      {
LABEL_59:
        v139 += 48;
        v125 += 48;
        v134 += 48;
        v44 += 48;
        ++v123;
        ++v130;
        v119 = v44;
      }
    }
    if ( v46 )
    {
      v60 = v130;
      v119 += 48;
      ++v123;
      v134 += 48;
      v125 += 48;
      v139 += 48;
      ++v130;
      memcpy(v60, v45 + 4, 0x30u);
      v44 = v119;
    }
    v43 = v108;
    v45 += 48;
    --v112;
    if ( v112 )
      continue;
    break;
  }
  if ( v123 < 3
    || (v116 = 0,
        memcpy(&array_508FF0[v123], array_508FF0, sizeof(array_508FF0[v123])),
        v61 = array_508FF0[0].vWorldViewProjY + 6.7553994e15,
        v62 = SLODWORD(v61) <= (signed int)pViewport->uViewportBR_Y,
        v123 < 1) )
    goto LABEL_112;
  v63 = (char *)&sr_508690[0].vWorldViewProjX;
  v131 = sr_508690;
  v120 = (char *)&sr_508690[0].vWorldViewProjX;
  v140 = (char *)&sr_508690[0].vWorldViewProjY;
  v126 = (char *)&sr_508690[0].flt_2C;
  v135 = (char *)&sr_508690[0]._rhw;
  v64 = (char *)&array_508FF0[0].flt_2C;
  v113 = v123;
  while ( 2 )
  {
    v99 = *((float *)v64 + 8) + 6.7553994e15;
    v65 = SLODWORD(v99) <= (signed int)pViewport->uViewportBR_Y;
    v109 = SLODWORD(v99) <= (signed int)pViewport->uViewportBR_Y;
    if ( v62 != v65 )
    {
      v66 = v102;
      if ( SLODWORD(v99) <= (signed int)pViewport->uViewportBR_Y )
      {
        v67 = (v66 - *((float *)v64 - 4)) / (*((float *)v64 + 8) - *((float *)v64 - 4));
        *(float *)v63 = (*((float *)v64 + 7) - *((float *)v64 - 5)) * v67 + *((float *)v64 - 5);
        *(float *)v135 = (*((float *)v64 + 9) - *((float *)v64 - 3)) * v67 + *((float *)v64 - 3);
        v68 = (*((float *)v64 + 12) - *(float *)v64) * v67 + *(float *)v64;
      }
      else
      {
        v69 = (v66 - *((float *)v64 + 8)) / (*((float *)v64 - 4) - *((float *)v64 + 8));
        *(float *)v63 = (*((float *)v64 - 5) - *((float *)v64 + 7)) * v69 + *((float *)v64 + 7);
        *(float *)v135 = (*((float *)v64 - 3) - *((float *)v64 + 9)) * v69 + *((float *)v64 + 9);
        v68 = (*(float *)v64 - *((float *)v64 + 12)) * v69 + *((float *)v64 + 12);
      }
      *(float *)v126 = v68;
      *(float *)v140 = v102;
      if ( v62 )
      {
        v70 = v102 + 6.7553994e15;
        v71 = *((float *)v64 - 4) + 6.7553994e15;
        if ( LODWORD(v70) == LODWORD(v71) )
        {
          v72 = *(float *)v63 + 6.7553994e15;
          v73 = *((float *)v64 - 5) + 6.7553994e15;
          v74 = LODWORD(v72) == LODWORD(v73);
          goto LABEL_75;
        }
        goto LABEL_76;
      }
      v75 = v102 + 6.7553994e15;
      v76 = *((float *)v64 + 8) + 6.7553994e15;
      if ( LODWORD(v75) != LODWORD(v76) )
        goto LABEL_76;
      v77 = *(float *)v63 + 6.7553994e15;
      v78 = *((float *)v64 + 7) + 6.7553994e15;
      v74 = LODWORD(v77) == LODWORD(v78);
LABEL_75:
      if ( !v74 )
      {
LABEL_76:
        v140 += 48;
        v126 += 48;
        v135 += 48;
        v63 += 48;
        ++v116;
        ++v131;
        v120 = v63;
      }
    }
    if ( v65 )
    {
      v79 = v131;
      v120 += 48;
      ++v116;
      v135 += 48;
      v126 += 48;
      v140 += 48;
      ++v131;
      memcpy(v79, v64 + 4, 0x30u);
      v63 = v120;
    }
    v62 = v109;
    v64 += 48;
    --v113;
    if ( v113 )
      continue;
    break;
  }
  if ( v116 < 3 )
    goto LABEL_112;
  v80 = v116;
  memcpy(&sr_508690[v116], sr_508690, sizeof(sr_508690[v116]));
  if ( v116 > 0 )
  {
    v81 = (char *)&sr_508690[0].vWorldViewProjX;
    v82 = v116;
    do
    {
      LODWORD(v83) = *(int *)v81;
      *((float *)v81 - 3) = 1.0 / *((float *)v81 + 2);
      v84 = v83 + 6.7553994e15;
      if ( SLODWORD(v84) <= (signed int)pViewport->uViewportBR_X )
      {
        v86 = *(float *)v81 + 6.7553994e15;
        if ( SLODWORD(v86) >= (signed int)pViewport->uViewportTL_X )
          goto LABEL_88;
        v85 = v101;
      }
      else
      {
        v85 = v100;
      }
      *(int *)v81 = LODWORD(v85);
LABEL_88:
      v87 = *((float *)v81 + 1) + 6.7553994e15;
      if ( SLODWORD(v87) > (signed int)pViewport->uViewportBR_Y )
      {
        v88 = v102;
LABEL_92:
        *((int *)v81 + 1) = LODWORD(v88);
        goto LABEL_93;
      }
      v89 = *((float *)v81 + 1) + 6.7553994e15;
      if ( SLODWORD(v89) < (signed int)pViewport->uViewportTL_Y )
      {
        v88 = v104;
        goto LABEL_92;
      }
LABEL_93:
      v81 += 48;
      --v82;
    }
    while ( v82 );
  }
  v91 = __OFSUB__(v116, 3);
  v90 = v116 - 3 < 0;
  if ( v116 > 3 )
  {
    memcpy(&sr_508690[v116 + 1], &sr_508690[1], sizeof(sr_508690[v116 + 1]));
    if ( v116 > 0 )
    {
      v105 = 1;
      do
      {
        v92 = v105;
        v93 = v105 - 1;
        v94 = v105 + 1;
        v95 = v105 + 1;
        if ( v105 - 1 >= v80 )
          v93 -= v80;
        if ( v105 >= v80 )
          v92 = v105 - v80;
        if ( v94 >= v80 )
          v95 = v94 - v80;
        if ( (sr_508690[v92].vWorldViewProjX - sr_508690[v93].vWorldViewProjX)
           * (sr_508690[v95].vWorldViewProjY - sr_508690[v93].vWorldViewProjY)
           - (sr_508690[v95].vWorldViewProjX - sr_508690[v93].vWorldViewProjX)
           * (sr_508690[v92].vWorldViewProjY - sr_508690[v93].vWorldViewProjY) < 0.0 )
        {
          ++v105;
        }
        else
        {
          v96 = v105;
          if ( v105 < v80 || (v96 = v105 - v80, v105 - v80 < v80) )
            memcpy(&sr_508690[v96], &sr_508690[v96 + 1], 4 * ((unsigned int)(48 * v80 - 48 * v96) >> 2));
          --v80;
        }
      }
      while ( v105 - 1 < v80 );
    }
    v91 = __OFSUB__(v80, 3);
    v90 = v80 - 3 < 0;
  }
  if ( v90 ^ v91 )
LABEL_112:
    result = 0;
  else
    result = v80;
  return result;
}


//----- (00424579) --------------------------------------------------------
int sr_424579(int uFaceID, LightsData *a2)
{
  BLVFace *v2; // eax@1
  Vec3_short_ *v3; // ebx@1
  Vec3_short_ *v4; // esi@1
  unsigned int v5; // esi@3
  int v7; // ST1C_4@5
  int v8; // ST1C_4@5
  int v9; // ST1C_4@5
  int v10; // ST1C_4@5
  int v11; // esi@5
  int v12; // ST1C_4@5
  unsigned int v15; // ecx@8
  unsigned int v19; // ecx@13
  signed int v20; // ecx@16
  signed int result; // eax@20
  signed int v24; // edx@22
  unsigned int v25; // [sp+Ch] [bp-1Ch]@3
  float v26; // [sp+14h] [bp-14h]@3
  float v27; // [sp+1Ch] [bp-Ch]@3
  float v28; // [sp+20h] [bp-8h]@3
  signed int v29; // [sp+24h] [bp-4h]@3

  v2 = &pIndoor->pFaces[uFaceID];
  v3 = pIndoor->pVertices;
  v4 = &pIndoor->pVertices[*v2->pVertexIDs];
  if ( v2->pFacePlane_old.vNormal.x * ((signed __int16)*(int *)&v4->x - pBLVRenderParams->vPartyPos.x)
     + v2->pFacePlane_old.vNormal.y * ((signed __int16)(*(int *)&v4->x >> 16) - pBLVRenderParams->vPartyPos.y)
     + v2->pFacePlane_old.vNormal.z * (v4->z - pBLVRenderParams->vPartyPos.z) < 0 )
  {
    PortalFace.field_0 = 1;
  }
  else
  {
    PortalFace.field_0 = 0;
    if ( !(v2->uAttributes & 1) )
      return 0;
  }
  v29 = 0;
  v5 = v2->uNumVertices;
  __asm { fld     pBLVRenderParams->fCosineY }
  v28 = pBLVRenderParams->fSineY;
  v26 = pBLVRenderParams->fCosineNegX;
  v27 = pBLVRenderParams->fSineNegX;
  v25 = v5;
  if ( (signed int)v5 > 0 )
  {
    _ECX = (char *)&VertexRenderList[0].vWorldPosition.z;
    do
    {
      v7 = v3[v2->pVertexIDs[v29]].x;
      __asm
      {
        fild    [ebp+var_10]
        fstp    dword ptr [ecx-8]
      }
      v8 = v3[v2->pVertexIDs[v29]].y;
      __asm
      {
        fild    [ebp+var_10]
        fstp    dword ptr [ecx-4]
      }
      v9 = v3[v2->pVertexIDs[v29]].z;
      __asm
      {
        fild    [ebp+var_10]
        fstp    dword ptr [ecx]
      }
      _ECX += 48;
      v10 = a2->pDeltaUV[0] + v2->pVertexUIDs[v29];
      __asm
      {
        fild    [ebp+var_10]
        fstp    dword ptr [ecx-14h]
      }
      v11 = a2->pDeltaUV[1] + v2->pVertexVIDs[v29++];
      v12 = v11;
      v5 = v25;
      __asm
      {
        fild    [ebp+var_10]
        fstp    dword ptr [ecx-10h]
      }
    }
    while ( v29 < (signed int)v25 );
  }
  _EDX = (char *)&VertexRenderList[0].vWorldViewPosition;
  if ( pBLVRenderParams->sPartyRotX )
  {
    if ( (signed int)v5 > 0 )
    {
      __asm
      {
        fild    pBLVRenderParams->vPartyPos.x
        fild    pBLVRenderParams->vPartyPos.y
        fild    pBLVRenderParams->vPartyPos.z
      }
      _EAX = (char *)&VertexRenderList[0].vWorldPosition.z;
      v15 = v5;
      do
      {
        __asm
        {
          fld     dword ptr [eax-8]
          fsub    st, st(3)
          fld     dword ptr [eax-4]
          fsub    st, st(3)
          fld     st(1)
          fmul    st, st(6)
          fld     st(1)
          fmul    [ebp+var_8]
          fsubp   st(1), st
          fstp    [ebp+var_4]
          fld     dword ptr [eax]
          fsub    st, st(3)
        }
        _EAX += 48;
        --v15;
        __asm
        {
          fstp    [ebp+var_10]
          fld     [ebp+var_4]
          fmul    [ebp+var_14]
          fld     [ebp+var_10]
          fmul    [ebp+var_C]
          fsubp   st(1), st
          fstp    dword ptr [eax-2Ch]
          fld     st(1)
          fmul    [ebp+var_8]
          fld     st(1)
          fmul    st, st(7)
          faddp   st(1), st
          fstp    dword ptr [eax-28h]
          fstp    st
          fstp    st
          fld     [ebp+var_4]
          fmul    [ebp+var_C]
          fld     [ebp+var_10]
          fmul    [ebp+var_14]
          faddp   st(1), st
          fstp    dword ptr [eax-24h]
        }
      }
      while ( v15 );
LABEL_15:
      __asm
      {
        fstp    st
        fstp    st
        fstp    st
      }
      goto LABEL_16;
    }
  }
  else
  {
    if ( (signed int)v5 > 0 )
    {
      __asm
      {
        fild    pBLVRenderParams->vPartyPos.x
        fild    pBLVRenderParams->vPartyPos.y
        fild    pBLVRenderParams->vPartyPos.z
      }
      _EAX = (char *)&VertexRenderList[0].vWorldViewPosition;
      v19 = v5;
      do
      {
        __asm
        {
          fld     dword ptr [eax-0Ch]
          fsub    st, st(3)
          fld     dword ptr [eax-8]
          fsub    st, st(3)
          fld     st(1)
          fmul    st, st(6)
          fld     st(1)
          fmul    [ebp+var_8]
          fsubp   st(1), st
          fstp    dword ptr [eax]
          fld     st(1)
          fmul    [ebp+var_8]
          fld     st(1)
          fmul    st, st(7)
        }
        _EAX += 48;
        --v19;
        __asm
        {
          faddp   st(1), st
          fstp    dword ptr [eax-2Ch]
          fstp    st
          fstp    st
          fld     dword ptr [eax-34h]
          fsub    st, st(1)
          fstp    dword ptr [eax-28h]
        }
      }
      while ( v19 );
      goto LABEL_15;
    }
  }
LABEL_16:
  v20 = 0;
  __asm { fstp    st }
  if ( (signed int)v5 <= 0 )
    return 0;
  do
  {
    __asm
    {
      fld     dword ptr [edx]
      fcomp   ds:flt_4D8524
      fnstsw  ax
    }
    if ( !(HIBYTE(_AX) & 1) )
      break;
    ++v20;
    _EDX += 48;
  }
  while ( v20 < (signed int)v5 );
  if ( v20 >= (signed int)v5 )
    return 0;
  result = ODM_NearClip(v5);
  if ( result > 0 )
  {
    __asm { fild    pBLVRenderParams->field_40 }
    _ECX = (char *)&array_507D30[0].vWorldViewPosition;
    v24 = result;
    __asm
    {
      fmul    ds:flt_4D84A4
      fild    pBLVRenderParams->uViewportCenterX
      fild    pBLVRenderParams->uViewportCenterY
    }
    do
    {
      __asm
      {
        fld1
        fdiv    dword ptr [ecx]
      }
      _ECX += 48;
      --v24;
      __asm
      {
        fld     st
        fmul    dword ptr [ecx-2Ch]
        fmul    st, st(4)
        fsubr   st, st(3)
        fstp    dword ptr [ecx-24h]
        fmul    dword ptr [ecx-28h]
        fmul    st, st(3)
        fsubr   st, st(1)
        fstp    dword ptr [ecx-20h]
      }
    }
    while ( v24 );
    __asm
    {
      fstp    st
      fstp    st
      fstp    st
    }
  }
  memcpy(&array_507D30[result], array_507D30, sizeof(array_507D30[result]));
  return result;
}


//----- (004AD504) --------------------------------------------------------
int sr_4AD504(signed int sFaceID)
{
  int result; // eax@1
  signed int v2; // ebx@1
  BLVFace *v3; // esi@3
  Texture_MM7 *v4; // edi@6
  int v5; // eax@7
  int v6; // edx@7
  int v7; // ecx@7
  int v8; // ebx@8
  int v9; // eax@8
  int v10; // ebx@8
  unsigned int v11; // ebx@9
  int v12; // esi@10
  int v13; // eax@13
  unsigned __int16 *v14; // eax@13
  unsigned int v15; // eax@14
  unsigned __int16 *v16; // ebx@14
  int v17; // eax@15
  int v18; // ebx@15
  int v19; // esi@15
  int v20; // ecx@15
  int v21; // eax@15
  stru193_math *v22; // ebx@15
  int v23; // eax@15
  char *v24; // esi@16
  int v25; // eax@18
  int v26; // eax@18
  int v27; // eax@20
  signed int v28; // esi@20
  signed int v29; // edx@20
  signed int v30; // esi@20
  signed int v31; // edi@20
  int v32; // esi@20
  int v33; // eax@26
  int *v34; // esi@27
  unsigned __int16 *v35; // edi@27
  unsigned int v36; // edx@27
  int v37; // ebx@27
  char v38; // cl@27
  char v39; // ch@27
  int v40; // ebx@29
  int v41; // edx@29
  unsigned int v42; // ebx@30
  int v43; // edx@30
  int v44; // eax@33
  int *v45; // esi@34
  unsigned __int16 *v46; // edi@34
  unsigned int v47; // edx@34
  int v48; // ebx@34
  char v49; // cl@34
  char v50; // ch@34
  int v51; // ebx@36
  int v52; // edx@36
  unsigned __int16 v53; // bx@37
  int v54; // edx@37
  int v55; // eax@40
  int v56; // eax@41
  int *v57; // esi@42
  unsigned __int16 *v58; // edi@42
  unsigned int v59; // edx@42
  int v60; // ebx@42
  char v61; // cl@42
  char v62; // ch@42
  unsigned int v63; // ebx@44
  int v64; // edx@44
  unsigned int v65; // ebx@44
  int v66; // edx@44
  unsigned int v67; // ebx@45
  int v68; // edx@45
  int v69; // ebx@45
  int v70; // edx@45
  int v71; // eax@48
  int *v72; // esi@49
  unsigned __int16 *v73; // edi@49
  unsigned int v74; // edx@49
  int v75; // ebx@49
  char v76; // cl@49
  char v77; // ch@49
  unsigned int v78; // ebx@51
  int v79; // edx@51
  unsigned int v80; // ebx@51
  int v81; // edx@51
  unsigned int v82; // ebx@52
  int v83; // edx@52
  int v84; // ebx@52
  int v85; // edx@52
  unsigned __int8 *v86; // [sp+Ch] [bp-98h]@9
  unsigned __int8 *v87; // [sp+10h] [bp-94h]@9
  unsigned __int8 *v88; // [sp+14h] [bp-90h]@9
  unsigned __int8 *v89; // [sp+18h] [bp-8Ch]@9
  int v90; // [sp+1Ch] [bp-88h]@20
  BLVFace *v91; // [sp+20h] [bp-84h]@3
  int v92; // [sp+24h] [bp-80h]@7
  int i; // [sp+28h] [bp-7Ch]@7
  unsigned __int16 *v94; // [sp+2Ch] [bp-78h]@9
  unsigned int v95; // [sp+30h] [bp-74h]@1
  Texture_MM7 *v96; // [sp+34h] [bp-70h]@6
  int v97; // [sp+38h] [bp-6Ch]@15
  unsigned int v98; // [sp+3Ch] [bp-68h]@9
  int v99; // [sp+40h] [bp-64h]@9
  int v100; // [sp+44h] [bp-60h]@24
  int v101; // [sp+48h] [bp-5Ch]@10
  int v102; // [sp+4Ch] [bp-58h]@20
  int v103; // [sp+50h] [bp-54h]@20
  int v104; // [sp+54h] [bp-50h]@9
  unsigned __int8 *v105; // [sp+58h] [bp-4Ch]@20
  int v106; // [sp+5Ch] [bp-48h]@24
  int v107; // [sp+60h] [bp-44h]@20
  int v108; // [sp+64h] [bp-40h]@20
  int v109; // [sp+68h] [bp-3Ch]@20
  int v110; // [sp+6Ch] [bp-38h]@15
  int v111; // [sp+70h] [bp-34h]@20
  int a1; // [sp+74h] [bp-30h]@12
  int a2; // [sp+78h] [bp-2Ch]@9
  int *v114; // [sp+7Ch] [bp-28h]@16
  int v115; // [sp+80h] [bp-24h]@18
  int v116; // [sp+84h] [bp-20h]@7
  unsigned int v117; // [sp+88h] [bp-1Ch]@15
  unsigned int v118; // [sp+8Ch] [bp-18h]@24
  int *v119; // [sp+90h] [bp-14h]@13
  int v120; // [sp+94h] [bp-10h]@15
  unsigned int v121; // [sp+98h] [bp-Ch]@15
  unsigned __int16 *v122; // [sp+9Ch] [bp-8h]@15
  unsigned int v123; // [sp+A0h] [bp-4h]@13

  result = render->uTargetSurfacePitch;
  v2 = sFaceID;
  v95 = render->uTargetSurfacePitch;
  if ( sFaceID >= 0 )
  {
    if ( sFaceID < (signed int)pIndoor->uNumFaces )
    {
      v3 = &pIndoor->pFaces[sFaceID];
      v91 = &pIndoor->pFaces[sFaceID];
      if ( !render->pRenderD3D )
      {
        result = GetPortalScreenCoord(sFaceID);
        if ( result )
        {
          result = PortalFrustrum(result, &stru_F8A590, pBLVRenderParams->field_7C, v2);
          if ( result )
          {
            result = (int)v3->GetTexture();
            v4 = (Texture_MM7 *)result;
            v96 = (Texture_MM7 *)result;
            if ( result )
            {
              v5 = *(short *)(result + 38);
              LOBYTE(v3->uAttributes) |= 0x80u;
              v92 = v5;
              sr_4AE5F1(v2);
              ++pBLVRenderParams->field_84;
              v6 = Lights.pDeltaUV[0];
              v116 = Lights.pDeltaUV[1];
              v7 = 0;
              for ( i = bUseLoResSprites; v7 < Lights.uNumLightsApplied; *(int *)v9 = v10 )
              {
                v8 = v116;
                Lights._blv_lights_xs[v7] += v6;
                v9 = 4 * v7 + 16297672;
                v10 = v8 - Lights._blv_lights_ys[v7++];
              }
              v94 = sr_sub_47C24C_get_palette(v3, v92, 0, 1);
              result = stru_F8A590._viewport_space_y;
              a2 = stru_F8A590._viewport_space_y;
              v11 = stru_F8A590._viewport_space_y * render->uTargetSurfacePitch;
              v99 = 640 * stru_F8A590._viewport_space_y;
              v86 = v4->pLevelOfDetail0_prolly_alpha_mask;
              v87 = v4->pLevelOfDetail1;
              v88 = v4->pLevelOfDetail2;
              v89 = v4->pLevelOfDetail3;
              v98 = stru_F8A590._viewport_space_y * render->uTargetSurfacePitch;
              v104 = 2 * pBLVRenderParams->field_0_timer_;
              if ( stru_F8A590._viewport_space_y <= stru_F8A590._viewport_space_w )
              {
                v12 = 2 * stru_F8A590._viewport_space_y;
                v101 = 2 * stru_F8A590._viewport_space_y;
                while ( 1 )
                {
                  a1 = *(__int16 *)((char *)stru_F8A590.viewport_left_side + v12);
                  sr_4AE313(a1, result, &stru_F81018.field_0);
                  if ( LOBYTE(viewparams->field_20) )
                  {
                    v15 = v95 * (v12 - pBLVRenderParams->uViewportY);
                    v119 = &pBLVRenderParams->pTargetZBuffer[2
                                                          * (*(__int16 *)((char *)stru_F8A590.viewport_left_side + v12)
                                                           + 320 * (v12 - pBLVRenderParams->uViewportY))
                                                          - pBLVRenderParams->uViewportX];
                    v16 = &pBLVRenderParams->pRenderTarget[v15
                                                        + 2 * *(__int16 *)((char *)stru_F8A590.viewport_left_side + v12)
                                                        - pBLVRenderParams->uViewportX];
                    v14 = &pBLVRenderParams->pRenderTarget[v15
                                                        + 2 * *(__int16 *)((char *)stru_F8A590.viewport_right_side + v12)
                                                        - pBLVRenderParams->uViewportX];
                    v123 = (unsigned int)v16;
                  }
                  else
                  {
                    v13 = *(__int16 *)((char *)stru_F8A590.viewport_left_side + v12);
                    v119 = &pBLVRenderParams->pTargetZBuffer[v13 + v99];
                    v123 = (unsigned int)&pBLVRenderParams->pRenderTarget[v13 + v11];
                    v14 = &pBLVRenderParams->pRenderTarget[v11 + *(__int16 *)((char *)stru_F8A590.viewport_right_side + v12)];
                  }
                  v117 = (unsigned int)v14;
                  HIWORD(v17) = HIWORD(stru_F81018.field_0.field_0);
                  v18 = stru_F81018.field_0.field_10;
                  LOWORD(v17) = 0;
                  v97 = Lights.field_0 | v17;
                  v110 = sr_4AE491(SHIWORD(stru_F81018.field_0.field_4), SHIWORD(stru_F81018.field_0.field_8));
                  v19 = stru_F81018.field_0.field_4 >> i;
                  v20 = 11 - v18;
                  v116 = stru_F81018.field_0.field_8 >> i;
                  v21 = stru_F81018.field_0.field_8 >> i >> (11 - v18);
                  v22 = stru_5C6E00;
                  v122 = (unsigned __int16 *)v20;
                  v121 = (stru_F81018.field_0.field_4 >> i) + 4 * stru_5C6E00->Cos(v104 + v21);
                  v23 = stru_5C6E00->Sin(v104 + (v19 >> (char)v122));
                  v120 = v116 + 4 * v23;
                  if ( v123 < v117 )
                  {
                    v24 = (char *)&stru_F81018.field_34.field_8;
                    v114 = &stru_F81018.field_34.field_8;
                    a1 += 16;
                    while ( 1 )
                    {
                      sr_4AE313(a1, a2, (stru337_stru0 *)(v24 - 8));
                      v25 = *((int *)v24 - 2);
                      LOWORD(v25) = 0;
                      v115 = v97;
                      v26 = Lights.field_0 | v25;
                      if ( v97 <= (unsigned int)v26 )
                        v115 = v26;
                      v97 = v26;
                      v27 = *((int *)v24 - 11);
                      v28 = *((int *)v24 - 1);
                      v105 = (&v86)[4 * v27];
                      v111 = v27 + 16;
                      v29 = v4->uWidthMinus1;
                      v109 = v27 + v27 + 16 - v4->uWidthLn2;
                      v107 = v29 >> v27;
                      v30 = v28 >> i;
                      v102 = v4->uHeightMinus1 >> v27 << (v27 + 16);
                      v31 = *v114 >> i;
                      v122 = (unsigned __int16 *)(11 - v27);
                      v116 = v30 + 4 * stru_5C6E00->Cos(v104 + (v31 >> (11 - v27)));
                      v90 = v31 + 4 * stru_5C6E00->Sin(v104 + (v30 >> (char)v122));
                      v108 = (signed int)(v116 - v121) >> 4;
                      v103 = (v90 - v120) >> 4;
                      v32 = v123 + 32;
                      if ( LOBYTE(viewparams->field_20) )
                        v32 = v123 + 64;
                      if ( v32 > v117 )
                        v32 = v117;
                      v118 = v32;
                      v100 = sr_4AE491(*((short *)v114 - 1), *((short *)v114 + 1));
                      v106 = (signed int)(v32 - v123) >> 1;
                      if ( v110 >> 16 == v100 >> 16 )
                      {
                        v122 = sr_sub_47C24C_get_palette(v91, v92, v110 >> 16, 1);
                        if ( LOBYTE(viewparams->field_20) )
                        {
                          v44 = v123;
                          if ( v123 < v118 )
                          {
                            v45 = v119;
                            v46 = v122;
                            v47 = v121;
                            v48 = v120;
                            v49 = v111;
                            v50 = v109;
                            if ( v106 & 2 )
                            {
                              *v119 = v115;
                              v45 -= 2;
                              v44 = v123 + 4;
                              goto LABEL_37;
                            }
                            do
                            {
                              v44 += 8;
                              v51 = *(&v105[v107 & (v47 >> v49)] + ((v102 & (unsigned int)v48) >> v50));
                              v52 = v115;
                              LOWORD(v51) = v46[v51];
                              *v45 = v115;
                              v45[1] = v52;
                              v45[640] = v52;
                              v45[641] = v52;
                              *(short *)(v44 - 8) = v51;
                              *(short *)(v44 - 6) = v51;
                              *(short *)(v44 + 1272) = v51;
                              *(short *)(v44 + 1274) = v51;
                              v45[2] = v52;
                              v45[3] = v52;
                              v45[642] = v52;
                              v45[643] = v52;
                              v121 += v108;
                              v120 += v103;
                              v47 = v121;
                              v48 = v120;
LABEL_37:
                              v45 += 4;
                              v53 = v46[*(&v105[v107 & (v47 >> v49)] + ((v102 & (unsigned int)v48) >> v50))];
                              v54 = v108;
                              *(short *)(v44 - 4) = v53;
                              *(short *)(v44 - 2) = v53;
                              *(short *)(v44 + 1276) = v53;
                              *(short *)(v44 + 1278) = v53;
                              v121 += v54;
                              v120 += v103;
                              v47 = v121;
                              v48 = v120;
                            }
                            while ( v44 < v118 );
                            v123 = v44;
                            v119 = v45;
                          }
                        }
                        else
                        {
                          v33 = v123;
                          if ( v123 < v118 )
                          {
                            v34 = v119;
                            v35 = v122;
                            v36 = v121;
                            v37 = v120;
                            v38 = v111;
                            v39 = v109;
                            if ( v106 & 1 )
                            {
                              *v119 = v115;
                              --v34;
                              v33 = v123 + 2;
                              goto LABEL_30;
                            }
                            do
                            {
                              v33 += 4;
                              v40 = *(&v105[v107 & (v36 >> v38)] + ((v102 & (unsigned int)v37) >> v39));
                              v41 = v115;
                              LOWORD(v40) = v35[v40];
                              *v34 = v115;
                              *(short *)(v33 - 4) = v40;
                              v34[1] = v41;
                              v121 += v108;
                              v120 += v103;
                              v36 = v121;
                              v37 = v120;
LABEL_30:
                              v42 = (unsigned int)(&v105[v107 & (v36 >> v38)] + ((v102 & (unsigned int)v37) >> v39));
                              v34 += 2;
                              v43 = v108;
                              *(short *)(v33 - 2) = v35[*(char *)v42];
                              v121 += v43;
                              v120 += v103;
                              v36 = v121;
                              v37 = v120;
                            }
                            while ( v33 < v118 );
                            v123 = v33;
                            v119 = v34;
                          }
                        }
                      }
                      else
                      {
                        v55 = v110 - ((v100 - v110) >> 4);
                        v110 = (v100 - v110) >> 4;
                        v122 = (unsigned __int16 *)v55;
                        if ( LOBYTE(viewparams->field_20) )
                        {
                          v71 = v123;
                          if ( v123 < v118 )
                          {
                            v72 = v119;
                            v73 = v94;
                            v74 = v121;
                            v75 = v120;
                            v76 = v111;
                            v77 = v109;
                            if ( v106 & 2 )
                            {
                              *v119 = v115;
                              v72 += 2;
                              v71 = v123 + 4;
                              goto LABEL_52;
                            }
                            do
                            {
                              v78 = (v107 & (v74 >> v76)) + ((v102 & (unsigned int)v75) >> v77);
                              v79 = (int)((char *)v122 + v110);
                              v71 += 8;
                              v122 = (unsigned __int16 *)v79;
                              v80 = ((v79 & 0xFFFF0000u) >> 8) + v105[v78];
                              v81 = v115;
                              LOWORD(v80) = v73[v80];
                              *v72 = v115;
                              v72[1] = v81;
                              v72[640] = v81;
                              v72[641] = v81;
                              *(short *)(v71 - 8) = v80;
                              *(short *)(v71 - 6) = v80;
                              *(short *)(v71 + 1272) = v80;
                              *(short *)(v71 + 1274) = v80;
                              v72[2] = v81;
                              v72[3] = v81;
                              v72[642] = v81;
                              v72[643] = v81;
                              v121 += v108;
                              v120 += v103;
                              v74 = v121;
                              v75 = v120;
                              v72 += 4;
LABEL_52:
                              v82 = (v107 & (v74 >> v76)) + ((v102 & (unsigned int)v75) >> v77);
                              v83 = (int)((char *)v122 + v110);
                              v84 = v105[v82];
                              v122 = (unsigned __int16 *)v83;
                              LOWORD(v84) = v73[((v83 & 0xFFFF0000u) >> 8) + v84];
                              v85 = v108;
                              *(short *)(v71 - 4) = v84;
                              *(short *)(v71 - 2) = v84;
                              *(short *)(v71 + 1276) = v84;
                              *(short *)(v71 + 1278) = v84;
                              v121 += v85;
                              v120 += v103;
                              v74 = v121;
                              v75 = v120;
                            }
                            while ( v71 < v118 );
                            v123 = v71;
                            v119 = v72;
                          }
                        }
                        else
                        {
                          v56 = v123;
                          if ( v123 < v118 )
                          {
                            v57 = v119;
                            v58 = v94;
                            v59 = v121;
                            v60 = v120;
                            v61 = v111;
                            v62 = v109;
                            if ( v106 & 1 )
                            {
                              *v119 = v115;
                              ++v57;
                              v56 = v123 + 2;
                              goto LABEL_45;
                            }
                            do
                            {
                              v63 = (v107 & (v59 >> v61)) + ((v102 & (unsigned int)v60) >> v62);
                              v64 = (int)((char *)v122 + v110);
                              v56 += 4;
                              v122 = (unsigned __int16 *)v64;
                              v65 = ((v64 & 0xFFFF0000u) >> 8) + v105[v63];
                              v66 = v115;
                              LOWORD(v65) = v58[v65];
                              *v57 = v115;
                              *(short *)(v56 - 4) = v65;
                              v57[1] = v66;
                              v121 += v108;
                              v120 += v103;
                              v59 = v121;
                              v60 = v120;
                              v57 += 2;
LABEL_45:
                              v67 = (v107 & (v59 >> v61)) + ((v102 & (unsigned int)v60) >> v62);
                              v68 = (int)((char *)v122 + v110);
                              v69 = v105[v67];
                              v122 = (unsigned __int16 *)v68;
                              LOWORD(v69) = v58[((v68 & 0xFFFF0000u) >> 8) + v69];
                              v70 = v108;
                              *(short *)(v56 - 2) = v69;
                              v121 += v70;
                              v120 += v103;
                              v59 = v121;
                              v60 = v120;
                            }
                            while ( v56 < v118 );
                            v123 = v56;
                            v119 = v57;
                          }
                        }
                      }
                      v114 += 13;
                      v110 = v100;
                      a1 += 16;
                      v4 = v96;
                      v121 = v116;
                      v120 = v90;
                      if ( v123 >= v117 )
                        break;
                      v24 = (char *)v114;
                      v22 = stru_5C6E00;
                    }
                  }
                  ++a2;
                  v98 += v95;
                  result = a2;
                  v101 += 2;
                  v99 += 640;
                  if ( a2 > stru_F8A590._viewport_space_w )
                    break;
                  v12 = v101;
                  result = a2;
                  v11 = v98;
                }
              }
            }
          }
        }
      }
    }
  }
  return result;
}

//----- (004AE313) --------------------------------------------------------
int sr_4AE313(int viewport_space_x, int viewport_space_y, stru337_stru0 *p)
{
  int _dy; // ebx@1
  int _dx; // edi@1
  int v5; // ecx@1
  int v6; // esi@1
  int v7; // ST18_4@2
  signed __int64 v8; // qtt@3
  unsigned int v9; // ecx@3
  int result; // eax@8
  int v11; // [sp+Ch] [bp-8h]@1

  _dy = pBLVRenderParams->uViewportCenterY - viewport_space_y;
  _dx = pBLVRenderParams->uViewportCenterX - viewport_space_x;
  ++pBLVRenderParams->field_88;
  v5 = (pBLVRenderParams->uViewportCenterY - viewport_space_y) * Lights.vec_80.y + Lights.field_7C;
  v6 = v5 + _dx * Lights.vec_80.x;
  v11 = v5 + _dx * Lights.vec_80.x;
  if ( v5 + _dx * Lights.vec_80.x && (v7 = abs(Lights.vec_80.z) >> 14, v7 <= abs(v6)) )
  {
    LODWORD(v8) = Lights.vec_80.z << 16;
    HIDWORD(v8) = Lights.vec_80.z >> 16;
    v9 = v8 / v11;
  }
  else
  {
    v9 = 0x40000000u;
  }
  if ( (signed int)v9 >= Lights.field_34 )
    p->field_0 = v9;
  else
    p->field_0 = Lights.field_34;
  p->field_4 = ((unsigned __int64)((_dy * Lights.vec_8C.z + Lights.vec_8C.x + _dx * Lights.vec_8C.y)
                                 * (signed __int64)(signed int)v9) >> 16)
             + Lights.field_98
             + (Lights.pDeltaUV[0] << 16);
  p->field_8 = ((unsigned __int64)((_dy * Lights.vec_9C.z + Lights.vec_9C.x + _dx * Lights.vec_9C.y)
                                 * (signed __int64)(signed int)v9) >> 16)
             + Lights.field_A8
             + (Lights.pDeltaUV[1] << 16);
  result = abs((__int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)(signed int)v9) >> 16);
  if ( result < 369620 || bUseLoResSprites )
  {
    if ( result < 184810 )
      p->field_10 = result > 92405;
    else
      p->field_10 = 2;
  }
  else
  {
    p->field_10 = 3;
  }
  return result;
}
//----- (004AE491) --------------------------------------------------------
int sr_4AE491(signed int a1, signed int a2)
{
  signed int v2; // eax@1
  signed int v3; // edi@1
  int v4; // ecx@1
  int v5; // esi@2
  int v6; // eax@2
  int v7; // ebx@2
  unsigned int v8; // ecx@2
  int v9; // edx@5
  int v10; // edx@7
  unsigned int v11; // eax@8
  int v13; // [sp+4h] [bp-18h]@2
  int v14; // [sp+8h] [bp-14h]@2
  signed int v15; // [sp+Ch] [bp-10h]@1
  int v16; // [sp+14h] [bp-8h]@1
  int v17; // [sp+18h] [bp-4h]@1

  v2 = a1 >> SLOBYTE(Lights.field_38);
  v3 = a2 >> SLOBYTE(Lights.field_38);
  v17 = Lights.uCurrentAmbientLightLevel;
  v4 = 0;
  v15 = v2;
  v16 = 0;
  if ( Lights.uNumLightsApplied > 0 )
  {
    do
    {
      v5 = v16;
      v13 = abs(v2 - Lights._blv_lights_xs[v16]);
      v14 = abs(v3 - Lights._blv_lights_ys[v16]);
      v6 = Lights._blv_lights_light_dot_faces[v16];
      v7 = v13;
      v8 = v14;
      if ( v6 < v13 )
      {
        v6 = v13;
        v7 = Lights._blv_lights_light_dot_faces[v16];
      }
      if ( v6 < v14 )
      {
        v9 = v6;
        v6 = v14;
        v8 = v9;
      }
      if ( v7 < (signed int)v8 )
      {
        v10 = v8;
        v8 = v7;
        v7 = v10;
      }
      v11 = ((unsigned int)(11 * v7) >> 5) + (v8 >> 2) + v6;
      if ( (signed int)v11 < Lights._blv_lights_radii[v5] )
        v17 += 30 * (v11 * Lights._blv_lights_inv_radii[v5] - 65536);
      ++v16;
      v2 = v15;
    }
    while ( v16 < Lights.uNumLightsApplied );
    v4 = 0;
  }
  if ( Lights.field_3E4 != v4 )
    v17 -= Lights.field_3E8 * (v2 - Lights.field_3F0) + Lights.field_3EC * (v3 - Lights.field_3F4);
  if ( v17 >= v4 )
  {
    if ( v17 > 2031616 )
      v17 = 2031616;
  }
  else
  {
    v17 = v4;
  }
  ++pBLVRenderParams->field_8C;
  return v17;
}

//----- (004AE5F1) --------------------------------------------------------
void sr_4AE5F1(unsigned int uFaceID)
{
  BLVFace *v1; // esi@1
  BLVFaceExtra *v2; // ebx@1
  int v3; // eax@1
  int v4; // edi@1
  Texture_MM7 *v5; // edi@1
  int v6; // eax@1
  unsigned int v7; // eax@1
  unsigned int v8; // ecx@1
  unsigned int v9; // eax@1
  unsigned int v10; // ecx@5
  int v11; // edi@10
  int v12; // ecx@10
  int v13; // eax@10
  int v14; // edx@10
  int v15; // ebx@12
  double v16; // st7@16
  int v17; // eax@16
  char *v18; // ebx@17
  int v19; // ecx@19
  int v20; // eax@19
  int v21; // edx@21
  int v22; // eax@23
  int v23; // ST04_4@26
  int v24; // edi@26
  double v25; // st6@26
  int v26; // eax@26
  double v27; // st6@26
  int v28; // ecx@26
  char v29; // al@26
  int v30; // edx@28
  int v31; // ecx@28
  BLVLightMM7 *v32; // ecx@32
  int v33; // edi@33
  int v34; // edx@33
  int v35; // eax@33
  int v36; // edi@35
  int v37; // edx@37
  int v38; // ebx@39
  int v39; // edi@42
  int v40; // eax@42
  char *v41; // ebx@45
  signed int v42; // ecx@47
  int v43; // edi@47
  int v44; // eax@49
  int v45; // edx@51
  int v46; // eax@53
  int v47; // ST04_4@55
  int v48; // edi@55
  double v49; // st6@55
  int v50; // eax@55
  double v51; // st6@55
  int v52; // eax@55
  int v53; // ecx@57
  int v54; // ecx@58
  int v55; // ecx@59
  int v56; // edx@62
  int v57; // ecx@62
  int v58; // eax@63
  int v59; // edx@64
  int v60; // ecx@67
  int v61; // edx@67
  int v62; // eax@68
  int v63; // edx@69
  signed int v64; // ecx@72
  double v65; // st7@75
  Vec3_int_ v66; // [sp+Ch] [bp-34h]@9
  Vec3_int_ v67; // [sp+18h] [bp-28h]@9
  BLVFaceExtra *v68; // [sp+24h] [bp-1Ch]@1
  int v69; // [sp+28h] [bp-18h]@10
  int v70; // [sp+2Ch] [bp-14h]@10
  int X; // [sp+30h] [bp-10h]@10
  int v72; // [sp+34h] [bp-Ch]@10
  int v73; // [sp+38h] [bp-8h]@10
  int v74; // [sp+3Ch] [bp-4h]@10

  v1 = &pIndoor->pFaces[uFaceID];
  v2 = &pIndoor->pFaceExtras[v1->uFaceExtraID];
  v3 = v1->uBitmapID;
  v4 = v1->uBitmapID;
  v68 = v2;
  v5 = (Texture_MM7 *)(v4 != -1 ? (int)&pBitmaps_LOD->pTextures[v3] : 0);
  v6 = 8 * uFaceID;
  LOBYTE(v6) = PID(OBJECT_BModel,uFaceID);
  Lights.field_0 = v6;
  Lights.plane_4.vNormal.x = v1->pFacePlane_old.vNormal.x;
  Lights.plane_4.vNormal.y = v1->pFacePlane_old.vNormal.y;
  Lights.plane_4.vNormal.z = v1->pFacePlane_old.vNormal.z;
  Lights.plane_4.dist = v1->pFacePlane_old.dist;
  Lights.pDeltaUV[0] = v2->sTextureDeltaU;
  Lights.pDeltaUV[1] = v2->sTextureDeltaV;
  v7 = GetTickCount();
  v8 = v1->uAttributes;
  v9 = v7 >> 3;
  if ( v8 & 4 )
  {
    Lights.pDeltaUV[1] -= v9 & v5->uHeightMinus1;
  }
  else
  {
    if ( v8 & 0x20 )
      Lights.pDeltaUV[1] += v9 & v5->uHeightMinus1;
  }
  v10 = v1->uAttributes;
  if ( BYTE1(v10) & 8 )
  {
    Lights.pDeltaUV[0] -= v9 & v5->uWidthMinus1;
  }
  else
  {
    if ( v10 & 0x40 )
      Lights.pDeltaUV[0] += v9 & v5->uWidthMinus1;
  }
  v1->_get_normals(&v67, &v66);
  Lights.vec_14.x = v67.x;
  Lights.vec_14.y = v67.y;
  Lights.vec_14.z = v67.z;
  Lights.vec_20.x = v66.x;
  Lights.vec_20.y = v66.y;
  Lights.vec_20.z = v66.z;
  Lights.uDefaultAmbientLightLevel = v2->field_22;
  if ( pBLVRenderParams->sPartyRotX )
  {
    v74 = (unsigned __int64)(Lights.plane_4.vNormal.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    v74 = ((unsigned __int64)(Lights.plane_4.vNormal.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16) - v74;
    X = (unsigned __int64)(Lights.plane_4.vNormal.z * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16;
    Lights.rotated_normal.x = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16) - X;
    Lights.rotated_normal.y = ((unsigned __int64)(Lights.plane_4.vNormal.x
                                                     * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                                 + ((unsigned __int64)(Lights.plane_4.vNormal.y
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.rotated_normal.z = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16)
                                 + ((unsigned __int64)(Lights.plane_4.vNormal.z
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16);
    v70 = (unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    v74 = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16) - v70;
    v70 = (unsigned __int64)(Lights.vec_14.z * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16;
    Lights.vec_60.y = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16) - v70;
    Lights.vec_60.z = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.field_6C = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16)
                         + ((unsigned __int64)(Lights.vec_14.z * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16);
    v70 = (unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    v74 = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16) - v70;
    X = (unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    v72 = (unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16;
    v70 = (unsigned __int64)(Lights.vec_20.z * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16;
    Lights.vec_70.x = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16) - v70;
    Lights.vec_70.y = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.vec_70.z = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16)
                         + ((unsigned __int64)(Lights.vec_20.z * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16);
    v74 = pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.y
        - pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.x;
    v11 = -(pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.y
          + pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.x);
    v73 = -65536 * pBLVRenderParams->vPartyPos.z;
    v70 = (unsigned __int64)(-65536 * pBLVRenderParams->vPartyPos.z * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16;
    v12 = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16) - v70;
    v69 = (unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16;
    v13 = pBLVRenderParams->vPartyPos.y;
    v70 = ((unsigned __int64)(v74 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16)
        + ((unsigned __int64)(-65536 * pBLVRenderParams->vPartyPos.z * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16);
    v14 = pBLVRenderParams->vPartyPos.x;
  }
  else
  {
    v70 = (unsigned __int64)(Lights.plane_4.vNormal.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    Lights.rotated_normal.x = ((unsigned __int64)(Lights.plane_4.vNormal.x
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                                 - v70;
    Lights.rotated_normal.z = Lights.plane_4.vNormal.z;
    Lights.rotated_normal.y = ((unsigned __int64)(Lights.plane_4.vNormal.x
                                                     * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                                 + ((unsigned __int64)(Lights.plane_4.vNormal.y
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    v70 = (unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    Lights.vec_60.y = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                         - v70;
    Lights.field_6C = Lights.vec_14.z;
    Lights.vec_60.z = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    v70 = (unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    Lights.vec_70.x = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                         - v70;
    v69 = (unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16;
    v14 = pBLVRenderParams->vPartyPos.x;
    Lights.vec_70.z = Lights.vec_20.z;
    v13 = pBLVRenderParams->vPartyPos.y;
    Lights.vec_70.y = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    v12 = pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.y
        - pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.x;
    v11 = -(pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.y
          + pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.x);
    v70 = -65536 * pBLVRenderParams->vPartyPos.z;
  }
  Lights.field_7C = Lights.rotated_normal.x;
  Lights.vec_60.x = Lights.plane_4.vNormal.z * pBLVRenderParams->vPartyPos.z
                       + Lights.plane_4.dist
                       + Lights.plane_4.vNormal.y * v13
                       + Lights.plane_4.vNormal.x * v14;
  Lights.vec_80.x = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44)
                                          * (signed __int64)Lights.rotated_normal.y) >> 16;
  Lights.vec_80.y = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44)
                                          * (signed __int64)Lights.rotated_normal.z) >> 16;
  Lights.vec_80.z = -Lights.vec_60.x;
  Lights.vec_8C.x = Lights.vec_60.y;
  Lights.vec_8C.y = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.vec_60.z) >> 16;
  Lights.vec_8C.z = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.field_6C) >> 16;
  X = (unsigned __int64)(Lights.vec_60.y * (signed __int64)v12) >> 16;
  v15 = v70;
  v70 = (unsigned __int64)(Lights.field_6C * (signed __int64)v70) >> 16;
  Lights.vec_9C.x = Lights.vec_70.x;
  Lights.field_98 = -(X + ((unsigned __int64)(Lights.vec_60.z * (signed __int64)v11) >> 16) + v70);
  Lights.vec_9C.y = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.vec_70.y) >> 16;
  Lights.vec_9C.z = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.vec_70.z) >> 16;
  X = (unsigned __int64)(Lights.vec_70.x * (signed __int64)v12) >> 16;
  v69 = (unsigned __int64)(Lights.vec_70.y * (signed __int64)v11) >> 16;
  v70 = (unsigned __int64)(Lights.vec_70.z * (signed __int64)v15) >> 16;
  Lights.field_38 = 0;
  Lights.field_A8 = -(X
                         + ((unsigned __int64)(Lights.vec_70.y * (signed __int64)v11) >> 16)
                         + ((unsigned __int64)(Lights.vec_70.z * (signed __int64)v15) >> 16));
  if ( *(int *)&v68->field_4 || *(int *)&v68->field_8 )
  {
    Lights.field_3E4 = 1;
    Lights.field_3E8 = *(int *)&v68->field_4;
    Lights.field_3EC = *(int *)&v68->field_8;
    Lights.field_3F0 = v68->field_1E;
    Lights.field_3F4 = v68->field_20;
  }
  else
  {
    Lights.field_3E4 = 0;
  }
  v16 = 0.0039215689;
  v17 = 116 * v1->uSectorID;
  v69 = v17;
  v74 = 0;
  v73 = 0;
  Lights.uCurrentAmbientLightLevel = (Lights.uDefaultAmbientLightLevel + *(__int16 *)((char *)&pIndoor->pSectors->uMinAmbientLightLevel + v17)) << 16;
  v70 = pMobileLightsStack->uNumLightsActive;
  if ( pMobileLightsStack->uNumLightsActive > 0 )
  {
    v18 = (char *)&pMobileLightsStack->pLights[0].vPosition.y;
    do
    {
      if ( v74 >= 20 )
        break;
      v19 = *((short *)v18 + 2);
      v20 = *((short *)v18 - 1);
      if ( v20 > v1->pBounding.x1 - v19 )
      {
        if ( v20 < v19 + v1->pBounding.x2 )
        {
          v21 = *(short *)v18;
          if ( v21 > v1->pBounding.y1 - v19 )
          {
            if ( v21 < v19 + v1->pBounding.y2 )
            {
              v22 = *((short *)v18 + 1);
              if ( v22 > v1->pBounding.z1 - v19 )
              {
                if ( v22 < v19 + v1->pBounding.z2 )
                {
                  X = (v1->pFacePlane_old.dist
                     + *((short *)v18 + 1) * v1->pFacePlane_old.vNormal.z
                     + v21 * v1->pFacePlane_old.vNormal.y
                     + *((short *)v18 - 1) * v1->pFacePlane_old.vNormal.x) >> 16;
                  if ( X <= v19 )
                  {
                    v23 = X;
                    v24 = v74;
                    Lights._blv_lights_radii[v74] = v19;
                    Lights._blv_lights_inv_radii[v24] = 65536 / v19;
                    *(int *)((char *)&Lights.field_240 + v24 * 4) = *((short *)v18 + 5) << 16;
                    Lights._blv_lights_xs[v24] = *((short *)v18 - 1);
                    Lights._blv_lights_ys[v24] = *(short *)v18;
                    Lights._blv_lights_zs[v24] = *((short *)v18 + 1);
                    v68 = (BLVFaceExtra *)(unsigned __int8)v18[6];
                    v25 = (double)(signed int)v68 * v16;
                    v68 = (BLVFaceExtra *)(unsigned __int8)v18[7];
                    v26 = (unsigned __int8)v18[8];
                    *(float *)(v24 * 4 + 16297992) = v25;
                    v27 = (double)(signed int)v68;
                    v68 = (BLVFaceExtra *)v26;
                    *(float *)(v24 * 4 + 16298072) = v27 * v16;
                    *(float *)(v24 * 4 + 16298152) = (double)(signed int)v68 * v16;
                    v16 = 0.0039215689;
                    Lights._blv_lights_light_dot_faces[v24] = abs(v23);
                    v28 = v74;
                    v29 = v18[9];
                    ++v74;
                    Lights._blv_lights_types[v28] = v29;
                  }
                }
              }
            }
          }
        }
      }
      ++v73;
      v18 += 18;
      v17 = v69;
    }
    while ( v73 < v70 );
  }
  v30 = 0;
  v73 = 0;
  v31 = *(__int16 *)((char *)&pIndoor->pSectors->uNumLights + v17);
  v70 = *(__int16 *)((char *)&pIndoor->pSectors->uNumLights + v17);
  if ( v31 > 0 )
  {
    while ( v74 < 20 )
    {
      v32 = &pIndoor->pLights[*(&(*(BLVLightMM7 **)((char *)&pIndoor->pSectors->pLights + v17))->vPosition.x + v30)];
      if ( !(v32->uAtributes & 8) )
      {
        v33 = v1->pBounding.x1;
        v34 = v32->vPosition.x;
        X = v32->uBrightness;
        v35 = v32->uRadius;
        v68 = (BLVFaceExtra *)v32->uRadius;
        if ( v34 > v33 - v35 )
        {
          if ( v34 < v35 + v1->pBounding.x2 )
          {
            v36 = v32->vPosition.y;
            if ( v36 > v1->pBounding.y1 - v35 )
            {
              if ( v36 < v35 + v1->pBounding.y2 )
              {
                v37 = v32->vPosition.z;
                if ( v37 > v1->pBounding.z1 - v35 )
                {
                  if ( v37 < v35 + v1->pBounding.z2 )
                  {
                    v38 = (v1->pFacePlane_old.dist
                         + v32->vPosition.x * v1->pFacePlane_old.vNormal.x
                         + v37 * v1->pFacePlane_old.vNormal.z
                         + v36 * v1->pFacePlane_old.vNormal.y) >> 16;
                    if ( v38 >= 0 )
                    {
                      if ( v38 <= v35 && v35 )
                      {
                        v39 = v74;
                        Lights._blv_lights_radii[v74] = v35;
                        Lights._blv_lights_inv_radii[v39] = 65536 / (signed int)v68;
                        *(int *)((char *)&Lights.field_240 + v39 * 4) = X << 16;
                        Lights._blv_lights_xs[v39] = v32->vPosition.x;
                        Lights._blv_lights_ys[v39] = v32->vPosition.y;
                        Lights._blv_lights_zs[v39] = v32->vPosition.z;
                        v68 = (BLVFaceExtra *)v32->uRed;
                        Lights._blv_lights_rs[v39] = (double)(signed int)v68 * v16;
                        v68 = (BLVFaceExtra *)v32->uGreen;
                        Lights._blv_lights_gs[v39] = (double)(signed int)v68 * v16;
                        v68 = (BLVFaceExtra *)v32->uBlue;
                        Lights._blv_lights_bs[v39] = (double)(signed int)v68 * v16;
                        v16 = 0.0039215689;
                        Lights._blv_lights_light_dot_faces[v39] = abs(v38);
                        v40 = v74++;
                        Lights._blv_lights_types[v40] = 1;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      v30 = v73++ + 1;
      if ( v73 >= v70 )
        break;
      v17 = v69;
    }
  }
  v73 = 0;
  if ( pStationaryLightsStack->uNumLightsActive > 0 )
  {
    v41 = (char *)&pStationaryLightsStack->pLights[0].vPosition.y;
    do
    {
      if ( v74 >= 20 )
        break;
      v42 = *((short *)v41 + 2);
      v43 = *((short *)v41 - 1);
      if ( v43 > v1->pBounding.x1 - v42 )
      {
        if ( v43 < v42 + v1->pBounding.x2 )
        {
          v44 = *(short *)v41;
          if ( v44 > v1->pBounding.y1 - v42 )
          {
            if ( v44 < v42 + v1->pBounding.y2 )
            {
              v45 = *((short *)v41 + 1);
              if ( v45 > v1->pBounding.z1 - v42 )
              {
                if ( v45 < v42 + v1->pBounding.z2 )
                {
                  v46 = (v1->pFacePlane_old.dist
                       + *(short *)v41 * v1->pFacePlane_old.vNormal.y
                       + v43 * v1->pFacePlane_old.vNormal.x
                       + v45 * v1->pFacePlane_old.vNormal.z) >> 16;
                  v69 = v46;
                  if ( v46 >= 0 )
                  {
                    if ( v46 <= v42 )
                    {
                      v47 = v69;
                      v48 = v74;
                      Lights._blv_lights_radii[v74] = v42;
                      Lights._blv_lights_inv_radii[v48] = 65536 / v42;
                      Lights._blv_lights_xs[v48] = *((short *)v41 - 1);
                      Lights._blv_lights_ys[v48] = *(short *)v41;
                      Lights._blv_lights_zs[v48] = *((short *)v41 + 1);
                      v68 = (BLVFaceExtra *)(unsigned __int8)v41[6];
                      v49 = (double)(signed int)v68 * v16;
                      v68 = (BLVFaceExtra *)(unsigned __int8)v41[7];
                      v50 = (unsigned __int8)v41[8];
                      Lights._blv_lights_rs[v48] = v49;
                      v51 = (double)(signed int)v68;
                      v68 = (BLVFaceExtra *)v50;
                      Lights._blv_lights_gs[v48] = v51 * v16;
                      Lights._blv_lights_bs[v48] = (double)(signed int)v68 * v16;
                      v16 = 0.0039215689;
                      Lights._blv_lights_light_dot_faces[v48] = abs(v47);
                      v52 = v74++;
                      Lights._blv_lights_types[v52] = 1;
                    }
                  }
                }
              }
            }
          }
        }
      }
      ++v73;
      v41 += 12;
    }
    while ( v73 < pStationaryLightsStack->uNumLightsActive );
  }
  Lights.uNumLightsApplied = v74;
  v53 = v1->pBounding.x2;
  if ( pBLVRenderParams->vPartyPos.x <= v53 )
  {
    v55 = v1->pBounding.x1;
    if ( pBLVRenderParams->vPartyPos.x >= v55 )
      v54 = 0;
    else
      v54 = v55 - pBLVRenderParams->vPartyPos.x;
  }
  else
  {
    v54 = pBLVRenderParams->vPartyPos.x - v53;
  }
  v56 = v1->pBounding.y2;
  v57 = v54 * v54;
  if ( pBLVRenderParams->vPartyPos.y <= v56 )
  {
    v59 = v1->pBounding.y1;
    if ( pBLVRenderParams->vPartyPos.y >= v59 )
      v58 = 0;
    else
      v58 = v59 - pBLVRenderParams->vPartyPos.y;
  }
  else
  {
    v58 = pBLVRenderParams->vPartyPos.y - v56;
  }
  v60 = v58 * v58 + v57;
  v61 = v1->pBounding.z2;
  if ( pBLVRenderParams->vPartyPos.z <= v61 )
  {
    v63 = v1->pBounding.z1;
    if ( pBLVRenderParams->vPartyPos.z >= v63 )
      v62 = 0;
    else
      v62 = v63 - pBLVRenderParams->vPartyPos.z;
  }
  else
  {
    v62 = pBLVRenderParams->vPartyPos.z - v61;
  }
  v64 = v62 * v62 + v60;
  if ( v64 )
    Lights.field_34 = integer_sqrt(v64) << 16;
  else
    Lights.field_34 = 0;
  v68 = (BLVFaceExtra *)abs(Lights.rotated_normal.y);
  v65 = (double)(signed int)v68;
  if ( v65 >= 655.36 )
  {
    if ( v65 >= 26214.4 )
    {
      if ( v65 >= 45875.2 )
      {
        Lights.field_44 = 8;
        Lights.field_48 = 3;
      }
      else
      {
        Lights.field_44 = 16;
        Lights.field_48 = 4;
      }
    }
    else
    {
      Lights.field_44 = 32;
      Lights.field_48 = 5;
    }
  }
  else
  {
    Lights.field_44 = 64;
    Lights.field_48 = 6;
  }
}

//----- (004AF412) --------------------------------------------------------
int  sr_4AF412()
{
  int v0; // ST20_4@2
  int v1; // ST20_4@2
  int v2; // ST20_4@2
  int v3; // esi@2
  int v4; // ST20_4@2
  int v5; // ecx@2
  int v6; // ebx@2
  int v7; // edi@2
  int v8; // edx@2
  int v9; // eax@2
  int result; // eax@4

  Lights.plane_4.vNormal.z = -65536;
  Lights.vec_20.y = -65536;
  Lights.plane_4.vNormal.x = 0;
  Lights.plane_4.vNormal.y = 0;
  Lights.plane_4.dist = (pBLVRenderParams->vPartyPos.z + 800) << 16;
  Lights.vec_14.x = 65536;
  Lights.vec_14.y = 0;
  Lights.vec_14.z = 0;
  Lights.vec_20.x = 0;
  Lights.vec_20.z = 0;
  Lights.uDefaultAmbientLightLevel = 0;
  if ( pBLVRenderParams->sPartyRotX )
  {
    v0 = ((unsigned __int64)(Lights.plane_4.vNormal.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
       - ((unsigned __int64)(Lights.plane_4.vNormal.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16);
    Lights.rotated_normal.x = ((unsigned __int64)(v0 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16)
                                 - ((unsigned __int64)(Lights.plane_4.vNormal.z
                                                     * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16);
    Lights.rotated_normal.y = ((unsigned __int64)(Lights.plane_4.vNormal.x
                                                     * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                                 + ((unsigned __int64)(Lights.plane_4.vNormal.y
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.rotated_normal.z = ((unsigned __int64)(v0 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16)
                                 + ((unsigned __int64)(Lights.plane_4.vNormal.z
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16);
    v1 = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
       - ((unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16);
    Lights.vec_60.y = ((unsigned __int64)(v1 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16)
                         - ((unsigned __int64)(Lights.vec_14.z * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16);
    Lights.vec_60.z = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.field_6C = ((unsigned __int64)(v1 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16)
                         + ((unsigned __int64)(Lights.vec_14.z * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16);
    v2 = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
       - ((unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16);
    Lights.vec_70.x = ((unsigned __int64)(v2 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16)
                         - ((unsigned __int64)(Lights.vec_20.z * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16);
    Lights.vec_70.y = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.vec_70.z = ((unsigned __int64)(v2 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16)
                         + ((unsigned __int64)(Lights.vec_20.z * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16);
    v3 = -(pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.y
         + pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.x);
    v4 = pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.y
       - pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.x;
    v5 = ((unsigned __int64)(v4 * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16)
       - ((unsigned __int64)(-65536 * pBLVRenderParams->vPartyPos.z * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16);
    v6 = pBLVRenderParams->vPartyPos.z;
    v7 = ((unsigned __int64)(-65536 * pBLVRenderParams->vPartyPos.z * (signed __int64)pIndoorCameraD3D->int_cosine_x) >> 16)
       + ((unsigned __int64)(v4 * (signed __int64)pIndoorCameraD3D->int_sine_x) >> 16);
    v8 = pBLVRenderParams->vPartyPos.y;
    v9 = pBLVRenderParams->vPartyPos.x;
  }
  else
  {
    Lights.rotated_normal.x = ((unsigned __int64)(Lights.plane_4.vNormal.x
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                                 - ((unsigned __int64)(Lights.plane_4.vNormal.y
                                                     * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16);
    Lights.rotated_normal.z = Lights.plane_4.vNormal.z;
    Lights.rotated_normal.y = ((unsigned __int64)(Lights.plane_4.vNormal.x
                                                     * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                                 + ((unsigned __int64)(Lights.plane_4.vNormal.y
                                                     * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.vec_60.y = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                         - ((unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16);
    Lights.field_6C = Lights.vec_14.z;
    Lights.vec_60.z = ((unsigned __int64)(Lights.vec_14.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_14.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.vec_70.x = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16)
                         - ((unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16);
    v8 = pBLVRenderParams->vPartyPos.y;
    Lights.vec_70.y = ((unsigned __int64)(Lights.vec_20.x * (signed __int64)pIndoorCameraD3D->int_sine_y) >> 16)
                         + ((unsigned __int64)(Lights.vec_20.y * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16);
    Lights.vec_70.z = Lights.vec_20.z;
    v9 = pBLVRenderParams->vPartyPos.x;
    v5 = pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.y
       - pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.x;
    v6 = pBLVRenderParams->vPartyPos.z;
    v3 = -(pIndoorCameraD3D->int_cosine_y * pBLVRenderParams->vPartyPos.y
         + pIndoorCameraD3D->int_sine_y * pBLVRenderParams->vPartyPos.x);
    v7 = -65536 * pBLVRenderParams->vPartyPos.z;
  }
  Lights.field_7C = Lights.rotated_normal.x;
  Lights.vec_60.x = Lights.plane_4.vNormal.y * v8
                       + Lights.plane_4.dist
                       + Lights.plane_4.vNormal.x * v9
                       + Lights.plane_4.vNormal.z * v6;
  Lights.vec_80.x = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44)
                                          * (signed __int64)Lights.rotated_normal.y) >> 16;
  Lights.vec_80.y = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44)
                                          * (signed __int64)Lights.rotated_normal.z) >> 16;
  Lights.vec_80.z = -Lights.vec_60.x;
  Lights.vec_8C.x = Lights.vec_60.y;
  Lights.vec_8C.y = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.vec_60.z) >> 16;
  Lights.vec_8C.z = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.field_6C) >> 16;
  Lights.vec_9C.x = Lights.vec_70.x;
  Lights.field_98 = -(((unsigned __int64)(Lights.vec_60.y * (signed __int64)v5) >> 16)
                         + ((unsigned __int64)(Lights.vec_60.z * (signed __int64)v3) >> 16)
                         + ((unsigned __int64)(Lights.field_6C * (signed __int64)v7) >> 16));
  Lights.vec_9C.y = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.vec_70.y) >> 16;
  Lights.vec_9C.z = (unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)Lights.vec_70.z) >> 16;
  result = 0;
  Lights.field_A8 = -(((unsigned __int64)(Lights.vec_70.x * (signed __int64)v5) >> 16)
                         + ((unsigned __int64)(Lights.vec_70.y * (signed __int64)v3) >> 16)
                         + ((unsigned __int64)(Lights.vec_70.z * (signed __int64)v7) >> 16));
  Lights.field_38 = 0;
  Lights.field_3E4 = 0;
  Lights.uCurrentAmbientLightLevel = 0;
  Lights.uNumLightsApplied = 0;
  Lights.field_34 = 0;
  return result;
}
//----- (004ADD1D) --------------------------------------------------------
void sr_4ADD1D(int uFaceID)
{
  int v1; // edi@1
  BLVFace *v2; // esi@3
  signed int v3; // ebx@4
  Texture_MM7 *v4; // edi@9
  signed int v5; // eax@9
  char *v6; // edi@12
  signed int v7; // eax@15
  unsigned int v8; // eax@16
  __int16 v9; // cx@19
  unsigned __int8 *v10; // eax@19
  unsigned __int16 *v11; // eax@19
  int v12; // edi@19
  int v13; // ebx@20
  stru352 *v14; // esi@20
  DWORD v15; // eax@22
  signed int v16; // ecx@22
  signed int v17; // ST68_4@22
  int v18; // eax@22
  int v19; // ecx@22
  unsigned int v20; // esi@23
  int v21; // edi@23
  int v22; // eax@23
  int *v23; // ebx@24
  int v24; // edx@24
  int v25; // ebx@25
  unsigned __int16 v26; // cx@25
  int v27; // edx@26
  unsigned __int16 v28; // cx@26
  unsigned __int8 v29; // sf@27
  unsigned __int8 v30; // of@27
  unsigned int v31; // esi@29
  int v32; // edi@29
  unsigned __int16 *v33; // eax@29
  int *v34; // ebx@30
  int v35; // edx@30
  int v36; // ebx@31
  unsigned __int16 v37; // cx@31
  int v38; // edx@32
  unsigned __int16 v39; // cx@32
  Texture_MM7 *v40; // [sp-10h] [bp-6Ch]@16
  int v41; // [sp-Ch] [bp-68h]@15
  unsigned int v42; // [sp+10h] [bp-4Ch]@1
  signed int v43; // [sp+14h] [bp-48h]@12
  signed int v44; // [sp+14h] [bp-48h]@22
  int v45; // [sp+1Ch] [bp-40h]@22
  int v46; // [sp+20h] [bp-3Ch]@22
  int v47; // [sp+24h] [bp-38h]@19
  char v48; // [sp+28h] [bp-34h]@19
  int v49; // [sp+2Ch] [bp-30h]@19
  unsigned __int8 *v50; // [sp+30h] [bp-2Ch]@19
  unsigned __int16 *v51; // [sp+34h] [bp-28h]@19
  int v52; // [sp+38h] [bp-24h]@22
  int v53; // [sp+3Ch] [bp-20h]@22
  signed int v54; // [sp+40h] [bp-1Ch]@12
  int v55; // [sp+40h] [bp-1Ch]@20
  int v56; // [sp+44h] [bp-18h]@20
  stru352 *i; // [sp+48h] [bp-14h]@20
  unsigned __int16 *v58; // [sp+4Ch] [bp-10h]@23
  int v59; // [sp+50h] [bp-Ch]@4
  int v60; // [sp+50h] [bp-Ch]@19
  int v61; // [sp+54h] [bp-8h]@22
  int *v62; // [sp+58h] [bp-4h]@23
  int *v63; // [sp+58h] [bp-4h]@29

  v1 = uFaceID;
  v42 = render->uTargetSurfacePitch;
  if ( uFaceID >= 0 && uFaceID < (signed int)pIndoor->uNumFaces )
  {
    v2 = &pIndoor->pFaces[uFaceID];
    if ( render->pRenderD3D )
    {
      v3 = sr_424579(uFaceID, &Lights);
      v59 = v3;
    }
    else
    {
      v59 = GetPortalScreenCoord(uFaceID);
      v3 = v59;
    }
    if ( v3 && (render->pRenderD3D || PortalFrustrum(v3, &stru_F8A590, pBLVRenderParams->field_7C, v1)) )
    {
      v4 = v2->GetTexture();
      v5 = 0;
      if ( v4 )
      {
        if ( render->pRenderD3D )
        {
          if ( v3 > 0 )
          {
            v54 = v3;
            v43 = v3;
            v6 = (char *)&array_507D30[0].v;
            do
            {
              *((float *)v6 - 1) = (double)((GetTickCount() >> 5) - pBLVRenderParams->vPartyPos.x) + *((float *)v6 - 1);
              *(float *)v6 = (double)(pBLVRenderParams->vPartyPos.y + (GetTickCount() >> 5)) + *(float *)v6;
              v6 += 48;
              --v54;
            }
            while ( v54 );
            v3 = v59;
            v5 = v43;
          }
          v7 = v5;
          v41 = Lights.field_0;
          array_507D30[v7].u = array_507D30[v7].u * 0.25;
          array_507D30[v7].v = array_507D30[v7].v * 0.25;
          if ( BYTE1(v2->uAttributes) & 0x40 )
          {
            v40 = v2->GetTexture();
            v8 = pTextureFrameTable->GetFrameTexture(v2->uBitmapID, pBLVRenderParams->field_0_timer_);
          }
          else
          {
            v40 = v2->GetTexture();
            v8 = v2->uBitmapID;
          }
          render->DrawIndoorPolygon(v3, v2, pBitmaps_LOD->pHardwareTextures[v8], v40, v41, -1, 0);
        }
        else
        {
          v49 = v4->uWidthMinus1;
          v47 = v4->uHeightMinus1 << 16;
          v9 = 16 - v4->uWidthLn2;
          v10 = v4->pLevelOfDetail0_prolly_alpha_mask;
          LOBYTE(v2->uAttributes) |= 0x80u;
          v48 = v9;
          v50 = v10;
          sr_4AF412();
          ++pBLVRenderParams->uNumFacesRenderedThisFrame;
          v11 = sr_sub_47C24C_get_palette(v2, v4->palette_id2, 0, 1);
          v12 = stru_F8A590._viewport_space_y;
          v51 = v11;
          v60 = stru_F8A590._viewport_space_y;
          if ( stru_F8A590._viewport_space_y <= stru_F8A590._viewport_space_w )
          {
            v13 = 2 * stru_F8A590._viewport_space_y;
            v14 = &stru_F83B80[stru_F8A590._viewport_space_y];
            v55 = 2 * stru_F8A590._viewport_space_y;
            v56 = 640 * stru_F8A590._viewport_space_y;
            for ( i = &stru_F83B80[stru_F8A590._viewport_space_y]; ; v14 = i )
            {
              sr_4AE1E7(v12, *(__int16 *)((char *)stru_F8A590.viewport_left_side + v13), v12);
              v14->field_0 += (GetTickCount() << 11) - (pBLVRenderParams->vPartyPos.x << 16);
              v15 = GetTickCount();
              v16 = v14->field_0;
              v14->field_4 += (32 * pBLVRenderParams->vPartyPos.y + v15) << 11;
              v45 = v14->field_4 >> 3;
              v44 = v16 >> 3;
              v17 = (signed int)((unsigned __int64)(SLODWORD(pBLVRenderParams->field_44) * (signed __int64)v14->field_28) >> 16) >> 3;
              v52 = (unsigned __int64)(v17 * (signed __int64)-pIndoorCameraD3D->int_sine_y) >> 16;
              v53 = (unsigned __int64)(v17 * (signed __int64)pIndoorCameraD3D->int_cosine_y) >> 16;
              v18 = v14->field_28;
              v19 = *(__int16 *)((char *)stru_F8A590.viewport_left_side + v13);
              LOWORD(v18) = 0;
              v46 = Lights.field_0 | v18;
              v61 = *(__int16 *)((char *)stru_F8A590.viewport_right_side + v13) - v19;
              if ( LOBYTE(viewparams->field_20) )
              {
                v63 = &pBLVRenderParams->pTargetZBuffer[2 * (v19 + 320 * (v13 - pBLVRenderParams->uViewportY))
                                                     - pBLVRenderParams->uViewportX];
                v31 = v44;
                v32 = v45;
                v33 = &pBLVRenderParams->pRenderTarget[v42 * (v13 - pBLVRenderParams->uViewportY)
                                                    + 2 * v19
                                                    - pBLVRenderParams->uViewportX];
                if ( v61 & 1 )
                {
                  --v61;
                  v33 = &pBLVRenderParams->pRenderTarget[v42 * (v13 - pBLVRenderParams->uViewportY)
                                                      + 2 * v19
                                                      - pBLVRenderParams->uViewportX
                                                      - 2];
                  v34 = &pBLVRenderParams->pTargetZBuffer[2 * (v19 + 320 * (v13 - pBLVRenderParams->uViewportY))
                                                       - pBLVRenderParams->uViewportX];
                  v35 = v46;
                  v63 += 2;
                  goto LABEL_32;
                }
                while ( 1 )
                {
                  v30 = __OFSUB__(v61, 2);
                  v29 = v61 - 2 < 0;
                  v61 -= 2;
                  if ( v29 ^ v30 )
                    break;
                  v36 = *(&v50[v49 & (v31 >> 16)] + ((v47 & (unsigned int)v32) >> v48));
                  v31 += v52;
                  v37 = v51[v36];
                  v32 += v53;
                  v34 = v63;
                  v35 = v46;
                  *v33 = v37;
                  v33[1] = v37;
                  v33[640] = v37;
                  v33[641] = v37;
                  v63 += 4;
                  v34[2] = v46;
                  v34[3] = v46;
                  v34[642] = v46;
                  v34[643] = v46;
LABEL_32:
                  *v34 = v35;
                  v34[1] = v35;
                  v34[640] = v35;
                  v34[641] = v35;
                  v38 = v49 & (v31 >> 16);
                  v33 += 4;
                  v31 += v52;
                  v39 = v51[*(&v50[v38] + ((v47 & (unsigned int)v32) >> v48))];
                  v32 += v53;
                  *(v33 - 2) = v39;
                  *(v33 - 1) = v39;
                  v33[638] = v39;
                  v33[639] = v39;
                }
              }
              else
              {
                v58 = &pBLVRenderParams->pRenderTarget[v19 + v12 * render->uTargetSurfacePitch];
                v62 = &pBLVRenderParams->pTargetZBuffer[v56 + v19];
                v20 = v44;
                v21 = v45;
                v22 = (int)v58;
                if ( v61 & 1 )
                {
                  --v61;
                  v22 = (int)(v58 - 1);
                  v23 = &pBLVRenderParams->pTargetZBuffer[v56 + v19];
                  v24 = v46;
                  ++v62;
                  goto LABEL_26;
                }
                while ( 1 )
                {
                  v30 = __OFSUB__(v61, 2);
                  v29 = v61 - 2 < 0;
                  v61 -= 2;
                  if ( v29 ^ v30 )
                    break;
                  v25 = *(&v50[v49 & (v20 >> 16)] + ((v47 & (unsigned int)v21) >> v48));
                  v20 += v52;
                  v26 = v51[v25];
                  v21 += v53;
                  v23 = v62;
                  v24 = v46;
                  *(short *)v22 = v26;
                  v62 += 2;
                  v23[1] = v46;
LABEL_26:
                  *v23 = v24;
                  v27 = v49 & (v20 >> 16);
                  v22 += 4;
                  v20 += v52;
                  v28 = v51[*(&v50[v27] + ((v47 & (unsigned int)v21) >> v48))];
                  v21 += v53;
                  *(short *)(v22 - 2) = v28;
                }
              }
              ++v60;
              ++i;
              v56 += 640;
              v55 += 2;
              if ( v60 > stru_F8A590._viewport_space_w )
                break;
              v13 = v55;
              v12 = v60;
            }
          }
        }
      }
    }
  }
}
//----- (004AE1E7) --------------------------------------------------------
int sr_4AE1E7(int a1, int a2, int a3)
{
  int v3; // ebx@1
  int v4; // edi@1
  int v5; // esi@1
  signed __int64 v6; // qtt@3
  int v7; // esi@3
  int v8; // eax@5
  int result; // eax@5
  int v10; // edx@5
  int v11; // [sp+Ch] [bp-8h]@1
  int v12; // [sp+1Ch] [bp+8h]@2

  v3 = pBLVRenderParams->uViewportCenterY - a3;
  v4 = pBLVRenderParams->uViewportCenterX - a2;
  v11 = a1;
  v5 = (pBLVRenderParams->uViewportCenterY - a3) * Lights.vec_80.y
     + Lights.field_7C
     + (pBLVRenderParams->uViewportCenterX - a2) * Lights.vec_80.x;
  if ( v5 && (v12 = abs(Lights.vec_80.z) >> 14, v12 <= abs(v5)) )
  {
    LODWORD(v6) = Lights.vec_80.z << 16;
    HIDWORD(v6) = Lights.vec_80.z >> 16;
    v7 = v6 / (v3 * Lights.vec_80.y + Lights.field_7C + v4 * Lights.vec_80.x);
  }
  else
  {
    v7 = 1073741824;
  }
  v8 = Lights.vec_9C.z;
  stru_F83B80[v11].field_0 = ((unsigned __int64)((v3 * Lights.vec_8C.z
                                                + Lights.vec_8C.x
                                                + v4 * Lights.vec_8C.y)
                                               * (signed __int64)v7) >> 16)
                           + Lights.field_98;
  result = (unsigned __int64)((v3 * v8 + Lights.vec_9C.x + v4 * Lights.vec_9C.y) * (signed __int64)v7) >> 16;
  v10 = result + Lights.field_A8;
  stru_F83B80[v11].field_28 = v7;
  stru_F83B80[v11].field_4 = v10;
  return result;
}
//----- (004AC1C9) --------------------------------------------------------
int  _4AC1C9_get_cpu_speed(unsigned int _this, Vec4_int_ *a2)
{
  unsigned int v2; // esi@1
  __int16 v3; // di@1
  signed int v4; // ebx@1
  int v5; // ecx@3
  Vec4_int_ *v6; // eax@10
  Vec4_int_ *v7; // esi@14
  int result; // eax@16
  char *v9; // esi@16
  Vec4_int_ a1; // [sp+Ch] [bp-34h]@10
  Vec4_int_ v11; // [sp+1Ch] [bp-24h]@14
  Vec4_int_ Dst; // [sp+2Ch] [bp-14h]@1
  int v13; // [sp+3Ch] [bp-4h]@1

  v2 = _this;
  v3 = _4382BC_get_cpu_speed_sub0(_this);
  v13 = _4383ED_get_cpu_speed_sub2();
  v4 = 0;
  memset(&Dst, 0, 0x10u);
  if ( v3 < 0 )
    goto LABEL_19;
  if ( (signed int)v2 > 0 )
  {
    if ( (signed int)v2 > 150 )
    {
      v5 = (int)a2;
    }
    else
    {
      v5 = 4000 * v2;
      v4 = 1;
    }
  }
  else
  {
    v5 = 4000 * dword_4F031C[(unsigned __int16)v3];
  }
  if ( v13 & 0x10 && !v4 )
  {
    if ( v2 )
      v6 = _4AC4FD_get_cpu_clocks_rdtsc(v5, &a1);
    else
      v6 = _4AC33A_get_cpu_clocks_QPC(v5, &a1);
    goto LABEL_14;
  }
  if ( (unsigned __int16)v3 < 3u )
  {
LABEL_19:
    v7 = &Dst;
    goto LABEL_16;
  }
  v6 = _4AC277_get_cpu_speed_sub3(v5, &a1);
LABEL_14:
  v11.x = v6->x;
  v11.y = v6->y;
  v11.z = v6->z;
  v11.w = v6->w;
  v7 = &v11;
LABEL_16:
  result = (int)a2;
  a2->x = v7->x;
  v9 = (char *)&v7->y;
  a2->y = *(int *)v9;
  v9 += 4;
  a2->z = *(int *)v9;
  a2->w = *((int *)v9 + 1);
  return result;
}
// 4F031C: using guessed type int dword_4F031C[];



//----- (004382BC) --------------------------------------------------------
__int16  _4382BC_get_cpu_speed_sub0(int _this)
{
  __int16 result; // ax@4
  __int16 v14; // sp@11
  int v15; // [sp-4h] [bp-Ch]@0
  signed __int16 v16; // [sp+4h] [bp-4h]@1
  __int16 v17; // [sp+4h] [bp-4h]@8
  __int16 v18; // [sp+4h] [bp-4h]@11

  v16 = 1;
  __asm { pushf }
  if ( v15 == (v15 ^ 0x200000) )
    v16 = 0;
  if ( v16 )
  {
    result = _43847A_get_cpu_speed_sub1();
  }
  else
  {
    //UNDEF(_ZF);
    //UNDEF(_SF);
    //UNDEF(_OF);
    _CF = 0;
    if ( !_ZF )
      _CF = 1;
    __asm { pushfw }
    bProcessorIsNotIntel = (BYTE2(v15) & 1 ^ 1) & 1;
    _CF = 0;
    _OF = 0;
    _ZF = 0;
    _SF = 1;
    __asm { pushfw }
    v17 = 2;
    if ( (HIWORD(v15) | 0xF000) & 0xF000 )
      v17 = -1;
    __asm { popfw }
    result = v17;
    if ( v17 != 2 )
    {
      _CF = 0;
      _OF = 0;
      _ZF = v14 == 0;
      _SF = v14 < 0;
      __asm { pushf }
      v18 = 3;
      if ( v15 != (v15 ^ 0x40000) )
        v18 = -1;
      __asm { popf }
      result = v18;
      if ( v18 != 3 )
        result = 4;
    }
  }
  if ( bProcessorIsNotIntel )
    HIBYTE(result) |= 0x80u;
  return result;
}



//----- (0043847A) --------------------------------------------------------
__int16  _43847A_get_cpu_speed_sub1()
{
  signed int v5; // eax@1
  char v12[12]; // [sp+Ch] [bp-28h]@1
  char v13; // [sp+18h] [bp-1Ch]@1
  char v14; // [sp+19h] [bp-1Bh]@1
  int v15[4]; // [sp+1Ch] [bp-18h]@1
  int v16; // [sp+2Eh] [bp-6h]@1
  char v17; // [sp+32h] [bp-2h]@6
  char v18; // [sp+33h] [bp-1h]@1

  *(int *)v12 = *(int *)"------------";
  *(int *)&v12[4] = *(int *)"--------";
  *(int *)&v12[8] = *(int *)"----";
  v13 = asc_4E4938[12];
  v14 = 0;
  v18 = 0;
  v15[0] = *(int *)"GenuineIntel";
  v15[1] = *(int *)"ineIntel";
  v15[2] = *(int *)"ntel";
  v16 = 65535;
  LOBYTE(v15[3]) = aGenuineintel[12];
  _EAX = 0;
  __asm { cpuid }
  *(int *)v12 = _EBX;
  *(int *)&v12[4] = _EDX;
  *(int *)&v12[8] = _ECX;
  v5 = 0;
  do
  {
    if ( v12[v5] != *((char *)v15 + v5) )
      bProcessorIsNotIntel = 1;
    ++v5;
  }
  while ( v5 < 12 );
  if ( v5 >= 1 )
  {
    _EAX = 1;
    __asm { cpuid }
    v18 = _EAX & 0xF;
    LOBYTE(_EAX) = (unsigned __int8)(_EAX & 0xF0) >> 4;
    v17 = _EAX;
    LOWORD(v16) = ((_EAX & 0xF00u) >> 8) & 0xF;
  }
  return v16;
}



//----- (004383ED) --------------------------------------------------------
int  _4383ED_get_cpu_speed_sub2()
{
  signed int v9; // eax@4
  int v16; // [sp-4h] [bp-38h]@0
  int v17[3]; // [sp+Ch] [bp-28h]@1
  char v18; // [sp+18h] [bp-1Ch]@1
  char v19; // [sp+19h] [bp-1Bh]@1
  int v20[4]; // [sp+1Ch] [bp-18h]@1
  int v21; // [sp+2Ch] [bp-8h]@1
  int v22; // [sp+30h] [bp-4h]@1

  v17[0] = *(int *)"------------";
  v17[1] = *(int *)"--------";
  v17[2] = *(int *)"----";
  v18 = asc_4E4938[12];
  v19 = 0;
  v22 = 0;
  v20[0] = *(int *)"GenuineIntel";
  v20[1] = *(int *)"ineIntel";
  v20[2] = *(int *)"ntel";
  LOBYTE(v20[3]) = aGenuineintel[12];
  _CF = 0;
  _OF = 0;
  _ZF = 0;
  _SF = 0;
  v21 = 1;
  __asm { pushf }
  if ( v16 == (v16 ^ 0x200000) )
    v21 = 0;
  if ( (short)v21 )
  {
    _EAX = 0;
    __asm { cpuid }
    v17[0] = _EBX;
    v17[1] = _EDX;
    v17[2] = _ECX;
    v9 = 0;
    do
    {
      if ( *((char *)v17 + v9) != *((char *)v20 + v9) )
        bProcessorIsNotIntel = 1;
      ++v9;
    }
    while ( v9 < 12 );
    if ( v9 >= 1 )
    {
      _EAX = 1;
      __asm { cpuid }
      v22 = _EDX;
    }
  }
  return v22;
}



//----- (004AC4FD) --------------------------------------------------------
Vec4_int_ * _4AC4FD_get_cpu_clocks_rdtsc(int _this, Vec4_int_ *a1)
{
  int v2; // eax@1
  int v3; // eax@4
  bool v4; // eax@5
  unsigned __int64 v5; // kr00_8@8
  int v6; // edi@9
  int v7; // eax@9
  bool v8; // eax@10
  unsigned __int64 v9; // kr08_8@13
  unsigned int v10; // eax@15
  Vec4_int_ *result; // eax@17
  int Dst; // [sp+Ch] [bp-2Ch]@1
  int v13; // [sp+10h] [bp-28h]@17
  int v14; // [sp+14h] [bp-24h]@15
  int v15; // [sp+18h] [bp-20h]@17
  unsigned int v16; // [sp+1Ch] [bp-1Ch]@8
  unsigned int v17; // [sp+20h] [bp-18h]@8
  unsigned int v18; // [sp+24h] [bp-14h]@13
  int nPriority; // [sp+28h] [bp-10h]@1
  __int64 v20; // [sp+2Ch] [bp-Ch]@1
  int v21; // [sp+34h] [bp-4h]@3

  HIDWORD(v20) = GetCurrentThread();
  memset(&Dst, 0, 0x10u);
  v2 = GetThreadPriority(HIDWORD(v20));
  nPriority = v2;
  if ( v2 != 0x7FFFFFFF )
    SetThreadPriority(HIDWORD(v20), v2 + 1);
  __outbyte(0x70u, 0);
  v21 = __inbyte(0x71u);
  do
  {
    __outbyte(0x70u, 0);
    LODWORD(v20) = __inbyte(0x71u);
    v3 = v20 - v21;
    if ( (signed int)v20 >= v21 )
      v4 = v3 > 0;
    else
      v4 = v3 + 10;
  }
  while ( !v4 );
  v5 = __rdtsc();
  v16 = HIDWORD(v5);
  v17 = v5;
  do
  {
    __outbyte(0x70u, 0);
    v21 = __inbyte(0x71u);
    v6 = v21;
    v7 = v21 - v20;
    if ( v21 >= (signed int)v20 )
      v8 = v7 > 0;
    else
      v8 = v7 + 10;
  }
  while ( !v8 );
  v9 = __rdtsc();
  v18 = HIDWORD(v9);
  v21 = v9;
  if ( nPriority != 0x7FFFFFFF )
    SetThreadPriority(HIDWORD(v20), nPriority);
  nPriority = v21;
  *(__int64 *)((char *)&v20 + 4) = __PAIR__(v18, v21) - __PAIR__(v16, v17);
  Dst = HIDWORD(v20);
  v10 = HIDWORD(v20) / 0xF4240;
  v14 = HIDWORD(v20) / 0xF4240;
  if ( HIDWORD(v20) / 0x186A0 - 10 * HIDWORD(v20) / 0xF4240 >= 6 )
    ++v10;
  v15 = v10;
  result = a1;
  v13 = 1000000 * v6 - 1000000 * v20;
  a1->x = Dst;
  a1->y = v13;
  a1->z = v14;
  a1->w = v15;
  return result;
}




//----- (004AC33A) --------------------------------------------------------
Vec4_int_ * _4AC33A_get_cpu_clocks_QPC(int _this, Vec4_int_ *a1)
{
  int v2; // esi@1
  int v3; // ebx@1
  unsigned __int64 v4; // qax@4
  unsigned __int64 v5; // qax@7
  DWORD v6; // edi@7
  DWORD v7; // eax@7
  unsigned int v8; // ecx@10
  unsigned __int64 v9; // qax@10
  unsigned int v10; // edi@10
  int v11; // eax@14
  unsigned int v12; // ecx@19
  Vec4_int_ *result; // eax@24
  int Dst; // [sp+Ch] [bp-4Ch]@1
  int v15; // [sp+10h] [bp-48h]@23
  int v16; // [sp+14h] [bp-44h]@21
  int v17; // [sp+18h] [bp-40h]@21
  LARGE_INTEGER Frequency; // [sp+1Ch] [bp-3Ch]@1
  LARGE_INTEGER PerformanceCount; // [sp+24h] [bp-34h]@2
  LARGE_INTEGER v20; // [sp+2Ch] [bp-2Ch]@2
  int v21; // [sp+34h] [bp-24h]@2
  int v22; // [sp+38h] [bp-20h]@2
  int v23; // [sp+3Ch] [bp-1Ch]@4
  int v24; // [sp+40h] [bp-18h]@7
  int nPriority; // [sp+44h] [bp-14h]@2
  unsigned int v26; // [sp+48h] [bp-10h]@1
  unsigned int v27; // [sp+4Ch] [bp-Ch]@1
  HANDLE hThread; // [sp+50h] [bp-8h]@1
  int v29; // [sp+54h] [bp-4h]@1

  v2 = 0;
  v3 = 0;
  v29 = 0;
  v27 = 0;
  v26 = 0;
  hThread = GetCurrentThread();
  memset(&Dst, 0, 0x10u);
  if ( QueryPerformanceFrequency(&Frequency) )
  {
    do
    {
      ++v29;
      v22 = v2;
      v21 = v3;
      QueryPerformanceCounter(&PerformanceCount);
      v20 = PerformanceCount;
      nPriority = GetThreadPriority(hThread);
      if ( nPriority != 2147483647 )
        SetThreadPriority(hThread, 15);
      while ( v20.s.LowPart - PerformanceCount.s.LowPart < 0x32 )
      {
        QueryPerformanceCounter(&v20);
        v4 = __rdtsc();
        v23 = v4;
      }
      PerformanceCount = v20;
      do
      {
        QueryPerformanceCounter(&v20);
        v5 = __rdtsc();
        v24 = v5;
        v6 = v20.s.LowPart;
        v7 = PerformanceCount.s.LowPart;
      }
      while ( v20.s.LowPart - PerformanceCount.s.LowPart < 0x3E8 );
      if ( nPriority != 2147483647 )
      {
        SetThreadPriority(hThread, nPriority);
        v7 = PerformanceCount.s.LowPart;
        v6 = v20.s.LowPart;
      }
      v8 = v24 - v23;
      v27 += v24 - v23;
      v9 = (100000 * v6 - 100000 * v7) / (Frequency.s.LowPart / 0xA);
      v10 = v9;
      v26 += v9;
      if ( v9 % Frequency.s.LowPart > Frequency.s.LowPart >> 1 )
        v10 = v9 + 1;
      v3 = v8 / v10;
      if ( v8 % v10 > v10 >> 1 )
        v3 = v8 / v10 + 1;
      v2 = v21;
      v11 = v3 + v22 + v21;
    }
    while ( v29 < 3
         || v29 < 20
         && ((unsigned int)(3 * v3 - v11) > 3 || (unsigned int)(3 * v21 - v11) > 3 || (unsigned int)(3 * v22 - v11) > 3) );
    v12 = 10 * v27 / v26;
    if ( 100 * v27 / v26 - 10 * v12 >= 6 )
      ++v12;
    v16 = v27 / v26;
    v17 = v27 / v26;
    if ( v12 - 10 * v27 / v26 >= 6 )
      v17 = v27 / v26 + 1;
    v15 = v26;
    Dst = v27;
  }
  result = a1;
  a1->x = Dst;
  a1->y = v15;
  a1->z = v16;
  a1->w = v17;
  return result;
}





//----- (004AC277) --------------------------------------------------------
Vec4_int_ * _4AC277_get_cpu_speed_sub3(unsigned int _this, Vec4_int_ *a2)
{
  signed int v2; // esi@1
  signed __int16 v4; // bx@3
  int v10; // esi@8
  int v11; // eax@10
  Vec4_int_ *result; // eax@13
  Vec4_int_ Dst; // [sp+8h] [bp-30h]@1
  LARGE_INTEGER Frequency; // [sp+18h] [bp-20h]@1
  LARGE_INTEGER PerformanceCount; // [sp+20h] [bp-18h]@3
  LARGE_INTEGER v16; // [sp+28h] [bp-10h]@5
  int v17; // [sp+30h] [bp-8h]@1
  int v18; // [sp+34h] [bp-4h]@2

  v17 = _this;
  v2 = -1;
  memset(&Dst, 0, 0x10u);
  if ( QueryPerformanceFrequency(&Frequency) )
  {
    v18 = 10;
    do
    {
      QueryPerformanceCounter(&PerformanceCount);
      _EAX = -2147483648;
      v4 = 4000;
      do
      {
        __asm { bsf     ecx, eax }
        --v4;
      }
      while ( v4 );
      QueryPerformanceCounter(&v16);
      if ( (signed int)v16.s.LowPart - (signed int)PerformanceCount.s.LowPart < (unsigned int)v2 )
        v2 = v16.s.LowPart - PerformanceCount.s.LowPart;
      --v18;
    }
    while ( v18 );
    v10 = 100000 * v2 / (Frequency.s.LowPart / 0xA);
    if ( v10 % Frequency.s.LowPart > Frequency.s.LowPart >> 1 )
      ++v10;
    v11 = v17 / (unsigned int)v10;
    Dst.z = v17 / (unsigned int)v10;
    if ( v17 % (unsigned int)v10 > (unsigned int)v10 >> 1 )
      ++v11;
    Dst.x = v17;
    Dst.y = v10;
    Dst.w = v11;
  }
  result = a2;
  a2->x = Dst.x;
  a2->y = Dst.y;
  a2->z = Dst.z;
  a2->w = Dst.w;
  return result;
}

//----- (004D714C) --------------------------------------------------------
int  sr_sub_4D714C(stru315 *a1)
{
  stru315 *v1; // ebp@0
  stru315 *v2; // ebp@1
  unsigned int v3; // esi@1
  int v4; // edi@1
  int result; // eax@1
  unsigned int *v6; // ebx@2
  int v7; // edx@2
  int v8; // ebx@3
  unsigned __int16 v9; // cx@3
  unsigned int *v10; // ebx@3
  int v11; // edx@3
  int v12; // ebx@4
  unsigned __int8 v13; // sf@5
  unsigned __int8 v14; // of@5

  v2 = (stru315 *)v1->field_8;
  v3 = v2->field_30;
  v4 = v2->field_2C;
  result = (int)v2->pColorBuffer;
  if ( !(v2->field_28 & 1) )
    goto LABEL_5;
  --v2->field_28;
  result += 2;
  v6 = v2->pDepthBuffer;
  v7 = v2->field_24;
  --v2->pDepthBuffer;
  *v6 = v7;
  while ( 1 )
  {
    v12 = *((char *)v2->pTextureLOD
          + (v2->field_C & (v3 >> 16))
          + ((v2->field_8 & (unsigned int)v4) >> LOBYTE(v2->field_10)));
    result -= 4;
    v3 += v2->field_4;
    v4 += v2->field_0;
    *(short *)(result + 2) = v2->field_34_palette[v12];
LABEL_5:
    v14 = __OFSUB__(v2->field_28, 2);
    v13 = v2->field_28 - 2 < 0;
    v2->field_28 -= 2;
    if ( v13 ^ v14 )
      break;
    v8 = *((char *)v2->pTextureLOD
         + (v2->field_C & (v3 >> 16))
         + ((v2->field_8 & (unsigned int)v4) >> LOBYTE(v2->field_10)));
    v3 += v2->field_4;
    v9 = v2->field_34_palette[v8];
    v4 += v2->field_0;
    v10 = v2->pDepthBuffer;
    v11 = v2->field_24;
    *(short *)result = v9;
    v2->pDepthBuffer -= 2;
    *v10 = v11;
    *(v10 - 1) = v11;
  }
  v2->pColorBuffer = (unsigned __int16 *)result;
  v2->field_30 = v3;
  v2->field_2C = v4;
  return result;
}

//----- (004D6FB0) --------------------------------------------------------
int sr_sub_4D6FB0(stru315 *a1)
{
  int v1; // ebp@1
  unsigned int v2; // esi@1
  unsigned int v3; // edi@1
  int result; // eax@1
  int v5; // ebx@2
  int v6; // edx@2
  int v7; // ebx@3
  __int16 v8; // cx@3
  int v9; // ebx@4
  unsigned __int8 v10; // sf@5
  unsigned __int8 v11; // of@5

  v1 = a1->field_8;
  v2 = *(int *)(v1 + 48);
  v3 = *(int *)(v1 + 44);
  result = *(int *)(v1 + 64);
  if ( !(*(int *)(v1 + 40) & 1) )
    goto LABEL_5;
  --*(int *)(v1 + 40);
  result -= 2;
  v5 = *(int *)(v1 + 60);
  v6 = *(int *)(v1 + 36);
  *(int *)(v1 + 60) += 4;
  while ( 1 )
  {
    *(int *)v5 = v6;
    v9 = *(char *)(*(int *)(v1 + 56)
                  + (*(int *)(v1 + 12) & (v2 >> 16))
                  + ((*(int *)(v1 + 8) & v3) >> *(char *)(v1 + 16)));
    result += 4;
    v2 += *(int *)(v1 + 4);
    v3 += *(int *)v1;
    *(short *)(result - 2) = *(short *)(*(int *)(v1 + 52) + 2 * v9);
LABEL_5:
    v11 = __OFSUB__(*(int *)(v1 + 40), 2);
    v10 = *(int *)(v1 + 40) - 2 < 0;
    *(int *)(v1 + 40) -= 2;
    if ( v10 ^ v11 )
      break;
    v7 = *(char *)(*(int *)(v1 + 56)
                  + (*(int *)(v1 + 12) & (v2 >> 16))
                  + ((*(int *)(v1 + 8) & v3) >> *(char *)(v1 + 16)));
    v2 += *(int *)(v1 + 4);
    v8 = *(short *)(*(int *)(v1 + 52) + 2 * v7);
    v3 += *(int *)v1;
    v5 = *(int *)(v1 + 60);
    v6 = *(int *)(v1 + 36);
    *(short *)result = v8;
    *(int *)(v1 + 60) += 8;
    *(int *)(v5 + 4) = v6;
  }
  *(int *)(v1 + 64) = result;
  *(int *)(v1 + 48) = v2;
  *(int *)(v1 + 44) = v3;
  return result;
}

//----- (004D705A) --------------------------------------------------------
int  sr_sub_4D705A(stru315 *a1)
{
  int v1; // ebp@1
  unsigned int v2; // esi@1
  unsigned int v3; // edi@1
  int result; // eax@1
  int v5; // ebx@2
  int v6; // edx@2
  unsigned int v7; // ebx@3
  unsigned int v8; // edx@3
  __int16 v9; // cx@11
  unsigned int v10; // ebx@12
  unsigned int v11; // edx@12
  unsigned __int8 v12; // sf@21
  unsigned __int8 v13; // of@21

  v1 = a1->field_8;
  v2 = *(int *)(v1 + 48);
  v3 = *(int *)(v1 + 44);
  result = *(int *)(v1 + 64);
  if ( !(*(int *)(v1 + 40) & 1) )
    goto LABEL_21;
  --*(int *)(v1 + 40);
  result -= 2;
  v5 = *(int *)(v1 + 60);
  v6 = *(int *)(v1 + 36);
  *(int *)(v1 + 60) += 4;
  while ( 1 )
  {
    *(int *)v5 = v6;
    v10 = v3;
    v11 = v2;
    if ( (signed int)v2 < *(int *)(v1 + 20) )
      v11 = *(int *)(v1 + 20);
    if ( (signed int)v3 < *(int *)(v1 + 28) )
      v10 = *(int *)(v1 + 28);
    if ( (signed int)v11 > *(int *)(v1 + 24) )
      v11 = *(int *)(v1 + 24);
    if ( (signed int)v10 > *(int *)(v1 + 32) )
      v10 = *(int *)(v1 + 32);
    result += 4;
    v2 += *(int *)(v1 + 4);
    v3 += *(int *)v1;
    *(short *)(result - 2) = *(short *)(*(int *)(v1 + 52)
                                      + 2
                                      * *(char *)(*(int *)(v1 + 56)
                                                 + (*(int *)(v1 + 12) & (v11 >> 16))
                                                 + ((*(int *)(v1 + 8) & v10) >> *(char *)(v1 + 16))));
LABEL_21:
    v13 = __OFSUB__(*(int *)(v1 + 40), 2);
    v12 = *(int *)(v1 + 40) - 2 < 0;
    *(int *)(v1 + 40) -= 2;
    if ( v12 ^ v13 )
      break;
    v7 = v3;
    v8 = v2;
    if ( (signed int)v2 < *(int *)(v1 + 20) )
      v8 = *(int *)(v1 + 20);
    if ( (signed int)v3 < *(int *)(v1 + 28) )
      v7 = *(int *)(v1 + 28);
    if ( (signed int)v8 > *(int *)(v1 + 24) )
      v8 = *(int *)(v1 + 24);
    if ( (signed int)v7 > *(int *)(v1 + 32) )
      v7 = *(int *)(v1 + 32);
    v2 += *(int *)(v1 + 4);
    v9 = *(short *)(*(int *)(v1 + 52)
                  + 2
                  * *(char *)(*(int *)(v1 + 56)
                             + (*(int *)(v1 + 12) & (v8 >> 16))
                             + ((*(int *)(v1 + 8) & v7) >> *(char *)(v1 + 16))));
    v3 += *(int *)v1;
    v5 = *(int *)(v1 + 60);
    v6 = *(int *)(v1 + 36);
    *(short *)result = v9;
    *(int *)(v1 + 60) += 8;
    *(int *)(v5 + 4) = v6;
  }
  *(int *)(v1 + 64) = result;
  *(int *)(v1 + 48) = v2;
  *(int *)(v1 + 44) = v3;
  return result;
}

//----- (004D71F8) --------------------------------------------------------
int  sr_sub_4D71F8(stru315 *a1)
{
  stru315 *v1; // ebp@0
  stru315 *v2; // ebp@1
  int v3; // esi@1
  int v4; // edi@1
  int result; // eax@1
  unsigned int *v6; // ebx@2
  int v7; // edx@2
  int v8; // ebx@3
  unsigned int v9; // edx@3
  unsigned __int16 v10; // cx@11
  unsigned int *v11; // ebx@11
  int v12; // edx@11
  int v13; // ebx@12
  unsigned int v14; // edx@12
  unsigned __int8 v15; // sf@21
  unsigned __int8 v16; // of@21

  v2 = (stru315 *)v1->field_8;
  v3 = v2->field_30;
  v4 = v2->field_2C;
  result = (int)v2->pColorBuffer;
  if ( !(v2->field_28 & 1) )
    goto LABEL_21;
  --v2->field_28;
  result += 2;
  v6 = v2->pDepthBuffer;
  v7 = v2->field_24;
  --v2->pDepthBuffer;
  *v6 = v7;
  while ( 1 )
  {
    v13 = v4;
    v14 = v3;
    if ( v3 < v2->field_14 )
      v14 = v2->field_14;
    if ( v4 < v2->field_1C )
      v13 = v2->field_1C;
    if ( (signed int)v14 > v2->field_18 )
      v14 = v2->field_18;
    if ( v13 > v2->field_20 )
      v13 = v2->field_20;
    result -= 4;
    v3 += v2->field_4;
    v4 += v2->field_0;
    *(short *)(result + 2) = v2->field_34_palette[*((char *)v2->pTextureLOD
                                                  + (v2->field_C & (v14 >> 16))
                                                  + ((v2->field_8 & (unsigned int)v13) >> LOBYTE(v2->field_10)))];
LABEL_21:
    v16 = __OFSUB__(v2->field_28, 2);
    v15 = v2->field_28 - 2 < 0;
    v2->field_28 -= 2;
    if ( v15 ^ v16 )
      break;
    v8 = v4;
    v9 = v3;
    if ( v3 < v2->field_14 )
      v9 = v2->field_14;
    if ( v4 < v2->field_1C )
      v8 = v2->field_1C;
    if ( (signed int)v9 > v2->field_18 )
      v9 = v2->field_18;
    if ( v8 > v2->field_20 )
      v8 = v2->field_20;
    v3 += v2->field_4;
    v10 = v2->field_34_palette[*((char *)v2->pTextureLOD
                               + (v2->field_C & (v9 >> 16))
                               + ((v2->field_8 & (unsigned int)v8) >> LOBYTE(v2->field_10)))];
    v4 += v2->field_0;
    v11 = v2->pDepthBuffer;
    v12 = v2->field_24;
    *(short *)result = v10;
    v2->pDepthBuffer -= 2;
    *v11 = v12;
    *(v11 - 1) = v12;
  }
  v2->pColorBuffer = (unsigned __int16 *)result;
  v2->field_30 = v3;
  v2->field_2C = v4;
  return result;
}

//----- (004D754B) --------------------------------------------------------
void  sr_sub_4D754B(stru315 *a1, stru316 *a2)
{
  int v2; // ecx@1
  unsigned int v3; // eax@2
  int v4; // ecx@2
  int v5; // eax@10
  unsigned __int16 *v6; // edx@10
  int v7; // ebx@11
  int v8; // ecx@13
  unsigned __int16 v9; // bx@16
  int v10; // ecx@16
  unsigned __int16 *v11; // eax@16
  unsigned int *v12; // edx@16
  int v13; // ecx@16
  int v14; // eax@16
  int v15; // ebx@16
  int v16; // [sp-4h] [bp-Ch]@2

  v2 = a1->field_28;
  if ( v2 )
  {
    do
    {
      v16 = v2;
      v3 = a1->field_30;
      v4 = a1->field_2C;
      if ( (signed int)v3 >= a1->field_18 )
        v3 = a1->field_18;
      if ( (signed int)v3 <= a1->field_14 )
        v3 = a1->field_14;
      if ( v4 >= a1->field_20 )
        v4 = a1->field_20;
      if ( v4 <= a1->field_1C )
        v4 = a1->field_1C;
      v5 = *((char *)a1->pTextureLOD + (a1->field_C & (v3 >> 16)) + (((unsigned int)v4 & a1->field_8) >> a1->field_10));
      v6 = a1->field_34_palette;
      if ( a2->field_20 )
      {
        v7 = (unsigned int)a2->field_10 >> 16;
        if ( v7 >= a2->field_C )
          v7 = a2->field_C;
        v8 = 0;
        if ( a2->field_8 - v7 >= 0 )
          v8 = a2->field_8 - v7;
        v5 += v8 << 8;
        v6 = a2->field_24_palette;
      }
      v9 = v6[v5];
      v10 = a1->field_24;
      v11 = a1->pColorBuffer;
      v12 = a1->pDepthBuffer;
      *v11 = v9;
      *v12 = v10;
      a1->pColorBuffer = v11 + 1;
      a1->pDepthBuffer = v12 + 1;
      v13 = a1->field_30;
      a2->field_10 += a2->field_18;
      v14 = a1->field_2C;
      v15 = a1->field_0;
      a1->field_30 = a1->field_4 + v13;
      a1->field_2C = v15 + v14;
      v2 = v16 - 1;
    }
    while ( v16 != 1 );
  }
}
//----- (004D73DF) --------------------------------------------------------
int  sr_sub_4D73DF(int a1)
{
  int v1; // ebp@1
  unsigned int v2; // esi@1
  unsigned int v3; // edi@1
  int result; // eax@1
  int v5; // ebx@2
  int v6; // edx@2
  int v7; // edx@3
  int v8; // edx@6
  unsigned __int8 v9; // sf@9
  unsigned __int8 v10; // of@9

  v1 = *(int *)(a1 + 8);
  v2 = *(int *)(v1 + 48);
  v3 = *(int *)(v1 + 44);
  result = *(int *)(v1 + 64);
  if ( !(*(int *)(v1 + 40) & 1) )
    goto LABEL_9;
  --*(int *)(v1 + 40);
  result -= 2;
  v5 = *(int *)(v1 + 60);
  v6 = *(int *)(v1 + 36);
  *(int *)(v1 + 60) += 4;
  while ( 1 )
  {
    *(int *)v5 = v6;
    v8 = *(int *)(v1 + 12) & (v2 >> 16);
    result += 4;
    v2 += *(int *)(v1 + 4);
    if ( *(char *)(*(int *)(v1 + 56) + v8 + ((*(int *)(v1 + 8) & v3) >> *(char *)(v1 + 16))) )
      *(short *)(result - 2) = *(short *)(*(int *)(v1 + 52)
                                        + 2
                                        * *(char *)(*(int *)(v1 + 56)
                                                   + v8
                                                   + ((*(int *)(v1 + 8) & v3) >> *(char *)(v1 + 16))));
    v3 += *(int *)v1;
LABEL_9:
    v10 = __OFSUB__(*(int *)(v1 + 40), 2);
    v9 = *(int *)(v1 + 40) - 2 < 0;
    *(int *)(v1 + 40) -= 2;
    if ( v9 ^ v10 )
      break;
    v7 = *(int *)(v1 + 12) & (v2 >> 16);
    v2 += *(int *)(v1 + 4);
    if ( *(char *)(*(int *)(v1 + 56) + v7 + ((*(int *)(v1 + 8) & v3) >> *(char *)(v1 + 16))) )
      *(short *)result = *(short *)(*(int *)(v1 + 52)
                                  + 2
                                  * *(char *)(*(int *)(v1 + 56)
                                             + v7
                                             + ((*(int *)(v1 + 8) & v3) >> *(char *)(v1 + 16))));
    v3 += *(int *)v1;
    v5 = *(int *)(v1 + 60);
    v6 = *(int *)(v1 + 36);
    *(int *)(v1 + 60) += 8;
    *(int *)(v5 + 4) = v6;
  }
  return result;
}

//----- (004D72EC) --------------------------------------------------------
int  sr_sub_4D72EC(int a1)
{
  int v1; // ebp@1
  unsigned int v2; // esi@1
  unsigned int v3; // edi@1
  int result; // eax@1
  int v5; // ebx@2
  int v6; // edx@2
  unsigned int v7; // edx@3
  unsigned int v8; // ebx@3
  int v9; // ebx@11
  unsigned int v10; // edx@14
  unsigned int v11; // ebx@14
  int v12; // ebx@22
  unsigned __int8 v13; // sf@25
  unsigned __int8 v14; // of@25

  v1 = *(int *)(a1 + 8);
  v2 = *(int *)(v1 + 48);
  v3 = *(int *)(v1 + 44);
  result = *(int *)(v1 + 64);
  if ( !(*(int *)(v1 + 40) & 1) )
    goto LABEL_25;
  --*(int *)(v1 + 40);
  result -= 2;
  v5 = *(int *)(v1 + 60);
  v6 = *(int *)(v1 + 36);
  *(int *)(v1 + 60) += 4;
  while ( 1 )
  {
    *(int *)v5 = v6;
    v10 = v2;
    v11 = v3;
    if ( (signed int)v2 < *(int *)(v1 + 20) )
      v10 = *(int *)(v1 + 20);
    if ( (signed int)v3 < *(int *)(v1 + 28) )
      v11 = *(int *)(v1 + 28);
    if ( (signed int)v10 > *(int *)(v1 + 24) )
      v10 = *(int *)(v1 + 24);
    if ( (signed int)v11 > *(int *)(v1 + 32) )
      v11 = *(int *)(v1 + 32);
    v12 = *(char *)(*(int *)(v1 + 56)
                   + (*(int *)(v1 + 12) & (v10 >> 16))
                   + ((*(int *)(v1 + 8) & v11) >> *(char *)(v1 + 16)));
    result += 4;
    v2 += *(int *)(v1 + 4);
    if ( v12 )
      *(short *)(result - 2) = *(short *)(*(int *)(v1 + 52) + 2 * v12);
    v3 += *(int *)v1;
LABEL_25:
    v14 = __OFSUB__(*(int *)(v1 + 40), 2);
    v13 = *(int *)(v1 + 40) - 2 < 0;
    *(int *)(v1 + 40) -= 2;
    if ( v13 ^ v14 )
      break;
    v7 = v2;
    v8 = v3;
    if ( (signed int)v2 < *(int *)(v1 + 20) )
      v7 = *(int *)(v1 + 20);
    if ( (signed int)v3 < *(int *)(v1 + 28) )
      v8 = *(int *)(v1 + 28);
    if ( (signed int)v7 > *(int *)(v1 + 24) )
      v7 = *(int *)(v1 + 24);
    if ( (signed int)v8 > *(int *)(v1 + 32) )
      v8 = *(int *)(v1 + 32);
    v9 = *(char *)(*(int *)(v1 + 56)
                  + (*(int *)(v1 + 12) & (v7 >> 16))
                  + ((*(int *)(v1 + 8) & v8) >> *(char *)(v1 + 16)));
    v2 += *(int *)(v1 + 4);
    if ( v9 )
      *(short *)result = *(short *)(*(int *)(v1 + 52) + 2 * v9);
    v3 += *(int *)v1;
    v5 = *(int *)(v1 + 60);
    v6 = *(int *)(v1 + 36);
    *(int *)(v1 + 60) += 8;
    *(int *)(v5 + 4) = v6;
  }
  return result;
}

//----- (004D7630) --------------------------------------------------------
void  sr_sub_4D7630(stru315 *a1, stru316 *a2)
{
  int v2; // ecx@1
  int v3; // eax@2
  unsigned __int16 *v4; // edx@2
  int v5; // ebx@3
  int v6; // ecx@5
  unsigned __int16 v7; // bx@8
  int v8; // ecx@8
  unsigned __int16 *v9; // eax@8
  unsigned int *v10; // edx@8
  int v11; // ecx@8
  int v12; // eax@8
  int v13; // ebx@8
  int v14; // [sp-4h] [bp-Ch]@2

  v2 = a1->field_28;
  if ( v2 )
  {
    do
    {
      v14 = v2;
      v3 = *((char *)a1->pTextureLOD
           + (a1->field_C & ((unsigned int)a1->field_30 >> 16))
           + ((unsigned int)(a1->field_2C & a1->field_8) >> a1->field_10));
      v4 = a1->field_34_palette;
      if ( a2->field_20 )
      {
        v5 = (unsigned int)a2->field_14 >> 16;
        if ( v5 >= a2->field_C )
          v5 = a2->field_C;
        v6 = 0;
        if ( a2->field_8 - v5 >= 0 )
          v6 = a2->field_8 - v5;
        v3 += v6 << 8;
        v4 = a2->field_24_palette;
      }
      v7 = v4[v3];
      v8 = a1->field_24;
      v9 = a1->pColorBuffer;
      v10 = a1->pDepthBuffer;
      *v9 = v7;
      *v10 = v8;
      a1->pColorBuffer = v9 - 1;
      a1->pDepthBuffer = v10 - 1;
      v11 = a1->field_30;
      a2->field_14 -= a2->field_18;
      v12 = a1->field_2C;
      v13 = a1->field_0;
      a1->field_30 = a1->field_4 + v11;
      a1->field_2C = v13 + v12;
      v2 = v14 - 1;
    }
    while ( v14 != 1 );
  }
}

//----- (004D76ED) --------------------------------------------------------
void  sr_sub_4D76ED(stru315 *a1, stru316 *a2)
{
  int v2; // ecx@1
  unsigned int v3; // eax@2
  int v4; // ecx@2
  int v5; // eax@10
  unsigned __int16 *v6; // edx@10
  int v7; // ebx@11
  int v8; // ecx@13
  unsigned __int16 v9; // bx@16
  int v10; // ecx@16
  unsigned __int16 *v11; // eax@16
  unsigned int *v12; // edx@16
  int v13; // ecx@16
  int v14; // eax@16
  int v15; // ebx@16
  int v16; // [sp-4h] [bp-Ch]@2

  v2 = a1->field_28;
  if ( v2 )
  {
    do
    {
      v16 = v2;
      v3 = a1->field_30;
      v4 = a1->field_2C;
      if ( (signed int)v3 >= a1->field_18 )
        v3 = a1->field_18;
      if ( (signed int)v3 <= a1->field_14 )
        v3 = a1->field_14;
      if ( v4 >= a1->field_20 )
        v4 = a1->field_20;
      if ( v4 <= a1->field_1C )
        v4 = a1->field_1C;
      v5 = *((char *)a1->pTextureLOD + (a1->field_C & (v3 >> 16)) + (((unsigned int)v4 & a1->field_8) >> a1->field_10));
      v6 = a1->field_34_palette;
      if ( a2->field_20 )
      {
        v7 = (unsigned int)a2->field_14 >> 16;
        if ( v7 >= a2->field_C )
          v7 = a2->field_C;
        v8 = 0;
        if ( a2->field_8 - v7 >= 0 )
          v8 = a2->field_8 - v7;
        v5 += v8 << 8;
        v6 = a2->field_24_palette;
      }
      v9 = v6[v5];
      v10 = a1->field_24;
      v11 = a1->pColorBuffer;
      v12 = a1->pDepthBuffer;
      *v11 = v9;
      *v12 = v10;
      a1->pColorBuffer = v11 - 1;
      a1->pDepthBuffer = v12 - 1;
      v13 = a1->field_30;
      a2->field_14 -= a2->field_18;
      v14 = a1->field_2C;
      v15 = a1->field_0;
      a1->field_30 = a1->field_4 + v13;
      a1->field_2C = v15 + v14;
      v2 = v16 - 1;
    }
    while ( v16 != 1 );
  }
}

//----- (004D77D2) --------------------------------------------------------
void  sr_sub_4D77D2(stru315 *a1, stru316 *a2)
{
  int v2; // ecx@1
  unsigned __int16 *v3; // ebx@2
  int v4; // eax@2
  unsigned __int16 *v5; // edx@2
  int v6; // ebx@4
  int v7; // ecx@6
  unsigned __int16 v8; // bx@9
  int v9; // ecx@9
  unsigned int *v10; // edx@9
  unsigned int *v11; // edx@10
  int v12; // ecx@10
  int v13; // eax@10
  int v14; // ebx@10
  int v15; // [sp-4h] [bp-Ch]@2

  v2 = a1->field_28;
  if ( v2 )
  {
    do
    {
      v15 = v2;
      v3 = a1->pTextureLOD;
      v4 = *((char *)v3
           + (a1->field_C & ((unsigned int)a1->field_30 >> 16))
           + ((unsigned int)(a1->field_2C & a1->field_8) >> a1->field_10));
      v5 = a1->field_34_palette;
      if ( *((char *)v3
           + (a1->field_C & ((unsigned int)a1->field_30 >> 16))
           + ((unsigned int)(a1->field_2C & a1->field_8) >> a1->field_10)) )
      {
        if ( a2->field_20 )
        {
          v6 = (unsigned int)a2->field_10 >> 16;
          if ( v6 >= a2->field_C )
            v6 = a2->field_C;
          v7 = 0;
          if ( a2->field_8 - v6 >= 0 )
            v7 = a2->field_8 - v6;
          v4 += v7 << 8;
          v5 = a2->field_24_palette;
        }
        v8 = v5[v4];
        v9 = a1->field_24;
        v10 = a1->pDepthBuffer;
        *a1->pColorBuffer = v8;
        *v10 = v9;
      }
      v11 = a1->pDepthBuffer + 1;
      ++a1->pColorBuffer;
      a1->pDepthBuffer = v11;
      v12 = a1->field_30;
      a2->field_10 += a2->field_18;
      v13 = a1->field_2C;
      v14 = a1->field_0;
      a1->field_30 = a1->field_4 + v12;
      a1->field_2C = v14 + v13;
      v2 = v15 - 1;
    }
    while ( v15 != 1 );
  }
}
//----- (004D789A) --------------------------------------------------------
void  sr_sub_4D789A(stru315 *a1, stru316 *a2)
{
  int v2; // ecx@1
  unsigned int v3; // eax@2
  int v4; // ecx@2
  int v5; // eax@10
  unsigned __int16 *v6; // edx@10
  int v7; // ebx@12
  int v8; // ecx@14
  unsigned __int16 v9; // bx@17
  int v10; // ecx@17
  unsigned int *v11; // edx@17
  unsigned int *v12; // edx@18
  int v13; // ecx@18
  int v14; // eax@18
  int v15; // ebx@18
  int v16; // [sp-4h] [bp-Ch]@2

  v2 = a1->field_28;
  if ( v2 )
  {
    do
    {
      v16 = v2;
      v3 = a1->field_30;
      v4 = a1->field_2C;
      if ( (signed int)v3 >= a1->field_18 )
        v3 = a1->field_18;
      if ( (signed int)v3 <= a1->field_14 )
        v3 = a1->field_14;
      if ( v4 >= a1->field_20 )
        v4 = a1->field_20;
      if ( v4 <= a1->field_1C )
        v4 = a1->field_1C;
      v5 = *((char *)a1->pTextureLOD + (a1->field_C & (v3 >> 16)) + (((unsigned int)v4 & a1->field_8) >> a1->field_10));
      v6 = a1->field_34_palette;
      if ( v5 )
      {
        if ( a2->field_20 )
        {
          v7 = (unsigned int)a2->field_10 >> 16;
          if ( v7 >= a2->field_C )
            v7 = a2->field_C;
          v8 = 0;
          if ( a2->field_8 - v7 >= 0 )
            v8 = a2->field_8 - v7;
          v5 += v8 << 8;
          v6 = a2->field_24_palette;
        }
        v9 = v6[v5];
        v10 = a1->field_24;
        v11 = a1->pDepthBuffer;
        *a1->pColorBuffer = v9;
        *v11 = v10;
      }
      v12 = a1->pDepthBuffer + 1;
      ++a1->pColorBuffer;
      a1->pDepthBuffer = v12;
      v13 = a1->field_30;
      a2->field_10 += a2->field_18;
      v14 = a1->field_2C;
      v15 = a1->field_0;
      a1->field_30 = a1->field_4 + v13;
      a1->field_2C = v15 + v14;
      v2 = v16 - 1;
    }
    while ( v16 != 1 );
  }
}
//----- (00481EB7) --------------------------------------------------------
void ResetPolygons()
{
  for (auto i = 0; i < pODMRenderParams->uNumPolygons; ++i)
  {
    array_77EC08[i].prolly_head = nullptr;
    array_77EC08[i].prolly_tail = nullptr;

    array_77EC08[i].flags = 0;
    array_77EC08[i].field_32 = 0;
  }
}
//----- (00466BE5) --------------------------------------------------------
void Abortf(const char *Format, ...)
{
  va_list va; // [sp+8h] [bp+8h]@1

  va_start(va, Format);
  if ( !render->bWindowMode )
    render->ChangeBetweenWinFullscreenModes();
  vsprintf(tmp_str.data(), Format, va);
  if ( pMouse )
    pMouse->ChangeActivation(0);
  ClipCursor(0);
  MessageBoxA(0, tmp_str.data(), "Error", 0x30u);
  Engine_DeinitializeAndTerminate(1);
}
//----- (00466B8C) --------------------------------------------------------
int  AbortWithError()
{
  if ( !aborting_app )
  {
    ClipCursor(0);
    aborting_app = 1;
    if ( !render->bWindowMode )
      render->ChangeBetweenWinFullscreenModes();
    if ( MessageBoxA(0, pGlobalTXT_LocalizationStrings[176], pGlobalTXT_LocalizationStrings[59], 0x34u) == 6 )
      SaveGame(1, 0);                           // "Internal Error"
                                                // "Might and Magic VII has detected an internal error and will be forced to close.  Would you like us to autosave your game before closing?"
    Engine_DeinitializeAndTerminate(1);
  }
  return 0;
}
//----- (0046271C) --------------------------------------------------------
void CreateDefaultBLVLevel()
{
  ofn.lStructSize = 0x4Cu;
  ofn.hwndOwner = hWnd;
  ofn.hInstance = 0;
  ofn.lpstrFilter = "Indoor  BLV Files (*.blv)";
  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = 0;
  ofn.nMaxFile = 260;
  ofn.nMaxFileTitle = 512;
  ofn.lpstrInitialDir = "levels";
  ofn.lpstrTitle = "Might and Magic® VII - Load Which Level?";
  ofn.Flags = 4;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lpstrDefExt = "blv";
  ofn.lCustData = 0;
  ofn.lpfnHook = 0;
  ofn.lpTemplateName = 0;
  ofn.lpstrFileTitle = tmp_str.data();
}
//----- (004268E3) --------------------------------------------------------
int sr_4268E3_smthn_to_a1r5g5b5(unsigned int uColor)
{
  unsigned int v1; // eax@1
  unsigned int v2; // edx@1
  int v3; // eax@1
  int v4; // edx@1
  unsigned int v5; // ecx@1

  v1 = uColor >> 9;
  v2 = uColor >> 6;
  LOWORD(v1) = (uColor >> 9) & 0x7C00;
  LOWORD(v2) = (uColor >> 6) & 0x3E0;
  v3 = v2 | v1;
  v4 = (unsigned __int8)uColor >> 3;
  v5 = uColor >> 16;
  LOWORD(v5) = v5 & 0x8000;
  return v5 | v4 | v3;
}
//----- (0042690D) --------------------------------------------------------
int sr_42690D_colors_cvt(unsigned int a1)
{
  unsigned int v1; // eax@1
  unsigned int v2; // edx@1

  v1 = a1 >> 8;
  v2 = a1 >> 5;
  LOWORD(v1) = (a1 >> 8) & 0xF800;
  LOWORD(v2) = (a1 >> 5) & 0x7E0;
  return ((unsigned __int8)a1 >> 3) | v2 | v1;
}
//----- (00487DBE) --------------------------------------------------------
void Software_ResetNewEdges()
{
  int v0; // ecx@1
  Edge **v1; // eax@1
  signed int v2; // eax@3

  v0 = 0;
  v1 = ptr_80CA10.data();
  do
  {
    ++v0;
    *((int *)&pNewEdges[v0] - 9) = (int)&defaultEdge;
    *v1 = 0;
    ++v1;
  }
  while ( (signed int)v1 < (signed int)&unk_80D190 );
  v2 = 2000;
  do
  {
    pSurfs->field_22 = 0;
    --v2;
  }
  while ( v2 );
}
//----- (00486A28) --------------------------------------------------------
void ODMRenderParams::AllocSoftwareDrawBuffers()
{
  if ( !this || !pSpans )
  {
    ReleaseSoftwareDrawBuffers();
    pSpans = (Span *)malloc(pSpans, 0x493E0u, "SPANS");
    pEdges = (Edge *)malloc(pEdges, 0x4C2C0u, "EDGES");
    pSurfs = (Surf *)malloc(pSurfs, 0x11940u, "SURFS");
    pNewEdges = (Edge *)malloc(pNewEdges, 0x6180u, "NEWEDGES");
    memset(pSpans, 0, 0x493E0u);
    memset(pEdges, 0, 0x4C2C0u);
    memset(pSurfs, 0, 0x11940u);
    memset(pNewEdges, 0, 0x6180u);
  }
}
//----- (00486AFC) --------------------------------------------------------
void ODMRenderParams::ReleaseSoftwareDrawBuffers()
{
  free(pSpans);
  free(pEdges);
  free(pSurfs);
  free(pNewEdges);
  pSpans = 0;
  pEdges = 0;
  pSurfs = 0;
  pNewEdges = 0;
}
//----- (0048901B) --------------------------------------------------------
bool OutdoorLocation::Release2()
{
  Release();
  pODMRenderParams->ReleaseSoftwareDrawBuffers();
  return true;
}

struct Allocator
{
  static Allocator *Create() {static Allocator a; return &a;}

  Allocator();
  bool  Initialize(unsigned int uNumKBytes);
  void *AllocNamedChunk(const void *pPrevPtrValue, unsigned int uSize, const char *pName);
  void FreeChunk(void *ptr);
  bool CreateFileDump(const char *a1);

  void *pMemoryBuffers[6000];
  unsigned int uMemoryBuffersSizes[6000];
  char pMemoryBuffersNames[6000][12];
  unsigned int uDumpsCount;
  unsigned int uNumBuffersUsed;
  void *pBigMemoryBuffer;
  unsigned int uNextFreeOffsetInBigBuffer;
  unsigned int uBigBufferSizeAligned;
  unsigned int bBigBufferAllocated;
  unsigned int bUseBigBuffer;
  void *pBigBufferAligned;
};

#include <string.h>
#include <stdio.h>

#include "Allocator.h"
#include "ErrorHandling.h"

#include "mm7_data.h"



Allocator *pAllocator; // idb

int aborting_app = false;




void CheckMemoryAccessibility(void *pBase, DWORD uSize);



//----- (00426755) --------------------------------------------------------
void *Allocator::AllocNamedChunk(const void *pPrevPtrValue, unsigned int uSize, const char *pName)
{
  //Allocator *v4; // esi@1
  unsigned int v5; // eax@7
  void *result; // eax@8
 // unsigned int *pNumBuffersUsed; // ebx@12
  unsigned int v8; // edi@15
  char v9; // zf@15
  int v10; // eax@16
  void **v11; // ebx@19
  void *v12; // eax@22
  unsigned int Size; // [sp+14h] [bp+Ch]@16

  int aligned_size; 


  if ( pPrevPtrValue && !aborting_app )
	  AbortWithError();
  if ( !bBigBufferAllocated && !aborting_app )
	  AbortWithError();

  if (!uSize)
	  return 0;

  if (((uSize & 0x80000000u) != 0) && !aborting_app)
	  AbortWithError();

  if (uNumBuffersUsed == 6000 && !aborting_app)
	  AbortWithError();

  ++uNumBuffersUsed;

  if (!bUseBigBuffer)
	  {
	  pMemoryBuffers[uNumBuffersUsed] = malloc(uSize);
	  if (pMemoryBuffers[uNumBuffersUsed])
		  uMemoryBuffersSizes[uNumBuffersUsed] = uSize;
	  else
		  {
		  if ( !aborting_app )
			  AbortWithError();
		  }
	  }
  else
	  {

	  aligned_size =(uSize&0xFFFFFFFC) + 4;

	  if ((uNextFreeOffsetInBigBuffer +aligned_size) > uBigBufferSizeAligned)
		  {
		  printf("Id: %s  Size: %i", pName, aligned_size);
		  CreateFileDump( "Memory");
		  if ( !aborting_app )
			  AbortWithError();
		  }

	  pMemoryBuffers[uNumBuffersUsed] = (char *)pBigMemoryBuffer + uNextFreeOffsetInBigBuffer;//(char *)(pBigBufferAligned) + uNextFreeOffsetInBigBuffer;
	  uMemoryBuffersSizes[uNumBuffersUsed] =aligned_size;

	  if (pName)
		  strncpy(pMemoryBuffersNames[uNumBuffersUsed], pName, 11);
	  uNextFreeOffsetInBigBuffer += aligned_size;
	  }
  return pMemoryBuffers[uNumBuffersUsed];
}
// 720018: using guessed type int aborting_app;



//----- (004266CD) --------------------------------------------------------
bool Allocator::Initialize(unsigned int uNumKBytes)
{
  if (bBigBufferAllocated)
    return false;

  if (!uNumKBytes)
    return false;

  pBigMemoryBuffer = malloc(uNumKBytes * 1024);
  if (!pBigMemoryBuffer)
    return false;

  pBigBufferAligned = (void *)((unsigned int)(pBigMemoryBuffer) & 0xFFFF0000 + 4096);
  //(char *)pBigMemoryBuffer + -((unsigned __int16)pBigMemoryBuffer & 0xFFF) + 4096;
  uBigBufferSizeAligned = (uNumKBytes * 1024) - 4096;

  uNumBuffersUsed = 0;
  uNextFreeOffsetInBigBuffer = 0;

  bUseBigBuffer = true;
  return bBigBufferAllocated = true;
}


//----- (00426474) --------------------------------------------------------
void Allocator::FreeChunk(void *ptr)
{
  Allocator *v2; // esi@1
  unsigned int v3; // eax@5
  signed int i; // edi@5
  unsigned int v5; // eax@13
  signed int v6; // ecx@16
  signed int j; // edx@16
  char v8; // zf@20

 
  
  unsigned int  uBuffindx; 
  unsigned int indx;

  if (ptr)
	  {
	  if ((!bBigBufferAllocated)&& (!aborting_app ))
		  AbortWithError();
	  for (indx = 0; indx <=uNumBuffersUsed; ++indx)
		  if (pMemoryBuffers[indx] == ptr)	
			  break;
	  //if ((indx>uNumBuffersUsed)&& !aborting_app )
		  //AbortWithError(); //to detect memory problems - uncomment
	  if (!bUseBigBuffer)
		  free(pMemoryBuffers[indx]);
	  pMemoryBuffers[indx] = NULL;
	  uMemoryBuffersSizes[indx] = 0;
	  pMemoryBuffersNames[indx][0] = 0;
	  if (indx == (uNumBuffersUsed-1))
		  {
		  if (indx>0)
			  {
			  uBuffindx = 0;
			  for (j = 0; j < uNumBuffersUsed-1; ++j)
				  {
				  if (pMemoryBuffers[j])
					  uBuffindx = j;
				  }
			  uNumBuffersUsed =  uBuffindx + 1;
			  if (bUseBigBuffer)
				  uNextFreeOffsetInBigBuffer = 
				  ((long)((char*)pMemoryBuffers[uBuffindx] -(char*)pBigBufferAligned)+ uMemoryBuffersSizes[uBuffindx]);
			  }
		  else
			  {
			  uNumBuffersUsed = 0;
			  uNextFreeOffsetInBigBuffer = 0;
			  }
		  }
	  }
}
// 720018: using guessed type int aborting_app;



//----- (00426429) --------------------------------------------------------
Allocator::Allocator()
{
  Allocator *v1; // eax@1
  signed int v2; // ecx@1
  int v3; // edx@1

  v1 = this;
  v2 = 0;
  v1->uBigBufferSizeAligned = 0;
  v1->bBigBufferAllocated = 0;
  v3 = (int)v1->pMemoryBuffersNames;
  do
  {
    v1->pMemoryBuffers[v2] = 0;
    v1->uMemoryBuffersSizes[v2] = 0;
    *(char *)v3 = 0;
    ++v2;
    v3 += 12;
  }
  while ( v2 < 6000 );
  v1->uDumpsCount = 0;
}


//----- (0042654C) --------------------------------------------------------
bool Allocator::CreateFileDump(const char *a1)
{
  Allocator *v2; // esi@1
  unsigned int v3; // ST44_4@1
  FILE *v4; // eax@1
  FILE *v5; // edi@1
  bool result; // eax@2
  unsigned int v7; // ecx@5
  int v8; // edx@5
  int v9; // eax@6
  int v10; // eax@8
  int v11; // ecx@8
  signed int v12; // ebx@8
  signed int v13; // ebx@10
  struct _MEMORYSTATUS memstat; // [sp+8h] [bp-38h]@5
  char Filename[20]; // [sp+28h] [bp-18h]@1
  unsigned int *v16; // [sp+3Ch] [bp-4h]@11
  int a1a; // [sp+48h] [bp+8h]@11

  v2 = this;
  v3 = this->uDumpsCount;
  this->uDumpsCount = v3 + 1;
  sprintf(Filename, "Mem%03i.txt", v3);
  v4 = fopen(Filename, "w");
  v5 = v4;
  if ( v4 )
  {
    if ( a1 )
      fprintf(v4, "%s\n\n", a1);
    fprintf(v5, "Windows Memory Status");
    GlobalMemoryStatus(&memstat);
    fprintf(v5, "dwLength:         %d\n\n", memstat.dwLength);
    fprintf(v5, "dwMemoryLoad:     %d\n", memstat.dwMemoryLoad);
    fprintf(v5, "dwTotalPhys:      %d\n", memstat.dwTotalPhys);
    fprintf(v5, "dwAvailPhys:      %d\n", memstat.dwAvailPhys);
    fprintf(v5, "dwTotalPageFile:  %d\n", memstat.dwTotalPageFile);
    fprintf(v5, "dwAvailPageFile:  %d\n", memstat.dwAvailPageFile);
    fprintf(v5, "dwTotalVirtual:   %d\n", memstat.dwTotalVirtual);
    fprintf(v5, "dwAvailVirtual:   %d\n\n\n", memstat.dwAvailVirtual);
    v7 = v2->uNumBuffersUsed;
    v8 = 0;
    if ( (signed int)v7 > 0 )
    {
      v9 = (int)v2->uMemoryBuffersSizes;
      do
      {
        v8 += *(int *)v9;
        v9 += 4;
        --v7;
      }
      while ( v7 );
    }
    v10 = 0;
    v11 = (int)v2->uMemoryBuffersSizes;
    v12 = 6000;
    do
    {
      v10 += *(int *)v11;
      v11 += 4;
      --v12;
    }
    while ( v12 );
    fprintf(v5, "Size: %i  %i  %i\n\n", v2->uNumBuffersUsed, v8, v10);
    v13 = 0;
    if ( (signed int)v2->uNumBuffersUsed > 0 )
    {
      v16 = v2->uMemoryBuffersSizes;
      a1a = (int)v2->pMemoryBuffersNames;
      do
      {
        fprintf(v5, "%10i %10i %s\n", v13, *v16, a1a);
        a1a += 12;
        ++v16;
        ++v13;
      }
      while ( v13 < (signed int)v2->uNumBuffersUsed );
    }
    fclose(v5);
    result = 0;
  }
  else
  {
    result = 1;
  }
  return result;
}








//----- (00426892) --------------------------------------------------------
static void CheckMemoryAccessibility(void *pBase, DWORD uSize)
{
  DWORD v2; // edi@1
  DWORD i; // eax@1
  struct _SYSTEM_INFO SystemInfo; // [sp+8h] [bp-24h]@1

  v2 = uSize;
  GetSystemInfo(&SystemInfo);
  for ( i = 16 * SystemInfo.dwPageSize; i < v2; i += SystemInfo.dwPageSize )
    ;
}





//----- (0045D3C7) --------------------------------------------------------
bool LightmapBuilder::_45D3C7_sw(struct Polygon *a1)
{
  LightmapBuilder *v2; // ebx@1
  struct Polygon *result; // eax@1
  float v4; // ecx@2
  Span *i; // edi@5

  v2 = this;
  result = (struct Polygon *)a1->prolly_head;
  if ( result && (v4 = result->field_4, *(int *)&v4) != 0 && *(unsigned int *)(LODWORD(v4) + 60) )
  {
    for ( i = a1->prolly_head; ; i = i->pNext )
    {
      result = i->pParent;
      if ( result->field_108 )
        result = (struct Polygon *)_45D426_sw(
                              i,
                              a1->pEdgeList1,
                              a1->uEdgeList1Size,
                              (Edge *)a1->pEdgeList2,
                              a1->uEdgeList2Size);
      if ( a1->prolly_tail == i )
        break;
    }
    LOBYTE(result) = 1;
  }
  else
  {
    LOBYTE(result) = 0;
  }
  return (bool)result;
}

//----- (0045D426) --------------------------------------------------------
int LightmapBuilder::_45D426_sw(Span *a1, Edge **a2, unsigned int a3, Edge *a4, int a5)
{
  int v6; // edi@1
  signed int v7; // ebx@3
  signed int v8; // ecx@3
  Span *v9; // esi@3
  double v10; // st7@4
  Edge *v11; // edx@5
  double v12; // st7@11
  int v13; // edx@12
  int v14; // ebx@17
  Edge *v15; // edi@17
  float v16; // ST04_4@17
  float v17; // ST04_4@17
  double v18; // st7@17
  int result; // eax@19
  //double v20; // ST14_8@20
  //double v21; // ST14_8@20
  double v22; // st7@20
  int v23; // ecx@20
  double v24; // st6@20
  double v25; // st7@23
  double v26; // st7@28
  __int16 v27; // fps@28
  char v28; // c0@28
  char v29; // c2@28
  char v30; // c3@28
  double v31; // st7@29
  double v32; // st7@30
  __int16 v33; // fps@30
  char v34; // c0@30
  char v35; // c2@30
  char v36; // c3@30
  LightmapBuilder *this_4; // [sp+14h] [bp-8h]@1
  signed int v38; // [sp+18h] [bp-4h]@3
  float a1a; // [sp+24h] [bp+8h]@20
  float a2a; // [sp+28h] [bp+Ch]@17
  float a3a; // [sp+2Ch] [bp+10h]@17
  float a5a; // [sp+34h] [bp+18h]@20
  float a5b; // [sp+34h] [bp+18h]@20

  v6 = 0;
  this_4 = this;
  if ( a5 && a3 )
  {
    v7 = -1;
    v8 = 0;
    v9 = a1;
    v38 = -1;
    if ( (signed int)a3 > 0 )
    {
      v10 = (double)a1->field_A;
      while ( 1 )
      {
        v11 = a2[v8];
        if ( v10 >= v11->field_2C )
        {
          if ( v10 <= v11->field_30 )
            break;
        }
        ++v8;
        if ( v8 >= (signed int)a3 )
          goto LABEL_10;
      }
      v38 = v8;
    }
LABEL_10:
    if ( a5 > 0 )
    {
      v12 = (double)a1->field_A;
      while ( 1 )
      {
        v13 = *((unsigned int *)&a4->field_0 + v6);
        if ( v12 >= *(float *)(v13 + 44) )
        {
          if ( v12 <= *(float *)(v13 + 48) )
            break;
        }
        ++v6;
        if ( v6 >= a5 )
          goto LABEL_17;
      }
      v7 = v6;
    }
LABEL_17:
    v14 = *((unsigned int *)&a4->field_0 + v7);
    v15 = a2[v38];
    v16 = (double)a1->field_A;
    a3a = _45D643_sw(a2[v38], v16);
    v17 = (double)a1->field_A;
    v18 = _45D643_sw((Edge *)v14, v17);
    a2a = v18;
    if ( byte_4D864C && BYTE1(pEngine->uFlags) & 0x80 )
    {
      HIWORD(result) = HIWORD(a3a);
      a1->field_14 = v18;
      a1->field_10 = a3a;
    }
    else
    {
      a5a = ((double)a1->field_A - v15->field_2C) * v15->field_4 + v15->field_24 - 0.5;
      //v20 = a5a + 6.7553994e15;
      //a5b = (double)SLODWORD(v20);
      a5b = (double)floorf(a5a + 0.5f);
      a1a = ((double)a1->field_A - *(float *)(v14 + 44)) * *(float *)(v14 + 4) + *(float *)(v14 + 36) + 0.5;
      //v21 = a1a + 6.7553994e15;
      //v22 = (double)SLODWORD(v21);
      v22 = (double)floorf(a1a + 0.5f);
      result = v9->field_8;
      v23 = v9->field_C;
      v24 = -((a3a - a2a) / (v22 - a5b));
      v9->field_10 = ((double)result - a5b) * v24 + a3a;
      v9->field_14 = a2a - (v22 - (double)(result + v23)) * v24;
    }
    if ( v9->field_10 < 0.0 || v9->field_10 <= 1.0 )
    {
      if ( v9->field_10 >= 0.0 )
        v25 = v9->field_10;
      else
        v25 = 0.0;
    }
    else
    {
      v25 = 1.0;
    }
    v9->field_10 = v25;
      __debugbreak(); // warning C4700: uninitialized local variable 'v27' used
    if ( v9->field_14 < 0.0
      || (v26 = v9->field_14,
          //UNDEF(v27),
          v28 = 1.0 < v26,
          v29 = 0,
          v30 = 1.0 == v26,
          BYTE1(result) = HIBYTE(v27),
          v26 <= 1.0) )
    {
      v32 = v9->field_14;
      //UNDEF(v33);
      v34 = 0.0 < v32;
      v35 = 0;
      v36 = 0.0 == v32;
      __debugbreak(); // warning C4700: uninitialized local variable 'v33' used
      BYTE1(result) = HIBYTE(v33);
      if ( v32 >= 0.0 )
        v31 = v9->field_14;
      else
        v31 = 0.0;
    }
    else
    {
      v31 = 1.0;
    }
    v9->field_14 = v31;
    LOBYTE(result) = 1;
  }
  else
  {
    LOBYTE(result) = 0;
  }
  return result;
}

//----- (0045D643) --------------------------------------------------------
double LightmapBuilder::_45D643_sw(Edge *a1, float a2)
{
  double result; // st7@1

  result = (a1->field_1C - a1->field_20) * -1.0 / (a1->field_30 - a1->field_2C) * (a2 - a1->field_2C) + a1->field_1C;
  if ( result < 0.0 || result <= 1.0 )
  {
    if ( result < 0.0 )
      result = 0.0;
  }
  else
  {
    result = 1.0;
  }
  return result;
}


//  143
#pragma pack(push, 1)
struct Span
{
  Span *pNext;
  struct Polygon *pParent;
  __int16 field_8;
  __int16 field_A;
  __int16 field_C;
  __int16 field_E;
  float field_10;
  float field_14;
};
#pragma pack(pop)



//  144
#pragma pack(push, 1)
struct Surf
{
  float field_0;
  float field_4;
  float field_8;
  float field_C;
  float field_10;
  Surf *pNext;
  Surf *pPrev;
  struct Polygon *pParent;
  __int16 field_20;
  __int16 field_22;
};
#pragma pack(pop)
extern Surf stru_80C980;




//  145
#pragma pack(push, 1)
struct Edge
{
  float field_0;
  float field_4;
  int field_8;
  Surf *pSurf;
  Edge *pNext;
  Edge *pPrev;
  Edge *ptr_18;
  float field_1C;
  float field_20;
  float field_24;
  float field_28;
  float field_2C;
  float field_30;
};
#pragma pack(pop)
extern Edge defaultEdge;

extern Edge stru_80C9A4;
extern Edge stru_80C9D8;



#define INDOOR_CAMERA_DRAW_SW_OUTLINES 1
#define INDOOR_CAMERA_DRAW_D3D_OUTLINES 2
#define INDOOR_CAMERA_DRAW_TERRAIN_OUTLINES 4
//  157
#pragma pack(push, 1)
struct IndoorCamera
{
  //----- (0042391F) --------------------------------------------------------
  IndoorCamera()
  {
    sRotationX = 0;
    sRotationY = 0;
    fov_rad = 2000.0f;
    fov_rad_inv = 32.768002f;

    flags = 0;
  }
  void Initialize(int degFov, unsigned int uViewportWidth, unsigned int uViewportHeight);
  void Initialize2();
  float ViewTransform_ODM(struct RenderVertexSoft *a1);


  Vec3_int_ pos;
  unsigned int uMapGridCellX;
  unsigned int uMapGridCellZ;
  int sRotationX;
  int sRotationY;
  float fov_rad;
  float fov_rad_inv;
  int field_24;
  int field_28;
  float fRotationYSine;
  float fRotationYCosine;
  float fRotationXSine;
  float fRotationXCosine;
  int _int_sine;
  int _int_cosine;
  int _int_sine_2;
  int _int_cosine_2;
  int flags;  // & INDOOR_CAMERA_DRAW_D3D_OUTLINES:  render terrain outlines in d3d
};
#pragma pack(pop)



extern IndoorCamera *pIndoorCamera; // idb


IndoorCamera *pIndoorCamera; // idb





//----- (00481CCE) --------------------------------------------------------
float IndoorCamera::ViewTransform_ODM(RenderVertexSoft *a1)
{
  float result; // eax@1
  double vCamToVertexZ; // st7@1
  double v3; // st6@1
  double v4; // st5@1
  double v5; // st4@1
  float v6; // ST04_4@3
  float v7; // [sp+0h] [bp-14h]@1
  float v8; // [sp+8h] [bp-Ch]@1
  float vCamToVertexX; // [sp+Ch] [bp-8h]@1
  float vCamToVertexY; // [sp+10h] [bp-4h]@1

  v8 = fRotationXCosine;
  result = fRotationXSine;
  v7 = fRotationXSine;
  vCamToVertexX = a1->vWorldPosition.x - (double)pos.x;
  vCamToVertexY = a1->vWorldPosition.y - (double)pos.y;
  vCamToVertexZ = a1->vWorldPosition.z - (double)pos.z;
  v3 = fRotationYCosine;
  v4 = fRotationYSine;
  v5 = vCamToVertexY * fRotationYSine + fRotationYCosine * vCamToVertexX;
  if ( sRotationX )
  {
    v6 = vCamToVertexY * fRotationYSine + fRotationYCosine * vCamToVertexX;
    a1->vWorldViewPosition.x = v5 * fRotationXCosine + fRotationXSine * vCamToVertexZ;
    a1->vWorldViewPosition.y = v3 * vCamToVertexY - v4 * vCamToVertexX;
    a1->vWorldViewPosition.z = v8 * vCamToVertexZ - v6 * v7;
  }
  else
  {
    a1->vWorldViewPosition.x = vCamToVertexY * fRotationYSine + fRotationYCosine * vCamToVertexX;
    a1->vWorldViewPosition.y = v3 * vCamToVertexY - v4 * vCamToVertexX;
    a1->vWorldViewPosition.z = vCamToVertexZ;
  }
  return result;
}



//----- (0042394D) --------------------------------------------------------
void IndoorCamera::Initialize(int degFov, unsigned int uViewportWidth, unsigned int uViewportHeight)
{
  IndoorCamera *v4; // esi@1
  double v5; // st7@1

  v4 = this;
  v5 = (double)(signed int)(signed __int64)((double)(signed int)uViewportWidth
                                          * 0.5
                                          / tan((double)(degFov >> 1) * 0.01745329)
                                          + 0.5);
  v4->fov_rad = v5;
  v4->fov_rad_inv = 65536.0 / v5;
}

//----- (004239A7) --------------------------------------------------------
void IndoorCamera::Initialize2()
{
  //IndoorCamera *v1; // esi@1
  double v2; // st7@4
  //double v3; // st7@6
  //int v4; // eax@6
  //int v5; // eax@6
  //int v6; // ST04_4@6
  //int v7; // eax@6
  //int v8; // ST04_4@6

  //v1 = this;
  fRotationYSine = sin((3.141592653589793 + 3.141592653589793) * (double)sRotationY * 0.00048828125);
  fRotationYCosine = cos((3.141592653589793 + 3.141592653589793) * (double)sRotationY * 0.00048828125);
  if ( byte_4D864C && pEngine->uFlags & 0x80 || uCurrentlyLoadedLevelType == LEVEL_Indoor)
  {
    fRotationXSine = sin((3.141592653589793 + 3.141592653589793) * (double)-sRotationX * 0.00048828125);
    v2 = (3.141592653589793 + 3.141592653589793) * (double)-sRotationX;
  }
  else
  {
    fRotationXSine = sin((3.141592653589793 + 3.141592653589793) * (double)sRotationX * 0.00048828125);
    v2 = (3.141592653589793 + 3.141592653589793) * (double)sRotationX;
  }
  //v3 = cos(v2 * 0.00048828125);
  //v4 = sRotationY;
  fRotationXCosine = cos(v2 * 0.00048828125);
  //v5 = stru_5C6E00->SinCos(sRotationY - stru_5C6E00->uIntegerHalfPi);
  //v6 = sRotationY;
  _int_sine = stru_5C6E00->Sin(sRotationY);
  _int_cosine = stru_5C6E00->Cos(sRotationY);
  //v7 = stru_5C6E00->SinCos(sRotationX - stru_5C6E00->uIntegerHalfPi);
  //v8 = sRotationX;
  _int_sine_2 = stru_5C6E00->Sin(sRotationX);
  _int_cosine_2 = stru_5C6E00->Cos(sRotationX);
}
//----- (0048600E) --------------------------------------------------------
void ODMRenderParams::RotationToInts()
{
  camera_rotation_y_int_sine   = stru_5C6E00->Sin(pIndoorCameraD3D->sRotationY);
  camera_rotation_y_int_cosine = stru_5C6E00->Cos(pIndoorCameraD3D->sRotationY);
  camera_rotation_x_int_sine   = stru_5C6E00->Sin(pIndoorCameraD3D->sRotationX);
  camera_rotation_x_int_cosine = stru_5C6E00->Cos(pIndoorCameraD3D->sRotationX);
}
//----- (004A169E) --------------------------------------------------------
bool Render::UsingDirect3D()
{
  return bUserDirect3D == 0;
}


//----- (004A46E6) --------------------------------------------------------
int sr_4A46E6_draw_particle_segment(unsigned int x, signed int y, signed int _z, int a4, unsigned int lightColor)
{
  int v5; // eax@1
  int z; // eax@1
  unsigned int v7; // eax@9
  unsigned int v8; // ecx@9
  int v9; // eax@9
  unsigned int v10; // eax@10
  int *v11; // esi@10
  int *v12; // edi@10
  int v13; // ecx@10
  int v14; // edx@10
  unsigned int v15; // eax@22
  int *v16; // esi@22
  int *v17; // edi@22
  int v18; // ecx@22
  int v19; // edx@22
  unsigned __int16 *pTarget; // [sp+Ch] [bp-8h]@9
  int *pTargetZ; // [sp+10h] [bp-4h]@9
  unsigned int v22; // [sp+1Ch] [bp+8h]@9
  signed int v23; // [sp+20h] [bp+Ch]@1

  v5 = a4;
  v23 = _z >> 16;
  z = x + v5;
  if ( z >= (signed int)pViewport->uViewportTL_X
    && (signed int)x <= (signed int)pViewport->uViewportBR_X
    && y >= (signed int)pViewport->uViewportTL_Y
    && y <= (signed int)pViewport->uViewportBR_Y )
  {
    if ( (signed int)x < (signed int)pViewport->uViewportTL_X )
      x = pViewport->uViewportTL_X;
    if ( z > (signed int)pViewport->uViewportBR_X )
      z = pViewport->uViewportBR_X;
    pTarget = &render->pTargetSurface[x + y * render->uTargetSurfacePitch];
    v22 = z - x;
    pTargetZ = &render->pActiveZBuffer[x + 640 * y];
    v7 = lightColor >> 3;
    v8 = lightColor & 0xF0;
    v9 = v7 & 0x1E0000;
    if ( render->uTargetGBits == 5 )
    {
      v10 = (v8 | (((unsigned __int16)(lightColor & 0xF000) | (unsigned int)v9) >> 3)) >> 4;
      v11 = (int *)pTarget;
      v12 = pTargetZ;
      v13 = v22;
      v14 = (v10 << 16) | v10;
      z = (unsigned __int8)pTarget & 2;
      if ( (unsigned __int8)pTarget & 2 )
      {
        z = (unsigned int)*pTargetZ >> 16;
        if ( z > v23 )
        {
          z = v14 + ((*pTarget & 0x7BDEu) >> 1);
          *pTarget = z;
        }
        v13 = v22 - 1;
        v11 = (int *)(pTarget + 1);
        v12 = pTargetZ + 1;
      }
      while ( v13 != 1 )
      {
        if ( v13 < 1 )
          return z;
        z = (unsigned int)*v12 >> 16;
        if ( z <= v23 )
        {
          v13 -= 2;
          ++v11;
          v12 += 2;
        }
        else
        {
          v12 += 2;
          z = v14 + ((*v11 & 0x7BDE7BDEu) >> 1);
          v13 -= 2;
          *v11 = z;
          ++v11;
        }
      }
      z = (unsigned int)*v12 >> 16;
      if ( z > v23 )
      {
        z = v14 + ((*(short *)v11 & 0x7BDEu) >> 1);
        *(short *)v11 = z;
      }
    }
    else
    {
      v15 = (v8 | (((unsigned __int16)(lightColor & 0xF800) | (unsigned int)v9) >> 2)) >> 4;
      v16 = (int *)pTarget;
      v17 = pTargetZ;
      v18 = v22;
      v19 = (v15 << 16) | v15;
      z = (unsigned __int8)pTarget & 2;
      if ( (unsigned __int8)pTarget & 2 )
      {
        z = (unsigned int)*pTargetZ >> 16;
        if ( z > v23 )
        {
          z = v19 + ((*pTarget & 0xF7DEu) >> 1);
          *pTarget = z;
        }
        v18 = v22 - 1;
        v16 = (int *)(pTarget + 1);
        v17 = pTargetZ + 1;
      }
      while ( v18 != 1 )
      {
        if ( v18 < 1 )
          return z;
        z = (unsigned int)*v17 >> 16;
        if ( z <= v23 )
        {
          v18 -= 2;
          ++v16;
          v17 += 2;
        }
        else
        {
          v17 += 2;
          z = v19 + ((*v16 & 0xF7DEF7DEu) >> 1);
          v18 -= 2;
          *v16 = z;
          ++v16;
        }
      }
      z = (unsigned int)*v17 >> 16;
      if ( z > v23 )
      {
        z = v19 + ((*(short *)v16 & 0xF7DEu) >> 1);
        *(short *)v16 = z;
      }
    }
  }
  return z;
}


//----- (004A57E9) --------------------------------------------------------
void Present_ColorKey()
{
  HRESULT v0; // eax@3
  HRESULT v1; // eax@3
  HRESULT v2; // eax@3
  HRESULT v3; // eax@3
  HRESULT v4; // eax@3
  RECT a2; // [sp+4h] [bp-14h]@3
  //CheckHRESULT_stru0 this; // [sp+14h] [bp-4h]@3

  if ( !render->uNumSceneBegins )
  {
    if ( render->using_software_screen_buffer )
    {
      a2.bottom = pViewport->uViewportTL_Y;
      a2.left = 0;
      a2.top = 0;
      a2.right = 640;
      ErrD3D(render->pBackBuffer4->BltFast(0, 0, render->pColorKeySurface4, &a2, 16u));
      a2.right = 640;
      a2.left = 0;
      a2.top = pViewport->uViewportBR_Y + 1;
      a2.bottom = 480;
      ErrD3D(render->pBackBuffer4->BltFast(
             0,
             pViewport->uViewportBR_Y + 1,
             render->pColorKeySurface4,
             &a2,
             16u));
      a2.right = pViewport->uViewportTL_X;
      a2.bottom = pViewport->uViewportBR_Y + 1;
      a2.left = 0;
      a2.top = pViewport->uViewportTL_Y;
      ErrD3D(render->pBackBuffer4->BltFast(
             0,
             pViewport->uViewportTL_Y,
             render->pColorKeySurface4,
             &a2,
             16u));
      a2.left = pViewport->uViewportBR_X;
      a2.top = pViewport->uViewportTL_Y;
      a2.right = 640;
      a2.bottom = pViewport->uViewportBR_Y + 1;
      ErrD3D(render->pBackBuffer4->BltFast(
             pViewport->uViewportBR_X,
             pViewport->uViewportTL_Y,
             render->pColorKeySurface4,
             &a2,
             16u));
      a2.right = pViewport->uViewportBR_X;
      a2.bottom = pViewport->uViewportBR_Y + 1;
      a2.left = pViewport->uViewportTL_X;
      a2.top = pViewport->uViewportTL_Y;
      ErrD3D(render->pBackBuffer4->BltFast(
             pViewport->uViewportTL_X,
             pViewport->uViewportTL_Y,
             render->pColorKeySurface4,
             &a2,
             17u));
    }
  }
}


//----- (004A48E4) --------------------------------------------------------
int Render::MakeParticleBillboardAndPush_BLV_Software(int screenSpaceX, int screenSpaceY, int z, int lightColor, int a6)
{
  int v6; // ecx@1
  int v7; // ebx@1
  int v8; // ecx@1
  int v9; // edx@1
  int v10; // edi@1
  unsigned int x; // esi@1
  int result; // eax@8
  int v13; // [sp+Ch] [bp-10h]@1
  int v14; // [sp+10h] [bp-Ch]@1
  int v15; // [sp+14h] [bp-8h]@1
  int v16; // [sp+18h] [bp-4h]@1
  int v17; // [sp+24h] [bp+8h]@1
  unsigned int v18; // [sp+28h] [bp+Ch]@1
  int v19; // [sp+34h] [bp+18h]@1

  v6 = screenSpaceX;
  v7 = (a6 >> 1) + screenSpaceY;
  v17 = 0;
  v15 = 0;
  v8 = (a6 >> 1) + v6;
  v14 = (a6 >> 1) * (a6 >> 1);
  v9 = 2 * (a6 >> 1);
  v10 = (a6 >> 1) * ((a6 >> 1) - 1);
  x = v8 - (a6 >> 1);
  v16 = (a6 >> 1) + screenSpaceY - v8;
  v19 = a6 >> 1;
  v13 = v9;
  v18 = v8;
  do
  {
    sr_4A46E6_draw_particle_segment(x, v16 + v18, z, 2 * v19, lightColor);
    if ( v15 )
      sr_4A46E6_draw_particle_segment(x, v17 + v7, z, 2 * v19, lightColor);
    v14 -= v15;
    if ( v14 <= v10 )
    {
      if ( v19 != v17 )
      {
        sr_4A46E6_draw_particle_segment(v18, v16 + x, z, 2 * v17, lightColor);
        sr_4A46E6_draw_particle_segment(v18, v19 + v7, z, 2 * v17, lightColor);
      }
      --v19;
      v13 -= 2;
      ++x;
      v10 -= v13;
    }
    result = v17++;
    v15 += 2;
    --v18;
  }
  while ( result < v19 );
  return result;
}

//----- (004637EB) --------------------------------------------------------
int __stdcall aWinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  //HANDLE v6; // eax@32
  //HDC v10; // edi@50
  //int v11; // esi@50
  //signed int v13; // eax@135
  //char v29; // dl@209
  //bool v31; // ebx@211
  //float v33; // ST04_4@246
  //float v34; // ST04_4@254
  //struct tagPAINTSTRUCT Paint; // [sp+24h] [bp-48h]@13
  //int pXY[2]; // [sp+64h] [bp-8h]@261
  //int a2; // [sp+7Ch] [bp+10h]@50

  switch (uMsg)
  {
    case WM_SIZING: return 1;
    
    case WM_CREATE:  case WM_NCCREATE:
    case WM_GETTEXT: case WM_SETTEXT:
    case WM_SHOWWINDOW:
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_DESTROY:
      SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
      PostQuitMessage(0);
      return 0;

    case WM_WINDOWPOSCHANGED:
    {
      if (pVideoPlayer && pVideoPlayer->AnyMovieLoaded() && pVideoPlayer->pBinkBuffer)
        BinkBufferSetOffset(pVideoPlayer->pBinkBuffer, 0, 0);

      return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    case WM_CHAR:
    {
      if (!pKeyActionMap->_459F10(wParam) && !viewparams->field_4C)
        GUI_HandleHotkey(wParam);
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);


    case WM_DEVICECHANGE:
    {
      if (wParam == 0x8000)          // CD or some device has been inserted - notify InsertCD dialog
        PostMessageW(hInsertCDWindow, WM_USER + 1, 0, 0);
      return 0;
    }

    case WM_COMMAND:
    {
      switch (wParam)
      {
        case 103:  render->SavePCXScreenshot();  return 0;

        case 101:  // Quit game
        case 40001:
          SendMessageW(hWnd, WM_DESTROY, 0, 0);
        return 0;


        case 104:
          render->ChangeBetweenWinFullscreenModes();
          if ( pArcomageGame->bGameInProgress )
            pArcomageGame->field_F6 = 1;
        return 0;

        //SubMenu "Party"
        case 40007:  pParty->SetGold(pParty->uNumGold + 10000); return 0;
        case 40008:  GivePartyExp(20000);  return 0;
        case 40013:  pParty->SetGold(0);   return 0;

        case 40059:
          for (uint i = 0; i < 4; ++i)
            pParty->pPlayers[i].uSkillPoints = 50;
        return 0;
        
        case 40029:  pPlayers[uActiveCharacter]->SetPertified(true);  return 0;
        case 40030:  pPlayers[uActiveCharacter]->SetWeak(true);       return 0;
        case 40031:  pPlayers[uActiveCharacter]->SetPoison3(true);    return 0;
        case 40032:  pPlayers[uActiveCharacter]->SetPoison2(true);    return 0;
        case 40033:  pPlayers[uActiveCharacter]->SetPoison1(true);    return 0;
        case 40034:  pPlayers[uActiveCharacter]->SetDisease3(true);   return 0;
        case 40035:  pPlayers[uActiveCharacter]->SetDisease2(true);   return 0;
        case 40036:  pPlayers[uActiveCharacter]->SetDisease1(true);   return 0;
        case 40037:  pPlayers[uActiveCharacter]->SetCursed(true);     return 0;
        case 40038:  pPlayers[uActiveCharacter]->SetInsane(true);     return 0;
        case 40039:  pPlayers[uActiveCharacter]->SetDrunk(true);      return 0;
        case 40040:  pPlayers[uActiveCharacter]->SetUnconcious(true); return 0;
        case 40041:  pPlayers[uActiveCharacter]->SetDead(true);       return 0;
        case 40042:  pPlayers[uActiveCharacter]->SetEradicated(true); return 0;
        case 40043:  pPlayers[uActiveCharacter]->SetAsleep(true);     return 0;
        case 40044:  pPlayers[uActiveCharacter]->SetAfraid(true);     return 0;
        case 40045:  pPlayers[uActiveCharacter]->SetParalyzed(true);  return 0;
        case 40073:  pPlayers[uActiveCharacter]->SetZombie(true);     return 0;

        case 40006:  pParty->SetFood(pParty->uNumFoodRations + 20);   return 0;

        case 40062:
          pParty->alignment = PartyAlignment_Good;
          SetUserInterface(pParty->alignment, true);
          return 0;
        case 40063:
          pParty->alignment = PartyAlignment_Neutral;
          SetUserInterface(pParty->alignment, true);
          return 0;
        case 40064:
          pParty->alignment = PartyAlignment_Evil;
          SetUserInterface(pParty->alignment, true);
          return 0;

        //SubMenu "Time"
        case 40009:  pParty->uTimePlayed += Timer::Day;   return 0;
        case 40010:  pParty->uTimePlayed += Timer::Week;   return 0;
        case 40011:  pParty->uTimePlayed += Timer::Month;   return 0;
        case 40012:  pParty->uTimePlayed += Timer::Year;   return 0;

        //SubMenu "Items"
        case 40015:
        {
          int pItemID = rand() % 500;
          for ( uint i = 0; i < 500; ++i )
          {
            if ( pItemID + i > 499 )
              pItemID = 0;
            if ( pItemsTable->pItems[pItemID + i].uItemID_Rep_St == 1 )
            {
              pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
              return 0;
            }
          }
        }
        return 0;
        case 40016:
        {
          int pItemID = rand() % 500;
          for ( uint i = 0; i < 500; ++i )
          {
            if ( pItemID + i > 499 )
              pItemID = 0;
            if ( pItemsTable->pItems[pItemID + i].uItemID_Rep_St == 2 )
            {
              pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
              return 0;
            }
          }
        }
        return 0;
        case 40017:
        {
          int pItemID = rand() % 500;
          for ( uint i = 0; i < 500; ++i )
          {
            if ( pItemID + i > 499 )
              pItemID = 0;
            if ( pItemsTable->pItems[pItemID + i].uItemID_Rep_St == 3 )
            {
              pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
              return 0;
            }
          }
        }
        return 0;
        case 40018:
        {
          int pItemID = rand() % 500;
          for ( uint i = 0; i < 500; ++i )
          {
            if ( pItemID + i > 499 )
              pItemID = 0;
            if ( pItemsTable->pItems[pItemID + i].uItemID_Rep_St == 4 )
            {
              pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
              return 0;
            }
          }
        }
        return 0;
        case 40019:
        {
          int pItemID = rand() % 500;
          for ( uint i = 0; i < 500; ++i )
          {
            if ( pItemID + i > 499 )
              pItemID = 0;
            if ( pItemsTable->pItems[pItemID + i].uItemID_Rep_St == 5 )
            {
              pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
              return 0;
            }
          }
        }
        return 0;
        case 40020:
        {
          int pItemID = rand() % 500;
          for ( uint i = 0; i < 500; ++i )
          {
            if ( pItemID + i > 499 )
              pItemID = 0;
            if ( pItemsTable->pItems[pItemID + i].uItemID_Rep_St == 6 )
            {
              pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
              return 0;
            }
          }
        }
        return 0;
        case 40061:
        {
          int pItemID = rand() % 500;
          for ( uint i = 0; i < 500; ++i )
          {
            if ( pItemID + i > 499 )
              pItemID = 0;
            if ( pItemsTable->pItems[pItemID + i].uItemID_Rep_St > 6 )
            {
              pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
              return 0;
            }
          }
        }
        return 0;

      }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_LBUTTONDOWN:
      if (pArcomageGame->bGameInProgress)
      {
        pArcomageGame->stru1.field_0 = 7;
        ArcomageGame::OnMouseClick(0, true);
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }

      goto __handle_mouse_click;

    case WM_RBUTTONDOWN:
      if (pArcomageGame->bGameInProgress)
      {
        pArcomageGame->stru1.field_0 = 8;
        ArcomageGame::OnMouseClick(1, true);
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }

      if (pVideoPlayer->pVideoFrame.pPixels)
        pVideoPlayer->bStopBeforeSchedule = 1;

      pMouse->SetMouseClick(LOWORD(lParam), HIWORD(lParam));

      if (pEngine)
      {
        pEngine->PickMouse(pIndoorCameraD3D->GetPickDepth(), LOWORD(lParam), HIWORD(lParam), 0, &vis_sprite_filter_2, &vis_door_filter);
      }

      UI_OnMouseRightClick(0);
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);


    case WM_LBUTTONUP:
      if ( !pArcomageGame->bGameInProgress )
      {
        back_to_game();
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }
      pArcomageGame->stru1.field_0 = 3;
      ArcomageGame::OnMouseClick(0, 0);
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    case WM_RBUTTONUP:
      if ( !pArcomageGame->bGameInProgress )
      {
        back_to_game();
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }
      pArcomageGame->stru1.field_0 = 4;
      ArcomageGame::OnMouseClick(1, false);
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_LBUTTONDBLCLK:
      if ( pArcomageGame->bGameInProgress )
      {
        pArcomageGame->stru1.field_0 = 7;
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }
        
__handle_mouse_click:
      if (pVideoPlayer->pVideoFrame.pPixels)
        pVideoPlayer->bStopBeforeSchedule = 1;

      pMouse->SetMouseClick(LOWORD(lParam), HIWORD(lParam));

      if (GetCurrentMenuID() == MENU_CREATEPARTY)
      {
        UI_OnKeyDown(VK_SELECT);
      }

      if (pEngine)
        pEngine->PickMouse(512.0, LOWORD(lParam), HIWORD(lParam), false, &vis_sprite_filter_3, &vis_door_filter);

      UI_OnMouseLeftClick(0);

      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_RBUTTONDBLCLK:
      if (pArcomageGame->bGameInProgress)
      {
        pArcomageGame->stru1.field_0 = 8;
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }

      if (pVideoPlayer->pVideoFrame.pPixels)
        pVideoPlayer->bStopBeforeSchedule = true;

      pMouse->SetMouseClick(LOWORD(lParam), HIWORD(lParam));

      if (pEngine)
        pEngine->PickMouse(pIndoorCameraD3D->GetPickDepth(), LOWORD(lParam), HIWORD(lParam), false, &vis_sprite_filter_2, &vis_door_filter);

      UI_OnMouseRightClick(0);
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);


    case WM_MBUTTONDOWN:
      if (render->pRenderD3D && pEngine)
      {
        pEngine->PickMouse(pIndoorCameraD3D->GetPickDepth(), LOWORD(lParam), HIWORD(lParam), 1, &vis_sprite_filter_3, &vis_face_filter);
      }

      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_MOUSEMOVE:
      if ( pArcomageGame->bGameInProgress )
      {
        ArcomageGame::OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        ArcomageGame::OnMouseClick(0, wParam == MK_LBUTTON);
        ArcomageGame::OnMouseClick(1, wParam == MK_RBUTTON);
      }
      else
      {
        pMouse->SetMouseClick(LOWORD(lParam), HIWORD(lParam));
      }

      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_SYSCOMMAND:
      if ( wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER )
        return 0;

      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    //case WM_KEYUP:
    //  if (wParam == VK_CONTROL)
    //  {
    //    dword_507B98_ctrl_pressed = 0;
    //  }

    //  return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_KEYDOWN:
      if ( uGameMenuUI_CurentlySelectedKeyIdx != -1 )
      {
        pKeyActionMap->_459F10(wParam);
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }
      if ( !pArcomageGame->bGameInProgress )
      {
        if ( pVideoPlayer->pVideoFrame.pPixels )
          pVideoPlayer->bStopBeforeSchedule = 1;
        if ( wParam == VK_RETURN )
        {
          if ( !viewparams->field_4C )
            UI_OnKeyDown(wParam);
          return 0;
        }
        //if ( wParam == VK_CONTROL )
        //{
        //  dword_507B98_ctrl_pressed = 1;
        //  return 0;
        //}
        if ( wParam == VK_ESCAPE )
        {
          pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, window_SpeakInHouse != 0, 0);
          return 0;
        }
        if ( wParam <= VK_HOME )
          return 0;
        if ( wParam > VK_DOWN )
        {
          if ( wParam != VK_F4 || pVideoPlayer->AnyMovieLoaded() )
            return 0;
          SendMessageW(hWnd, WM_COMMAND, 104, 0);
          return 0;
        }
        if ( wParam >= VK_LEFT && wParam <= VK_DOWN )
        {
          if ( current_screen_type != SCREEN_GAME && current_screen_type != SCREEN_MODAL_WINDOW )
          {
            if ( !viewparams->field_4C )
              UI_OnKeyDown(wParam);
            return 0;
          }
        }
        if ( current_screen_type != SCREEN_GAME && current_screen_type != SCREEN_MODAL_WINDOW )
          return 0;
      }

      pArcomageGame->stru1.field_0 = 1;

      set_stru1_field_8_InArcomage(MapVirtualKey(wParam, MAPVK_VK_TO_CHAR));
      if ( wParam == 27 )
      {
        pArcomageGame->GameOver = 1;
        pArcomageGame->field_F4 = 1;
        pArcomageGame->uGameWinner = 2;
        pArcomageGame->Victory_type = -2;
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }
      if ( wParam != 114 )
      {
        if ( wParam == 115 && !pVideoPlayer->AnyMovieLoaded() )
          SendMessage(hWnd, WM_COMMAND, 0x68u, 0);
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      }
      SendMessageW(hWnd, WM_COMMAND, 103, 0);
      return 0;

    case WM_ACTIVATEAPP:
      if ( wParam && (GetForegroundWindow() == hWnd || GetForegroundWindow() == hInsertCDWindow) )
      {
        if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE)
        {
          dword_4E98BC_bApplicationActive = 1;
          if ( render->bWindowMode )
          {
            HDC hDC = GetDC(hWnd);
            int bitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL);
            int planes = GetDeviceCaps(hDC, PLANES);
            ReleaseDC(hWnd, hDC);
            if (bitsPerPixel != 16 || planes != 1)
              Error(pGlobalTXT_LocalizationStrings[62]);
          }
          dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_APP_INACTIVE;

          if ( pArcomageGame->bGameInProgress )
          {
            pArcomageGame->field_F9 = 1;
          }
          else
          {
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0200_EVENT_TIMER)
              dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0200_EVENT_TIMER;
            else
              pEventTimer->Resume();
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0400_MISC_TIMER)
              dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0400_MISC_TIMER;
            else
              pMiscTimer->Resume();

            viewparams->bRedrawGameUI = true;
            if ( pVideoPlayer->pSmackerMovie )
            {
              render->RestoreFrontBuffer();
              render->RestoreBackBuffer();
              BackToHouseMenu();
            }
          }
          if ( pAudioPlayer->hAILRedbook && !bGameoverLoop && !pVideoPlayer->pSmackerMovie )
            AIL_redbook_resume(pAudioPlayer->hAILRedbook);
        }
      }
      else
      {
        if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE))
        {
          dword_4E98BC_bApplicationActive = 0;
          if ( (pVideoPlayer->pSmackerMovie || pVideoPlayer->pBinkMovie) && pVideoPlayer->bPlayingMovie )
            pVideoPlayer->bStopBeforeSchedule = 1;

          //if (render->bUserDirect3D && render->uAcquiredDirect3DDevice == 1)
          if (render->uAcquiredDirect3DDevice == 1)
            SetWindowPos(hWnd, (HWND)0xFFFFFFFE, 0, 0, 0, 0, 0x18u);
          ClipCursor(0);
          dword_6BE364_game_settings_1 |= GAME_SETTINGS_APP_INACTIVE;
          if ( pEventTimer->bPaused )
            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0200_EVENT_TIMER;
          else
            pEventTimer->Pause();
          if ( pMiscTimer->bPaused )
            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0400_MISC_TIMER;
          else
            pMiscTimer->Pause();

          pAudioPlayer->StopChannels(-1, -1);
          if ( pAudioPlayer->hAILRedbook )
            AIL_redbook_pause(pAudioPlayer->hAILRedbook);
        }
      }
      return 0;

    case WM_SETFOCUS:
      dword_4E98BC_bApplicationActive = 0;
      if (render)
      {
        //if (render->bUserDirect3D && render->uAcquiredDirect3DDevice == 1)
        if (render->uAcquiredDirect3DDevice == 1)
          SetWindowPos(hWnd, (HWND)0xFFFFFFFE, 0, 0, 0, 0, 0x18u);
      }
      ClipCursor(0);
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_KILLFOCUS:
      dword_4E98BC_bApplicationActive = 1;
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    case WM_PAINT:
      if ( !GetUpdateRect(hWnd, 0, 0) || !dword_4E98BC_bApplicationActive && !render->bWindowMode )
        return 0;
      PAINTSTRUCT Paint;
      BeginPaint(hWnd, &Paint);
      if ( pArcomageGame->bGameInProgress )
      {
        pArcomageGame->field_F9 = 1;
      }
      else
      {
        //if (!render->pRenderD3D && !render->UsingDirect3D() || !AreRenderSurfacesOk())
        if (!AreRenderSurfacesOk())
        {
          EndPaint(hWnd, &Paint);
          return 0;
        }
      }
      render->Present();
      EndPaint(hWnd, &Paint);
      return 0;

    default:
      return DefWindowProcA(hWnd, uMsg, wParam, lParam);
  }
}

const wchar_t *UIMessage2String(UIMessageType msg)
{
  #define CASE(xxx) case xxx: swprintf(b, wcslen(L"%03X/%s"), L"%03X/%s", msg, L#xxx); return b;
  static wchar_t b[256]; // bad for threads
  switch (msg)
  {
    CASE(UIMSG_SelectSpell)
    CASE(UIMSG_ChangeGameState)
    CASE(UIMSG_Attack)
    CASE(UIMSG_PlayArcomage)
    CASE(UIMSG_MainMenu_ShowPartyCreationWnd)
    CASE(UIMSG_MainMenu_ShowLoadWindow)
    CASE(UIMSG_ShowCredits)
    CASE(UIMSG_ExitToWindows)
    CASE(UIMSG_PlayerCreationChangeName)
    CASE(UIMSG_PlayerCreationClickPlus)
    CASE(UIMSG_PlayerCreationClickMinus)
    CASE(UIMSG_PlayerCreationSelectActiveSkill)
    CASE(UIMSG_PlayerCreationSelectClass)
    CASE(UIMSG_PlayerCreationClickOK)
    CASE(UIMSG_PlayerCreationClickReset)
    CASE(UIMSG_ClickBooksBtn)
    CASE(UIMSG_PlayerCreationRemoveUpSkill)
    CASE(UIMSG_PlayerCreationRemoveDownSkill)
    CASE(UIMSG_SPellbook_ShowHightlightedSpellInfo)
    CASE(UIMSG_LoadGame)
    CASE(UIMSG_SaveGame)
    CASE(UIMSG_ShowStatus_DateTime)
    CASE(UIMSG_ShowStatus_ManaHP)
    CASE(UIMSG_ShowStatus_Player)
    CASE(UIMSG_Wait5Minutes)
    CASE(UIMSG_Wait1Hour)
    CASE(UIMSG_ShowStatus_Food)
    CASE(UIMSG_ShowStatus_Funds)
    CASE(UIMSG_QuickReference)
    CASE(UIMSG_GameMenuButton)
    CASE(UIMSG_AlreadyResting)
    CASE(UIMSG_SelectCharacter)
    CASE(UIMSG_ChangeSoundVolume)
    CASE(UIMSG_ChangeMusicVolume)
    CASE(UIMSG_Escape)
    CASE(UIMSG_PlayerCreation_SelectAttribute)
    CASE(UIMSG_InventoryLeftClick)
    CASE(UIMSG_SkillUp)
    CASE(UIMSG_GameMenu_ReturnToGame)
    CASE(UIMSG_StartNewGame)
    CASE(UIMSG_Game_OpenLoadGameDialog)
    CASE(UIMSG_Game_OpenSaveGameDialog)
    CASE(UIMSG_Game_OpenOptionsDialog)
    CASE(UIMSG_SetGraphicsMode)
    CASE(UIMSG_Quit)
    CASE(UIMSG_StartHireling1Dialogue)
    CASE(UIMSG_StartHireling2Dialogue)
    CASE(UIMSG_SelectNPCDialogueOption)
    CASE(UIMSG_CastSpellFromBook)
    CASE(UIMSG_PlayerCreation_VoicePrev)
    CASE(UIMSG_PlayerCreation_VoiceNext)
    CASE(UIMSG_StartNPCDialogue)
    CASE(UIMSG_ArrowUp)
    CASE(UIMSG_DownArrow)
    CASE(UIMSG_SaveLoadBtn)
    CASE(UIMSG_SelectLoadSlot)
    CASE(UIMSG_Cancel)
    CASE(UIMSG_ExitRest)
    CASE(UIMSG_PlayerCreation_FacePrev)
    CASE(UIMSG_PlayerCreation_FaceNext)
    CASE(UIMSG_CycleCharacters)
    CASE(UIMSG_SetTurnSpeed)
    CASE(UIMSG_ToggleWalkSound)
    CASE(UIMSG_ChangeVoiceVolume)
    CASE(UIMSG_ToggleShowDamage)
    CASE(UIMSG_ScrollNPCPanel)
    CASE(UIMSG_ShowFinalWindow)
    CASE(UIMSG_OpenQuestBook)
    CASE(UIMSG_OpenAutonotes)
    CASE(UIMSG_OpenMapBook)
    CASE(UIMSG_OpenCalendar)
    CASE(UIMSG_OpenHistoryBook)
    CASE(UIMSG_ToggleAlwaysRun)
    CASE(UIMSG_ToggleFlipOnExit)
    CASE(UIMSG_Game_Action)
    CASE(UIMSG_RentRoom)
    CASE(UIMSG_TransitionUI_Confirm)
    CASE(UIMSG_OpenKeyMappingOptions)
    CASE(UIMSG_SelectKeyPage1)
    CASE(UIMSG_SelectKeyPage2)
    CASE(UIMSG_ResetKeyMapping)
    CASE(UIMSG_ChangeKeyButton)
    CASE(UIMSG_OpenVideoOptions)
    CASE(UIMSG_ToggleBloodsplats)
    CASE(UIMSG_ToggleColoredLights)
    CASE(UIMSG_ToggleTint)
    default:
      swprintf(b, wcslen(L"UIMSG_%03X") , L"UIMSG_%03X", msg); return b;
  };
  #undef CASE
}

//----- (0046A99B) --------------------------------------------------------
int  sub_46A99B()
{
  int v0; // ebx@1
  signed int v1; // ecx@1
  signed int v2; // esi@1
  int v3; // edx@1
  signed int v4; // eax@1
  int v5; // ecx@2
  int *v6; // eax@3
  int v7; // ecx@3
  int v8; // edx@5
  int v9; // edi@6
  unsigned __int8 v10; // zf@14
  char v11; // sf@14
  char v12; // of@14
  int *v14; // esi@19
  signed int v15; // ebx@19
  int i; // edi@20
  int v17; // ecx@21
  int v18; // edi@26
  int v19; // esi@27
  unsigned int v20; // ecx@27
  BLVFace *v21; // eax@32
  unsigned int v22; // ecx@33
  const char *v23; // eax@35
  const char *v24; // ecx@36
  LevelDecoration *v25; // ecx@43
  LevelDecoration *v26; // edi@43
  __int16 v27; // ax@43
  int v28; // [sp+Ch] [bp-18h]@1
  int v29; // [sp+14h] [bp-10h]@2
  int v30; // [sp+18h] [bp-Ch]@1
  signed int v31; // [sp+1Ch] [bp-8h]@3
  int v32; // [sp+20h] [bp-4h]@1

  v0 = 0;
  v1 = (signed int)(viewparams->uScreen_BttmR_Y + viewparams->uScreen_topL_Y) >> 1;
  v2 = (signed int)(viewparams->uScreen_topL_X + viewparams->uScreen_BttmR_X) >> 1;
  v3 = v1 - 50;
  v32 = 0;
  v4 = 100;
  v30 = v1 - 50;
  v28 = v1 + 50;
  if ( v1 - 50 >= v1 + 50 )
    return 0;
  v5 = 640 * v3;
  v29 = 640 * v3;
  while ( 2 )
  {
    v6 = &render->pActiveZBuffer[v2 + v5 - v4 / 2]-1;
    v7 = v0;
    //while ( 1 )
    for( int i = 0; i < 100; i++)
	{
	  ++v6;
      v8 = *v6;
      if ( (unsigned int)*v6 <= 0x2000000 )
      {
        v9 = 0;
        if ( v7 > 0 )
        {
          do
          {
            if ( dword_7201B0_zvalues[v9] == (unsigned __int16)v8 )
              break;
            ++v9;
          }
          while ( v9 < v0 );
        }
        if ( v9 == v0 )
        {
          if ( v8 & 7 )
          {
            dword_720020_zvalues[v7] = v8;
            dword_7201B0_zvalues[v7] = (unsigned __int16)v8;
            ++v7;
            ++v0;
            v32 = v0;
            if ( v7 == 99 )
            {
              v12 = 0;
              v10 = v0 == 0;
              v11 = v0 < 0;
              goto LABEL_18;
            }
          }
        }
      }
     }
    v4 = -1;
    ++v30;
    v5 = v29 + 640;
    v29 += 640;
    if ( v30 >= v28 )
      break;
  }
  v12 = 0;
  v10 = v0 == 0;
  v11 = v0 < 0;
  if ( !v0 )
    return 0;
LABEL_18:
  if ( !((unsigned __int8)(v11 ^ v12) | v10) )
  {
    v14 = dword_720020_zvalues.data();
    v15 = 1;
    do
    {
      for ( i = v15; i < v32; ++i )
      {
        v17 = dword_720020_zvalues[i];
        if ( v17 < *v14 )
        {
          dword_720020_zvalues[i] = *v14;
          *v14 = v17;
        }
      }
      ++v15;
      ++v14;
    }
    while ( v15 - 1 < v32 );
    v0 = v32;
  }
  v18 = 0;
  if ( v0 <= 0 )
    return 0;
  while ( 1 )
  {
    v19 = dword_720020_zvalues[v18] & 0xFFFF;
    v20 = PID_ID(dword_720020_zvalues[v18]);
    if ( PID_TYPE(dword_720020_zvalues[v18]) == OBJECT_Item)
    {
      if ( v20 >= 0x3E8 )
        return 0;
      if ( !(pSpriteObjects[v20].uAttributes & 0x10) )
      {
        v23 = pSpriteObjects[v20].stru_24.GetDisplayName();
        v24 = v23;
	    GameUI_StatusBar_Set(v24);
	    return v19;
      }
    }
    else
	{
		if ( PID_TYPE(dword_720020_zvalues[v18]) == OBJECT_Actor)
		{
		  if ( v20 >= 0xBB8 )
			return 0;
		  v24 = (const char *)&pActors[v20];
		  GameUI_StatusBar_Set(v24);
		  return v19;
		}
		if ( PID_TYPE(dword_720020_zvalues[v18]) == OBJECT_Decoration)
		  break;
		if ( PID_TYPE(dword_720020_zvalues[v18]) == OBJECT_BModel)
		{
		  if ( uCurrentlyLoadedLevelType == LEVEL_Indoor )
		  {
			v21 = &pIndoor->pFaces[v20];
			if ( BYTE3(v21->uAttributes) & 2 )
				v22 = pIndoor->pFaceExtras[v21->uFaceExtraID].uEventID;
		  }
		  else
		  {
			if ( (dword_720020_zvalues[v18] & 0xFFFFu) >> 9 < pOutdoor->uNumBModels )
			{
				v22 = pOutdoor->pBModels[(dword_720020_zvalues[v18] & 0xFFFFu) >> 9].pFaces[v20 & 0x3F].sCogTriggeredID;
				if ( v22 )
				{
					v23 = GetEventHintString(v22);
					if ( v23 )
					{
						v24 = v23;
						GameUI_StatusBar_Set(v24);
						return v19;
					}
				}
			}
		  }
		}
	}
	++v18;
    if ( v18 >= v0 )
      return 0;
  }
  v25 = &pLevelDecorations[v20];
  v26 = v25;
  v27 = pLevelDecorations[v20].uEventID;
  if ( v27 )
  {
    v23 = GetEventHintString(v27);
    if ( !v23 )
      return v19;
	v24 = v23;
	GameUI_StatusBar_Set(v24);
	return v19;
  }
  if ( v25->IsInteractive() )
    v24 = pNPCTopics[stru_5E4C90_MapPersistVars._decor_events[v26->_idx_in_stru123 - 75] + 379].pTopic;
  else
    v24 = pDecorationList->pDecorations[v26->uDecorationDescID].field_20;
  GameUI_StatusBar_Set(v24);
  return v19;
}


//----- (0049EBF1) --------------------------------------------------------
void Render::_stub_49EBF1()
{
  signed int uNumRedBits; // edx@1
  signed int uNuGreenBits; // edi@1
  signed int uNumBlueBits; // esi@1
  unsigned int v4; // edx@4
  unsigned int v5; // edi@4
  int v6; // ebx@4
  int v7; // edx@4
  signed int v8; // [sp+8h] [bp-24h]@1
  signed int v9; // [sp+Ch] [bp-20h]@1
  signed int v10; // [sp+20h] [bp-Ch]@1
  signed int i; // [sp+24h] [bp-8h]@2
  signed int v12; // [sp+28h] [bp-4h]@3

  v10 = 0;
  uNumRedBits = 1 << this->uTargetRBits;
  uNuGreenBits = 1 << this->uTargetGBits;
  uNumBlueBits = 1 << this->uTargetBBits;
  v9 = 1 << this->uTargetRBits;
  v8 = 1 << this->uTargetGBits;
  if ( uNumRedBits > 0 )
  {
    do
    {
      for ( i = 0; i < uNuGreenBits; ++i )
      {
        v12 = 0;
        if ( uNumBlueBits > 0 )
        {
          do
          {
            v4 = this->uTargetBBits;
            v5 = v4 + this->uTargetGBits;
            v6 = (v12 >> 1) + (v10 >> 1 << v5) + (i >> 1 << this->uTargetBBits);
            v7 = (v10 << v5) + v12++ + (i << v4);
            this->unused_2C[v7] = v6;
          }
          while ( v12 < uNumBlueBits );
          uNumRedBits = v9;
          uNuGreenBits = v8;
        }
      }
      ++v10;
    }
    while ( v10 < uNumRedBits );
  }
}


//----- (004B0967) --------------------------------------------------------
void BspRenderer::DrawFaceOutlines()
{
  signed int i; // edi@1
  int v1; // esi@2
  unsigned int v2; // ecx@4
  int v3; // eax@4
  int v4; // eax@6
  unsigned __int16 *v5; // edx@6
  int v6; // ecx@7
  int v7; // esi@8

  for ( i = 0; i < (signed int)pBspRenderer->num_faces; ++i )
  {
    v1 = pBspRenderer->faces[i].uFaceID;
    if ( v1 >= 0 )
    {
      if ( v1 < (signed int)pIndoor->uNumFaces )
      {
        v2 = pBspRenderer->faces[i].uFaceID;
        pBLVRenderParams->field_7C = &pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID].PortalScreenData;
        v3 = GetPortalScreenCoord(v2);
        if ( v3 )
        {
          if ( PortalFrustrum(v3, &stru_F8A590, pBLVRenderParams->field_7C, v1) )
          {
            v4 = stru_F8A590._viewport_space_y;
            v5 = pBLVRenderParams->pRenderTarget;
            if ( stru_F8A590._viewport_space_y <= stru_F8A590._viewport_space_w )
            {
              v6 = 640 * stru_F8A590._viewport_space_y;
              do
              {
                v5[v6 + stru_F8A590.viewport_left_side[v4]] = -1;
                v7 = v6 + stru_F8A590.viewport_right_side[v4];
                v6 += 640;
                v5[v7] = -1;
                ++v4;
              }
              while ( v4 <= stru_F8A590._viewport_space_w );
            }
          }
        }
      }
    }
  }
}


//----- (004A80DC) --------------------------------------------------------
void SpellFxRenderer::_4A80DC_implosion_particle_sw(SpriteObject *a2)
{
  signed int v3; // ebx@1
  Particle_sw local_0; // [sp+Ch] [bp-68h]@1

  memset(&local_0, 0, 0x68u);
  local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_1;
  local_0.uDiffuse = 0x7E7E7E;
  local_0.timeToLive = (rand() & 0x7F) + 128;
  local_0.uTextureID = effpar01;
  v3 = 8;
  local_0.flt_28 = 1.0;
  do
  {
    local_0.x = pRnd->GetRandom() * 40.0 + (double)a2->vPosition.x - 20.0;
    local_0.y = pRnd->GetRandom() * 40.0 + (double)a2->vPosition.y - 20.0;
    local_0.z = (double)a2->vPosition.z;
    local_0.r = pRnd->GetRandom() * 800.0 - 400.0;
    local_0.g = pRnd->GetRandom() * 800.0 - 400.0;
    local_0.b = pRnd->GetRandom() * 350.0 + 50.0;
    pEngine->pParticleEngine->AddParticle(&local_0);
    --v3;
  }
  while ( v3 );
}


//----- (004AFB86) --------------------------------------------------------
void BspRenderer::AddFaceToRenderList_sw(unsigned int node_id, unsigned int uFaceID)
{
  BspRenderer *v3; // ebx@1
  BLVFace *v4; // eax@1
  char *v5; // ecx@2
  unsigned __int16 v6; // ax@11
  int v7; // ecx@13
  Vec3_short_ *v8; // esi@16
  int v9; // edx@16
  signed int v10; // eax@19
  signed int v11; // edi@20
  signed int v12; // ecx@20
  signed int v13; // esi@20
  int v14; // edx@21
  int v15; // edx@25
  unsigned __int16 v16; // ax@35
  signed int v17; // eax@37
  int v18; // eax@38
  signed int v19; // [sp+Ch] [bp-14h]@19
  char *v20; // [sp+14h] [bp-Ch]@2
  BLVFace *v21; // [sp+18h] [bp-8h]@1
  signed int v22; // [sp+1Ch] [bp-4h]@20
  signed int v23; // [sp+28h] [bp+8h]@20

  v3 = this;
  v4 = &pIndoor->pFaces[uFaceID];
  v21 = v4;
  if (v4->Portal())
  {
    v5 = (char *)this + 2252 * node_id;
    v20 = v5;
    if ( uFaceID == *((short *)v5 + 2982) )
      return;
    if (!node_id
      && pIndoorCameraD3D->vPartyPos.x >= v4->pBounding.x1 - 16
      && pIndoorCameraD3D->vPartyPos.x <= v4->pBounding.x2 + 16
      && pIndoorCameraD3D->vPartyPos.y >= v4->pBounding.y1 - 16
      && pIndoorCameraD3D->vPartyPos.y <= v4->pBounding.y2 + 16
      && pIndoorCameraD3D->vPartyPos.z >= v4->pBounding.z1 - 16
      && pIndoorCameraD3D->vPartyPos.z <= v4->pBounding.z2 + 16 )
    {
      if ( abs(v4->pFacePlane_old.dist + pIndoorCameraD3D->vPartyPos.x * v4->pFacePlane_old.vNormal.x
                                       + pIndoorCameraD3D->vPartyPos.y * v4->pFacePlane_old.vNormal.y
                                       + pIndoorCameraD3D->vPartyPos.z * v4->pFacePlane_old.vNormal.z) <= 589824 )
      {
        v6 = v21->uSectorID;
        if ( v3->nodes[0].uSectorID == v6 )
          v6 = v21->uBackSectorID;
        v3->nodes[v3->num_nodes].uSectorID = v6;
        v3->nodes[v3->num_nodes].uFaceID = uFaceID;
        v3->nodes[v3->num_nodes].uViewportX = LOWORD(pBLVRenderParams->uViewportX);
        v3->nodes[v3->num_nodes].uViewportZ = LOWORD(pBLVRenderParams->uViewportZ);
        v3->nodes[v3->num_nodes].uViewportY = LOWORD(pBLVRenderParams->uViewportY);
        v3->nodes[v3->num_nodes].uViewportW = LOWORD(pBLVRenderParams->uViewportW);
        v3->nodes[v3->num_nodes++].PortalScreenData.GetViewportData(
          SLOWORD(pBLVRenderParams->uViewportX),
          pBLVRenderParams->uViewportY,
          SLOWORD(pBLVRenderParams->uViewportZ),
          pBLVRenderParams->uViewportW);
        v7 = v3->num_nodes - 1;
        goto LABEL_14;
      }
      v4 = v21;
      v5 = v20;
    }
    v8 = &pIndoor->pVertices[*v4->pVertexIDs];
    v9 = v4->pFacePlane_old.vNormal.x * (v8->x - pIndoorCameraD3D->vPartyPos.x)
       + v4->pFacePlane_old.vNormal.y * (v8->y - pIndoorCameraD3D->vPartyPos.y)
       + v4->pFacePlane_old.vNormal.z * (v8->z - pIndoorCameraD3D->vPartyPos.z);
    if ( *((short *)v5 + 2004) != v4->uSectorID )
      v9 = -v9;
    if ( v9 < 0 )
    {
      v10 = GetPortalScreenCoord(uFaceID);
      v19 = v10;
      if ( v10 )
      {
        v11 = PortalFace._screen_space_x[0];
        v12 = PortalFace._screen_space_y[0];
        v23 = PortalFace._screen_space_x[0];
        v13 = 1;
        v22 = PortalFace._screen_space_y[0];
        if ( v10 > 1 )
        {
          do
          {
            v14 = PortalFace._screen_space_x[v13];
            if ( v14 < v23 )
              v23 = PortalFace._screen_space_x[v13];
            if ( v14 > v11 )
              v11 = PortalFace._screen_space_x[v13];
            v15 = PortalFace._screen_space_y[v13];
            if ( v15 < v22 )
              v22 = PortalFace._screen_space_y[v13];
            if ( v15 > v12 )
              v12 = PortalFace._screen_space_y[v13];
            v10 = v19;
            ++v13;
          }
          while ( v13 < v19 );
        }
        if ( v11 >= *((short *)v20 + 2005)
          && v23 <= *((short *)v20 + 2007)
          && v12 >= *((short *)v20 + 2006)
          && v22 <= *((short *)v20 + 2008)
          && PortalFrustrum(v10, &v3->nodes[v3->num_nodes].PortalScreenData, (BspRenderer_PortalViewportData *)(v20 + 4020), uFaceID) )
        {
          v16 = v21->uSectorID;
          if ( *((short *)v20 + 2004) == v16 )
            v16 = v21->uBackSectorID;
          v3->nodes[v3->num_nodes].uSectorID = v16;
          v3->nodes[v3->num_nodes].uFaceID = uFaceID;
          v3->nodes[v3->num_nodes].uViewportX = LOWORD(pBLVRenderParams->uViewportX);
          v3->nodes[v3->num_nodes].uViewportZ = LOWORD(pBLVRenderParams->uViewportZ);
          v3->nodes[v3->num_nodes].uViewportY = LOWORD(pBLVRenderParams->uViewportY);
          v3->nodes[v3->num_nodes].uViewportW = LOWORD(pBLVRenderParams->uViewportW);
          v17 = v3->num_nodes;
          if ( v17 < 150 )
          {
            v18 = v17 + 1;
            v3->num_nodes = v18;
            v7 = v18 - 1;
LABEL_14:
            AddBspNodeToRenderList(v7);
            return;
          }
        }
      }
    }
  }
  else
  {
    if (num_faces < 1000)
    {
      faces[num_faces].uFaceID = uFaceID;
      faces[num_faces++].uNodeID = node_id;
    }
  }
}


//----- (0046A6AC) --------------------------------------------------------
int _46A6AC_spell_render(int a1, int a2, int a3)
{
  int result; // eax@2
  int *v5; // esi@6
  unsigned int v6; // ebx@6
  signed int v7; // edi@9
  int i; // eax@14
  int v10; // ecx@19
  unsigned int v13; // [sp+8h] [bp-10h]@4
  int *v15; // [sp+10h] [bp-8h]@4
  int v16; // [sp+14h] [bp-4h]@3

  if ( render->pRenderD3D )
  {
    result = _46А6АС_GetActorsInViewport(a3);
  }
  else
  {
    __debugbreak(); // SW render never called
    v16 = 0;
    if ( (signed int)viewparams->uScreen_topL_Y < (signed int)viewparams->uScreen_BttmR_Y )
    {
      v15 = &render->pActiveZBuffer[viewparams->uScreen_topL_X + 640 * viewparams->uScreen_topL_Y];
      v13 = ((viewparams->uScreen_BttmR_Y - viewparams->uScreen_topL_Y - 1) >> 1) + 1;
      for ( v13; v13; --v13 )
      {
        if ( (signed int)viewparams->uScreen_topL_X < (signed int)viewparams->uScreen_BttmR_X )
        {
          v5 = v15;
          v6 = ((viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X - 1) >> 1) + 1;
          for ( v6; v6; --v6 )
          {
            if ( PID_TYPE(*(char *)v5) == OBJECT_Actor)
            {
              if ( *v5 <= a3 << 16 )
              {
                v7 = PID_ID((signed int)(unsigned __int16)*v5);
                if ( pActors[v7].uAIState != Dead && pActors[v7].uAIState != Dying && pActors[v7].uAIState != Removed
                  && pActors[v7].uAIState != Summoned && pActors[v7].uAIState != Disabled )
                {
                  for ( i = 0; i < v16; ++i )
                  {
                    if ( *(int *)(a1 + 4 * i) == v7 )
                      break;
                  }
                  if ( i == v16 )
                  {
                    if ( i < a2 - 1 )
                    {
                      v10 = v16++;
                      *(int *)(a1 + 4 * v10) = v7;
                    }
                  }
                }
              }
            }
            v5 += 2;
          }
        }
        v15 += 1280;
      }
    }
    result = v16;
  }
  return result;
}
//----- (0047F5C6) --------------------------------------------------------
float Render::DrawBezierTerrain()
{
  unsigned int pDirectionIndicator1; // ebx@1
  unsigned int pDirectionIndicator2; // edi@1
  unsigned int v2; // eax@1
  int v3; // eax@3
  int v4; // edi@3
  int v5; // ebx@3
  int v6; // esi@3
  unsigned int v7; // eax@3
  int v8; // eax@4
  unsigned int v9; // eax@6
  int v10; // eax@7
  //int v11; // ebx@9
  //int v12; // edi@9
  int v13; // eax@21
  int v14; // eax@31
  int v15; // edi@33
  int v16; // eax@34
  int v17; // edx@34
  int v18; // ebx@34
  int v19; // eax@36
  int v20; // eax@39
  int v21; // ecx@43
  //char v22; // zf@44
  int v23; // ecx@47
  //int v24; // edi@52
  int v25; // eax@54
  int v26; // ecx@54
  int v27; // eax@56
  int v28; // edx@60
  int v29; // ecx@61
  int v30; // ecx@64
  int v31; // ecx@68
  int v32; // eax@70
  //int v33; // ecx@71
  int v34; // eax@73
  int v35; // ecx@77
  int v36; // ecx@81
  int v37; // ecx@86
  int v38; // eax@88
  int v39; // ecx@88
  int v40; // eax@90
  int v41; // edx@94
  //int v42; // ecx@95
  int v43; // ecx@98
  int v44; // ecx@102
  int v45; // eax@104
  int v46; // eax@107
  int v47; // ecx@111
  int v48; // ecx@115
  int v49; // edi@120
  int v50; // eax@122
  int v51; // ecx@122
  int v52; // eax@124
  int v53; // edx@128
  int v54; // ecx@129
  int v55; // ecx@132
  int v56; // eax@139
  int v57; // ecx@140
  int v58; // eax@142
  int v59; // ecx@146
  //int v60; // ecx@147
  int v61; // ecx@150
  int v62; // ecx@155
  int v63; // eax@157
  int v64; // ecx@157
  int v65; // eax@159
  int v66; // edx@163
  int v67; // ecx@164
  int v68; // ecx@167
  //int v69; // eax@173
  int v70; // edi@178
  //int v71; // eax@178
  //int v72; // ecx@178
  //int x; // ebx@180
  //int v74; // eax@182
  //int v75; // eax@184
  IndoorCameraD3D *pIndoorCameraD3D_3; // ecx@184
  int uStartZ; // ecx@184
  int v79; // ebx@185
  int v127; // esi@185
  int v86; // edi@196
  //int v87; // eax@196
  //int v88; // ecx@196
  //int v89; // eax@198
  //int v90; // ecx@200
  int v92; // ebx@203
  //int v93; // ST08_4@204
  int v97; // ST08_4@204
  float result; // eax@212
  //struct 
  //{
  int v106; // [sp+Ch] [bp-68h]@191
  int v103; // [sp+10h] [bp-64h]@190
  int v104; // [sp+12h] [bp-62h]@190
  //} v102;
  int v105; // [sp+1Ch] [bp-58h]@1
  int v107; // [sp+20h] [bp-54h]@3
  int uEndZ; // [sp+24h] [bp-50h]@3
  int v108; // [sp+28h] [bp-4Ch]@9
  int v109; // [sp+2Ch] [bp-48h]@9
  int v110; // [sp+30h] [bp-44h]@9
  int v111; // [sp+34h] [bp-40h]@3
  int v112; // [sp+38h] [bp-3Ch]@6
  IndoorCameraD3D *pIndoorCameraD3D_4; // [sp+3Ch] [bp-38h]@9
  int v114; // [sp+40h] [bp-34h]@9
  int v115; // [sp+44h] [bp-30h]@9
  int v116; // [sp+48h] [bp-2Ch]@9
  //int v117; // [sp+4Ch] [bp-28h]@9
  int v118; // [sp+50h] [bp-24h]@9
  int v119; // [sp+54h] [bp-20h]@1
  int v120; // [sp+58h] [bp-1Ch]@1
  int i; // [sp+5Ch] [bp-18h]@1
  int v122; // [sp+60h] [bp-14h]@1
  int v123; // [sp+64h] [bp-10h]@1
  int v124; // [sp+68h] [bp-Ch]@1
  int v125; // [sp+6Ch] [bp-8h]@9
  int v126; // [sp+70h] [bp-4h]@9

  v105 = pIndoorCameraD3D->sRotationY / ((signed int)stru_5C6E00->uIntegerHalfPi / 2);//2
  pDirectionIndicator1 = stru_5C6E00->uDoublePiMask & (stru_5C6E00->uIntegerDoublePi - pIndoorCameraD3D->sRotationY);//1536
  pDirectionIndicator2 = stru_5C6E00->uDoublePiMask & (stru_5C6E00->uIntegerPi + pDirectionIndicator1);//512
  v124 = ((pIndoorCamera->uMapGridCellX << 16) + 3 * stru_5C6E00->Cos(stru_5C6E00->uDoublePiMask & (stru_5C6E00->uIntegerPi + pDirectionIndicator1))) >> 16;//88
  v123 = ((pIndoorCamera->uMapGridCellZ << 16) + 3 * stru_5C6E00->Sin(pDirectionIndicator2)) >> 16;// 66
  v120 = pODMRenderParams->outdoor_grid_band_3 + v124;//+- range X
  v119 = pODMRenderParams->outdoor_grid_band_3 + v123;
  v2 = pODMRenderParams->uCameraFovInDegrees + 15;//90
  i = v124 - pODMRenderParams->outdoor_grid_band_3;
  v122 = v123 - pODMRenderParams->outdoor_grid_band_3;

  if ( v2 > 90 )
    v2 = 90;
  //v3 = (v2 << 11) / 720;
  v4 = stru_5C6E00->uDoublePiMask & (pDirectionIndicator1 - ((v2 << 11) / 720));
  v5 = stru_5C6E00->uDoublePiMask & (((v2 << 11) / 720) + pDirectionIndicator1);

  v106 = stru_5C6E00->Cos(v4);
  uEndZ = stru_5C6E00->Sin(v4);

  v111 = stru_5C6E00->Cos(v5);
  v6 = stru_5C6E00->Sin(v5);

  v7 = v4 & stru_5C6E00->uPiMask;
  if ( (v4 & stru_5C6E00->uPiMask) >= stru_5C6E00->uIntegerHalfPi )
    v8 = -stru_5C6E00->pTanTable[stru_5C6E00->uIntegerPi - v7];
  else
    v8 = stru_5C6E00->pTanTable[v7];
  v112 = abs(v8);

  v9 = v5 & stru_5C6E00->uPiMask;
  if ( (v5 & stru_5C6E00->uPiMask) >= stru_5C6E00->uIntegerHalfPi )
    v10 = -stru_5C6E00->pTanTable[stru_5C6E00->uIntegerPi - v9];
  else
    v10 = stru_5C6E00->pTanTable[v9];
  v108 = abs(v10);

  //v11 = v124;
  //v12 = v123;
  v114 = 0;
  v115 = 0;
  pIndoorCameraD3D_4 = 0;
  v125 = 0;
  v126 = v124;
  v118 = v123;

  v110 = (v106 >= 0 ? 1: -1);//2 * (v106 >= 0) - 1;
  v109 = (uEndZ >= 0 ? 1: -1);//2 * (v107 >= 0) - 1;
  uEndZ = (v111 >= 0 ? 1: -1);//2 * (v111 >= 0) - 1;
  v106 = (v6 >= 0 ? 1: -1);//2 * (v6 >= 0) - 1;

  uint _i = 1;
  uint j = 1;

  terrain_76DDC8[0] = -1;
  terrain_76DFC8[0] = -1;
  terrain_76E1C8[0] = -1;
  terrain_76E3C8[0] = -1;

  for( uint _i = 1; _i < 128; _i++)
  {
    if ( v112 >= 0x10000 )
    {
      int v1, v2;
      //v111 = 4294967296i64 / v112;
      //v114 += v111;
      //if ( v114 >= 65536 )
      //{
      //  v11 += v110;
      //  v114 = (unsigned __int16)v114;
      //}
      //v12 += v109;
    }
    else
    {
      v124 += v110;
      v115 += v112;
      if ( v112 + v115 >= 65536 )
      {
        v123 += v109;
        v115 = (unsigned __int16)v115;
      }
    }
    if ( v124 < _i || v124 > v120 || v123 < v122 || v123 > v119 )
      break;
    //v13 = v116++;
    terrain_76E3C8[_i] = v124;
    terrain_76E1C8[_i] = v123;
  }

  for( j = 1; j < 128; j++ )
  {
    if ( v108 >= 65536 )
    {
      v111 = 4294967296i64 / v108;
      v114 += v111;//
      if ( v111 + v114 >= 65536 )
      {
        v126 += uEndZ;
        v114 = (unsigned __int16)v114;//
      }
      v118 += v106;
    }
    else
    {
      v125 += v108;
      v126 += uEndZ;
      if ( v125 >= 65536 )
      {
        v118 += v106;
        v125 = (unsigned __int16)v125;
      }
    }
    //if ( v117 >= 128 )
      //break;
    if ( v126 < _i )
      break;
    if ( v126 > v120 )
      break;
    v14 = v118;
    if ( v118 < v122 )
      break;
    if ( v118 > v119 )
      break;
    terrain_76DFC8[j] = v126;
    terrain_76DDC8[j] = v14;
  }
  v16 = 0;
  v126 = 0;
  v17 = j - 1;
  v18 = _i - 1;

  switch ( v105 )//напрвление камеры
  {
    case 0:
    case 7:
    {
      //v116 = terrain_76DFC8[v17];
      if ( v120 > terrain_76DFC8[v17] )
      {
        v125 = v120;
        memset32(terrain_76D9C8.data(), v119 + 1, 4 * (v120 - terrain_76DFC8[v17] + 1));
        v19 = v120;
        do
          terrain_76DBC8[v126++] = v19--;
        while ( v19 >= terrain_76DFC8[v17] );
        if ( terrain_76DFC8[v17] == terrain_76DDC8[v17 + 127] )
        {
          do
            v20 = terrain_76DDC8[v17-- -1];
          while ( v20 == terrain_76DDC8[v17 -1] );
        }
        v16 = v126;
        --v17;
      }
      if ( v17 < 0 )
        v17 = 0;
      
      //while ( 1 )
	  for ( v21 = terrain_76DFC8[v17]; v21 < v124; v21 = terrain_76DFC8[v17] - 1; )
      {
        //v125 = terrain_76DFC8[v17];
        terrain_76DBC8[v16] = v21;
        //v22 = terrain_76DDC8[v17] == 65535;
        terrain_76D9C8[v16] = terrain_76DDC8[v17] + 1;
        if ( terrain_76DDC8[v17] == 65535 )
        {
          terrain_76D9C8[v16] = v123 + 1;
          break;
        }
        if ( !v17 )
          break;
        if ( terrain_76DFC8[v17] == terrain_76DDC8[v17 - 1] )
        {
          do
            v23 = terrain_76DDC8[v17-- -1];
          while ( v23 == terrain_76DDC8[v17 -1] );
        }
        --v17;
        ++v16;
      }
      v16 = 0;
      //v24 = terrain_76E3C8[v18];
      v126 = 0;
      if ( v120 > terrain_76E3C8[v18] )
      {
        v125 = v120;
        memset32(terrain_76D5C8.data(), v122, 4 * (v120 - terrain_76E3C8[v18] + 1));
        do
        {
          v25 = v126;
          v26 = v125--;
          ++v126;
          terrain_76D7C8[v25] = v26;
        }
        while ( v125 >= terrain_76E3C8[v18] );
        if ( terrain_76E3C8[v18] == terrain_76E1C8[v18 -1] )
        {
          do
            v27 = terrain_76E1C8[v18-- -1];
          while ( v27 == terrain_76E1C8[v18 -1] );
        }
        v16 = v126;
        --v18;
      }
      if ( v18 < 0 )
        v18 = 0;
      v28 = terrain_76E3C8[v18];
      while ( v28 >= v124 )
      {
        v29 = terrain_76E1C8[v18];
        terrain_76D7C8[v16] = v28;
        terrain_76D5C8[v16] = v29;
        if ( v29 == 65535 )
        {
          v31 = v123;
          terrain_76D5C8[v16] = v31;
          break;
        }
        if ( !v18 )
          break;
        if ( terrain_76E3C8[v18] == terrain_76E1C8[v18 -1] )
        {
          do
            v30 = terrain_76E1C8[v18-- -1];
          while ( v30 == terrain_76E1C8[v18 -1] );
        }
        --v18;
        --v28;
        ++v16;
      }
      break;
    }
    case 1:
    case 2:
    {
      //v116 = terrain_76DDC8[v17];
      if ( v122 < terrain_76DDC8[v17] )
      {
        v106 = v122;
        memset32(terrain_76DBC8.data(), v120 + 1, 4 * (terrain_76DDC8[v17] - v122 + 1));
        for ( v32 = v122; v32 <= terrain_76DDC8[v17]; v32++)
          terrain_76D9C8[v126++] = v32;
        if ( terrain_76DDC8[v17] == terrain_76DBC8[v17 -1] )
        {
          do
            v34 = terrain_76DBC8[v17-- -1];
          while ( v34 == terrain_76DBC8[v17 -1] );
        }
        v16 = v126;
        --v17;
      }
      if ( v17 < 0 )
        v17 = 0;
      v35 = terrain_76DDC8[v17];
      v125 = terrain_76DDC8[v17];
      while ( v35 <= v123 )
      {
        //v22 = terrain_76DFC8[v17] == 65535;
        terrain_76DBC8[v16] = terrain_76DFC8[v17] + 1;
        terrain_76D9C8[v16] = v125;
        if ( terrain_76DFC8[v17] == 65535 )
        {
          terrain_76DBC8[v16] = v124 + 1;
          break;
        }
        if ( !v17 )
          break;
        if ( terrain_76DDC8[v17] == terrain_76DBC8[v17 -1] )
        {
          do
            v36 = terrain_76DBC8[v17-- -1];
          while ( v36 == terrain_76DBC8[v17 -1] );
        }
        --v17;
        ++v125;
        v35 = v125;
        ++v16;
      }
      v16 = 0;
      v126 = 0;
      v37 = terrain_76E1C8[v18];
      if ( v122 < v37 )
      {
        v114 = v122;
        memset32(terrain_76D7C8.data(), i, 4 * (v37 - v122 + 1));
        do
        {
          v38 = v126;
          v39 = v114;
          ++v126;
          ++v114;
          terrain_76D5C8[v38] = v39;
        }
        while ( v114 <= terrain_76E1C8[v18] );
        if ( terrain_76E1C8[v18] == terrain_76DFC8[v18 -1] )
        {
          do
            v40 = terrain_76DFC8[v18-- -1];
          while ( v40 == terrain_76DFC8[v18 -1] );
        }
        v16 = v126;
        --v18;
      }
      if ( v18 < 0 )
        v18 = 0;
      v41 = terrain_76E1C8[v18];
      while ( v41 <= v123 )
      {
        terrain_76D5C8[v16] = v41;
        terrain_76D7C8[v16] = terrain_76E3C8[v18];
        if ( terrain_76E3C8[v18] == 65535 )
        {
          terrain_76D7C8[v16] = v124;
          break;
        }
        if ( !v18 )
          break;
        if ( terrain_76E1C8[v18] == terrain_76DFC8[v18 -1] )
        {
          do
            v43 = terrain_76DFC8[v18-- -1];
          while ( v43 == terrain_76DFC8[v18 -1] );
        }
        --v18;
        ++v41;
        ++v16;
      }
      break;
    }
    case 5:
    case 6:
    {
      //v116 = terrain_76DDC8[v17];
      if ( v119 > terrain_76DDC8[v17] )
      {
        v106 = v119;
        memset32(terrain_76DBC8.data(), i, 4 * (v119 - terrain_76DDC8[v17] + 1));
        for ( v45 = v119; v45 >= terrain_76DDC8[v17]; v45--)
          terrain_76D9C8[v126++] = v45;
        if ( terrain_76DDC8[v17] == terrain_76DBC8[v17 -1] )
        {
          do
            v46 = terrain_76DBC8[v17-- -1];
          while ( v46 == terrain_76DBC8[v17 -1] );
        }
        v16 = v126;
        --v17;
      }
      if ( v17 < 0 )
        v17 = 0;
      v47 = terrain_76DDC8[v17];
      v125 = terrain_76DDC8[v17];
      while ( v47 >= v123 )
      {
        //v22 = terrain_76DFC8[v17] == 65535;
        terrain_76DBC8[v16] = terrain_76DFC8[v17];
        terrain_76D9C8[v16] = terrain_76DDC8[v17];
        if ( terrain_76DFC8[v17] == 65535 )
        {
          terrain_76DBC8[v16] = v124;
          break;
        }
        if ( !v17 )
          break;
        if ( terrain_76DDC8[v17] == terrain_76DBC8[v17 -1] )
        {
          do
            v48 = terrain_76DBC8[v17-- -1];
          while ( v48 == terrain_76DBC8[v17 -1] );
        }
        --v17;
        --v125;
        v47 = v125;
        ++v16;
      }
      v16 = 0;
      v49 = terrain_76E1C8[v18];
      v126 = 0;
      if ( v119 > v49 )
      {
        v125 = v119;
        memset32(terrain_76D7C8.data(), v120 + 1, 4 * (v119 - v49 + 1));
        do
        {
          v50 = v126;
          v51 = v125--;
          ++v126;
          terrain_76D5C8[v50] = v51;
        }
        while ( v125 >= terrain_76E1C8[v18] );
        if ( terrain_76E1C8[v18] == terrain_76DFC8[v18 -1] )
        {
          do
            v52 = terrain_76DFC8[v18-- -1];
          while ( v52 == terrain_76DFC8[v18 -1] );
        }
        v16 = v126;
        --v18;
      }
      if ( v18 < 0 )
        v18 = 0;
      v53 = terrain_76E1C8[v18];
      while ( v53 >= v123 )
      {
        v54 = terrain_76E3C8[v18];
        terrain_76D5C8[v16] = v53;
        terrain_76D7C8[v16] = v54 + 1;
        if ( v54 == 65535 )
        {
          terrain_76D7C8[v16] = v124 + 1;
          break;
        }
        if ( !v18 )
          break;
        if ( terrain_76E1C8[v18] == terrain_76DFC8[v18 -1] )
        {
          do
            v55 = terrain_76DFC8[v18-- -1];
          while ( v55 == terrain_76DFC8[v18 -1] );
        }
        --v18;
        --v53;
        ++v16;
      }
      break;
    }
    case 3:
    case 4:
    {
      //v116 = terrain_76DFC8[v17];
      if ( i < terrain_76DFC8[v17] )
      {
        v106 = i;
        memset32(terrain_76D9C8.data(), v122, 4 * (terrain_76DFC8[v17] - i + 1));
        v56 = i;
        do
        {
          v57 = v126++;
          terrain_76DBC8[v57] = v56++;
        }
        while ( v56 <= terrain_76DFC8[v17] );
        if ( terrain_76DFC8[v17] == terrain_76DDC8[v17 -1] )
        {
          do
            v58 = terrain_76DDC8[v17-- -1];
          while ( v58 == terrain_76DDC8[v17 -1] );
        }
        v16 = v126;
        --v17;
      }
      if ( v17 < 0 )
        v17 = 0;
      v59 = terrain_76DFC8[v17];
      while ( 1 )
      {
        v125 = terrain_76DFC8[v17];
        if ( terrain_76DFC8[v17] > v124 )
          break;
        terrain_76DBC8[v16] = terrain_76DFC8[v17];
        //v60 = terrain_76DDC8[v17];
        terrain_76D9C8[v16] = terrain_76DDC8[v17];
        if ( terrain_76DDC8[v17] == 65535 )
        {
          terrain_76D9C8[v16] = v123;
          break;
        }
        if ( !v17 )
          break;
        if ( terrain_76DFC8[v17] == terrain_76DDC8[v17 -1] )
        {
          do
            v61 = terrain_76DDC8[v17-- -1];
          while ( v61 == terrain_76DDC8[v17 -1] );
        }
        --v17;
        v59 = v125 + 1;
        ++v16;
      }
      v16 = 0;
      v126 = 0;
      v62 = terrain_76E3C8[v18];
      if ( i < v62 )
      {
        v114 = i;
        memset32(terrain_76D5C8.data(), v119 + 1, 4 * (v62 - i + 1));
        do
        {
          v63 = v126;
          v64 = v114;
          ++v126;
          ++v114;
          terrain_76D7C8[v63] = v64;
        }
        while ( v114 <= terrain_76E3C8[v18] );
        if ( terrain_76E3C8[v18] == terrain_76E1C8[v18 -1] )
        {
          do
            v65 = terrain_76E1C8[v18-- -1];
          while ( v65 == terrain_76E1C8[v18 -1] );
        }
        v16 = v126;
        --v18;
      }
      if ( v18 < 0 )
        v18 = 0;
      v66 = terrain_76E3C8[v18];
      while ( v66 <= v124 )
      {
        v67 = terrain_76E1C8[v18];
        terrain_76D7C8[v16] = v66;
        terrain_76D5C8[v16] = terrain_76E1C8[v18] + 1;
        if ( terrain_76E1C8[v18] == 65535 )
        {
          v31 = v123 + 1;
          terrain_76D5C8[v16] = v31;
          break;
        }
        if ( !v18 )
          break;
        if ( terrain_76E3C8[v18] == terrain_76E1C8[v18 -1] )
        {
          do
            v68 = terrain_76E1C8[v18-- -1];
          while ( v68 == terrain_76E1C8[v18 -1] );
        }
        --v18;
        ++v66;
        ++v16;
      }
      break;
    }
    default:
      break;
  }
  //v69 = v16 - 1;
  ptr_801A08 = pVerticesSR_806210;
  ptr_801A04 = pVerticesSR_801A10;
  //v126 = v69;

  if ( v105 && v105 != 7 && v105 != 3 && v105 != 4 )//блок, ориентация камеры 1(СВ), 2(С), 5(ЮЗ), 6(Ю)
  {
    for ( i = v16 - 1; i >= 1; --i )
    {
      //v70 = i;
      //v71 = terrain_76D7C8[i];//88
      //v72 = terrain_76DBC8[i];//0
      if ( terrain_76D7C8[i] < terrain_76DBC8[i] )//swap
      {
        terrain_76DBC8[i] = terrain_76D7C8[i];
        terrain_76D7C8[i] = terrain_76DBC8[i];
      }
      //x = terrain_76DBC8[i];//0
      v111 = 0;
      if ( terrain_76DBC8[i] <= 0 )
        terrain_76DBC8[i] = -terrain_76DBC8[i];
      //v74 = terrain_76D7C8[i];
      if ( terrain_76D7C8[i] <= 0 )
        terrain_76D7C8[i] = -terrain_76D7C8[i];
      uEndZ = terrain_76D7C8[i] + 2;
      //pIndoorCameraD3D_3 = pIndoorCameraD3D;
      //uEndZ = v75;
      //pIndoorCameraD3D_4 = pIndoorCameraD3D_3;
      uStartZ = terrain_76DBC8[i] - 2;
      if ( terrain_76DBC8[i] - 2 < uEndZ )
      {
        v127 = 0;
        //v79 = (v73 - 66) << 9;
        //v116 = v77;
        //pHeight = v79;
        v111 = uEndZ - uStartZ;
        for (int z = uStartZ; z < uEndZ; ++z)
        {
          ptr_801A08[v127].vWorldPosition.x = (-64 + terrain_76DBC8[i]) * 512;//pTerrainVertices[z * 128 + x].vWorldPosition.x = (-64 + (signed)x) * 512;
          ptr_801A08[v127].vWorldPosition.y = (64 - terrain_76D9C8[i]) * 512;
          ptr_801A08[v127].vWorldPosition.z = pOutdoor->GetHeightOnTerrain( z, terrain_76D9C8[i]);

          ptr_801A04[v127].vWorldPosition.x = (-64 + terrain_76DBC8[i]) * 512;
          ptr_801A04[v127].vWorldPosition.y = (63 - terrain_76D9C8[i]) * 512;
          ptr_801A04[v127].vWorldPosition.z = pOutdoor->GetHeightOnTerrain( z, terrain_76D9C8[i] + 1);

          if ( !byte_4D864C || !(pEngine->uFlags & 0x80) )
          {
            pIndoorCameraD3D_4->ViewTransform(&ptr_801A08[v127], 1);
            pIndoorCameraD3D_4->ViewTransform(&ptr_801A04[v127], 1);

            pIndoorCameraD3D_4->Project(&ptr_801A08[v127], 1, 0);
            pIndoorCameraD3D_4->Project(&ptr_801A04[v127], 1, 0);
          }
          //v79 += 512;
          v127 ++;
          //++v116;
          //pHeight = v79;
       }
        //while ( v116 < v107 );
      }
      v103 = abs((int)pIndoorCamera->uMapGridCellZ - terrain_76D9C8[i]);
      v104 = abs((int)pIndoorCamera->uMapGridCellX - terrain_76DBC8[i]);
      if ( render->pRenderD3D )//Ritor1: do comment to test
        Render::DrawTerrainD3D(v111, 0, v103, v104);
        //Render::RenderTerrainD3D();
      //else
        //Render::DrawTerrainSW(v111, 0, v103, v104);
    }
  }
  else//ориентация камеры 0(В), 3(СЗ), 4(З), 7(ЮВ)
  {
    for ( i = v16 - 1; i >= 1; --i )
    {
      //v86 = i;
      //v87 = terrain_76D5C8[i];
      //v88 = terrain_76D9C8[i];
      if ( terrain_76D5C8[i] < terrain_76D9C8[i] )
      {
        terrain_76D9C8[i] = terrain_76D5C8[i];
        terrain_76D5C8[i] = terrain_76D9C8[i];
      }
      //v89 = terrain_76D9C8[i];
      v111 = 0;
      if ( terrain_76D9C8[i] <= 0 )
        terrain_76D9C8[i] = -terrain_76D9C8[i];
      //v90 = terrain_76D5C8[i];
      if ( terrain_76D5C8[i] <= 0 )
        terrain_76D5C8[i] = -terrain_76D5C8[i];
      pIndoorCameraD3D_4 = pIndoorCameraD3D;
      v107 = terrain_76D5C8[i] + 2;
      if ( terrain_76D9C8[i] - 2 < terrain_76D5C8[i] + 2 )
      {
        v86 = 0;
        //v116 = terrain_76D9C8[i] - 2;
        v92 = (66 - terrain_76D9C8[i]) << 9;
        //pHeight = (66 - terrain_76D9C8[i]) << 9;
        v111 = terrain_76D5C8[i] + 2 - (terrain_76D9C8[i] - 2);
        //do
        for ( v116 = terrain_76D9C8[i] - 2; v116 < v107; ++v116 )
        {
          ptr_801A08[v86].vWorldPosition.x = (terrain_76DBC8[v86] - 64) << 9;
          ptr_801A08[v86].vWorldPosition.y = v92;
          ptr_801A08[v86].vWorldPosition.z = pOutdoor->GetHeightOnTerrain(terrain_76DBC8[v86], v116);

          ptr_801A04[v86].vWorldPosition.x = (terrain_76DBC8[v86] - 63) << 9;
          ptr_801A04[v86].vWorldPosition.y = v92;
          ptr_801A04[v86].vWorldPosition.z = pOutdoor->GetHeightOnTerrain(terrain_76DBC8[v86] + 1, v116);
          if ( !byte_4D864C || !(pEngine->uFlags & 0x80) )
          {
           pIndoorCameraD3D_4->ViewTransform((RenderVertexSoft *)(char *)ptr_801A08 + v86, 1);
           pIndoorCameraD3D_4->ViewTransform((RenderVertexSoft *)(char *)ptr_801A04 + v86, 1);
           pIndoorCameraD3D_4->Project((RenderVertexSoft *)(char *)ptr_801A08 + v86, 1, 0);
           pIndoorCameraD3D_4->Project((RenderVertexSoft *)(char *)ptr_801A04 + v86, 1, 0);
          }
          v92 -= 512;
          v86 += 48;
          //++v116;
          //pHeight = v92;
        }
        //while ( v116 < v107 );
      }
      v103 = abs((int)pIndoorCamera->uMapGridCellX - terrain_76DBC8[v86]);
	  v104 = abs((int)pIndoorCamera->uMapGridCellZ - terrain_76D9C8[v86]);
	  if ( render->pRenderD3D )
        Render::DrawTerrainD3D(v111, 1, v103, v104);
      //else
        //Render::DrawTerrainSW(v111, 1, v103, v104);
    }
  }
  result = v126;
  pODMRenderParams->field_40 = v126;
  return result;
}

//----- (0048034E) --------------------------------------------------------
void Render::DrawTerrainD3D(int a1, int a2, int a3, int unk4)
{
  //int v3; // esi@1
  int v4; // edi@1
  int v5; // ebx@2
  int v6; // eax@2
  int v7; // eax@3
  RenderVertexSoft *v8; // edi@3
  RenderVertexSoft *v9; // ebx@4
  RenderVertexSoft *v10; // ecx@4
  float v11; // eax@6
  double v12; // ST5C_8@6
  double v13; // ST2C_8@6
  int v14; // eax@6
  double v15; // st7@6
  struct Polygon *pTile; // ebx@12
  unsigned __int16 v17; // ax@12
  int v18; // eax@13
  signed int v22; // eax@13
  Vec3_float_ *norm; // eax@15
  //double v24; // st6@17
  double v25; // ST54_8@17
  unsigned __int8 v26; // sf@17
  unsigned __int8 v27; // of@17
  double v28; // st5@19
  double v29; // st5@19
  double v30; // st5@19
  double v31; // st5@19
  struct struct8 *v32; // esi@21
  double v3a; // st7@32
  int v33; // edi@38
  unsigned int v34; // ecx@47
  char v35; // zf@47
  unsigned int v36; // eax@50
  int v37; // eax@54
  //Polygon *v38; // ecx@55
  unsigned int v39; // eax@59
  struct Polygon *v40; // ebx@62
  unsigned __int16 pTileBitmapsID; // ax@62
  int v42; // eax@63
  LightmapBuilder *v43; // ecx@63
  int v44; // eax@63
  int v45; // eax@63
  int v46; // eax@63
  signed int v47; // eax@63
  Vec3_float_ *v48; // eax@65
  double v49; // st6@67
  double v50; // ST4C_8@67
  double v51; // st5@71
  double v52; // st5@71
  double v53; // st5@71
  double v54; // st7@84
  unsigned int v55; // ecx@98
  unsigned int v56; // eax@101
  int v57; // eax@105
  unsigned int v58; // eax@109
  struct Polygon *v59; // esi@112
  unsigned __int16 v60; // ax@112
  int v61; // eax@113
  signed int v62; // eax@113
  Vec3_float_ *v63; // eax@114
  double v64; // st6@116
  double v65; // ST3C_8@116
  double v66; // st5@120
  double v67; // st5@120
  double v68; // st5@120
  double v69; // st7@133
  int v70; // edi@138
  struct Polygon *v71; // esi@147
  unsigned int v72; // ecx@147
  unsigned int v73; // eax@150
  int v74; // eax@154
  unsigned int v75; // eax@158
  //unsigned int v76; // [sp-10h] [bp-E0h]@61
  int v77; // [sp-Ch] [bp-DCh]@61
  IDirect3DTexture2 *v78; // [sp-8h] [bp-D8h]@61
  //int v79; // [sp-4h] [bp-D4h]@61
  bool v80; // [sp+0h] [bp-D0h]@59
  bool v81; // [sp+0h] [bp-D0h]@109
  int v82; // [sp+54h] [bp-7Ch]@1
  int v83; // [sp+60h] [bp-70h]@1
  int v84; // [sp+6Ch] [bp-64h]@1
  int v85; // [sp+70h] [bp-60h]@63
  float a4; // [sp+74h] [bp-5Ch]@73
  float v87; // [sp+78h] [bp-58h]@122
  int v88; // [sp+7Ch] [bp-54h]@1
  int v89; // [sp+80h] [bp-50h]@6
  int v93; // [sp+90h] [bp-40h]@2
  int X; // [sp+94h] [bp-3Ch]@1
  float v95; // [sp+98h] [bp-38h]@21
  LightmapBuilder *v96; // [sp+9Ch] [bp-34h]@73
  int v97; // [sp+A0h] [bp-30h]@6
  int sX; // [sp+A4h] [bp-2Ch]@6
  unsigned int uNumVertices; // [sp+A8h] [bp-28h]@73
  int v100; // [sp+ACh] [bp-24h]@122
  int sY; // [sp+B0h] [bp-20h]@6
  RenderVertexSoft *v102; // [sp+B4h] [bp-1Ch]@3
  unsigned int a5; // [sp+B8h] [bp-18h]@21
  RenderVertexSoft *v101; // [sp+BCh] [bp-14h]@6
  Vec3_float_ *v99; // [sp+C0h] [bp-10h]@17
  RenderVertexSoft *pVertices; // [sp+C4h] [bp-Ch]@6
  RenderVertexSoft *pVertices2; // [sp+C8h] [bp-8h]@6
  char v108; // [sp+CFh] [bp-1h]@36
  float thisd; // [sp+D8h] [bp+8h]@6
  float thise; // [sp+D8h] [bp+8h]@6
  float thisf; // [sp+D8h] [bp+8h]@17
  IndoorCameraD3D *thisa; // [sp+D8h] [bp+8h]@23
  float thisg; // [sp+D8h] [bp+8h]@67
  IndoorCameraD3D *thisb; // [sp+D8h] [bp+8h]@75
  float thish; // [sp+D8h] [bp+8h]@116
  IndoorCameraD3D *thisc; // [sp+D8h] [bp+8h]@124
  char this_3; // [sp+DBh] [bp+Bh]@30
  char this_3a; // [sp+DBh] [bp+Bh]@82
  char this_3b; // [sp+DBh] [bp+Bh]@131

  __debugbreak();
  static stru154 static_sub_0048034E_stru_154;
  static stru154 stru_76D5A8;
  //v3 = a1;
  v82 = a2;
  v83 = a3;
  X = abs(unk4);
  v4 = 0;
  v88 = 0;
  v84 = a1 - 1;
  if ( a1 - 1 > 0 )
  {
    while ( 1 )
    {
      v5 = abs(X);//v5 = 13108
      v6 = abs(v83);//v6 = 13108
      --X;
      //__debugbreak(); // uncoment & refactor following large if
      v93 = (int)&terrain_76E5C8[(v5 << 7) + v6];
      if ( !v93->field_0 || ((v7 = 48 * v4, v8 = &pVerticesSR_806210[v4], a2 = v8, !v82) ? (v9 = (RenderVertexSoft *)((char *)&pVerticesSR_801A10 + v7),
                                                                       v10 = &pVerticesSR_806210[1] + v7) : (v9 = &pVerticesSR_806210[1] + v7, v10 = (RenderVertexSoft *)((char *)&pVerticesSR_801A10 + v7)),
             ((a8 = v9,
               pVertices = &pVerticesSR_801A10[1] + v7,
               v11 = v8->vWorldPosition.x,
               v101 = v10,
               v12 = v11 + 6.755399441055744e15,
               sX = LODWORD(v12),
               v13 = v8->vWorldPosition.y + 6.755399441055744e15,
               sY = LODWORD(v13),
               thisd = (v10->vWorldPosition.x + v8->vWorldPosition.x) * 0.5,
               v14 = WorldPosToGridCellX(floorf(thisd + 0.5f)),//maybe current camera position X
               v15 = v9->vWorldPosition.y + v8->vWorldPosition.y,
               v89 = v14,
               thise = v15 * 0.5,
               _this = (LightmapBuilder *)WorldPosToGridCellZ(floorf(thisd + 0.5f)),//maybe current camera position Z
               WorldPosToGridCellX(sX),
               WorldPosToGridCellZ(sY),
               !byte_4D864C)
           || !(pEngine->uFlags & 0x80))
          && !_481EFA_clip_terrain_poly(v8, v9, v101, pVertices, 1)) )
      if ( !&terrain_76E5C8[(v5 << 7) + v6] )
        goto LABEL_162
      v8 = &pVerticesSR_806210[v4];
      //pVertices2 = &pVerticesSR_801A10[v4 + 1];
      //v102 = v8;
      if (!v82)
      {
        pVertices = &pVerticesSR_801A10[v4];
        v101 = &pVerticesSR_806210[v4 + 1];
      }
      else
      {
        pVertices = &pVerticesSR_801A10[v4 + 1];
        v101 = &pVerticesSR_806210[v4];
      }
      sX = floorf(v8->vWorldPosition.x + 0.5f);
      sY = floorf(v8->vWorldPosition.z + 0.5f);
      v89 = WorldPosToGridCellX(floorf((v101->vWorldPosition.x + v8->vWorldPosition.x) / 2 + 0.5f));
      v97 = WorldPosToGridCellZ(floorf((pVertices->vWorldPosition.z + v8->vWorldPosition.z) / 2 + 0.5f));
      WorldPosToGridCellX(sX);
      WorldPosToGridCellZ(sY);
      if ((!byte_4D864C || !(pEngine->uFlags & 0x80)) && !_481EFA_clip_terrain_poly(v8, pVertices, v101, pVertices2, 1))
        if ( v8->vWorldPosition.y != pVertices->vWorldPosition.y || pVertices->vWorldPosition.y != pVertices2->vWorldPosition.y 
             || pVertices2->vWorldPosition.y != v101->vWorldPosition.y )
          break;
        pTile = &array_77EC08[pODMRenderParams->uNumPolygons];
        pTile->uTileBitmapID = pOutdoor->GetTileTexture(sX, sY);
        if ( pTile->uTileBitmapID != -1 )
        {
          pTile->flags = 0x8010 |pOutdoor->GetSomeOtherTileInfo(sX, sY);
          pTile->field_32 = 0;
          pTile->field_59 = 1;
          pTile->terrain_grid_x = (char)v97;
          __debugbreak(); // warning C4700: uninitialized local variable 'v93' used
          pTile->field_34 = *(_WORD *)(v93 + 2);
          pTile->terrain_grid_z = v89;
          v22 = pTerrainNormalIndices[2 * (v97 + 128 * v89) + 1];
          if ( v22 < 0 || v22 > uNumTerrainNormals - 1 )
            norm = 0;
          else
            norm = &pTerrainNormals[v22];
          thisf = 20.0 - ( -(((float)pOutdoor->vSunlight.x / 65536.0) * norm->x) -
                            (((float)pOutdoor->vSunlight.y / 65536.0) * norm->y) -
                            (((float)pOutdoor->vSunlight.z / 65536.0) * norm->z)) * 20.0;
          //v25 = thisf + 6.7553994e15;
          //v27 = pODMRenderParams->uNumPolygons > 1999;
          //v26 = pODMRenderParams->uNumPolygons - 1999 < 0;
          pTile->dimming_level = floorf(thisf + 0.5f);
          if ( pODMRenderParams->uNumPolygons >= 1999 )
            return;
          ++pODMRenderParams->uNumPolygons;
          //if ( !_481FC9_terrain(v8, pVertices, v101, v16) )//Ritor1: It's temporary
          //goto LABEL_126;
          //{
            //--pODMRenderParams->uNumPolygons;
            //goto LABEL_162;
          //}
          __debugbreak(); // warning C4700: uninitialized local variable 'v102' used
          memcpy(&VertexRenderList[0], v102, 0x30u);
          VertexRenderList[0]._rhw = 1.0 / (v102->vWorldViewPosition.x + 0.0000001000000011686097);
          VertexRenderList[0].u = 0.0;
          VertexRenderList[0].v = 0.0;
          memcpy(&VertexRenderList[1], pVertices, sizeof(VertexRenderList[1]));
          VertexRenderList[1]._rhw = 1.0 / (pVertices->vWorldViewPosition.x + 0.0000001000000011686097);
          VertexRenderList[1].u = 0.0;
          VertexRenderList[1].v = 1.0;
          __debugbreak(); // warning C4700: uninitialized local variable 'pVertices2' used
          memcpy(&VertexRenderList[2], pVertices2, sizeof(VertexRenderList[2]));
          VertexRenderList[2]._rhw = 1.0 / (pVertices2->vWorldViewPosition.x + 0.0000001000000011686097);
          VertexRenderList[2].u = 1.0;
          VertexRenderList[2].v = 1.0;
          memcpy(&VertexRenderList[3], v101, sizeof(VertexRenderList[3]));
          VertexRenderList[3]._rhw = 1.0 / (v101->vWorldViewPosition.x + 0.0000001000000011686097);
          VertexRenderList[3].u = 1.0;
          VertexRenderList[3].v = 0.0;
          if ( !(_76D5C0_static_init_flag & 1) )
          {
            _76D5C0_static_init_flag |= 1u;
            stru154(stru_76D5A8);
            atexit(loc_481199);
          }
          v32 = (struct8 *)VertexRenderList;
          v97 = (int)pEngine->pLightmapBuilder;
          pEngine->pLightmapBuilder->StackLights_TerrainFace(norm, &v95, VertexRenderList, 4, 1);
          pDecalBuilder->_49BE8A(pTile, norm, &v95, VertexRenderList, 4, 1);
          a5 = 4;
          if ( byte_4D864C && pEngine->uFlags & 0x80 )
          {
            thisa = pIndoorCameraD3D;
            if ( pIndoorCameraD3D->_4371C3(VertexRenderList, &a5, 0) == 1 && !a5 )
              goto LABEL_162;
            thisa->ViewTransform(VertexRenderList, a5);
            thisa->Project(VertexRenderList, a5, 0);
          }
          this_3 = v102->vWorldViewPosition.x < 8.0 || pVertices->vWorldViewPosition.x < 8.0
              || v101->vWorldViewPosition.x < 8.0 || pVertices2->vWorldViewPosition.x < 8.0;
          v3a = (double)pODMRenderParams->shading_dist_mist;
          v108 = v3a < v102->vWorldViewPosition.x || v3a < pVertices->vWorldViewPosition.x
              || v3a < v101->vWorldViewPosition.x || v3a < pVertices2->vWorldViewPosition.x;
          v33 = 0;
          pEngine->pLightmapBuilder->StationaryLightsCount = 0;
          if ( Lights.uNumLightsApplied > 0 || pDecalBuilder->uNumDecals > 0 )
          {
            if ( this_3 )
              v33 = 3;
            else
              v33 = v108 != 0 ? 5 : 0;
            static_sub_0048034E_stru_154.ClassifyPolygon(norm, v95);
            if ( pDecalBuilder->uNumDecals > 0 )
              pDecalBuilder->ApplyDecals(31 - pTile->dimming_level, 4, &static_sub_0048034E_stru_154, a5, VertexRenderList, 0, *(float *)&v33, -1);
          }
          if ( Lights.uNumLightsApplied > 0 )
            pEngine->pLightmapBuilder->ApplyLights(&Lights, &static_sub_0048034E_stru_154, a5, VertexRenderList, 0, v33);
          v34 = a5;
          //v35 = byte_4D864C == 0;
          pTile->uNumVertices = a5;
          if ( !byte_4D864C || ~pEngine->uFlags & 0x80 )
          {
            if ( this_3 )
            {
              v36 = ODM_NearClip(v34);
              pTile->uNumVertices = v36;
              ODMRenderParams::Project(v36);
            }
            if ( v108 )
            {
              v36 = ODM_FarClip(v34);
              pTile->uNumVertices = v36;
              ODMRenderParams::Project(v36);
            }
          }
          //v37 = *(int *)&v16->flags;
          if ( ~pTile->flags & 1 )
          {
            if ( pTile->flags & 2 && pTile->uTileBitmapID == render->hd_water_tile_id )
            {
              v80 = false;
              v39 = render->pHDWaterBitmapIDs[render->hd_water_current_frame];
            }
            else
            {
              v39 = pTile->uTileBitmapID;
              v80 = true;
            }
            //v79 = 0;
            //v78 = pBitmaps_LOD->pHardwareTextures[v39];
            pTile->pTexture = (Texture_MM7 *)&pBitmaps_LOD->pHardwareTextures[v39];// Ritor1: It's temporary
            v77 = (int)pTile;
            //v76 = v16->uNumVertices;
//LABEL_161:
            render->DrawTerrainPolygon(pTile->uNumVertices, pTile, pBitmaps_LOD->pHardwareTextures[v39], false, v80);
            goto LABEL_162;
          }
LABEL_56:
          pTile->DrawBorderTiles();
        }
LABEL_162:
        v4 = v88 + 1;
        if ( ++v88 >= v84 )
          return;
      }
      v40 = &array_77EC08[pODMRenderParams->uNumPolygons];
      v40->uTileBitmapID = pOutdoor->GetTileTexture(sX, sY);
      if ( v40->uTileBitmapID == -1 )
        goto LABEL_162;
      v42 = pOutdoor->GetSomeOtherTileInfo(sX, sY);
      BYTE1(v42) |= 0x80u;
      v43 = pEngine->pLightmapBuilder;
      *(int *)&v40->flags = v42;
      v44 = v93;
      v40->field_59 = 1;
      v40->terrain_grid_x = (char)v43;
      v40->field_34 = *(_WORD *)(v44 + 2);
      v45 = v89;
      v40->terrain_grid_z = v89;
      v46 = 4 * ((char)v43 + (v45 << 7));
      v85 = v46;
      v47 = *(unsigned __int16 *)((char *)pTerrainNormalIndices + v46 + 2);//    v47 = pTerrainNormalIndices[v46 + 1];
      if ( v47 < 0 || v47 > (signed int)(uNumTerrainNormals - 1) )
        v48 = 0;
      else
        v48 = &pTerrainNormals[v47];
      v49 = v92 * v48->y;
      //v99 = v48;
      thisg = 20.0 - (-v49 - v91 * v48->z - v90 * v48->x) * 20.0;
      v50 = thisg + 6.755399441055744e15;
      v40->dimming_level = LOBYTE(v50);
      if ( LOBYTE(v50) < 0 )
        v40->dimming_level = 0;
      if ( pODMRenderParams->uNumPolygons >= 1999 )
        return;
      ++pODMRenderParams->uNumPolygons;
      if ( !_481FC9_terrain(pVertices, pVertices2, v8, v40) ) // Ritor1: It's temporary
        //goto LABEL_77;
        {
          --pODMRenderParams->uNumPolygons;
          goto LABEL_112;
        }
      memcpy(&VertexRenderList[0], v102, 0x30u);
      VertexRenderList[0]._rhw = 1.0 / (v102->vWorldViewPosition.x + 0.0000001000000011686097);
      VertexRenderList[0].u = 0.0;
      VertexRenderList[0].v = 0.0;
      memcpy(&VertexRenderList[1], pVertices, sizeof(VertexRenderList[1]));
      VertexRenderList[1]._rhw = 1.0 / pVertices->vWorldViewPosition.x + 0.0000001000000011686097;
      VertexRenderList[1].u = 0.0;
      VertexRenderList[1].v = 1.0;
      memcpy(&VertexRenderList[2], pVertices2, sizeof(VertexRenderList[2]));
      VertexRenderList[2]._rhw = 1.0 / pVertices2->vWorldViewPosition.x + 0.0000001000000011686097;
      VertexRenderList[2].u = 1.0;
      VertexRenderList[2].v = 1.0;
      static stru154 static_sub_0048034E_stru_76D590;
      static bool __init_flag2 = false;
      if (!__init_flag2)
      {
        __init_flag2 = true;
        stru154::stru154(&static_sub_0048034E_stru_76D590);
      }
      if ( !(_76D5C0_static_init_flag & 2) )
      {
        _76D5C0_static_init_flag |= 2;
        Polygon(stru_76D590);
        atexit(loc_48118F);
      }
      v96 = pEngine->pLightmapBuilder;
      pEngine->pLightmapBuilder->StackLights_TerrainFace(v48, (float *)&a4, VertexRenderList, 3, 0);
      pDecalBuilder->_49BE8A(v40, v48, &a4, VertexRenderList, 3, 0);
      uNumVertices = 3;
      if ( byte_4D864C && pEngine->uFlags & 0x80 )
      {
        thisb = pIndoorCameraD3D;
        if ( pIndoorCameraD3D->_4371C3(VertexRenderList, &uNumVertices, 0) == 1 && !uNumVertices )
        {
//LABEL_77:
          --pODMRenderParams->uNumPolygons;
          goto LABEL_112;
        }
        thisb->ViewTransform(VertexRenderList, uNumVertices);
        thisb->Project(VertexRenderList, uNumVertices, 0);
      }
      this_3a = v102->vWorldViewPosition.x < 8.0 || pVertices->vWorldViewPosition.x < 8.0 || pVertices2->vWorldViewPosition.x < 8.0;
      v54 = (double)pODMRenderParams->shading_dist_mist;
      v108 = v54 < v102->vWorldViewPosition.x || v54 < pVertices->vWorldViewPosition.x || v54 < pVertices2->vWorldViewPosition.x;
      pVertices = 0;
      v96->StationaryLightsCount = 0;
      if ( Lights.uNumLightsApplied > 0 || pDecalBuilder->uNumDecals > 0 )
      {
        if ( this_3a )
          pVertices = (RenderVertexSoft *)3;
        else
          pVertices = (RenderVertexSoft *)(v108 != 0 ? 5 : 0);
        //a8 = (RenderVertexSoft *)(this_3a ? 3 : v108 != 0 ? 5 : 0);
        static_sub_0048034E_stru_76D590.ClassifyPolygon(v48, *(float *)&a4);
        if ( pDecalBuilder->uNumDecals > 0 )
          pDecalBuilder->ApplyDecals(31 - v40->dimming_level, 4, &static_sub_0048034E_stru_76D590, uNumVertices, VertexRenderList, 0, (char)pVertices, -1);
      }
      if ( Lights.uNumLightsApplied > 0 )
        v96->ApplyLights(&Lights, &static_sub_0048034E_stru_76D590, uNumVertices, VertexRenderList, 0, (char)pVertices);
      v55 = uNumVertices;
      //v35 = byte_4D864C == 0;
      v40->uNumVertices = uNumVertices;
      if ( !_76D5C0_static_init_flag || !(pEngine->uFlags & 0x80) )
      {
        if ( this_3a )
        {
          v56 = ODM_NearClip(v55);
        }
        else
        {
          if ( !v108 )
            goto LABEL_105;
          v56 = sr_424EE0_MakeFanFromTriangle(v55);
        }
        v40->uNumVertices = v56;
        ODMRenderParams::Project(v56);
      }
LABEL_105:
      v57 = *(int *)&v40->flags;
      if ( BYTE1(v57) & 1 )
      {
        v40->DrawBorderTiles();
      }
      else
      {
        if ( v57 & 2 && v40->uTileBitmapID == render->hd_water_tile_id )
        {
          v81 = false;
          v58 = render->pHDWaterBitmapIDs[render->hd_water_current_frame];
        }
        else
        {
          v58 = v40->uTileBitmapID;
          v81 = true;
        }
        render->DrawTerrainPolygon(v40->uNumVertices, v40, pBitmaps_LOD->pHardwareTextures[v58], 0, v81);
      }
LABEL_112:
      v59 = &array_77EC08[pODMRenderParams->uNumPolygons];
      //a8 = (RenderVertexSoft *)&array_77EC08[pODMRenderParams->uNumPolygons];
      v59->uTileBitmapID = pOutdoor->GetTileTexture(sX, sY);
      if ( v59->uTileBitmapID  == -1 )
        goto LABEL_162;
      *(int *)&v59->flags = pOutdoor->GetSomeOtherTileInfo(sX, sY);
      v61 = v93;
      v59->field_59 = 1;
      v59->field_34 = *(_WORD *)(v61 + 2);
      v59->terrain_grid_z = v89;
      v59->terrain_grid_x = v97;
      v62 = *(unsigned __int16 *)((char *)pTerrainNormalIndices + v85);
      if ( v62 > (signed int)(uNumTerrainNormals - 1) )
        v63 = 0;
      else
        v63 = &pTerrainNormals[v62];
      v64 = v92 * v63->y;
      //v99 = v63;
      thish = 20.0 - (-v64 - v91 * v63->y - v90 * v63->x) * 20.0;
      v59->dimming_level = floorf(thish + 0.5f);
      if ( v59->dimming_level < 0 )
        v59->dimming_level = 0;
      if ( pODMRenderParams->uNumPolygons >= 1999 )
        return;
      ++pODMRenderParams->uNumPolygons;
      if ( !_481FC9_terrain(v101, v102, pVertices2, v59) )
      {
//LABEL_126:
        --pODMRenderParams->uNumPolygons;
        goto LABEL_162;
      }
      memcpy(&VertexRenderList[0], v102, 0x30u);
      VertexRenderList[0]._rhw = 1.0 / (v102->vWorldViewPosition.x + 0.0000001000000011686097);
      VertexRenderList[0].u = 0.0;
      VertexRenderList[0].v = 0.0;
      memcpy(&VertexRenderList[1], pVertices2, sizeof(VertexRenderList[1]));
      VertexRenderList[1]._rhw = 1.0 / pVertices2->vWorldViewPosition.x + 0.0000001000000011686097;
      VertexRenderList[1].u = 1.0;
      VertexRenderList[1].v = 1.0;
      memcpy(&VertexRenderList[2], v101, sizeof(VertexRenderList[2]));
      VertexRenderList[2]._rhw = 1.0 / v101->vWorldViewPosition.x + 0.0000001000000011686097;
      VertexRenderList[2].u = 1.0;
      VertexRenderList[2].v = 0.0;
      static stru154 static_sub_0048034E_stru_76D578;
      static bool __init_flag1 = false;
      if (!__init_flag1)
      {
        __init_flag1 = true;
        stru154::stru154(&static_sub_0048034E_stru_76D578);
      }
      v96 = pEngine->pLightmapBuilder;
      pEngine->pLightmapBuilder->StackLights_TerrainFace(v63, &v87, VertexRenderList, 3, 1);
      pDecalBuilder->_49BE8A(v40, v63, &v87, VertexRenderList, 3, 1);
      v100 = 3;
      if ( byte_4D864C && pEngine->uFlags & 0x80 )
      {
        thisc = pIndoorCameraD3D;
        if ( pIndoorCameraD3D->_4371C3(VertexRenderList, (unsigned int *)&v100, 0) == 1 && !v100 )
          //goto LABEL_126;
        {
          --pODMRenderParams->uNumPolygons;
          goto LABEL_162;
        }
        thisc->ViewTransform(VertexRenderList, v100);
        thisc->Project(VertexRenderList, v100, 0);
      }
      this_3b = v102->vWorldViewPosition.x < 8.0 || pVertices2->vWorldViewPosition.x < 8.0
           || v101->vWorldViewPosition.x < 8.0;
      v69 = (double)pODMRenderParams->shading_dist_mist;
      v108 = v69 < v102->vWorldViewPosition.x || v69 < pVertices2->vWorldViewPosition.x || v69 < v101->vWorldViewPosition.x;
      v70 = 0;
      v96->StationaryLightsCount = 0;
      if ( Lights.uNumLightsApplied > 0 || pDecalBuilder->uNumDecals > 0 )
      {
        if ( this_3b )
          v70 = 3;
        else
          v70 = v108 != 0 ? 5 : 0;
        static_sub_0048034E_stru_76D578.ClassifyPolygon(v63, v87);
        if ( pDecalBuilder->uNumDecals > 0 )
          pDecalBuilder->ApplyDecals(31 - v40->dimming_level, 4, &static_sub_0048034E_stru_76D578, v100, VertexRenderList, 0, v70, -1);
      }
      if ( Lights.uNumLightsApplied > 0 )
        v96->ApplyLights(&Lights, &static_sub_0048034E_stru_76D578, v100, VertexRenderList, 0, v70);
      v71 = v59;
      v72 = v100;
      //v35 = byte_4D864C == 0;
      v59->uNumVertices = v100;//???
      if ( !byte_4D864C && pEngine->uFlags & 0x80 )
        goto LABEL_154;
      if ( this_3b )
      {
        v73 = ODM_NearClip(v72);
      }
      else
      {
        if ( !v108 )
        {
LABEL_154:
          v74 = v71->flags;
          if ( !(BYTE1(v74) & 1) )
          {
            if ( v74 & 2 && v71->uTileBitmapID == render->hd_water_tile_id )
            {
              v80 = false;
              v75 = render->pHDWaterBitmapIDs[render->hd_water_current_frame];
            }
            else
            {
              v75 = v71->uTileBitmapID;
              v80 = true;
            }
            //v79 = 0;
            v78 = pBitmaps_LOD->pHardwareTextures[v75];
            v71->pTexture = (Texture_MM7 *)&pBitmaps_LOD->pHardwareTextures[v75];// Ritor1: It's temporary
            //v77 = (int)v71;
            //v76 = v71->uNumVertices;
            //goto LABEL_161;
            render->DrawTerrainPolygon(v71->uNumVertices, (Polygon *)v71, v78, 0, v80);
            goto LABEL_162;
          }
          v38 = (Polygon *)v71;
          goto LABEL_56;
        }
        v73 = sr_424EE0_MakeFanFromTriangle(v72);
      }
      v71->uNumVertices = v73;
      ODMRenderParams::Project(v73);
      goto LABEL_154;
    }
  }
//----- (004BF3F9) --------------------------------------------------------
bool MPlayer::AnyMovieLoaded()
{
  return pMovie_Track != 0;
}
//----- (004BF8F6) --------------------------------------------------------
void MPlayer::PlayDeathMovie()
{
  bStopBeforeSchedule = 0;
  PlayFullscreenMovie(MOVIE_Death, true);
}












#pragma once
#include "lib\legacy_dx\d3d.h"

#pragma pack(push, 1)
class GammaController
{
public:
GammaController();
inline ~GammaController() {} //----- (0044F22E)

void Initialize(float gamma); // 44F4D9

void GetRamp();
double _44F377(DDGAMMARAMP *a1);
void SetGammaRamp(DDGAMMARAMP *pRamp);
int InitGammaRamp(DDGAMMARAMP *pRamp);

void InitializeFromSurface(struct IDirectDrawSurface4 *a2);



void(***vdestructor_ptr)(GammaController *, bool);
IDirectDrawSurface4 *pSurface;
IDirectDrawGammaControl *pGammaControl;
DDGAMMARAMP pDefaultRamp;
DDGAMMARAMP field_60C;
float fGamma;//field_C0C;
};
#pragma pack(pop)



#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <string>

#include "GammaControl.h"
#include "Render.h"

#include "../mm7_data.h"



//----- (0044F324) --------------------------------------------------------
void GammaController::GetRamp()
{
if (pGammaControl)
ErrD3D(pGammaControl->GetGammaRamp(0, &pDefaultRamp));
}

//----- (0044F377) --------------------------------------------------------
double GammaController::_44F377(DDGAMMARAMP *a1)
{
double v2; // st7@1
unsigned __int16 *v3; // ecx@1
double v4; // st6@2
signed int v6; // [sp+0h] [bp-8h]@1
signed int v7; // [sp+4h] [bp-4h]@1

v2 = 0.0;
v6 = 256;
v3 = a1->green;
for ( v7 = 0; v7 < 256; ++v7 )
{
v4 = ((double)*(v3 - 256) + (double)v3[256] + (double)*v3) * 0.000015259022 * 0.33333334;
if ( v4 == 0.0 )
--v6;
else
v2 = v2 + (double)v7 * 0.0039215689 / v4;
++v3;
}
if ( v6 )
return v2 / (double)v6;
else
return 1.0;
}

//----- (0044F408) --------------------------------------------------------
void GammaController::SetGammaRamp(DDGAMMARAMP *pRamp)
{
if (pGammaControl)
ErrD3D(pGammaControl->SetGammaRamp(0, pRamp));
}

//----- (0044F45B) --------------------------------------------------------
int GammaController::InitGammaRamp(DDGAMMARAMP *pRamp)
{
unsigned __int16 *v2; // esi@1
double v3; // st7@2
signed __int64 v4; // qax@3
signed int v6; // [sp+Ch] [bp-4h]@1

v2 = pRamp->green;
for ( v6 = 0; v6 < 256; ++v6 )
{
v3 = (double)v6 * 0.0039215689 * this->fGamma;
if ( v3 >= 1.0 || (v4 = (signed __int64)(v3 * 65535.0), (signed int)v4 > 65535) )
LODWORD(v4) = 65535;
else if ( (signed int)v4 < 0 )
LODWORD(v4) = 0;
else
{
if ( (signed int)v4 > 65535 )
LODWORD(v4) = 65535;
}
v2[256] = v4;
*v2 = v4;
*(v2 - 256) = v4;
++v2;
}
return v4;
}

//----- (0044F4D9) --------------------------------------------------------
void GammaController::Initialize(float gamma)
{
double v3; // st7@3
double v4; // st6@4

//if (pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ||
//pVersion->pVersionInfo.dwMajorVersion != 4)
{
InitializeFromSurface(render->pFrontBuffer4);
v3 = 2.8;
if ( gamma <= 2.8f )
v4 = gamma;
else
v4 = 2.8;
if ( v4 >= 0.1f )
{
if ( gamma <= 2.8f )
v3 = gamma;
}
else
v3 = 0.1;
this->fGamma = v3;
InitGammaRamp(&this->field_60C);
SetGammaRamp(&this->field_60C);
}
}

//----- (0044F215) --------------------------------------------------------
GammaController::GammaController()
{
static float flt_4D8670_default_gamma = 1.0f;

pGammaControl = nullptr;
fGamma = flt_4D8670_default_gamma;
}

//----- (0044F24B) --------------------------------------------------------
void GammaController::InitializeFromSurface(IDirectDrawSurface4 *a2)
{
this->pSurface = a2;
if ( this->pGammaControl )
{
this->pGammaControl->Release();
this->pGammaControl = 0;
}
if ( render->IsGammaSupported() )
{
ErrD3D(a2->QueryInterface(IID_IDirectDrawGammaControl, (LPVOID *)&this->pGammaControl));
GetRamp();
memcpy(&this->field_60C, &this->pDefaultRamp, 0x600u);
fGamma = _44F377(&this->field_60C);
}
}





//----- (0044E904) --------------------------------------------------------
void Game::_44E904_gamma_saturation_adjust()
{
//Game *v1; // esi@1
unsigned __int64 v2; // qax@1
unsigned int v3; // ecx@1
int v4; // edi@1
unsigned __int8 v5; // cf@7
double v6; // st7@13
double v7; // st7@15
signed __int64 v8; // [sp+Ch] [bp-8h]@1

//v1 = this;
v2 = pEventTimer->Time();
v4 = (v2 - uSomeGammaStartTime) >> 32;
v3 = v2 - LODWORD(uSomeGammaStartTime);
v8 = v2 - uSomeGammaStartTime;
if ( v4 < 0
|| SHIDWORD(v2) < ((unsigned int)v2 < LODWORD(uSomeGammaStartTime)) + HIDWORD(uSomeGammaStartTime) | v4 == 0
&& v3 <= 0x80 )
{
if ( v4 < 0 )
{
v3 = 0;
v4 = 0;
uSomeGammaStartTime = v2;
v8 = __PAIR__(v4, v3);
}
}
else
{
if ( uSomeGammaDeltaTime )
{
uSomeGammaDeltaTime = 0;
}
else
{
LODWORD(uSomeGammaDeltaTime) = v3;
HIDWORD(uSomeGammaDeltaTime) = v4;
}
v5 = __CFADD__(v3, -128);
v3 -= 128;
v4 = v5 + v4 - 1;
uSomeGammaStartTime = v2;
v8 = __PAIR__(v4, v3);
}
if ( uSomeGammaDeltaTime )
v6 = (double)(signed __int64)(uSomeGammaDeltaTime - __PAIR__(v4, v3));
else
v6 = (double)v8;
v7 = v6 * 0.0078125;
if ( v7 < 0.0 || v7 <= 1.0 )
{
if ( v7 < 0.0 )
v7 = 0.0;
}
else
v7 = 1.0;
//if ( render->pRenderD3D )
fSaturation = v7;
//else
//  fSaturation = (1.0 - 0.5) * v7 + 0.5;
}

//----- (0044EA17) --------------------------------------------------------
bool Game::InitializeGammaController()
{
//if (pVersion->pVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT ||
//pVersion->pVersionInfo.dwMajorVersion != 4 )
pGammaController->InitializeFromSurface(render->pFrontBuffer4);

uSomeGammaStartTime = pEventTimer->Time();
return true;
}
*/