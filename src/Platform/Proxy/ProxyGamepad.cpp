#include "ProxyGamepad.h"

ProxyGamepad::ProxyGamepad(PlatformGamepad *base): ProxyBase<PlatformGamepad>(base) {}

std::string ProxyGamepad::Model() const {
    return NonNullBase()->Model();
}

std::string ProxyGamepad::Serial() const {
    return NonNullBase()->Serial();
}

uint32_t ProxyGamepad::Id() const {
    return NonNullBase()->Id();
}
