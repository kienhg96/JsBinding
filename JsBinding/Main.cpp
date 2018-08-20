#include <iostream>
#include <cstdio>
#include "jsapi.h"
#include "js/Initialization.h"

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

static JSClass Hello_class {
	"HelloClass"
};

static JSObject * helloClassProto = nullptr;

bool HelloClass_constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
	JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
	std::cout << "Created Object \n";
	JS::RootedObject proto(cx, helloClassProto);
	JSObject * instance = JS_NewObjectWithGivenProto(cx, &Hello_class, proto);
	
	args.rval().setObject(*instance);
	return true;
}

bool myjs_hello(JSContext *cx, unsigned argc, JS::Value *vp)
{
	// Every JSNative should start with this line. The C++ CallArgs object is
	// how you access the arguments passed from JS and set the return value.
	JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
	
	// Do the work this function is supposed to do. In this case just call the
	std::cout << "Hello from C++\n";

	// Set the return value. Every JSNative must do this before returning true.
	// args.rval() returns a reference to a JS::Value, which has a variety of
	// setXyz() methods.
	//
	// In our case, we want to return a number, so we use setNumber(). It
	// requires either a double or a uint32_t argument. result is neither of
	// those, so we cast it.
	
	args.rval().setString(JS_NewStringCopyZ(cx, "hi and hello"));
	// Return true to indicate success. Later on we'll see that if a JSNative
	// throws an exception or encounters an error, it must return false.
	return true;
}

// An array of all our JSNatives and their names. We'll pass this table to
// JS_DefineFunctions below.
//
// (The third argument to JS_FS is the argument count: rand.length will be 0
// and srand.length will be 1. The fourth argument is a "flags" argument,
// almost always 0.)
static JSFunctionSpec myjs_global_functions[] = {
	JS_FN("hello", myjs_hello, 0, 0),
	JS_FS_END
};

JSFunctionSpec helloClassFunctions[] = {
	JS_FN("show", myjs_hello, 0, 0),
	JS_FS_END
};

void reportWarning(JSContext * cx, JSErrorReport * report) {
	std::cout << "Error: " << report->filename << " ("
		<< report->lineno << ":" << report->column + 1 << ") "
		<< report->message().c_str() << "\n";
}

bool run(JSContext * cx) {
	// Scope for our various stack objects (JSAutoRequest, RootedObject), so they all go
	// out of scope before we JS_DestroyContext.

	JSAutoRequest ar(cx); // In practice, you would want to exit this any
						  // time you're spinning the event loop

	JS::CompartmentOptions options;
	
	JS::RootedObject global(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::FireOnNewGlobalHook, options));
	if (!global)
		return false;
	JS::RootedValue rval(cx);
	{ // Scope for JSAutoCompartment
		JSAutoCompartment ac(cx, global);
		JS_InitStandardClasses(cx, global);
		helloClassProto = JS_InitClass(cx, global, nullptr, &Hello_class, HelloClass_constructor, 0, nullptr, helloClassFunctions, nullptr, nullptr);
		
		// JS_DefineFunctions(cx, global, myjs_global_functions);
		// JS_DefineFunction(cx, global, "hi", myjs_hello, 0, 0);
		// JSObject * globalObject = JS_GetGlobalForObject(cx, global);
		// JS::RootedValue globalVal(cx);
		// globalVal.setObject(*globalObject);
		
		// JS_SetProperty(cx, global, "global", globalVal);

		const char *filename = "script.js";
		char * script;
		FILE * fp = fopen(filename, "rb");
		if (!fp) {
			std::cout << "Cannot open " << filename << "\n";
			return false;
		}

		fseek(fp, 0, SEEK_END);
		size_t fsize = ftell(fp);
		rewind(fp);

		script = (char *)malloc(sizeof(char) * (fsize + 1));
		int readSize = fread(script, 1, fsize, fp);
		script[fsize] = 0;
		fclose(fp);

		int lineno = 1;
		JS::CompileOptions opts(cx);

		opts.setFileAndLine(filename, lineno);
		bool ok = JS::Evaluate(cx, opts, script, strlen(script), &rval);
		free(script);
		if (!ok) {
			if (JS_IsExceptionPending(cx)) {
				JS_GetPendingException(cx, &rval);
				JS::RootedObject ex(cx, &rval.toObject());
				JS::AutoValueVector args(cx);
				JS_CallFunctionName(cx, ex, "toString", args, &rval);
				std::cout << JS_EncodeString(cx, rval.toString()) << "\n";

				JS_ClearPendingException(cx);

				//std::cout << JS_EncodeString(cx, name.toString()) << ": " << JS_EncodeString(cx, message.toString()) << "\n";
			}

			return false;
		}
	}
	if (!rval.isUndefined()) {
		JSString *str = rval.toString();
		printf("result: %s\n", JS_EncodeString(cx, str));
	}
	return true;
}

/*

int main(int argc, const char *argv[])
{
	JS_Init();

	JSContext *cx = JS_NewContext(8L * 1024 * 1024);
	if (!cx)
		return 1;
	if (!JS::InitSelfHostedCode(cx))
		return 1;

	JS::SetWarningReporter(cx, reportWarning);

	if (!run(cx)) {
		std::cout << "Run failed\n";
	}

	JS_DestroyContext(cx);
	JS_ShutDown();
	getchar();
	return 0;
}

*/