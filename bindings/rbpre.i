//ignore Context::start and start_threaded because ruby doesn't support native threads
%ignore Context::start();
%ignore Context::start_threaded();

//here we load up some ruby code which overwrites and extends some of the freej methods
//this file freej_extensions must be in ruby's load path
%init %{
   rb_eval_string("begin; require 'freej_extensions'\n \
         rescue LoadError; puts \"An error occurred: \", $!; end\n");
%}

%{
#define RSHIFT(x,y) ((x)>>(int)y)

%}

%typemap(in) int16_t {
   $1 = (int16_t)FIX2INT($input);
}

%typemap(in) uint16_t {
   $1 = (uint16_t)FIX2UINT($input);
}

%typemap(out) int16_t {
   $result = INT2FIX($1);
}

//XXX i think we might lose some data here..
// was it a typo?
%typemap(out) uint16_t {
   $result = INT2FIX((uint16_t)$1);
}

