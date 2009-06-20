//ignore Context::start and start_threaded because ruby doesn't support native threads
%ignore Context::start();
%ignore Context::start_threaded();

//insert some ruby code into the module
//this is actually just calling a ruby c api function and passing the ruby code as a string
//there is no 'rubycode' method for swig at this point

//create a new start method which uses a ruby thread and cafudda
//extend BaseLinklist to allow for array style indexing with strings
%init %{
   rb_eval_string("\
class Freej::Context\n\
  def start\n\
    #create a ruby thread and call cafudda in a loop so that we render\n\
    @thread = Thread.new { loop { self.cafudda(1); sleep(0.04) } }\n\
    self\n\
  end\n\
  def stop\n\
    #if the thread exists terminate it\n\
    if @thread\n\
      @thread.terminate\n\
    end\n\
    self\n\
  end\n\
end\n\
class Freej::BaseLinklist\n\
  def [](item)\n\
    if item.is_a?(String)\n\
      res = self.search(item)\n\
      if res == 0\n\
        return nil\n\
      else\n\
        return res[0]\n\
      end\n\
    else\n\
      return self.pick(item)\n\
    end\n\
  end\n\
end\n\
");
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

