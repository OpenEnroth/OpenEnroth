#include "BaseRenderer.h"

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Party.h"

#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"

#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Image.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Random/Random.h"

#include "Library/Logger/Logger.h"

#include "Utility/Math/TrigLut.h"
#include "Utility/Memory/MemSet.h"

bool BaseRenderer::Initialize() {
    updateRenderDimensions();
    return true;
}

unsigned int BaseRenderer::NextBillboardIndex() {
    if (uNumBillboardsToDraw >= MAX_BILLBOARDS_D3D) {
        return 0;
    }

    int index = uNumBillboardsToDraw++;
    pSortedBillboardRenderListD3D[index] = &pBillboardRenderListD3D[index];

    return index;
}

void BaseRenderer::SortBillboards() {
    // we need to loop over all billboards from farthest to nearest
    std::stable_sort(pSortedBillboardRenderListD3D.begin(), pSortedBillboardRenderListD3D.begin() + uNumBillboardsToDraw,
        [](const auto& a, const auto& b) {
            return a->view_space_z < b->view_space_z;
        });
}


// TODO: Move this to sprites ?
// combined with IndoorLocation::PrepareItemsRenderList_BLV() (0044028F)
void BaseRenderer::DrawSpriteObjects() {
    for (unsigned int i = 0; i < pSpriteObjects.size(); ++i) {
        // exit if we are at max sprites
        if (::uNumBillboardsToDraw >= 500) {
            logger->warning("Billboards Full");
            break;
        }

        SpriteObject *object = &pSpriteObjects[i];
        if (!object->uObjectDescID) {  // item probably pciked up - this also gets wiped at end of sprite anims/ particle effects
            continue;
        }
        if (!object->HasSprite()) {
            continue;
        }

        // view culling
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            bool onlist = false;
            for (unsigned j = 0; j < pBspRenderer->uNumVisibleNotEmptySectors; j++) {
                if (pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[j] == object->uSectorID) {
                    onlist = true;
                    break;
                }
            }
            if (!onlist) continue;
        } else {
            if (!IsCylinderInFrustum(object->vPosition, 512.0f)) continue;
        }

        // render as sprte 500 - 9081
        if (spell_fx_renderer->RenderAsSprite(object) ||
            ((object->uType < SPRITE_SPELL_FIRE_TORCH_LIGHT || object->uType >= SPRITE_10000) && // Not a spell sprite.
             (object->uType < SPRITE_PROJECTILE_AIR_BOLT || object->uType >= SPRITE_OBJECT_EXPLODE) && // Not a projectile.
             (object->uType < SPRITE_TRAP_FIRE || object->uType > SPRITE_TRAP_BODY))) { // Not a trap.
            SpriteFrame *frame = object->getSpriteFrame();
            if (frame->animationName == "null" || frame->textureName == "null") {
                logger->trace("Trying to draw sprite with null frame");
                continue;
            }

            // sprite angle to camera
            unsigned int angle = TrigLUT.atan2(object->vPosition.x - pCamera3D->vCameraPos.x, object->vPosition.y - pCamera3D->vCameraPos.y);
            int octant = ((TrigLUT.uIntegerPi + (TrigLUT.uIntegerPi >> 3) + object->uFacing - angle) >> 8) & 7;

            pBillboardRenderList[::uNumBillboardsToDraw].hwsprite = frame->sprites[octant];
            // error catching
            if (frame->sprites[octant]->texture->height() == 0 || frame->sprites[octant]->texture->width() == 0) {
                logger->trace("Trying to draw sprite with empty octant texture");
                continue;
            }

            Vec3f posMod = object->vPosition;
            // centre sprite
            if (frame->flags & SPRITE_FRAME_CENTER) {
                posMod.z -= (frame->scale * frame->sprites[octant]->uHeight) / 2;
            }

            BillboardFlags setflags = billboardFlagsForSprite(frame->flags, octant);

            // lighting
            int lightradius = frame->glowRadius * object->field_22_glow_radius_multiplier;

            Color color = pSpriteObjects[i].GetParticleTrailColor();
            if (color.r == 0) color.r = 0xFF;
            if (color.g == 0) color.g = 0xFF;
            if (color.b == 0) color.b = 0xFF;
            if (lightradius) {
                pMobileLightsStack->AddLight(object->vPosition,
                                             object->uSectorID, lightradius, color, _4E94D3_light_type);
            }

            Vec3f viewSpace;
            bool visible = pCamera3D->ViewClip(posMod, &viewSpace);

            if (visible) {
                Vec2f projected = pCamera3D->Project(viewSpace);
                float billb_scale = frame->scale * pCamera3D->ViewPlaneDistPixels / viewSpace.x;
                float billboardWidth = billb_scale * frame->sprites[octant]->uWidth;
                float billboardHeight = billb_scale * frame->sprites[octant]->uHeight;
                Rectf billboardRect(projected.x - billboardWidth / 2, projected.y - billboardHeight, billboardWidth, billboardHeight);

                if (pViewport.intersects(billboardRect)) {
                    object->uAttributes |= SPRITE_VISIBLE;
                    pBillboardRenderList[::uNumBillboardsToDraw].uPaletteId = frame->paletteId;
                    pBillboardRenderList[::uNumBillboardsToDraw].uIndoorSectorID = object->uSectorID;

                    pBillboardRenderList[::uNumBillboardsToDraw].screenspace_projection_factor = { billb_scale, billb_scale };

                    pBillboardRenderList[::uNumBillboardsToDraw].flags = setflags;
                    pBillboardRenderList[::uNumBillboardsToDraw].worldPos = posMod;

                    pBillboardRenderList[::uNumBillboardsToDraw].screenPos = projected;
                    pBillboardRenderList[::uNumBillboardsToDraw].view_space_z = viewSpace.x;
                    pBillboardRenderList[::uNumBillboardsToDraw].view_space_L2 = viewSpace.length();
                    pBillboardRenderList[::uNumBillboardsToDraw].object_pid = Pid(OBJECT_Sprite, i);
                    pBillboardRenderList[::uNumBillboardsToDraw].dimming_level = 0;
                    pBillboardRenderList[::uNumBillboardsToDraw].sTintColor = Color();

                    assert(::uNumBillboardsToDraw < 500);
                    ++::uNumBillboardsToDraw;
                    ++uNumSpritesDrawnThisFrame;
                }
            }
        }
    }
}

