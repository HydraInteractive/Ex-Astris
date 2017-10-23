#include <barcode/menustate.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <barcode/gamestate.hpp>
#include <barcode/editorstate.hpp>
#include <imgui/imgui.h>

namespace Barcode {
	MenuState::MenuState() : _engine(Hydra::IEngine::getInstance()) {}

	void MenuState::load() {
		_textureLoader = Hydra::IO::GLTextureLoader::create();
		_meshLoader = Hydra::IO::GLMeshLoader::create(_engine->getRenderer());

		{
			auto& batch = _viewBatch;
			batch.vertexShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::vertex, "assets/shaders/view.vert");
			batch.fragmentShader = Hydra::Renderer::GLShader::createFromSource(Hydra::Renderer::PipelineStage::fragment, "assets/shaders/null.frag");

			batch.pipeline = Hydra::Renderer::GLPipeline::create();
			batch.pipeline->attachStage(*batch.vertexShader);
			batch.pipeline->attachStage(*batch.fragmentShader);
			batch.pipeline->finalize();

			batch.batch.clearColor = glm::vec4(0.5, 0, 0.5, 1);
			batch.batch.clearFlags = Hydra::Renderer::ClearFlags::color | Hydra::Renderer::ClearFlags::depth;
			batch.batch.renderTarget = _engine->getView();
			batch.batch.pipeline = batch.pipeline.get();
		}

		_initSystem();
		_initWorld();
	}

	MenuState::~MenuState() { }

	void MenuState::onMainMenu() {}

	void MenuState::runFrame(float delta) {
		{ // Update UI & views
			int oneTenthX = _engine->getView()->getSize().x / 10;
			int oneThirdX = _engine->getView()->getSize().x / 3;
			int oneEightY = _engine->getView()->getSize().y / 8;
			int oneThirdY = _engine->getView()->getSize().y / 3;
			int oneHalfY = _engine->getView()->getSize().y / 2;
			int twoFiftsY = (_engine->getView()->getSize().y / 5) * 2;
			int twoThirdY = (_engine->getView()->getSize().y / 3) * 2;


			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, float(0.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(1920, 1200));
			ImGui::Begin("Background", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
			ImGui::Image(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackgroundBig.png")->getID()), ImVec2(1920, 1200));
			ImGui::End();
			{
				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX, oneThirdY));
				ImGui::Begin("Main menu Play", nullptr, windowFlags);
				if (ImGui::Button("Play"))
				{
					menu = 1; subMenu = 0;
				}
				//_engine->setState<GameState>();
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 2, oneThirdY));
				ImGui::Begin("Main menu Create", nullptr, windowFlags);
				if (ImGui::Button("Create"))
				{
					menu = 2; subMenu = 0;
				}
				//_engine->setState<EditorState>();
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 3, oneThirdY));
				ImGui::Begin("Main menu Options", nullptr, windowFlags);
				if (ImGui::Button("Options"))
				{
					menu = 3; subMenu = 0;
				}
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 4, oneThirdY));
				ImGui::Begin("Main menu Quit", nullptr, windowFlags);
				if (ImGui::Button("Quit"))
					_engine->quit();
				ImGui::End();
			}
			
			switch (menu)
			{
					case 0: //emty defult screen
				break;
				{
					case 1: //Play menu
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY * 2), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, oneHalfY - (oneEightY / 1.6)));
						ImGui::Begin("Main menu 3", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 1;
							}
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 1;
							}
						}
						ImGui::End();
					}
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY));
						ImGui::Begin("Main menu 4", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 2;
							}
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 2;
							}
						}
						ImGui::End();
					}	
						switch (subMenu)
						{
							case 0: //emty defult screen
						break;
							case 1:
							{
								ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
								ImGui::SetNextWindowPos(ImVec2(0, oneHalfY));
								ImGui::Begin("Main menu 1", nullptr, windowFlags);
								if (ImGui::IsItemHovered())
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostSelected.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));

								}
								else
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostTransparent.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								ImGui::End();
							}
							{
								ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
								ImGui::SetNextWindowPos(ImVec2(0, oneThirdY * 2));
								ImGui::Begin("Main menu 2", nullptr, windowFlags);
								if (ImGui::IsItemHovered())
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinSelected.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								else
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinTransparent.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								ImGui::End();
							}
						break;
							case 2:
							{
								ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
								ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneHalfY));
								ImGui::Begin("Main menu 5", nullptr, windowFlags);
								if (ImGui::IsItemHovered())
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newSelected.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								else
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newTransparent.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								ImGui::End();
							}
							{
								ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiSetCond_Once);
								ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneThirdY * 2));
								ImGui::Begin("Main menu 6", nullptr, windowFlags);
								if (ImGui::IsItemHovered())
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueSelected.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								else
								{
									if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueTransparent.png")->getID()),
										ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								ImGui::End();
							}
						break;
						}
				break;
				}
				{
					case 2: //Create menu
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY * 2), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, oneHalfY - (oneEightY / 1.6)));
						ImGui::Begin("Main menu 3", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 1;
							}
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 1;
							}
						}
						ImGui::End();
					}
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY));
						ImGui::Begin("Main menu 4", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 2;
							}
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 2;
							}
						}
						ImGui::End();
					}
					switch (subMenu)
					{
					case 0: //emty defult screen
						break;
					case 1:
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(0, oneHalfY));
						ImGui::Begin("Main menu 1", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}


						ImGui::End();
					}
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(0, oneThirdY * 2));
						ImGui::Begin("Main menu 2", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}
						ImGui::End();
					}
					break;
					case 2:
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneHalfY));
						ImGui::Begin("Main menu 5", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}
						ImGui::End();
					}
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneThirdY * 2));
						ImGui::Begin("Main menu 6", nullptr, windowFlags);
						if (ImGui::IsItemHovered())
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}
						else
						{
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueTransparent.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
						}
						ImGui::End();
					}
					break;
					}
				}
				{
			case 3: //Options menu
				ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 8, oneThirdY));
				ImGui::Begin("Main menu 1", nullptr, windowFlags);
				if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackground.png")->getID()),
					ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
				//gamplay
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 8, oneThirdY));
				ImGui::Begin("Main menu 2", nullptr, windowFlags);
				if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackground.png")->getID()),
					ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
				//grafics
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 8, oneThirdY));
				ImGui::Begin("Main menu 3", nullptr, windowFlags);
				if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackground.png")->getID()),
					ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
				//sound
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 8, oneThirdY * 2));
				ImGui::Begin("Main menu 4", nullptr, windowFlags);
				if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackground.png")->getID()),
					ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));

				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 8, oneThirdY));
				ImGui::Begin("Main menu 5", nullptr, windowFlags);
				if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackground.png")->getID()),
					ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));

				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 8, oneThirdY));
				ImGui::Begin("Main menu 6", nullptr, windowFlags);
				if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/menuScreenBackground.png")->getID()),
					ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));

				ImGui::End();
				break;
				}
			}


			_engine->getRenderer()->render(_viewBatch.batch);

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
	}
	void MenuState::_initSystem() {
		const std::vector<Hydra::World::ISystem*> systems = {_engine->getDeadSystem()};
		_engine->getUIRenderer()->registerSystems(systems);
	}

	void MenuState::_initWorld() {
		using world = Hydra::World::World;

		world::newEntity("Menu entity", world::root);
	}
}
