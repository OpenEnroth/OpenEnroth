#include "RmlFontEngineInterface.h"

#include <RmlUi/Core/MeshUtilities.h>
#include <RmlUi/Core/RenderManager.h>
#include <RmlUi/Core/StringUtilities.h>
#include <Engine/AssetsManager.h>

bool RmlFontEngineInterface::LoadFontFace(const Rml::String &file_name, bool fallback_face, Rml::Style::FontWeight weight) {
    /*
    * Fonts are always loaded by the game in other context
    * so we limit ourselves to return false right now
    * Ideally this will be the right place where fonts for UI are loaded
    */
    return false;
}

Rml::FontFaceHandle RmlFontEngineInterface::GetFontFaceHandle(const Rml::String &family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size) {
    GUIFont *font = assets->getFont(family);
    if (font != nullptr) {
        return reinterpret_cast<Rml::FontFaceHandle>(font);
    }

    return 0;
}

Rml::FontEffectsHandle RmlFontEngineInterface::PrepareFontEffects(Rml::FontFaceHandle handle, const Rml::FontEffectList &font_effects) {
    //We don't support any bitmap font effects for now
    return 0;
}

const Rml::FontMetrics &RmlFontEngineInterface::GetFontMetrics(Rml::FontFaceHandle handle) {
    auto font = reinterpret_cast<GUIFont *>(handle);
    static Rml::FontMetrics metrics;
    metrics.ascent = 0;
    metrics.descent = 0;
    metrics.line_spacing = 0;
    metrics.size = font->GetHeight();
    metrics.underline_position = 0;
    metrics.underline_thickness = 0;
    metrics.x_height = font->GetHeight();
    return metrics;
}

int RmlFontEngineInterface::GetStringWidth(
    Rml::FontFaceHandle handle,
    const Rml::String &str,
    const Rml::TextShapingContext &text_shaping_context,
    Rml::Character prior_character) {
    auto font = reinterpret_cast<GUIFont *>(handle);
    //We ignore the textShapingContext and the previous character kerning atm
    return font->GetLineWidth(str);
}

int RmlFontEngineInterface::GenerateString(
    Rml::RenderManager &render_manager,
    Rml::FontFaceHandle face_handle,
    Rml::FontEffectsHandle font_effects_handle,
    const Rml::String &str,
    const Rml::Vector2f &position,
    Rml::ColourbPremultiplied colour,
    float opacity,
    const Rml::TextShapingContext &text_shaping_context,
    Rml::TexturedMeshList &mesh_list
) {
    //Ignore effects, opacity and text shaping atm

    auto font = reinterpret_cast<GUIFont *>(face_handle);
    //let's generate a single mesh with all the quads representing each character
    mesh_list.resize(1);
    Rml::TexturedMesh& texturedMesh = mesh_list[0];
    texturedMesh.texture = render_manager.LoadTexture(font->fontFile);
    auto &vertices = texturedMesh.mesh.vertices;
    auto &indices = texturedMesh.mesh.indices;

    vertices.reserve(str.size() * 4);
    indices.reserve(str.size() * 6);

    int width = 0;
    Rml::Vector2f finalPosition = position.Round();
    for (auto itChar = Rml::StringIteratorU8(str); itChar; ++itChar) {
        Rml::Character character = *itChar;
        const FontData& fontData = font->getData();
        Rml::Character firstChar = static_cast<Rml::Character>(fontData.header.cFirstChar);
        Rml::Character lastChar = static_cast<Rml::Character>(fontData.header.cLastChar);
        if (character >= firstChar && character <= lastChar) {
            uint8_t mmChar = static_cast<uint8_t>(character);
            const GUICharMetric& charMetric = fontData.header.pMetrics[mmChar];

            finalPosition.x += charMetric.uLeftSpacing;
            width += charMetric.uLeftSpacing;

            int xsq = mmChar % 16;
            int ysq = mmChar / 16;
            float u1 = (xsq * 32.0f) / 512.0f;
            float u2 = (xsq * 32.0f + charMetric.uWidth) / 512.0f;
            float v1 = (ysq * 32.0f) / 512.0f;
            float v2 = (ysq * 32.0f + fontData.header.uFontHeight) / 512.0f;

            Rml::MeshUtilities::GenerateQuad(
                texturedMesh.mesh,
                finalPosition.Round(),
                { (float)charMetric.uWidth, (float)fontData.header.uFontHeight },
                colour,
                {u1, v1},
                {u2, v2}
            );

            finalPosition.x += charMetric.uWidth;
            width += charMetric.uWidth;

            //We should check if this is the final character of the string
            finalPosition.x += charMetric.uRightSpacing;
            width += charMetric.uRightSpacing;
        }
    }
    return width;
}
