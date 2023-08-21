#include "ProxyGamepad.h"

#include "Platform/PlatformGamepad.h"
#include "Platform/Proxy/ProxyBase.h"

ProxyGamepad::ProxyGamepad(PlatformGamepad *base): ProxyBase<PlatformGamepad>(base) {}

std::string ProxyGamepad::model() const {
    return nonNullBase()->model();
}

std::string ProxyGamepad::serial() const {
    return nonNullBase()->serial();
}
