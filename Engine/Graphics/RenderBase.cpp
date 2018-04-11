#include "Engine/Graphics/RenderBase.h"

#include <cassert>

#include "Engine/Engine.h"
#include "Engine/MM7.h"
#include "Engine/SpellFxRenderer.h"

#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/Actor.h"

#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/IndoorCameraD3D.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/Lights.h"
#include "Engine/Graphics/Outdoor.h"

unsigned int RenderBase::Billboard_ProbablyAddToListAndSortByZOrder(float z) {
  if (uNumBillboardsToDraw >= 999) {
    return 0;
  }

  if (!uNumBillboardsToDraw) {
    uNumBillboardsToDraw = 1;
    return 0;
  }

  unsigned int v7 = 0;
  for (int left = 0, right = uNumBillboardsToDraw; left < right;) {  // binsearch
    v7 = left + (right - left) / 2;
    if (z <= render->pBillboardRenderListD3D[v7].z_order)
      right = v7;
    else
      left = v7 + 1;
  }

  if (z > render->pBillboardRenderListD3D[v7].z_order) {
    if (v7 == render->uNumBillboardsToDraw - 1) {
      v7 = render->uNumBillboardsToDraw;
    } else {
      if (render->uNumBillboardsToDraw > v7) {
        for (unsigned int i = 0; i < render->uNumBillboardsToDraw - v7; i++) {
          memcpy(&render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i],
            &render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - (i + 1)],
            sizeof(render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i]));
        }
      }
      ++v7;
    }
    uNumBillboardsToDraw++;
    return v7;
  }

  if (z <= render->pBillboardRenderListD3D[v7].z_order) {
    if (render->uNumBillboardsToDraw > v7) {
      for (unsigned int i = 0; i < render->uNumBillboardsToDraw - v7; i++) {
        memcpy(&render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i],
          &render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - (i + 1)],
          sizeof(render->pBillboardRenderListD3D[render->uNumBillboardsToDraw - i]));
      }
    }
    uNumBillboardsToDraw++;
    return v7;
  }

  return v7;
}

