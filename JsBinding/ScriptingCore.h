#ifndef __SCRIPTING_CORE__
#define __SCRIPTING_CORE__
#include "jsapi.h"

static JSClassOps global_ops = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	JS_GlobalObjectTraceHook
};

/* The class of the global object. */
static JSClass global_class = {
	"global",
	JSCLASS_GLOBAL_FLAGS,
	&global_ops
};

class ScriptingCore {
public:
	static ScriptingCore * getInstance();
private:
	void createContext();
	void createGlobal();
private:
	ScriptingCore();
	~ScriptingCore();
private:
	static ScriptingCore * _instance;
	JSContext * _cx;
	JS::PersistentRootedObject _global;
};

#endif // __SCRIPTING_CORE__
