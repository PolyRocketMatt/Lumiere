#pragma once

namespace Lumiere {

class Layer {
public:
	Layer() = default;
	virtual ~Layer() = default;

	virtual void OnAttach() {}
	virtual void OnDetach() {}

	virtual void OnUpdate(float ts) {}
	virtual void OnUIRender() {}
};

}