#pragma once
#include <hydra/ext/api.hpp>

#include <string>

namespace Hydra::Ext {
	HYDRA_API size_t getPeakRSS();
	HYDRA_API size_t getCurrentRSS();
};