// TODO(pskelton): Move to outdoors - clean up
void BaseRenderer::PrepareDecorationsRenderList_ODM() {
    Duration v6;        // edi@9
    int v7;                 // eax@9
    SpriteFrame *frame;     // eax@9
    int v13;                // ecx@9
    Color color;
    Particle_sw local_0;    // [sp+Ch] [bp-98h]@7
    BillboardFlags v38;                // [sp+88h] [bp-1Ch]@9

    for (unsigned int i = 0; i < pLevelDecorations.size(); ++i) {
        if (::uNumBillboardsToDraw >= 500) {
            logger->warning("Billboards Full");
            return;
        }

        // view cull
        if (!IsCylinderInFrustum(pLevelDecorations[i].vPosition, 512.0f)) continue;

        // LevelDecoration *decor = &pLevelDecorations[i];
        if ((!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_OBELISK_CHEST) ||
            pLevelDecorations[i].IsObeliskChestActive()) &&
            !(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            const DecorationDesc *decor_desc = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);
            if (!(decor_desc->uFlags & DECORATION_DESC_EMITS_FIRE)) {
                if (!(decor_desc->uFlags & (DECORATION_DESC_MARKER | DECORATION_DESC_DONT_DRAW))) {
                    v6 = pMiscTimer->time();
                    v7 = std::abs(pLevelDecorations[i].vPosition.x +
                        pLevelDecorations[i].vPosition.y);

                    frame = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                        v6 + Duration::fromTicks(v7));

                    if (config->graphics.SeasonsChange.value()) {
                        frame = LevelDecorationChangeSeason(decor_desc, v6 + Duration::fromTicks(v7), pParty->uCurrentMonth);
                    }

                    if (!frame || frame->textureName == "null" || frame->sprites[0] == NULL) {
                        continue;
                    }

                    // v8 = pSpriteFrameTable->GetFrame(decor_desc->uSpriteID,
                    // v6 + v7);

                    int v10 = TrigLUT.atan2(pLevelDecorations[i].vPosition.x - pCamera3D->vCameraPos.x,
                                            pLevelDecorations[i].vPosition.y - pCamera3D->vCameraPos.y);
                    v13 = ((signed int)(TrigLUT.uIntegerPi +
                        ((signed int)TrigLUT.uIntegerPi >>
                            3) +
                        pLevelDecorations[i]._yawAngle -
                        (int64_t)v10) >>
                        8) &
                        7;
                    int v37 = v13;

                    v38 = billboardFlagsForSprite(frame->flags, v13);

                    // for light
                    if (frame->glowRadius) {
                        color = colorTable.White;
                        if (render->config->graphics.ColoredLights.value()) {
                            color = decor_desc->uColoredLight;
                            // to avoid blank lights
                            if (color == Color()) {
                                color = colorTable.White;
                            }
                        }
                        pStationaryLightsStack->AddLight(pLevelDecorations[i].vPosition +
                            Vec3f(0, 0, decor_desc->uDecorationHeight / 2),
                            frame->glowRadius, color, _4E94D0_light_type);
                    }  // for light

                       // v17 = (pLevelDecorations[i].vPosition.x -
                       // pCamera3D->vCameraPos.x) << 16; v40 =
                       // (pLevelDecorations[i].vPosition.y -
                       // pCamera3D->vCameraPos.y) << 16;

                    Vec3f viewSpace;
                    bool visible = pCamera3D->ViewClip(pLevelDecorations[i].vPosition, &viewSpace);

                    if (visible) {
                        Vec2f projected = pCamera3D->Project(viewSpace);
                        float billb_scale = frame->scale * pCamera3D->ViewPlaneDistPixels / viewSpace.x;
                        float billboardWidth = billb_scale * frame->sprites[(int64_t)v37]->uWidth;
                        float billboardHeight = billb_scale * frame->sprites[(int64_t)v37]->uHeight;
                        Rectf billboardRect(projected.x - billboardWidth / 2, projected.y - billboardHeight, billboardWidth, billboardHeight);

                        if (pViewport.intersects(billboardRect)) {
                            ::uNumBillboardsToDraw++;
                            ++uNumDecorationsDrawnThisFrame;

                            pBillboardRenderList[::uNumBillboardsToDraw - 1].hwsprite = frame->sprites[(int64_t)v37];
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].worldPos = pLevelDecorations[i].vPosition;
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].screenPos = projected;
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].view_space_z = viewSpace.x;
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].view_space_L2 = viewSpace.length();
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].screenspace_projection_factor = { billb_scale, billb_scale };
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].uPaletteId = frame->paletteId;
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].flags = v38 | BILLBOARD_0X200;
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].uIndoorSectorID = 0;
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].object_pid = Pid(OBJECT_Decoration, i);
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].dimming_level = 0;
                            pBillboardRenderList[::uNumBillboardsToDraw - 1].sTintColor = Color();
                        }
                    }
                }
            } else {
                // Emit fire particles.
                memset(&local_0, 0, sizeof(Particle_sw));
                local_0.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
                local_0.uDiffuse = colorTable.OrangeyRed;
                local_0.x = static_cast<float>(pLevelDecorations[i].vPosition.x);
                local_0.y = static_cast<float>(pLevelDecorations[i].vPosition.y);
                local_0.z = static_cast<float>(pLevelDecorations[i].vPosition.z);
                local_0.r = 0.0f;
                local_0.g = 0.0f;
                local_0.b = 0.0f;
                local_0.particle_size = 1.0f;
                local_0.timeToLive = Duration::randomRealtimeSeconds(vrng, 1, 2); // was either 1 or 2 secs, we made it into [1, 2).
                local_0.texture = spell_fx_renderer->effpar01;
                particle_engine->AddParticle(&local_0);
            }
        }
    }
}

