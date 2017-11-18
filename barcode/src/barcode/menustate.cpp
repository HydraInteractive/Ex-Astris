#include <barcode/menustate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <barcode/gamestate.hpp>
#include <barcode/editorstate.hpp>
#include <imgui/imgui.h>

namespace Barcode {
	bool MenuState::ssaoEnabled = true;
	bool MenuState::glowEnabled = true;
	bool MenuState::shadowEnabled = true;

	MenuState::MenuState() : _engine(Hydra::IEngine::getInstance()) {}

	void MenuState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());

		_viewBatch = RenderBatch<Hydra::Renderer::Batch>("assets/shaders/view.vert", "", "assets/shaders/null.frag", _engine->getView());

		_initSystem();
		_initWorld();
	}

	MenuState::~MenuState() { }

	void MenuState::onMainMenu() {}

	static constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;

	void MenuState::runFrame(float delta) {
		oneTenthX = _engine->getView()->getSize().x / 10;
		oneThirdX = _engine->getView()->getSize().x / 3;
		oneEightY = _engine->getView()->getSize().y / 8;
		oneThirdY = _engine->getView()->getSize().y / 3;
		oneHalfY = _engine->getView()->getSize().y / 2;
		twoFiftsY = (int)(_engine->getView()->getSize().y / 2.5);
		twoThirdY = (int)(_engine->getView()->getSize().y / 1.5);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(1920, 1200)); //TODO: FIX
		ImGui::Begin("Background", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackgroundBig.png")->getID()), ImVec2(1920, 1200));
		ImGui::End();
		{
			ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY/2), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneTenthX - (oneTenthX/2), oneThirdY));
			ImGui::Begin("Main menu Play", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/playSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/playTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
					_menu = Menu::play;
					_submenu = SubMenu::Play::none;
				}
			}
			ImGui::End();

			ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY / 2), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneTenthX * 2 - (oneTenthX / 2), oneThirdY));
			ImGui::Begin("Main menu Create", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
					_menu = Menu::create;
					_submenu = SubMenu::Create::none;
				}
			}
			ImGui::End();

			ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY / 2), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneTenthX * 3 - (oneTenthX / 2), oneThirdY));
			ImGui::Begin("Main menu Options", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/optionsSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/optionsTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
					_menu = Menu::options;
					_submenu = SubMenu::Options::none;
				}
			}
			ImGui::End();

			ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY / 2), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneTenthX * 4 - (oneTenthX / 2), oneThirdY));
			ImGui::Begin("Main menu Quit", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/quitSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/quitTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_engine->quit();
			}
			ImGui::End();
		}

		switch (_menu) {
		case Menu::none:
			break;
		case Menu::play:
			_playMenu();
			break;
		case Menu::create:
			_createMenu();
			break;
		case Menu::options:
			_optionsMenu();
			break;
		};

		_engine->getRenderer()->render(_viewBatch.batch);

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	void MenuState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = {_engine->getDeadSystem()};
		_engine->getUIRenderer()->registerSystems(systems);
	}

	void MenuState::_initWorld() {
		using world = Hydra::World::World;

		world::newEntity("Menu entity", world::root());
	}

	void MenuState::_playMenu() {
		{
			ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY * 2), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneThirdX, oneHalfY - (oneEightY / 1.6)));
			ImGui::Begin("Main menu 3", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_submenu = SubMenu::Play::coop;
			}
			ImGui::End();
		}
		{
			ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY));
			ImGui::Begin("Main menu 4", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_submenu = SubMenu::Play::solo;
			}
			ImGui::End();
		}
		switch (_submenu.play) {
		case SubMenu::Play::none:
			break;
		case SubMenu::Play::coop:
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, oneHalfY));
				ImGui::Begin("Main menu 1", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, oneThirdY * 2));
				ImGui::Begin("Main menu 2", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			break;
		case SubMenu::Play::solo:
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneHalfY));
				ImGui::Begin("Main menu 5", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newTransparent.png")->getID());
					if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
						_engine->setState<Barcode::GameState>();
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneThirdY * 2));
				ImGui::Begin("Main menu 6", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			break;
		}
	}

	void MenuState::_createMenu() {
		{
			ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY * 2), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneThirdX, oneHalfY - (oneEightY / 1.6)));
			ImGui::Begin("Main menu 2 3", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createRoomSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createRoomTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_submenu = SubMenu::Create::createRoom;
			}
			ImGui::End();
		}
		{
			ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY));
			ImGui::Begin("Main menu 2 4", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/viewSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/viewTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_submenu = SubMenu::Create::view;
			}
			ImGui::End();
		}
		switch (_submenu.create) {
		case SubMenu::Create::none:
			break;
		case SubMenu::Create::createRoom:
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, oneHalfY));
				ImGui::Begin("Main menu 2 1", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/nothingSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/nothingTransparent.png")->getID());
					if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
						_engine->setState<Barcode::EditorState>();
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, oneThirdY * 2));
				ImGui::Begin("Main menu 2 2", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/exsistingSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/exsistingTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			break;
		case SubMenu::Create::view:
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneHalfY));
				ImGui::Begin("Main menu 2 5", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneThirdY * 2));
				ImGui::Begin("Main menu 2 6", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			break;
		}
	}
	void MenuState::_optionsMenu() {
		{
			ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY - (oneEightY * 2 )));
			ImGui::Begin("Main menu 3 3", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/gameplaySelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/gameplayTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_submenu = SubMenu::Options::gameplay;
			}
			ImGui::End();
		}
		{
			ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY - oneEightY));
			ImGui::Begin("Main menu 3 4", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/visualSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/visualTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_submenu = SubMenu::Options::visual;
			}
			ImGui::End();
		}
		{
			ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY));
			ImGui::Begin("Main menu 3 41", nullptr, windowFlags);
			{
				auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soundSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soundTransparent.png")->getID());
				if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					_submenu = SubMenu::Options::sound;
			}
			ImGui::End();
		}
		switch (_submenu.options) {
		case SubMenu::Options::none:
				break;
		case SubMenu::Options::visual:
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, twoThirdY - (oneEightY * 2)));
				ImGui::Begin("Main menu 3 1", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/particlesSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/particlesTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, twoThirdY - (oneEightY * 1.2)));
				ImGui::Begin("Main menu 3 2", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/msaaSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/msaaTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, twoThirdY - (oneEightY * 0.4)));
				ImGui::Begin("Main menu 3 21", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/shadowsSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/shadowsTransparent.png")->getID());
					if(ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
						shadowEnabled = !shadowEnabled;
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, twoThirdY + (oneEightY * 0.4)));
				ImGui::Begin("Main menu 3 22", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/ssaoSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/ssaoTransparent.png")->getID());
					if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
						ssaoEnabled = !ssaoEnabled;
				}

				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(0, twoThirdY + (oneEightY * 1.2)));
				ImGui::Begin("Main menu 3 23", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/glowSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/glowTransparent.png")->getID());
					if(ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
						glowEnabled = !glowEnabled;
				}
				ImGui::End();
			}
			break;
		case SubMenu::Options::gameplay:
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, twoThirdY - (oneEightY * 2)));
				ImGui::Begin("Main menu 3 5", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, twoThirdY - (oneEightY * 1)));
				ImGui::Begin("Main menu 3 6", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			break;
		case SubMenu::Options::sound:
			{
				ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, twoThirdY));
				ImGui::Begin("Main menu 3 7", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/volumeSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/volumeTransparent.png")->getID());
					ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
				}
				ImGui::End();
			}
			break;
		}
	}
}