void RenderBase::DrawSpriteObjects_ODM() {
  for (unsigned int i = 0; i < uNumSpriteObjects; ++i) {
    SpriteObject* object = &pSpriteObjects[i];
    //auto v0 = (char *)&pSpriteObjects[i].uSectorID;
    //v0 = (char *)&pSpriteObjects[0].uSectorID;
    //do
    //{
    if (!object->uObjectDescID)  // item probably pciked up
      continue;

    assert(object->uObjectDescID < pObjectList->uNumObjects);
    ObjectDesc* object_desc = &pObjectList->pObjects[object->uObjectDescID];
    if (object_desc->NoSprite())
      continue;

    //v1 = &pObjectList->pObjects[*((short *)v0 - 13)];
    //if ( !(v1->uFlags & 1) )
    //{
    //v2 = *((short *)v0 - 14)
    //v2 = object->uType;
    if ((object->uType < 1000 || object->uType >= 10000) && (object->uType < 500 || object->uType >= 600)
      || pEngine->GetSpellFxRenderer()->RenderAsSprite(object))
    {
      //a5 = *(short *)v0;
      int x = object->vPosition.x;
      int y = object->vPosition.y;
      int z = object->vPosition.z;
      SpriteFrame *frame = pSpriteFrameTable->GetFrame(object_desc->uSpriteID, object->uSpriteFrameID);
      int a6 = frame->uGlowRadius * object->field_22_glow_radius_multiplier;
      unsigned int v6 = stru_5C6E00->Atan2(object->vPosition.x - pIndoorCameraD3D->vPartyPos.x, object->vPosition.y - pIndoorCameraD3D->vPartyPos.y);
      //LOWORD(v7) = object->uFacing;
      //v8 = v36;
      int v9 = ((int)(stru_5C6E00->uIntegerPi + ((int)stru_5C6E00->uIntegerPi >> 3) + object->uFacing - v6) >> 8) & 7;
      pBillboardRenderList[::uNumBillboardsToDraw].hwsprite = frame->hw_sprites[v9];
      if (frame->uFlags & 0x20) {
        //v8 = v36;
        z -= fixpoint_mul(frame->scale._internal, frame->hw_sprites[v9]->uBufferHeight) / 2;
      }
      int16_t v46 = 0;
      if (frame->uFlags & 2)
        v46 = 2;
      //v11 = (int *)(256 << device_caps);
      if ((256 << v9) & frame->uFlags)
        v46 |= 4;
      if (frame->uFlags & 0x40000)
        v46 |= 0x40;
      if (frame->uFlags & 0x20000)
        v46 |= 0x80;
      if (a6) {
        pMobileLightsStack->AddLight(x, y, z, object->uSectorID, a6, 0xFF, 0xFF, 0xFF, _4E94D3_light_type);
      }


      int view_x = 0;
      int view_y = 0;
      int view_z = 0;

      bool visible = pIndoorCameraD3D->ViewClip(x, y, z, &view_x, &view_y, &view_z);
      if (visible) {
        if (abs(view_x) >= abs(view_y)) {
          int projected_x = 0;
          int projected_y = 0;
          pIndoorCameraD3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

          object->uAttributes |= 1;
          pBillboardRenderList[::uNumBillboardsToDraw].uPalette = frame->uPaletteIndex;
          pBillboardRenderList[::uNumBillboardsToDraw].uIndoorSectorID = object->uSectorID;
          pBillboardRenderList[::uNumBillboardsToDraw].pSpriteFrame = frame;

          pBillboardRenderList[::uNumBillboardsToDraw].screenspace_projection_factor_x = frame->scale * fixed::FromInt(pODMRenderParams->int_fov_rad) / fixed::FromInt(view_x);
          pBillboardRenderList[::uNumBillboardsToDraw].screenspace_projection_factor_y = frame->scale * fixed::FromInt(pODMRenderParams->int_fov_rad) / fixed::FromInt(view_x);

          pBillboardRenderList[::uNumBillboardsToDraw].field_1E = v46;
          pBillboardRenderList[::uNumBillboardsToDraw].world_x = x;
          pBillboardRenderList[::uNumBillboardsToDraw].world_y = y;
          pBillboardRenderList[::uNumBillboardsToDraw].world_z = z;

          pBillboardRenderList[::uNumBillboardsToDraw].screen_space_x = projected_x;
          pBillboardRenderList[::uNumBillboardsToDraw].screen_space_y = projected_y;
          pBillboardRenderList[::uNumBillboardsToDraw].screen_space_z = view_x;

          pBillboardRenderList[::uNumBillboardsToDraw].object_pid = PID(OBJECT_Item, i);
          pBillboardRenderList[::uNumBillboardsToDraw].dimming_level = 0;
          pBillboardRenderList[::uNumBillboardsToDraw].sTintColor = 0;
//          if (!(object->uAttributes & 0x20)) {
//            if (!pRenderD3D) {
//              __debugbreak();
//              pBillboardRenderList[::uNumBillboardsToDraw].screen_space_z = 0;
//              pBillboardRenderList[::uNumBillboardsToDraw].object_pid = 0;
//            }
//          }

          assert(::uNumBillboardsToDraw < 500);
          ++::uNumBillboardsToDraw;
          ++uNumSpritesDrawnThisFrame;
        }
      }
    }
  }
}