void BaseRenderer::TransformBillboards() {
    pODMRenderParams->uNumBillboards = ::uNumBillboardsToDraw;

    for (unsigned int i = 0; i < ::uNumBillboardsToDraw; ++i) {
        RenderBillboard *p = &pBillboardRenderList[i];
        if (p->hwsprite) {
            TransformBillboard(p, i);
        } else {
            logger->trace("Billboard with no sprite!");
        }
    }
}

Color BlendColors(Color a1, Color a2) {
    int alpha = (a1.a * a2.a + 127) / 255;
    int blue = (a1.b * a2.b + 127) / 255;
    int green = (a1.g * a2.g + 127) / 255;
    int red = (a1.r * a2.r + 127) / 255;
    return Color(red, green, blue, alpha);
}

void BaseRenderer::TransformBillboard(const RenderBillboard *pBillboard, int parent) {
    Sprite *pSprite = pBillboard->hwsprite;
    // error catching
    if (pSprite->texture->height() == 0 || pSprite->texture->width() == 0)
        assert(false);

    unsigned int billboard_index = NextBillboardIndex();
    RenderBillboardD3D *billboard = &pBillboardRenderListD3D[billboard_index];

    float scr_proj_x = pBillboard->screenspace_projection_factor.x;
    float scr_proj_y = pBillboard->screenspace_projection_factor.y;

    int dimming_level = pBillboard->dimming_level;
    Color diffuse = ::GetActorTintColor(dimming_level, 0, pBillboard->view_space_z, 0, pBillboard);

    bool opaquetest{ false };
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        opaquetest = pBillboard->sTintColor.a;
    } else {
        opaquetest = dimming_level & 0xFF000000;
    }

    if (config->graphics.Tinting.value() && pBillboard->sTintColor.c32() & 0x00FFFFFF) {
        diffuse = BlendColors(pBillboard->sTintColor, diffuse);
        if (opaquetest)
            diffuse = Color::fromC32(0x007F7F7F & (diffuse.c32() >> 1)); // TODO(captainurist): what's going on here?
    }

    if (opaquetest)
        billboard->opacity = RenderBillboardD3D::Opaque_3;
    else
        billboard->opacity = RenderBillboardD3D::Transparent;

    float point_x = pSprite->uWidth / 2 - pSprite->uAreaX;
    float point_y = pSprite->uHeight - pSprite->uAreaY;
    if (pBillboard->flags & BILLBOARD_MIRRORED) point_x *= -1.f;
    billboard->pQuads[0].diffuse = diffuse;
    billboard->pQuads[0].pos.x = pBillboard->screenPos.x - point_x * scr_proj_x;
    billboard->pQuads[0].pos.y = pBillboard->screenPos.y - point_y * scr_proj_y;
    billboard->pQuads[0].pos.z = pBillboard->view_space_z;
    billboard->pQuads[0].texcoord.x = 0.f;
    billboard->pQuads[0].texcoord.y = 0.f;

    point_x = pSprite->uWidth / 2 - pSprite->uAreaX;
    point_y = -pSprite->uAreaY;
    if (pBillboard->flags & BILLBOARD_MIRRORED) point_x = point_x * -1.f;
    billboard->pQuads[1].diffuse = diffuse;
    billboard->pQuads[1].pos.x = pBillboard->screenPos.x - point_x * scr_proj_x;
    billboard->pQuads[1].pos.y = pBillboard->screenPos.y - point_y * scr_proj_y;
    billboard->pQuads[1].pos.z = pBillboard->view_space_z;
    billboard->pQuads[1].texcoord.x = 0.f;
    billboard->pQuads[1].texcoord.y = 1.f;

    point_x = pSprite->uWidth / 2 + pSprite->uAreaX;
    point_y = -pSprite->uAreaY;
    if (pBillboard->flags & BILLBOARD_MIRRORED) point_x *= -1.f;
    billboard->pQuads[2].diffuse = diffuse;
    billboard->pQuads[2].pos.x = pBillboard->screenPos.x + point_x * scr_proj_x;
    billboard->pQuads[2].pos.y = pBillboard->screenPos.y - point_y * scr_proj_y;
    billboard->pQuads[2].pos.z = pBillboard->view_space_z;
    billboard->pQuads[2].texcoord.x = 1.f;
    billboard->pQuads[2].texcoord.y = 1.f;

    point_x = pSprite->uWidth / 2 + pSprite->uAreaX;
    point_y = pSprite->uHeight - pSprite->uAreaY;
    if (pBillboard->flags & BILLBOARD_MIRRORED) point_x *= -1.f;
    billboard->pQuads[3].diffuse = diffuse;
    billboard->pQuads[3].pos.x = pBillboard->screenPos.x + point_x * scr_proj_x;
    billboard->pQuads[3].pos.y = pBillboard->screenPos.y - point_y * scr_proj_y;
    billboard->pQuads[3].pos.z = pBillboard->view_space_z;
    billboard->pQuads[3].texcoord.x = 1.f;
    billboard->pQuads[3].texcoord.y = 0.f;

    billboard->uNumVertices = 4;

    billboard->texture = pSprite->texture;
    billboard->view_space_z = pBillboard->view_space_z;
    billboard->view_space_L2 = pBillboard->view_space_L2;
    billboard->sParentBillboardID = parent;
    billboard->paletteId = pBillboard->uPaletteId;
}

