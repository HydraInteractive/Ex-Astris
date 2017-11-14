#include <hydra/component/pickupcomponent.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

using world = Hydra::World::World;

PickUpComponent::~PickUpComponent() { }

void PickUpComponent::serialize(nlohmann::json& json) const {

}

void PickUpComponent::deserialize(nlohmann::json& json) {

}

void PickUpComponent::registerUI() {

}
