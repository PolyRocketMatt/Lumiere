#pragma once

#include "KeyCodes.h"

#include <glm/glm.hpp>

namespace Lumiere {

class Input {
public:
	static bool IsKeyDown(KeyCode keycode);
	static bool IsMouseButtonDown(MouseButton button);

	static glm::vec2 GetMousePosition();
	static glm::vec2 GetMouseScrollDelta();
	static bool HasScrollDelta();

	static void SetCursorMode(CursorMode mode);
	static void SetScrollDelta(float xOffset, float yOffset);
};

}