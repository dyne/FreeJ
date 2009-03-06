// andy@engagemedia.org
// templated example script for using FreeJ and Plumi (plone video CMS) 
//

header_text = new TextLayer();
header_text.set_position(70,0);
header_text.size(50);
header_text.color(255,0,0);
header_text.print("$channel_title")
//header_text.set_fps(25);
header_text.start();
add_layer(header_text);

logo_image = new ImageLayer();
logo_image.open("$image_file_path")
logo_image.activate(true);
logo_image.start()
logo_image.set_position(0,0);
add_layer(logo_image);

#for $video in $videos

    $video.js_var_name = new MovieLayer("$video.url")
    add_layer($video.js_var_name);

    ${video.js_var_name}.set_position(50,70);
    ${video.js_var_name}.activate(true);

#end for

// START streaming

encoder = new VideoEncoder(10, 64000, 5, 24000); 
encoder.stream_host("giss.tv"); 
encoder.stream_port(8000); 
encoder.stream_title("$channel_title")
encoder.stream_username("source"); 
encoder.stream_password("2t645"); 
encoder.stream_mountpoint("freej-test.ogg"); 
register_encoder(encoder); 
encoder.start_stream(); 

//END streaming

