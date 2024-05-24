#include "sandbox/systems/game/guisystem.hpp"

namespace rythe::game
{
	gfx::framebuffer* GUISystem::mainFBO;
	gfx::framebuffer* GUISystem::pickingFBO;
	ImGuizmo::OPERATION GUISystem::currentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE GUISystem::currentGizmoMode = ImGuizmo::WORLD;

	void GUISystem::setup()
	{
		log::info("Initializing GUI system");
		gfx::gui_stage::addGuiRender<GUISystem, &GUISystem::guiRender>(this);

		bindEvent<key_input<inputmap::method::MOUSE_LEFT>, &GUISystem::doClick>();

		glfwSetFramebufferSizeCallback(gfx::WindowProvider::activeWindow->getGlfwWindow(), GUISystem::framebuffer_size_callback);
	}

	void GUISystem::update()
	{

	}

	void GUISystem::guiRender(core::transform camTransf, gfx::camera camera)
	{
		pickingFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("PickingBuffer");
		mainFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("MainBuffer");
		ImGuiIO& io = ImGui::GetIO();
		m_isHoveringWindow = io.WantCaptureMouse;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


		ImGui::Begin("Editor", 0, window_flags);

		ImGui::Text(std::format("FPS:{}", ImGui::GetIO().Framerate).c_str());

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		//ImGui::ShowDemoWindow();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("Entity"))
				{
					createEmptyEntity();
				}
				ImGui::MenuItem("Material");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::Begin("Heirarchy"))
		{
			ImGui::Separator();
			ImGui::Indent();
			drawHeirarchy(m_filter.m_entities);
			ImGui::Unindent();

			
			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("New Empty"))
					createEmptyEntity();

				if (ImGui::MenuItem("New Cube"))
					createCubeEntity();

				if (ImGui::MenuItem("New Sphere"))
					createSphereEntity();

