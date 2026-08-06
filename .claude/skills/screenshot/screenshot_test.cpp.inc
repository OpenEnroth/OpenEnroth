// Temporary instrumentation for A/B screenshot comparison. Append to test/Bin/GameTest/GameTests_0000.cpp
// in BOTH revisions being compared, together with these includes at the top of the file:
//
//     #include <cstdlib>
//     #include "Library/Image/Png.h"
//     #include "Engine/Graphics/Renderer/Renderer.h"
//     #include "Utility/Streams/FileOutputStream.h"
//
// REMOVE BEFORE COMMIT. Adjust the driven screens to whatever the change under test affects.
GAME_TEST(Screenshots, CharacterScreen) {
    const char *dir = std::getenv("OE_SHOT_DIR");
    ASSERT_NE(dir, nullptr);

    auto screenshot = [&](std::string_view name) {
        game.tick(5);
        game.runGameRoutine([&] {
            Blob data = png::encode(render->MakeFullScreenshot());
            FileOutputStream out(fmt::format("{}/{}.png", dir, name));
            out.write(data.data(), data.size());
            out.close();
        });
    };

    game.startNewGame();
    game.goToInventory(1); // Opens the character window. KEY_C from the game view opens the spellbook, not this.
    screenshot("inventory");
    game.pressAndReleaseKey(PlatformKey::KEY_C); // Tab keys work only once the character window is open.
    screenshot("stats");
    game.pressAndReleaseKey(PlatformKey::KEY_S);
    screenshot("skills");
    game.pressAndReleaseKey(PlatformKey::KEY_A);
    screenshot("awards");
}