void RenderBase::TransformBillboardsAndSetPalettesODM() {
  SoftwareBillboard billboard = { 0 };
  billboard.sParentBillboardID = -1;
  //  billboard.pTarget = render->pTargetSurface;
  billboard.pTargetZ = render->pActiveZBuffer;
  //  billboard.uTargetPitch = render->uTargetSurfacePitch;
  billboard.uViewportX = pViewport->uViewportTL_X;
  billboard.uViewportY = pViewport->uViewportTL_Y;
  billboard.uViewportZ = pViewport->uViewportBR_X - 1;
  billboard.uViewportW = pViewport->uViewportBR_Y;
  pODMRenderParams->uNumBillboards = ::uNumBillboardsToDraw;

  for (unsigned int i = 0; i < ::uNumBillboardsToDraw; ++i) {
    auto p = &pBillboardRenderList[i];
    if (p->hwsprite) {
      billboard.screen_space_x = p->screen_space_x;
      billboard.screen_space_y = p->screen_space_y;
      billboard.screen_space_z = p->screen_space_z;
      billboard.sParentBillboardID = i;
      billboard.screenspace_projection_factor_x = p->screenspace_projection_factor_x;
      billboard.screenspace_projection_factor_y = p->screenspace_projection_factor_y;
      billboard.sTintColor = p->sTintColor;
      billboard.object_pid = p->object_pid;
      billboard.uFlags = p->field_1E;

      TransformBillboard(&billboard, p);
    }
  }
}

