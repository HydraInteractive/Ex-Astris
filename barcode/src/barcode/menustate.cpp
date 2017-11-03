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
				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY/2), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX - (oneTenthX/2), oneThirdY));
				ImGui::Begin("Main menu Play", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/playSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/playTransparent.png")->getID());
					if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
						menu = 1;
						subMenu = 0;
					}
				}
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY / 2), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 2 - (oneTenthX / 2), oneThirdY));
				ImGui::Begin("Main menu Create", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createTransparent.png")->getID());
					if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
						menu = 2;
						subMenu = 0;
					}
				}
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY / 2), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 3 - (oneTenthX / 2), oneThirdY));
				ImGui::Begin("Main menu Options", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/optionsSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/optionsTransparent.png")->getID());
					if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					{
						menu = 3;
						subMenu = 0;
					}
				}
				ImGui::End();

				ImGui::SetNextWindowSize(ImVec2(oneTenthX, oneEightY / 2), ImGuiSetCond_Once);
				ImGui::SetNextWindowPos(ImVec2(oneTenthX * 4 - (oneTenthX / 2), oneThirdY));
				ImGui::Begin("Main menu Quit", nullptr, windowFlags);
				{
					auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/quitSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/quitTransparent.png")->getID());
					if (ImGui::ImageButton(image, ImVec2(oneTenthX, oneEightY / 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
					{
						_engine->quit();
					}
				}
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
						{
							auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/coopTransparent.png")->getID());
							if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
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
						{
							auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soloTransparent.png")->getID());
							if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
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
								{
									auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/hostTransparent.png")->getID());
									if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								ImGui::End();
							}
							{
								ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
								ImGui::SetNextWindowPos(ImVec2(0, oneThirdY * 2));
								ImGui::Begin("Main menu 2", nullptr, windowFlags);
								{
									auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/joinTransparent.png")->getID());
									if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
								}
								ImGui::End();
							}
						break;
							case 2:
							{
								ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
								ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneHalfY));
								ImGui::Begin("Main menu 5", nullptr, windowFlags);
								{
									auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/newTransparent.png")->getID());
									if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
									{
										_engine->setState<Barcode::GameState>();
									}
								}
								ImGui::End();
							}
							{
								ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiSetCond_Once);
								ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneThirdY * 2));
								ImGui::Begin("Main menu 6", nullptr, windowFlags);
								{
									auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/continueTransparent.png")->getID());
									if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
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
						ImGui::Begin("Main menu 2 3", nullptr, windowFlags);
						{
							auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createRoomSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/createRoomTransparent.png")->getID());
							if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY * 2), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 1;
							}
						}
						ImGui::End();
					}
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY));
						ImGui::Begin("Main menu 2 4", nullptr, windowFlags);
						{
							auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/viewSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/viewTransparent.png")->getID());
							if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
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
							ImGui::Begin("Main menu 2 1", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/nothingSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/nothingTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
								{
									_engine->setState<Barcode::EditorState>();
								}
							}

							ImGui::End();
						}
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(0, oneThirdY * 2));
							ImGui::Begin("Main menu 2 2", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/exsistingSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/exsistingTransparent.png")->getID());
								if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/exsistingSelected.png")->getID()),
									ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						break;
						case 2:
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneHalfY));
							ImGui::Begin("Main menu 2 5", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, oneThirdY * 2));
							ImGui::Begin("Main menu 2 6", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						break;
						}
					
				break;
				}
				{
					case 3: //Create menu
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY - (oneEightY * 2 )));
						ImGui::Begin("Main menu 3 3", nullptr, windowFlags);
						{
							auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/gameplaySelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/gameplayTransparent.png")->getID());
							if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 2;
							}
						}
						ImGui::End();
					}
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY - oneEightY));
						ImGui::Begin("Main menu 3 4", nullptr, windowFlags);
						{
							auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/visualSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/visualTransparent.png")->getID());
							if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/visualSelected.png")->getID()),
								ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 1;
							}
						
						ImGui::End();
					}
					{
						ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
						ImGui::SetNextWindowPos(ImVec2(oneThirdX, twoThirdY));
						ImGui::Begin("Main menu 3 41", nullptr, windowFlags);
						{
							auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soundSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/soundTransparent.png")->getID());
							if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
							{
								subMenu = 3;
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
							ImGui::SetNextWindowPos(ImVec2(0, twoThirdY - (oneEightY * 2)));
							ImGui::Begin("Main menu 3 1", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/particlesSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/particlesTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(0, twoThirdY - (oneEightY * 1.2)));
							ImGui::Begin("Main menu 3 2", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/msaaSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/msaaTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(0, twoThirdY - (oneEightY * 0.4)));
							ImGui::Begin("Main menu 3 21", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/shadowsSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/shadowsTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(0, twoThirdY + (oneEightY * 0.4)));
							ImGui::Begin("Main menu 3 22", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/ssaoSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/ssaoTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(0, twoThirdY + (oneEightY * 1.2)));
							ImGui::Begin("Main menu 3 23", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/glowSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/glowTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						break;
						case 2:
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, twoThirdY - (oneEightY * 2)));
							ImGui::Begin("Main menu 3 5", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/defaultTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, 400), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, twoThirdY - (oneEightY * 1)));
							ImGui::Begin("Main menu 3 6", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/customTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						break;
						case 3:
						{
							ImGui::SetNextWindowSize(ImVec2(oneThirdX, oneEightY), ImGuiSetCond_Once);
							ImGui::SetNextWindowPos(ImVec2(oneThirdX * 2, twoThirdY));
							ImGui::Begin("Main menu 3 7", nullptr, windowFlags);
							{
								auto image = ImGui::IsItemHovered() ? reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/volumeSelected.png")->getID()) : reinterpret_cast<ImTextureID>(_textureLoader->getTexture("assets/ui/volumeTransparent.png")->getID());
								if (ImGui::ImageButton(image, ImVec2(oneThirdX, oneEightY), ImVec2(0, 0), ImVec2(1, 1), 0, ImColor(0, 0, 0, 0), ImVec4(1, 1, 1, 1)));
							}
							ImGui::End();
						}
						break;
						}
				
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

		world::newEntity("Menu entity", world::root());
	}
}