void BaseRenderer::MakeParticleBillboardAndPush(const Particle& p) {
    unsigned int billboard_index = NextBillboardIndex();
    RenderBillboardD3D *billboard = &pBillboardRenderListD3D[billboard_index];

    billboard->opacity = RenderBillboardD3D::Opaque_1;
    billboard->view_space_z = p.view_space_z;
    billboard->view_space_L2 = p.view_space_L2;
    billboard->sParentBillboardID = -1;
    billboard->texture = p.type & ParticleType_Diffuse ? nullptr : p.texture;
    billboard->paletteId = p.paletteID;
    billboard->uNumVertices = 4;

    float acos = std::cos(p.angle); // TODO(captainurist): taking cos of an INT angle? WTF?
    float asin = std::sin(p.angle);

    {
        float v16 = -12.f;
        float v17 = -12.f;
        billboard->pQuads[0].pos.x = (acos * v16 - asin * v17) * p.screenspace_scale + (float)p.uScreenSpaceX;
        billboard->pQuads[0].pos.y = (acos * v17 + asin * v16 - 12.f) * p.screenspace_scale + (float)p.uScreenSpaceY;
        billboard->pQuads[0].pos.z = p.view_space_z;
        billboard->pQuads[0].diffuse = p.uLightColor_bgr;
        billboard->pQuads[0].texcoord.x = 0.f;
        billboard->pQuads[0].texcoord.y = 0.f;
    }

    {
        float v31 = -12;
        float v32 = 12;
        billboard->pQuads[1].pos.x = (acos * v31 - asin * v32) * p.screenspace_scale + (float)p.uScreenSpaceX;
        billboard->pQuads[1].pos.y = (acos * v32 + asin * v31 - 12.f) * p.screenspace_scale + (float)p.uScreenSpaceY;
        billboard->pQuads[1].pos.z = p.view_space_z;
        billboard->pQuads[1].diffuse = p.uLightColor_bgr;
        billboard->pQuads[1].texcoord.x = 0.0;
        billboard->pQuads[1].texcoord.y = 1.0;
    }

    {
        float v23 = 12;
        float v24 = 12;
        billboard->pQuads[2].pos.x = (acos * v23 - asin * v24) * p.screenspace_scale + (float)p.uScreenSpaceX;
        billboard->pQuads[2].pos.y = (acos * v24 + asin * v23 - 12.f) * p.screenspace_scale + (float)p.uScreenSpaceY;
        billboard->pQuads[2].pos.z = p.view_space_z;
        billboard->pQuads[2].diffuse = p.uLightColor_bgr;
        billboard->pQuads[2].texcoord.x = 1.0;
        billboard->pQuads[2].texcoord.y = 1.0;
    }

    {
        float v39 = 12;
        float v40 = -12;
        billboard->pQuads[3].pos.x = (acos * v39 - asin * v40) * p.screenspace_scale + (float)p.uScreenSpaceX;
        billboard->pQuads[3].pos.y = (acos * v40 + asin * v39 - 12.f) * p.screenspace_scale + (float)p.uScreenSpaceY;
        billboard->pQuads[3].pos.z = p.view_space_z;
        billboard->pQuads[3].diffuse = p.uLightColor_bgr;
        billboard->pQuads[3].texcoord.x = 1.0;
        billboard->pQuads[3].texcoord.y = 0.0;
    }
}