unsigned int BlendColors(unsigned int a1, unsigned int a2) {
  uint alpha = (uint)floorf(0.5f + (a1 >> 24) / 255.0f * (a2 >> 24) / 255.0f * 255.0f);
  uint red = (uint)floorf(0.5f + ((a1 >> 16) & 0xFF) / 255.0f * ((a2 >> 16) & 0xFF) / 255.0f * 255.0f);
  uint green = (uint)floorf(0.5f + ((a1 >> 8) & 0xFF) / 255.0f * ((a2 >> 8) & 0xFF) / 255.0f * 255.0f);
  uint blue = (uint)floorf(0.5f + ((a1 >> 0) & 0xFF) / 255.0f * ((a2 >> 0) & 0xFF) / 255.0f * 255.0f);
  return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

void RenderBase::TransformBillboard(SoftwareBillboard *a2, RenderBillboard *pBillboard) {
  Sprite *pSprite = pBillboard->hwsprite;
  int dimming_level = pBillboard->dimming_level;

  unsigned int v8 = Billboard_ProbablyAddToListAndSortByZOrder(a2->screen_space_z);

  float v30 = a2->screenspace_projection_factor_x.GetFloat();
  float v29 = a2->screenspace_projection_factor_y.GetFloat();

  unsigned int diffuse = ::GetActorTintColor(dimming_level, 0, a2->screen_space_z, 0, pBillboard);
  if (a2->sTintColor & 0x00FFFFFF && bTinting) {
    diffuse = BlendColors(a2->sTintColor, diffuse);
    if (a2->sTintColor & 0xFF000000)
      diffuse = 0x007F7F7F & ((unsigned int)diffuse >> 1);
  }

  unsigned int specular = 0;
  if (bUsingSpecular) {
    specular = sub_47C3D7_get_fog_specular(0, 0, a2->screen_space_z);
  }

  double v14 = (double)((int)pSprite->uBufferWidth / 2 - pSprite->uAreaX);
  double v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaY);
  if (a2->uFlags & 4) {
    v14 *= -1.0;
  }
  pBillboardRenderListD3D[v8].pQuads[0].diffuse = diffuse;
  pBillboardRenderListD3D[v8].pQuads[0].pos.x = (double)a2->screen_space_x - v14 * v30;
  pBillboardRenderListD3D[v8].pQuads[0].pos.y = (double)a2->screen_space_y - v15 * v29;
  pBillboardRenderListD3D[v8].pQuads[0].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / pIndoorCameraD3D->GetFarClip());
  pBillboardRenderListD3D[v8].pQuads[0].rhw = 1.0 / a2->screen_space_z;
  pBillboardRenderListD3D[v8].pQuads[0].specular = specular;
  pBillboardRenderListD3D[v8].pQuads[0].texcoord.x = 0.0;
  pBillboardRenderListD3D[v8].pQuads[0].texcoord.y = 0.0;

  v14 = (double)((int)pSprite->uBufferWidth / 2 - pSprite->uAreaX);
  v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaHeight - pSprite->uAreaY);
  if (a2->uFlags & 4)
    v14 = v14 * -1.0;
  pBillboardRenderListD3D[v8].pQuads[1].specular = specular;
  pBillboardRenderListD3D[v8].pQuads[1].diffuse = diffuse;
  pBillboardRenderListD3D[v8].pQuads[1].pos.x = (double)a2->screen_space_x - v14 * v30;
  pBillboardRenderListD3D[v8].pQuads[1].pos.y = (double)a2->screen_space_y - v15 * v29;
  pBillboardRenderListD3D[v8].pQuads[1].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / pIndoorCameraD3D->GetFarClip());
  pBillboardRenderListD3D[v8].pQuads[1].rhw = 1.0 / a2->screen_space_z;
  pBillboardRenderListD3D[v8].pQuads[1].texcoord.x = 0.0;
  pBillboardRenderListD3D[v8].pQuads[1].texcoord.y = 1.0;

  v14 = (double)((int)pSprite->uAreaWidth + pSprite->uAreaX + pSprite->uBufferWidth / 2 - pSprite->uBufferWidth);
  v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaHeight - pSprite->uAreaY);
  if (a2->uFlags & 4)
    v14 *= -1.0;
  pBillboardRenderListD3D[v8].pQuads[2].diffuse = diffuse;
  pBillboardRenderListD3D[v8].pQuads[2].specular = specular;
  pBillboardRenderListD3D[v8].pQuads[2].pos.x = (double)a2->screen_space_x + v14 * v30;
  pBillboardRenderListD3D[v8].pQuads[2].pos.y = (double)a2->screen_space_y - v15 * v29;
  pBillboardRenderListD3D[v8].pQuads[2].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / pIndoorCameraD3D->GetFarClip());
  pBillboardRenderListD3D[v8].pQuads[2].rhw = 1.0 / a2->screen_space_z;
  pBillboardRenderListD3D[v8].pQuads[2].texcoord.x = 1.0;
  pBillboardRenderListD3D[v8].pQuads[2].texcoord.y = 1.0;

  v14 = (double)((int)pSprite->uAreaWidth + pSprite->uAreaX + pSprite->uBufferWidth / 2 - pSprite->uBufferWidth);
  v15 = (double)((int)pSprite->uBufferHeight - pSprite->uAreaY);
  if (a2->uFlags & 4)
    v14 *= -1.0;
  pBillboardRenderListD3D[v8].pQuads[3].diffuse = diffuse;
  pBillboardRenderListD3D[v8].pQuads[3].specular = specular;
  pBillboardRenderListD3D[v8].pQuads[3].pos.x = (double)a2->screen_space_x + v14 * v30;
  pBillboardRenderListD3D[v8].pQuads[3].pos.y = (double)a2->screen_space_y - v15 * v29;
  pBillboardRenderListD3D[v8].pQuads[3].pos.z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / pIndoorCameraD3D->GetFarClip());
  pBillboardRenderListD3D[v8].pQuads[3].rhw = 1.0 / a2->screen_space_z;
  pBillboardRenderListD3D[v8].pQuads[3].texcoord.x = 1.0;
  pBillboardRenderListD3D[v8].pQuads[3].texcoord.y = 0.0;

  pBillboardRenderListD3D[v8].uNumVertices = 4;
  pBillboardRenderListD3D[v8].texture = pSprite->texture;
  pBillboardRenderListD3D[v8].z_order = a2->screen_space_z;
  pBillboardRenderListD3D[v8].field_90 = a2->field_44;
  pBillboardRenderListD3D[v8].screen_space_z = a2->screen_space_z;
  pBillboardRenderListD3D[v8].object_pid = a2->object_pid;
  pBillboardRenderListD3D[v8].sParentBillboardID = a2->sParentBillboardID;

  if (a2->sTintColor & 0xFF000000)
    pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Opaque_3;
  else
    pBillboardRenderListD3D[v8].opacity = RenderBillboardD3D::Transparent;
}

double fix2double(int fix) {
  return (((double)(fix & 0xFFFF) / (double)0xFFFF) + (double)(fix >> 16));
}

