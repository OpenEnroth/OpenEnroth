// Hint to Hybrid laptop drivers that we would rather use the NVidia/AMD GPU rather than Integrated Graphics...
// See https://gist.github.com/statico/6809850727c708f08458.
#ifdef _WINDOWS
#   include <windows.h>
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif // _WINDOWS
