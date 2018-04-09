#include <stdlib.h>

#include "BSPModel.h"

void BSPModel::Load(uint8_t *data) {
  static_assert(sizeof(BSPModelData) == 188, "Wrong type size");
  static_assert(sizeof(BSPNode) == 8, "Wrong type size");

}

void BSPModel::Release() {
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
