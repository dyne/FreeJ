#ifndef __JSPARSER_H__
#define __JSPARSER_H__

/*
 * Tune this to avoid wasting space for shallow stacks, while saving on
 * malloc overhead/fragmentation for deep or highly-variable stacks.
 */
#define STACK_CHUNK_SIZE    8192

#include <jsapi.h>
#include <layer.h>

JSBool layer_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool add_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool kolos(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

static	JSFunctionSpec shell_functions[] = {
    /*    name          native          nargs    */
    {"add_layer",         add_layer,        1},
    {"kolos",         kolos,        2},
    {0}
};
/**
 * TODO!!! da inserire in un header separato tipo jsparser_data_structures.h
 *
 */
static JSClass layer_class;
static JSClass global_class;
class JsParser {
    public:
	JsParser(Context *_env);
	~JsParser();
	int open(const char* script_file);
	int parse();
    private:
	JSRuntime *js_runtime;
	JSContext *js_context;
	JSObject *global_object;
	JSObject *layer_object;
	void init();
	void init_structs();

	JSPropertySpec layer_properties[3];

	int parse_count;
//	JSFunctionSpec shell_functions[3];
};
#endif
