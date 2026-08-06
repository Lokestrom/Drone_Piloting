#pragma once

namespace settingNames {

namespace categories {
inline constexpr char camera[] = "Camera";
inline constexpr char console[] = "Console";
inline constexpr char keyBindings[] = "Key Bindings";
inline constexpr char performance[] = "Performance";
inline constexpr char player[] = "Player";
inline constexpr char rendering[] = "Rendering";
inline constexpr char safety[] = "Safety";
inline constexpr char simulation[] = "Simulation";
}

namespace camera {
inline constexpr char fieldOfView[] = "Field of view";
inline constexpr char keyboardRotationSpeed[] = "Keyboard rotation speed";
inline constexpr char maximumOrbitDistance[] = "Maximum orbit distance";
inline constexpr char minimumOrbitDistance[] = "Minimum orbit distance";
inline constexpr char mouseSensitivity[] = "Mouse sensitivity";
inline constexpr char moveSpeed[] = "Move speed";
inline constexpr char zoomSpeed[] = "Zoom speed";
}

namespace cameraKeys {
inline constexpr char moveForward[] = "Move forward";
inline constexpr char moveBackwards[] = "Move backwards";
inline constexpr char moveLeft[] = "Move left";
inline constexpr char moveRight[] = "Move right";
inline constexpr char moveUp[] = "Move up";
inline constexpr char moveDown[] = "Move down";
inline constexpr char rotateLeft[] = "Rotate left";
inline constexpr char rotateRight[] = "Rotate right";
inline constexpr char rotateUp[] = "Rotate up";
inline constexpr char rotateDown[] = "Rotate down";
inline constexpr char rollLeft[] = "Roll left";
inline constexpr char rollRight[] = "Roll right";
inline constexpr char freeCamera[] = "Free camera";
inline constexpr char orbitCamera[] = "Orbit camera";
}

namespace console {
inline constexpr char errorColor[] = "Error color";
inline constexpr char messageColor[] = "Message color";
inline constexpr char warningColor[] = "Warning color";
}

namespace interfaceKeys {
inline constexpr char subCategory[] = "Interface";
inline constexpr char toggleMenu[] = "Toggle menu";
inline constexpr char toggleOverlay[] = "Toggle overlay";
}

namespace performance {
inline constexpr char mapLoadingThreads[] = "Map loading threads";
}

namespace player {
inline constexpr char swapOnCreation[] = "Swap to player on creation";
}

namespace rendering {
inline constexpr char backgroundColor[] = "Background color";
inline constexpr char dynamicObjectViewDistance[] = "Dynamic object view distance";
inline constexpr char shadowsEnabled[] = "Enable shadows";
inline constexpr char shadowDistance[] = "Shadow distance";
inline constexpr char textureFullResolutionDistance[] = "Texture full resolution distance";
inline constexpr char textureMediumResolutionDistance[] = "Texture medium resolution distance";
inline constexpr char vectorLengthScale[] = "Debug vector length scale";
inline constexpr char vectorWidth[] = "Debug vector width";
}

namespace safety {
inline constexpr char repeatedLoopErrorLimit[] = "Repeated loop error limit";
}

namespace simulation {
inline constexpr char maximumDeltaTime[] = "Maximum delta time";
}

}
