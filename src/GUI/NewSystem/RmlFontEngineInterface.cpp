#include "RmlFontEngineInterface.h"

#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/RenderManager.h>
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
    auto font = reinterpret_cast<GUIFont *>(face_handle);
    mesh_list.resize(1);
    mesh_list[0].texture = render_manager.LoadTexture(font->fontFile);
    //Ignore effects, opacity and text shaping atm
    for (auto itChar = Rml::StringIteratorU8(str); itChar; ++itChar) {
        //font->DrawText()
    }
    return font->GetLineWidth(str);
}