float BaseRenderer::GetGamma() {
    const float base = 0.60f;
    const float mult = 0.1f;
    int level = config->graphics.Gamma.value();
    return base + mult * level;
}

//----- (004A4CC9) ---------------------------------------
void BaseRenderer::BillboardSphereSpellFX(SpellFX_Billboard *a1, Color diffuse) {
    // fireball / implosion sphere
    // TODO(pskelton): could draw in 3d rather than convert to billboard for ogl

    if (a1->uNumVertices < 3) {
        return;
    }

    float depth = 1000000.0;
    int index = 0;
    for (unsigned i = 0; i < (unsigned int)a1->uNumVertices; ++i) {
        if (a1->field_104[i].pos.z < depth) {
            depth = a1->field_104[i].pos.z;
            index = i;
        }
    }
    float view_space_L2 = a1->field_104[index].pos.length();

    unsigned int v5 = NextBillboardIndex();
    pBillboardRenderListD3D[v5].sParentBillboardID = -1;
    pBillboardRenderListD3D[v5].opacity = RenderBillboardD3D::Opaque_2;
    pBillboardRenderListD3D[v5].texture = nullptr;
    pBillboardRenderListD3D[v5].uNumVertices = a1->uNumVertices;
    pBillboardRenderListD3D[v5].paletteId = 0;
    pBillboardRenderListD3D[v5].view_space_z = depth;
    pBillboardRenderListD3D[v5].view_space_L2 = view_space_L2;

    pBillboardRenderListD3D[v5].pQuads[3].pos.x = 0.0f;
    pBillboardRenderListD3D[v5].pQuads[3].pos.y = 0.0f;
    pBillboardRenderListD3D[v5].pQuads[3].pos.z = 0.0f;

    for (unsigned int i = 0; i < (unsigned int)a1->uNumVertices; ++i) {
        pBillboardRenderListD3D[v5].pQuads[i].pos = a1->field_104[i].pos;

        Color v12;
        if (diffuse.a) {
            v12 = a1->field_104[i].diffuse;
        } else {
            v12 = diffuse;
        }
        pBillboardRenderListD3D[v5].pQuads[i].diffuse = v12;

        pBillboardRenderListD3D[v5].pQuads[i].texcoord.x = 0.5;
        pBillboardRenderListD3D[v5].pQuads[i].texcoord.y = 0.5;
    }
}