void RenderBase::MakeParticleBillboardAndPush_BLV(SoftwareBillboard *a2, Texture *texture, unsigned int uDiffuse, int angle) {
  if (a2->screen_space_z == 0) {
    return;
  }

  unsigned int billboard_index = Billboard_ProbablyAddToListAndSortByZOrder(a2->screen_space_z);
  RenderBillboardD3D *billboard = &pBillboardRenderListD3D[billboard_index];

  billboard->opacity = RenderBillboardD3D::Opaque_1;
  billboard->field_90 = a2->field_44;
  billboard->screen_space_z = a2->screen_space_z;
  billboard->object_pid = a2->object_pid;
  billboard->sParentBillboardID = a2->sParentBillboardID;
  billboard->texture = texture;
  billboard->z_order = a2->screen_space_z;
  billboard->uNumVertices = 4;

  float screenspace_projection_factor = a2->screenspace_projection_factor_x.GetFloat();

  float rhw = 1.0 / a2->screen_space_z;
  float z = 1.0 - 1.0 / (a2->screen_space_y * 0.061758894);

  double acos = cos(angle);
  double asin = sin(angle);

  {
    double v16 = -12.f;
    double v17 = -12.f;
    billboard->pQuads[0].pos.x = (acos * v16 - asin * v17) * screenspace_projection_factor + (double)a2->screen_space_x;
    billboard->pQuads[0].pos.y = (acos * v17 + asin * v16 - 12.) * screenspace_projection_factor + (double)a2->screen_space_y;
    billboard->pQuads[0].pos.z = z;
    billboard->pQuads[0].specular = 0;
    billboard->pQuads[0].diffuse = uDiffuse;
    billboard->pQuads[0].rhw = rhw;
    billboard->pQuads[0].texcoord.x = 0.0;
    billboard->pQuads[0].texcoord.y = 0.0;
  }

  {
    double v31 = -12;
    double v32 = 12;
    billboard->pQuads[1].pos.x = (acos * v31 - asin * v32) * screenspace_projection_factor + (double)a2->screen_space_x;
    billboard->pQuads[1].pos.y = (acos * v32 + asin * v31 - 12.0) * screenspace_projection_factor + (double)a2->screen_space_y;
    billboard->pQuads[1].pos.z = z;
    billboard->pQuads[1].specular = 0;
    billboard->pQuads[1].diffuse = uDiffuse;
    billboard->pQuads[1].rhw = rhw;
    billboard->pQuads[1].texcoord.x = 0.0;
    billboard->pQuads[1].texcoord.y = 1.0;
  }

  {
    double v23 = 12;
    double v24 = 12;
    billboard->pQuads[2].pos.x = (acos * v23 - asin * v24) * screenspace_projection_factor + (double)a2->screen_space_x;
    billboard->pQuads[2].pos.y = (acos * v24 + asin * v23 - 12.0) * screenspace_projection_factor + (double)a2->screen_space_y;
    billboard->pQuads[2].pos.z = z;
    billboard->pQuads[2].specular = 0;
    billboard->pQuads[2].diffuse = uDiffuse;
    billboard->pQuads[2].rhw = rhw;
    billboard->pQuads[2].texcoord.x = 1.0;
    billboard->pQuads[2].texcoord.y = 1.0;
  }

  {
    double v39 = 12;
    double v40 = -12;
    billboard->pQuads[3].pos.x = (acos * v39 - asin * v40) * screenspace_projection_factor + (double)a2->screen_space_x;
    billboard->pQuads[3].pos.y = (acos * v40 + asin * v39 - 12.0) * screenspace_projection_factor + (double)a2->screen_space_y;
    billboard->pQuads[3].pos.z = z;
    billboard->pQuads[3].specular = 0;
    billboard->pQuads[3].diffuse = uDiffuse;
    billboard->pQuads[3].rhw = rhw;
    billboard->pQuads[3].texcoord.x = 1.0;
    billboard->pQuads[3].texcoord.y = 0.0;
  }
}

