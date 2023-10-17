#include "Cli.h"

void CliApp::parse(int argc, const char *const *argv, bool &helpPrinted) {
    // Note: we don't call base_type::ensure_utf8 here, it's the caller's responsibility.
    try {
        base_type::parse(argc, argv);
    } catch (const CLI::CallForHelp &e) {
        base_type::exit(e);
        helpPrinted = true;
    } catch (const CLI::CallForAllHelp &e) {
        base_type::exit(e);
        helpPrinted = true;
    } catch (const CLI::CallForVersion &e) {
        base_type::exit(e);
        helpPrinted = true;
    }
    // Other exception are propagated.
}