void BaseRenderer::DrawMonsterPortrait(const Recti &rc, SpriteFrame *Portrait, int Y_Offset) {
    Recti rct;
    rct.x = rc.x + 64 + Portrait->sprites[0]->uAreaX - Portrait->sprites[0]->uWidth / 2;
    rct.y = rc.y + Y_Offset + Portrait->sprites[0]->uAreaY;
    rct.w = Portrait->sprites[0]->uWidth;
    rct.h = Portrait->sprites[0]->uHeight;

    render->SetUIClipRect(rc);
    render->DrawImage(Portrait->sprites[0]->texture, rct, Portrait->paletteId);
    render->ResetUIClipRect();
}

void BaseRenderer::DrawSpecialEffectsQuad(GraphicsImage *texture, int palette) {
    // TODO(captainurist): Old code used BR - TL (one less than actual width/height), preserving that behavior for now.
    Recti targetrect(pViewport.x, pViewport.y, pViewport.w - 1, pViewport.h - 1);
    DrawImage(texture, targetrect, palette, colorTable.MediumGrey);
}

void BaseRenderer::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() {
    engine->draw_debug_outlines();
    render->DoRenderBillboards_D3D();
    spell_fx_renderer->RenderSpecialEffects();
}

// TODO: should this be combined / moved out of render
std::vector<Actor*> BaseRenderer::getActorsInViewport(int pDepth) {
    std::vector<Actor*> foundActors;

    for (int i = 0; i < render->uNumBillboardsToDraw; i++) {
        int renderId = render->pSortedBillboardRenderListD3D[i]->sParentBillboardID;
        if(renderId == -1) {
            continue; // E.g. spell particle.
        }

        Pid pid = pBillboardRenderList[renderId].object_pid;
        if (pid.type() == OBJECT_Actor) {
            if (pBillboardRenderList[renderId].view_space_z <= pDepth) {
                int id = pid.id();
                if (pActors[id].aiState != Dead &&
                    pActors[id].aiState != Dying &&
                    pActors[id].aiState != Removed &&
                    pActors[id].aiState != Disabled &&
                    pActors[id].aiState != Summoned) {
                    if (vis->DoesRayIntersectBillboard(static_cast<float>(pDepth), i)) {
                        // Limit for 100 actors was removed
                        foundActors.push_back(&pActors[id]);
                    }
                }
            }
        }
    }
    return foundActors;
}

