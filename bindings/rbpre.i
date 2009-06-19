%ignore Context::start();
%ignore Context::start_threaded();

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
%typemap(out) uint16_t {
   $result = INT2FIX((int16_t)$1);
}