void RenderBase::MakeParticleBillboardAndPush_ODM(SoftwareBillboard *a2, Texture *texture, unsigned int uDiffuse, int angle) {
  unsigned int billboard_index = Billboard_ProbablyAddToListAndSortByZOrder(a2->screen_space_z);
  RenderBillboardD3D *billboard = &pBillboardRenderListD3D[billboard_index];

  billboard->opacity = RenderBillboardD3D::Opaque_1;
  billboard->field_90 = a2->field_44;
  billboard->screen_space_z = a2->screen_space_z;
  billboard->object_pid = a2->object_pid;
  billboard->sParentBillboardID = a2->sParentBillboardID;
  billboard->texture = texture;
  billboard->z_order = a2->screen_space_z;
  billboard->uNumVertices = 4;

  float screenspace_projection_factor = a2->screenspace_projection_factor_x.GetFloat();

  float rhw = 1.0 / a2->screen_space_z;
  float z = 1.0 - 1.0 / (a2->screen_space_z * 1000.0 / pIndoorCameraD3D->GetFarClip());

  float acos = cos(angle);
  float asin = sin(angle);

  {
    float v16 = -12.f;
    float v17 = -12.f;
    billboard->pQuads[0].pos.x = (acos * v16 - asin * v17) * screenspace_projection_factor + (float)a2->screen_space_x;
    billboard->pQuads[0].pos.y = (acos * v17 + asin * v16 - 12.0) * screenspace_projection_factor + (float)a2->screen_space_y;
    billboard->pQuads[0].pos.z = z;
    billboard->pQuads[0].specular = 0;
    billboard->pQuads[0].diffuse = uDiffuse;
    billboard->pQuads[0].rhw = rhw;
    billboard->pQuads[0].texcoord.x = 0.0;
    billboard->pQuads[0].texcoord.y = 0.0;
  }

  {
    float v26 = -12.f;
    float v27 = 12.f;
    billboard->pQuads[1].pos.x = (acos * v26 - asin * v27) * screenspace_projection_factor + (float)a2->screen_space_x;
    billboard->pQuads[1].pos.y = (acos * v27 + asin * v26 - 12.0) * screenspace_projection_factor + (float)a2->screen_space_y;
    billboard->pQuads[1].pos.z = z;
    billboard->pQuads[1].specular = 0;
    billboard->pQuads[1].diffuse = uDiffuse;
    billboard->pQuads[1].rhw = rhw;
    billboard->pQuads[1].texcoord.x = 0.0;
    billboard->pQuads[1].texcoord.y = 1.0;
  }

  {
    float v33 = 12.f;
    float v34 = 12.f;
    billboard->pQuads[2].pos.x = (acos * v33 - asin * v34) * screenspace_projection_factor + (float)a2->screen_space_x;
    billboard->pQuads[2].pos.y = (acos * v34 + asin * v33 - 12.0) * screenspace_projection_factor + (float)a2->screen_space_y;
    billboard->pQuads[2].pos.z = z;
    billboard->pQuads[2].diffuse = uDiffuse;
    billboard->pQuads[2].specular = 0;
    billboard->pQuads[2].rhw = rhw;
    billboard->pQuads[2].texcoord.x = 1.0;
    billboard->pQuads[2].texcoord.y = 1.0;
  }

  {
    float v40 = 12.f;
    float v41 = -12.f;
    billboard->pQuads[3].pos.x = (acos * v40 - asin * v41) * screenspace_projection_factor + (float)a2->screen_space_x;
    billboard->pQuads[3].pos.y = (acos * v41 + asin * v40 - 12.0) * screenspace_projection_factor + (float)a2->screen_space_y;
    billboard->pQuads[3].pos.z = z;
    billboard->pQuads[3].specular = 0;
    billboard->pQuads[3].diffuse = uDiffuse;
    billboard->pQuads[3].rhw = rhw;
    billboard->pQuads[3].texcoord.x = 1.0;
    billboard->pQuads[3].texcoord.y = 0.0;
  }
}
