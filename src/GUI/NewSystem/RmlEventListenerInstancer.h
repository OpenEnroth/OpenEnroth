#pragma once

#include <RmlUi/Core/EventListenerInstancer.h>
#include <RmlUi/Core/EventListener.h>

class EventListenerCallback : public Rml::EventListener {
 public:
    explicit EventListenerCallback(std::function<void(Rml::Event &)> callback);
    void ProcessEvent(Rml::Event &event) override;
    void OnAttach(Rml::Element *element) override;
    void OnDetach(Rml::Element *element) override;

 private:
    std::function<void(Rml::Event &)> _callback;
};

class RmlEventListenerInstancer : public Rml::EventListenerInstancer {
 public:
    Rml::EventListener *InstanceEventListener(const Rml::String &value, Rml::Element *element) override;
};