bool BaseRenderer::Reinitialize(bool firstInit) {
    updateRenderDimensions();
    return true;
}

Sizei BaseRenderer::GetRenderDimensions() {
    return outputRender;
}

Sizei BaseRenderer::GetPresentDimensions() {
    return outputPresent;
}

void BaseRenderer::updateRenderDimensions() {
    outputPresent = window->size();

    if (config->graphics.RenderFilter.value() != 0)
        outputRender = {config->graphics.RenderWidth.value(), config->graphics.RenderHeight.value()};
    else
        outputRender = outputPresent;

    // Set viewport from config values (inclusive TL/BR coordinates).
    int tlX = config->graphics.ViewPortX1.value();  // 8 in vanilla
    int tlY = config->graphics.ViewPortY1.value();  // 8 in vanilla
    int brX = outputRender.w - config->graphics.ViewPortX2.value();  // 468 in vanilla
    int brY = outputRender.h - config->graphics.ViewPortY2.value();  // 352 in vanilla
    pViewport = Recti(tlX, tlY, brX - tlX + 1, brY - tlY + 1);
}

Pointi BaseRenderer::MapToRender(Pointi position) {
    Sizef renDims = { (float)GetRenderDimensions().w, (float)GetRenderDimensions().h };
    Sizef prDims = { (float)GetPresentDimensions().w, (float)GetPresentDimensions().h };
    Pointi result = position;

    if (renDims != prDims) {
        Sizef ratioCorections = { prDims.w / renDims.w, prDims.h / renDims.h };
        float ratioCorrection = std::min(ratioCorections.w, ratioCorections.h);

        float w = renDims.w * ratioCorrection;
        float h = renDims.h * ratioCorrection;

        result.x = std::round((position.x - (prDims.w / 2 - w / 2)) / ratioCorrection);
        result.y = std::round((position.y - (prDims.h / 2 - h / 2)) / ratioCorrection);
    }

    return result;
}

Pointi BaseRenderer::MapToPresent(Pointi position) {
    Sizef renDims = { (float)GetRenderDimensions().w, (float)GetRenderDimensions().h };
    Sizef prDims = { (float)GetPresentDimensions().w, (float)GetPresentDimensions().h };
    Pointi result = position;

    if (renDims != prDims) {
        Sizef ratioCorections = { prDims.w / renDims.w, prDims.h / renDims.h };
        float ratioCorrection = std::min(ratioCorections.w, ratioCorections.h);

        float w = renDims.w * ratioCorrection;
        float h = renDims.h * ratioCorrection;

        result.x = std::round(position.x * ratioCorrection + (prDims.w / 2 - w / 2));
        result.y = std::round(position.y * ratioCorrection + (prDims.h / 2 - h / 2));
    }

    return result;
}
