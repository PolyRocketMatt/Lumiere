#pragma once

#include <vector>

#include "geometry/Primitives.h"

namespace Lumiere {

class Scene {
	
};

class PrimitiveScene : public Scene {
private:
	std::vector<Primitive>	primitives;
};

}