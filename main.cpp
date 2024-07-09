/**
#include <imgui.h>
#include <entt.hpp>
#include <string>
#include <vector>

#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"

#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#    include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Components
struct Position {
    float x, y;
};
struct Size {
    float width, height;
};
struct Label {
    std::string text;
};
struct Active {
    bool value;
};
struct Parent {
    entt::entity parent;
};

// Widget types
enum class WidgetType { Button, TextInput };
struct WidgetTypeComponent {
    WidgetType type;
};

// Window components
struct ToolbarWindow {};
struct EffectWindow {
    int activeEffect;
};
struct ChatWindow {
    std::vector<std::string> messages;
    std::string inputBuffer;
};

// Rendering system
void renderWidgets(entt::registry& registry, entt::entity parent) {
    auto view =
        registry.view<const WidgetTypeComponent, const Position, const Size, const Label, const Active, const Parent>();

    for (auto entity : view) {
        auto [type, pos, size, label, active, parentComp] =
            view.get<const WidgetTypeComponent, const Position, const Size, const Label, const Active, const Parent>(
                entity);

        if (parentComp.parent != parent)
            continue;

        ImGui::SetCursorPos(ImVec2(pos.x, pos.y));

        switch (type.type) {
            case WidgetType::Button:
                if (ImGui::Button(label.text.c_str(), ImVec2(size.width, size.height))) {
                    registry.patch<Active>(entity, [](auto& a) { a.value = true; });
                } else {
                    registry.patch<Active>(entity, [](auto& a) { a.value = false; });
                }
                break;
            case WidgetType::TextInput:
                ImGui::InputText(label.text.c_str(), &registry.get<ChatWindow>(parent).inputBuffer[0],
                                 registry.get<ChatWindow>(parent).inputBuffer.capacity());
                break;
        }
    }
}

void renderToolbarWindow(entt::registry& registry, entt::entity toolbar) {
    ImGui::Begin("Toolbar");
    renderWidgets(registry, toolbar);
    ImGui::End();
}

void renderEffectWindow(entt::registry& registry, entt::entity effect) {
    ImGui::Begin("Effect Window");
    auto& effectWindow = registry.get<EffectWindow>(effect);
    ImGui::Text("Active Effect: %d", effectWindow.activeEffect);
    ImGui::End();
}

void renderChatWindow(entt::registry& registry, entt::entity chat) {
    ImGui::Begin("Chat Window");
    auto& chatWindow = registry.get<ChatWindow>(chat);

    // Display messages
    ImGui::BeginChild("Messages", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2));
    for (const auto& message : chatWindow.messages) {
        ImGui::TextWrapped("%s", message.c_str());
    }
    ImGui::EndChild();

    // Input and buttons
    renderWidgets(registry, chat);

    ImGui::SameLine();
    if (ImGui::Button("Send")) {
        chatWindow.messages.push_back(chatWindow.inputBuffer);
        chatWindow.inputBuffer.clear();
    }

    ImGui::End();
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
#endif

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    entt::registry registry;

    // Create windows
    auto toolbarWindow = registry.create();
    registry.emplace<ToolbarWindow>(toolbarWindow);

    auto effectWindow = registry.create();
    registry.emplace<EffectWindow>(effectWindow, 0);

    auto chatWindow = registry.create();
    registry.emplace<ChatWindow>(chatWindow, std::vector<std::string>(), std::string(256, '\0'));

    // Create toolbar buttons
    for (int i = 0; i < 3; ++i) {
        auto button = registry.create();
        registry.emplace<WidgetTypeComponent>(button, WidgetType::Button);
        registry.emplace<Position>(button, 10.0f + i * 110.0f, 10.0f);
        registry.emplace<Size>(button, 100.0f, 30.0f);
        registry.emplace<Label>(button, "Effect " + std::to_string(i + 1));
        registry.emplace<Active>(button, false);
        registry.emplace<Parent>(button, toolbarWindow);
    }

    // Create chat input
    auto chatInput = registry.create();
    registry.emplace<WidgetTypeComponent>(chatInput, WidgetType::TextInput);
    registry.emplace<Position>(chatInput, 10.0f, -40.0f);
    registry.emplace<Size>(chatInput, 300.0f, 30.0f);
    registry.emplace<Label>(chatInput, "Input");
    registry.emplace<Active>(chatInput, true);
    registry.emplace<Parent>(chatInput, chatWindow);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderToolbarWindow(registry, toolbarWindow);
        renderEffectWindow(registry, effectWindow);
        renderChatWindow(registry, chatWindow);

        // Update effect based on toolbar buttons
        auto buttonView = registry.view<const WidgetTypeComponent, const Active, const Label, const Parent>();
        for (auto entity : buttonView) {
            auto [type, active, label, parent] =
                buttonView.get<const WidgetTypeComponent, const Active, const Label, const Parent>(entity);
            if (type.type == WidgetType::Button && parent.parent == toolbarWindow && active.value) {
                registry.patch<EffectWindow>(
                    effectWindow, [&label](auto& ew) { ew.activeEffect = std::stoi(label.text.substr(7)) - 1; });
            }
        }

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
*/

int main() {
  return 0;
}


