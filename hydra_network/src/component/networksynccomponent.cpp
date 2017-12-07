#include <hydra/component/networksynccomponent.hpp>

#include <imgui/imgui.h>
#include <hydra/engine.hpp>

using namespace Hydra::Component;

NetworkSyncComponent::NetworkSyncComponent() {}

NetworkSyncComponent::~NetworkSyncComponent() {}

void NetworkSyncComponent::serialize(nlohmann::json& json) const {}

void NetworkSyncComponent::deserialize(nlohmann::json& json) {}

void NetworkSyncComponent::registerUI() {}
