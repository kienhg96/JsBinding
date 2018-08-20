#include "ScriptingCore.h"

ScriptingCore * ScriptingCore::_instance = nullptr;

ScriptingCore::ScriptingCore() {
}

ScriptingCore::~ScriptingCore() {
}

ScriptingCore * ScriptingCore::getInstance() {
	if (!_instance) {
		_instance = new ScriptingCore();
	}
	return _instance;
}

void ScriptingCore::createContext() {
	_cx = JS_NewContext(32L * 1024 * 1024);
}

void ScriptingCore::createGlobal() {
	JS::CompartmentOptions options;
	_global = JS::PersistentRootedObject(_cx, JS_NewGlobalObject(_cx, &global_class, nullptr, JS::DontFireOnNewGlobalHook, options));
	JSAutoCompartment ac(_cx, _global);
	JS_InitStandardClasses(_cx, _global);
	JS_FireOnNewGlobalObject(_cx, _global);
}