				ImGui::EndPopup();
			}

			ImGui::End();
		}

		if (ImGui::Begin("Inspector"))
		{
			if (GUI::selected == invalid_id)
			{
				ImGui::End();
			}
			else
			{
				auto ent = GUI::selected;
				ImGui::BeginChild(ent->name.c_str(), ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() * .03f), true);
				ImGui::Checkbox("", &ent->enabled);
				ImGui::SameLine();
				ImGui::Text(ent->name.c_str());
				ImGui::EndChild();


				ImGui::Indent();
				if (ent.hasComponent<core::transform>())
					componentEditor<core::transform>(ent);

				if (ent.hasComponent<gfx::mesh_renderer>())
					componentEditor<gfx::mesh_renderer>(ent);

				if (ent.hasComponent<gfx::light>())
					lightEditor(ent);
					//componentEditor<gfx::light>(ent);

				if (ent.hasComponent<examplecomp>())
					componentEditor<examplecomp>(ent);
				ImGui::Unindent();


				if (ImGui::BeginPopupContextItem("ComponentList"))
				{
					static const char* current = "None";
					for (auto& [id, comp] : ecs::Registry::componentFamilies)
					{
						if (ent.hasComponent(id)) continue;
						const bool is_selected = (ecs::Registry::componentNames[id] == current);
						if (ImGui::Selectable(ecs::Registry::componentNames[id].c_str(), is_selected))
						{
							if (!ent.hasComponent(id))
							{
								current = ecs::Registry::componentNames[id].c_str();
								ent.addComponent(id);
							}
						}
					}
					ImGui::EndPopup();
				}

				if (ImGui::Button("Add Component"))
				{
					ImGui::OpenPopup("ComponentList");
				}
				ImGui::End();
			}
		}

		if (ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoBackground))
		{
#ifdef RenderingAPI_OGL
			auto mainTex = mainFBO->getAttachment(gfx::AttachmentSlot::COLOR0).m_data->getId();
#else
			auto mainTex = mainFBO->getAttachment(gfx::AttachmentSlot::COLOR0).m_data->getInternalHandle();
#endif
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			const float width = viewportPanelSize.x;
			const float height = viewportPanelSize.y;
			math::vec2 windowPos = math::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			//gfx::Renderer::RI->setViewport(1, 0, 0, width, height);
			mainFBO->rescale(width, height);
			gfx::WindowProvider::activeWindow->checkError();
			pickingFBO->rescale(width, height);
			gfx::WindowProvider::activeWindow->checkError();

			if (!Input::mouseCaptured && m_readPixel && ImGui::IsWindowHovered() && !ImGuizmo::IsOver())
			{
				auto color = gfx::Renderer::RI->readPixels(*pickingFBO, math::ivec2(input::Input::mousePos.x, input::Input::mousePos.y) - windowPos, math::ivec2(1, 1));
				rsl::id_type id = color.x + (color.y * 256) + (color.z * 256 * 256) + (color.w * 256 * 256 * 256);
				if (id != invalid_id)
					GUI::selected = ecs::entity{ &ecs::Registry::entities[id] };
				else
					GUI::selected = ecs::entity();

				m_readPixel = false;
			}
			else
			{
				m_readPixel = false;
			}

			/*		if (mainTex != nullptr)*/
			ImGui::Image(reinterpret_cast<ImTextureID>(mainTex), ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
			if (GUI::selected != invalid_id)
				drawGizmo(camTransf, camera, math::ivec2(width, height));

			ImGui::End();
		}

		ImGui::End();
		ImGui::PopStyleVar(3);
	}

	void GUISystem::drawHeirarchy(ecs::entity_set heirarchy)
	{
		for (auto ent : heirarchy)
		{
			if (ent.hasComponent<gfx::camera>()) continue;

			rsl::uint flags = ImGuiTreeNodeFlags_OpenOnArrow;

			if (ent == GUI::selected)
				flags |= ImGuiTreeNodeFlags_Selected;

			if (ent->children.size() == 0)
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			if (!ent->enabled)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));

			ImGui::PushID(std::format("Hierarchy##{}", ent->id).c_str());
			ImGui::TreeNodeEx(ent->name.c_str(), flags);
			if (!ent->enabled)
				ImGui::PopStyleColor();

			ImGui::PopID();

			if (ImGui::IsItemClicked())
			{
				GUI::selected = ent;
			}
		}
	}

	void GUISystem::lightEditor(core::ecs::entity ent)
	{
		ImGui::PushID(std::format("EntityLightEditor##{}", ent->id).c_str());
		auto& comp = ent.getComponent<gfx::light>();
		ImGui::Checkbox("", &comp.enabled);
		ImGui::SameLine();
		if (comp.type == gfx::LightType::DIRECTIONAL)
		{
			bool open = ImGui::TreeNodeEx("Directional Light", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			if (!ent->enabled)
				pushDisabledInspector();

			if (open)
			{
				ImGui::ColorEdit4("Light Color", comp.dir_data.color.data);
				ImGui::InputFloat("Intensity", &comp.dir_data.intensity);
			}

			if (!ent->enabled)
				popDisabledInspector();
		}
		else if (comp.type == gfx::LightType::POINT)
		{
			bool open = ImGui::TreeNodeEx("Point Light", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			if (!ent->enabled)
				pushDisabledInspector();

			if (open)
			{
				ImGui::ColorEdit4("Light Color", comp.point_data.color.data);
				ImGui::InputFloat("Intensity", &comp.point_data.intensity);
				ImGui::InputFloat("Range", &comp.point_data.range);
			}

			if (!ent->enabled)
				popDisabledInspector();
		}
		ImGui::PopID();
	}
	//void GUISystem::exampleCompEditor(core::ecs::entity ent)
	//{
	//	ImGui::PushID(std::format("EntityExampleComp##{}", ent->id).c_str());
	//	auto& comp = ent.getComponent<core::examplecomp>();
	//	ImGui::Checkbox("", &comp.enabled);
	//	ImGui::SameLine();
	//	bool open = ImGui::TreeNodeEx("Example Component", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	//	if (!ent->enabled)
	//		pushDisabledInspector();

	//	if (open)
	//	{
	//		ImGui::InputFloat("Range", &comp.range);
	//		ImGui::InputFloat("Speed", &comp.speed);
	//		ImGui::InputFloat("Angular Speed", &comp.angularSpeed);
	//		ImGui::InputFloat3("Direction", comp.direction.data);
	//	}

	//	if (!ent->enabled)
	//		popDisabledInspector();

	//	ImGui::PopID();
	//}
	//void GUISystem::meshrendererEditor(core::ecs::entity ent)
	//{
	//	auto& renderer = ent.getComponent<gfx::mesh_renderer>();
	//	ImGui::PushID(std::format("EntityMeshRenderer##{}", ent->id).c_str());
	//	ImGui::Checkbox("", &renderer.enabled);
	//	ImGui::SameLine();
	//	bool open = ImGui::TreeNodeEx("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	//	if (!ent->enabled)
	//		pushDisabledInspector();

	//	if (open)
	//	{
	//		createAssetDropDown("Mesh", renderer.model, gfx::ModelCache::getModels(), &GUISystem::setModel);
	//		createAssetDropDown("MainMaterial", renderer.mainMaterial, gfx::MaterialCache::getMaterials(), &GUISystem::setMaterial);
	//		if (ImGui::Checkbox("Casts Shadows?", &renderer.castShadows))
	//		{
	//			renderer.dirty = true;
	//		}
	//	}

	//	if (!ent->enabled)
	//		popDisabledInspector();

	//	ImGui::PopID();
	//}
	//void GUISystem::transformEditor(core::ecs::entity ent)
	//{
	//	auto& transf = ent.getComponent<core::transform>();
	//	ImGui::PushID(std::format("Entity##{}", ent->id).c_str());
	//	bool open = ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	//	if (!ent->enabled)
	//		pushDisabledInspector();

	//	if (open)
	//	{
	//		math::vec3 rot = math::toEuler(transf.rotation);
	//		ImGui::InputFloat3("Position##1", transf.position.data);
	//		if (ImGui::InputFloat3("Rotation##2", rot.data))
	//			transf.rotation = math::toQuat(rot);
	//		ImGui::InputFloat3("Scale##3", transf.scale.data);
	//	}

	//	if (!ent->enabled)
	//		popDisabledInspector();

	//	ImGui::PopID();
	//}
	void GUISystem::setModel(ast::asset_handle<gfx::model> handle)
	{
		auto& renderer = GUI::selected.getComponent<gfx::mesh_renderer>();
		if (handle != &renderer.model)
		{
			renderer.model = handle;
			renderer.dirty = true;
		}
	}
	void GUISystem::setMaterial(ast::asset_handle<gfx::material> handle)
	{
		auto& renderer = GUI::selected.getComponent<gfx::mesh_renderer>();
		if (handle != &renderer.mainMaterial)
		{
			renderer.mainMaterial = handle;
			renderer.dirty = true;
		}
	}

	void GUISystem::createEmptyEntity()
	{
		auto ent = createEntity();
		ent.addComponent<core::transform>();
	}
	void GUISystem::createCubeEntity()
	{
		auto ent = createEntity();
		ent.addComponent<core::transform>();
		ent.addComponent<gfx::mesh_renderer>({ .mainMaterial = gfx::MaterialCache::getMaterial("cube-material"), .model = gfx::ModelCache::getModel("cube"), .castShadows = false });
	}
	void GUISystem::createSphereEntity()
	{
		auto ent = createEntity();
		ent.addComponent<core::transform>();
		ent.addComponent<gfx::mesh_renderer>({ .mainMaterial = gfx::MaterialCache::getMaterial("sphere-material"), .model = gfx::ModelCache::getModel("sphere"), .castShadows = false });
	}

	void GUISystem::drawGizmo(core::transform camTransf, gfx::camera camera, math::ivec2 dims)
	{
		if (GUI::selected == invalid_id || !GUI::selected.hasComponent<core::transform>()) return;

		auto ent = GUI::selected;
		if (!Input::mouseCaptured)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_1))//1 key
				currentGizmoOperation = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_2))//2 key
				currentGizmoOperation = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_3))//3 key
				currentGizmoOperation = ImGuizmo::SCALE;
		}

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, dims.x, dims.y);

		core::transform& transf = ent.getComponent<core::transform>();
		float* matrix = transf.to_world().data;
		if (ImGuizmo::Manipulate(camera.view.data, camera.projection.data, currentGizmoOperation, currentGizmoMode, matrix))
		{
			math::vec3 rot;
			ImGuizmo::DecomposeMatrixToComponents(matrix, transf.position.data, rot.data, transf.scale.data);
			transf.rotation = math::toQuat(rot);
		}
	}
	void GUISystem::doClick(key_input<inputmap::method::MOUSE_LEFT>& action)
	{
		if (Input::mouseCaptured) return;

		if (action.isPressed() && !Input::isPressed)
		{
			Input::isPressed = true;
			m_readPixel = true;
			return;
		}

		if (action.wasPressed())
		{
			Input::isPressed = false;
			return;
		}
	}
	void GUISystem::framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		gfx::Renderer::RI->setViewport(1, 0, 0, width, height);
		mainFBO->rescale(width, height);
	}
	void GUISystem::pushDisabledInspector()
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.5, 0.5, 0.5, 1.0));
	}
	void GUISystem::popDisabledInspector()
	{
		ImGui::PopStyleColor(2);
	}
}