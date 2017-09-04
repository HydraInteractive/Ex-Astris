#include <hydra/engine.hpp>

 Hydra::IEngine* Hydra::IEngine::_instance;

 Hydra::IEngine*& Hydra::IEngine::getInstance() { return _instance; }