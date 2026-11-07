#include "FontToolOptions.h"

#include <map>
#include <memory>
#include <string>

#include "Library/Cli/CliApp.h"

FontToolOptions FontToolOptions::parse(int argc, char **argv) {
    FontToolOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>(
        "Bitmap font tool. Reads LOD fonts (.fnt) and OpenEnroth fonts (.oef), renders outline fonts with FreeType,\n"
        "and searches font collections for the typeface a bitmap font was rendered from.\n"
        "\n"
        "Font paths can point into archives, as in 'data/icons.lod/arrus.fnt'.\n");

    app->set_help_flag("-h,--help", "Print help and exit.");
    app->require_subcommand();

    std::map<std::string, TextEncoding> encodings = {
        {"ascii", ENCODING_ASCII},
        {"latin1", ENCODING_ISO_8859_1},
        {"cp1251", ENCODING_WINDOWS_1251},
        {"cp1252", ENCODING_WINDOWS_1252},
    };
    auto addEncoding = [&](CLI::App *command) {
        command->add_option("--encoding", result.encoding, "Encoding of LOD font inputs, cp1252 by default. Use cp1251 for Russian fonts.")
            ->transform(CLI::CheckedTransformer(encodings))->option_text("ENC");
    };
    auto addText = [&](CLI::App *command, std::string_view description) {
        command->add_option("--text", result.text, std::string(description))->option_text("STR");
    };

    CLI::App *convert = app->add_subcommand("convert", "Convert a font to .oef, or render its glyphs into a .png sheet.",
                                            result.subcommand, SUBCOMMAND_CONVERT)->fallthrough();
    addEncoding(convert);
    convert->add_option("INPUT", result.input, "Font to convert.")->required()->option_text(" ");
    convert->add_option("OUTPUT", result.output, "Output path, the format is taken from the extension.")->required()->option_text(" ");

    CLI::App *merge = app->add_subcommand("merge", "Merge fonts of the same height, the first font that has a glyph wins.",
                                          result.subcommand, SUBCOMMAND_MERGE)->fallthrough();
    addEncoding(merge);
    merge->add_option("OUTPUT", result.output, "Output path, .oef or .png.")->required()->option_text(" ");
    merge->add_option("INPUTS", result.inputs, "Fonts to merge.")->required()->option_text(" ");

    CLI::App *render = app->add_subcommand("render", "Render an outline or bitmap font into a bitmap font.",
                                           result.subcommand, SUBCOMMAND_RENDER)->fallthrough();
    render->add_option("--face-index", result.faceIndex, "Face to render, for font collections.")->option_text("N");
    render->add_option("--ppem", result.render.ppem, "Pixels per em, or the strike height for a bitmap font.")->required()->option_text("N");
    render->add_flag("--gray", result.gray, "Render anti-aliased and threshold the result, instead of rendering in mono.");
    render->add_option("--threshold", result.render.threshold, "Coverage threshold for --gray, 1-255.")->option_text("N");
    render->add_flag("--bold", result.render.bold, "Embolden the way GDI does, by drawing every glyph again a pixel to the right.");
    render->add_flag("--oblique", result.render.oblique, "Slant the outlines the way FreeType and GDI simulate italics.");
    render->add_option("--baseline", result.render.baseline, "Row of the baseline in the glyph cell.")->required()->option_text("N");
    render->add_option("--height", result.render.height, "Height of the glyph cell.")->required()->option_text("N");
    render->add_option("--shadow", result.shadow, "Shadow to draw: none, drop or outline.")
        ->transform(CLI::CheckedTransformer(std::map<std::string, ShadowMode>{
            {"none", SHADOW_NONE}, {"drop", SHADOW_DROP}, {"outline", SHADOW_OUTLINE}}))->option_text("MODE");
    addText(render, "Characters to render. Defaults to printable ASCII, Latin-1 and Cyrillic.");
    render->add_option("FONT", result.input, "Font file to render.")->required()->option_text(" ");
    render->add_option("OUTPUT", result.output, "Output path, .oef or .png.")->required()->option_text(" ");

    CLI::App *diff = app->add_subcommand("diff", "Draw two fonts side by side, and their letterforms on top of each other.",
                                         result.subcommand, SUBCOMMAND_DIFF)->fallthrough();
    addEncoding(diff);
    addText(diff, "Characters to draw. Defaults to printable ASCII.");
    diff->add_option("--scale", result.scale, "Upscaling factor.")->check(CLI::PositiveNumber)->option_text("N");
    diff->add_option("A", result.input, "First font.")->required()->option_text(" ");
    diff->add_option("B", result.secondInput, "Second font.")->required()->option_text(" ");
    diff->add_option("OUTPUT", result.output, "Output .png path.")->required()->option_text(" ");

    CLI::App *search = app->add_subcommand(
        "search",
        "Find the outline or bitmap fonts that render into the reference fonts. Distances are the number of pixels\n"
        "that differ. A real match scores near zero, and a font that doesn't match scores in the hundreds or more.",
        result.subcommand, SUBCOMMAND_SEARCH)->fallthrough();
    addEncoding(search);
    addText(search, "Characters to match on. Defaults to every printable ASCII and Cyrillic glyph a reference has.");
    search->add_option("--reference", result.references, "Bitmap font to search for, can be repeated.")->required()->option_text("FONT");
    search->add_option("--list", result.listPath, "File listing candidate fonts, one path per line.")->check(CLI::ExistingFile)->option_text("FILE");
    search->add_option("--top", result.top, "Number of matches to print per reference.")->check(CLI::PositiveNumber)->option_text("N");
    search->add_option("--ppem-window", result.ppemWindow, "Sizes to try on either side of the estimated one.")->check(CLI::NonNegativeNumber)->option_text("N");
    search->add_flag("--exhaustive", result.exhaustive, "Try every size, baseline and offset instead of estimating them. Slow, for checking the estimates.");
    search->add_flag("--oblique", result.oblique, "Also try slanted renders, the way FreeType and GDI simulate italics. Doubles the search time.");
    search->add_option("--threads", result.threads, "Number of threads, one per core by default.")->check(CLI::PositiveNumber)->option_text("N");
    search->add_option("FONTS", result.inputs, "Candidate font files, and directories to search recursively.")->option_text(" ");

    CLI::App *generate = app->add_subcommand("generate", "Generate the game's .oef fonts by merging the English and Russian MM7 fonts.",
                                             result.subcommand, SUBCOMMAND_GENERATE)->fallthrough();
    generate->add_option("EN_ICONS_LOD", result.enIconsLodPath, "Path to icons.lod from an English localization.")->required()->option_text(" ");
    generate->add_option("RU_ICONS_LOD", result.ruIconsLodPath, "Path to icons.lod from a Russian localization.")->required()->option_text(" ");
    generate->add_option("OUTPUT", result.output, "Directory to write the fonts into.")->required()->option_text(" ");

    app->parse(argc, argv, result.helpPrinted);
    result.render.mode = result.gray ? RASTER_GRAY : RASTER_MONO;
    return result;
}
