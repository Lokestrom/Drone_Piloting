#include "ImGui/imgui.h"

namespace settings {
static inline ImGuiKey moveForward = ImGuiKey::ImGuiKey_W;
static inline ImGuiKey moveBackwards = ImGuiKey::ImGuiKey_S;
static inline ImGuiKey moveLeft = ImGuiKey::ImGuiKey_A;
static inline ImGuiKey moveRight = ImGuiKey::ImGuiKey_D;
static inline ImGuiKey moveUp = ImGuiKey::ImGuiKey_Space;
static inline ImGuiKey moveDown = ImGuiKey::ImGuiKey_LeftShift;
static inline ImGuiKey rotateLeft = ImGuiKey::ImGuiKey_LeftArrow;
static inline ImGuiKey rotateRight = ImGuiKey::ImGuiKey_RightArrow;
static inline ImGuiKey rotateUp = ImGuiKey::ImGuiKey_UpArrow;
static inline ImGuiKey rotateDown = ImGuiKey::ImGuiKey_DownArrow;
static inline ImGuiKey rollLeft = ImGuiKey::ImGuiKey_Q;
static inline ImGuiKey rollRight = ImGuiKey::ImGuiKey_E;

static inline double mouseSensitivity = 20.0;
}