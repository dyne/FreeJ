/*  FreeJ
 *
 *  Copyright (C) 2004
 *  Silvano Galliani aka kysucix <kysucix@dyne.org>
 *  Denis Rojo aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */

#ifndef __JSPARSER_H__
#define __JSPARSER_H__

#include <context.h>
#include <config.h>
#ifdef WITH_JAVASCRIPT

/*
 * Tune this to avoid wasting space for shallow stacks, while saving on
 * malloc overhead/fragmentation for deep or highly-variable stacks. */
#define STACK_CHUNK_SIZE    8192

#include <linklist.h>
#include <jsapi.h> // spidermonkey header

extern Context *global_environment;

void js_debug_property(JSContext *cx, jsval val);
void js_debug_argument(JSContext *cx, jsval val);

// This class represents the execution context for a single script,
// holding its context, runtime and global object.
class JsExecutionContext : public Entry {
    friend class JsParser;
public:
    JsExecutionContext(JsParser *jsParser);
    ~JsExecutionContext();
private:
    void init_class();
    
    
    JsParser  *parser;
    JSContext *cx;
    JSRuntime *rt;
    JSObject  *obj; // the global object
};

class JsParser {
    public:
	JsParser(Context *_env);
	~JsParser();
	int include(JSContext *cx, const char* jscript);
	///< include javascript libraries from known path (current or PREFIX)
	int open(const char* script_file);
	int use(JSContext *cx, JSObject *obj, const char* script_file);

	int parse(const char *command);
	void stop();
	void gc();
	char* readFile(FILE *file,int *len);
	int reset();

	JSBool branch_callback(JSContext* Context, JSScript* Script);

    /* DEPRECATED!! here for retrocompatibility */
	JSContext *global_context;
	JSObject *global_object;
	JSRuntime *js_runtime;
    /** **/
    
    JsExecutionContext *global_runtime;
    Linklist<JsExecutionContext> runtimes;
    
 private:
    void init();
    void init_class(JSContext *cx, JSObject *obj);
    int open(JSContext *cx, JSObject *obj, const char* script_file);
    int evaluate(JSContext *cx, JSObject *obj, const char *name, const char *buf, unsigned int len);
    
};
#endif

#endif
