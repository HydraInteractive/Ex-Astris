#pragma once

#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_NETWORK_API NetworkSyncComponent final : public IComponent<NetworkSyncComponent, ComponentBits::NetworkSync> {
		NetworkSyncComponent();
		~NetworkSyncComponent() final;

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
		void registerUI() final;

		const std::string type() const final { return "NetworkSyncComponent"; }
	};
}
