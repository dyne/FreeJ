#include <callbacks_js.h>
#include <jsparser_data.h>
#include <config.h>

#include <movie_layer.h>

DECLARE_CLASS("MovieLayer",movie_layer_class,movie_layer_constructor);


JSFunctionSpec movie_layer_methods[] = {
//  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  {0}
};

JS_CONSTRUCTOR("MovieLayer",movie_layer_constructor,MovieLayer);

