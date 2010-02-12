/* BIG BANG(2.0)
por Bernardo Silva C.
*/
float margen, sp,x,y,d,m,M;//variables


void setup(){
  margen = 1;
  sp =150;
  size(500, 500);//tamaño
  smooth();//suavizado
  stroke(255,100);//linea
  fill(0,100);//relleno negro con transparencia
  strokeWeight(3.7);//grosor de la linea
  m=0.000005;// aumenta en ese valor
  background(0);//fondo negro

   
}


void draw(){
  if(M<0||M>=0.004){//si M es menor que 0 o mayor que 0.0003 
  m=-m;/* m es -m.entonces matematicamente hablando cuando sea
  postivo y sea mayor o igual que 0.0003,m se volvera negativo,por
  lo cual regrasa a comprimirse el big bang
  */
  }
  fill(0,15);//transparencia para logra el difuminado
  rect(0,0,height,width);// cuadrado como fondo
  
 M+=m;//variable en la que incrementa(escala)


 translate(height/2,width/2);//posición de la figura
  for(y=margen ; y < height; y += sp/2){
    for(x = margen; x < width; x += sp/12){
    //esta es la distancia
      d = 100+sqrt((mouseX - x)*(mouseX - x) + (mouseY - y)*(mouseY - y));  
 scale(d * M);//se increneta la distancia en m
      quad(x,y,x,y,x,d,x,d);//cuadrilateros
      quad(x/12,y+6,x,d,x,d,x*2,d);
        rotate(100);//la figura se rota en torno a su posicion en x e y
  
  }   
    } 
  } 


void keyPressed(){//si se aprieta el boton(f) se graba la foto
  saveFrame("BIG BANG.jpg");
  println("fotograma grabado");


}
