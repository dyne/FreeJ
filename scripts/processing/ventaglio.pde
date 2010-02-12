int W = 400;
int H = 300;
int cycle = 0;
int count = 0;
void setup() { 
    size(W,H);
    background(0);
    smooth();
};
void draw(){
    if(cycle > 5) {
	cycle = 0;
	stroke(floor(random(255)), 
	       floor(random(255)),
	       floor(random(255)), 60 );
    };
    if(count > 799) {   
	count = 0; 
    };
    for (int i = 0; i < 100; i++) {  
	float r = random(2); 
	strokeWeight(r);
	float offset = r * count;  
	line(i-20, H, i+offset, 0); 
    } 
    cycle++;  
    count+=5;
}
