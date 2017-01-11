#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>

#include "BSPModel.h"



//----- (00478389) --------------------------------------------------------
void BSPModel::Release()
{
  free(this->pVertices.pVertices);
  this->pVertices.pVertices = 0;
  free(this->pFaces);
  this->pFaces = nullptr;
  free(this->pFacesOrdering);
  this->pFacesOrdering = nullptr;
  free(this->pNodes);
  this->pNodes = nullptr;
  this->uNumNodes = 0;
  this->uNumFaces = 0;
  this->pVertices.uNumVertices = 0;
  this->uNumConvexFaces = 0;
}
