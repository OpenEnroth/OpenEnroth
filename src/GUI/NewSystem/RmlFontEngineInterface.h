#pragma once

#include <RmlUi/Core/FontEngineInterface.h>

class RmlFontEngineInterface : public Rml::FontEngineInterface {
 public:
    /// Called by RmlUi when it wants to load a font face from file.
    /// @param[in] file_name The file to load the face from.
    /// @param[in] fallback_face True to use this font face for unknown characters in other font faces.
    /// @param[in] weight The weight to load when the font face contains multiple weights, otherwise the weight to register the font as.
    /// @return True if the face was loaded successfully, false otherwise.
    bool LoadFontFace(const Rml::String &file_name, bool fallback_face, Rml::Style::FontWeight weight) override;

    /// Called by RmlUi when it wants to load a font face from memory, registered using the provided family, style, and weight.
    /// @param[in] data The font data.
    /// @param[in] family The family to register the font as.
    /// @param[in] style The style to register the font as.
    /// @param[in] weight The weight to load when the font face contains multiple weights, otherwise the weight to register the font as.
    /// @param[in] fallback_face True to use this font face for unknown characters in other font faces.
    /// @return True if the face was loaded successfully, false otherwise.
    /// @note The debugger plugin will load its embedded font faces through this method using the family name 'rmlui-debugger-font'.
    bool LoadFontFace(Rml::Span<const Rml::byte> data, const Rml::String &fontFamily, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallbackFace) override;

    // Called by RmlUi when a font configuration is resolved for an element. Should return a handle that
    // can later be used to resolve properties of the face, and generate string geometry to be rendered.
    // @param[in] family The family of the desired font handle.
    // @param[in] style The style of the desired font handle.
    // @param[in] weight The weight to load when the font face contains multiple weights, otherwise the weight to register the font as.
    // @param[in] size The size of desired handle, in points.
    // @return A valid handle if a matching (or closely matching) font face was found, NULL otherwise.
    Rml::FontFaceHandle GetFontFaceHandle(const Rml::String &family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size) override;

    // Called by RmlUi when a list of font effects is resolved for an element with a given font face.
    // @param[in] handle The font handle.
    // @param[in] font_effects The list of font effects to generate the configuration for.
    // @return A handle to the prepared font effects which will be used when generating geometry for a string.
    Rml::FontEffectsHandle PrepareFontEffects(Rml::FontFaceHandle handle, const Rml::FontEffectList &font_effects) override;

    /// Should return the font metrics of the given font face.
    /// @param[in] handle The font handle.
    /// @return The face's metrics.
    const Rml::FontMetrics &GetFontMetrics(Rml::FontFaceHandle handle) override;

    /// Called by RmlUi when it wants to retrieve the width of a string when rendered with this handle.
    /// @param[in] handle The font handle.
    /// @param[in] string The string to measure.
    /// @param[in] text_shaping_context Additional parameters that provide context for text shaping.
    /// @param[in] prior_character The optionally-specified character that immediately precedes the string. This may have an impact on the string
    /// width due to kerning.
    /// @return The width, in pixels, this string will occupy if rendered with this handle.
    int GetStringWidth(
        Rml::FontFaceHandle handle,
        const Rml::String &str,
        const Rml::TextShapingContext &text_shaping_context,
        Rml::Character prior_character = Rml::Character::Null
    ) override;

    /// Called by RmlUi when it wants to retrieve the meshes required to render a single line of text.
    /// @param[in] render_manager The render manager responsible for rendering the string.
    /// @param[in] face_handle The font handle.
    /// @param[in] font_effects_handle The handle to the prepared font effects for which the geometry should be generated.
    /// @param[in] string The string to render.
    /// @param[in] position The position of the baseline of the first character to render.
    /// @param[in] colour The colour to render the text.
    /// @param[in] opacity The opacity of the text, should be applied to font effects.
    /// @param[in] text_shaping_context Additional parameters that provide context for text shaping.
    /// @param[out] mesh_list A list to place the meshes and textures representing the string to be rendered.
    /// @return The width, in pixels, of the string mesh.
    int GenerateString(
        Rml::RenderManager &render_manager,
        Rml::FontFaceHandle face_handle,
        Rml::FontEffectsHandle font_effects_handle,
        const Rml::String &str,
        const Rml::Vector2f &position,
        Rml::ColourbPremultiplied colour,
        float opacity,
        const Rml::TextShapingContext &text_shaping_context,
        Rml::TexturedMeshList &mesh_list
    ) override;
};
