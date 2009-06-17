// andy@engagemedia.org
// templated example script for using FreeJ and Plumi (plone video CMS) 
//

header_text = new TextLayer();
header_text.set_position(70,0);
header_text.size(50);
header_text.color(255,0,0);
header_text.print("EngageMedia Featured Videos Channel")
//header_text.set_fps(25);
header_text.start();
add_layer(header_text);

logo_image = new ImageLayer();
logo_image.open("/home/jaromil/devel/freej/scripts/javascript/examples/script_generator/logo02.gif")
logo_image.activate(true);
logo_image.start()
logo_image.set_position(0,0);
add_layer(logo_image);


    video_0 = new MovieLayer("http://www.engagemedia.org/Members/forumlenteng/videos/andang_sarjo_english.avi/download")
    add_layer(video_0);

    video_0.set_position(50,70);
    video_0.activate(true);



//END streaming

