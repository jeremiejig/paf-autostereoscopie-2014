uniform int nbviews;

// Tells the orientation of the screen parallax barriers
// 1 if it's towards the left, else -1
uniform bool leftInterlacing; 

uniform sampler2D tex0;
uniform sampler2D tex1;

uniform sampler2D depth0;
uniform sampler2D depth1;

uniform float width;

uniform float ZFAR;

uniform float ZNEAR;


//define constants for second image interpolation

float lineardepthtrigger = 12.0;
float pc_DELTA = 64.0; //64
float DECRAN = 80.0; // Distance écran - utilisateur (en cm)
float P = 10.0; //plage de perception de la profondeur (en centimètre ~ profondeur du cube)
float distance_inter_oculaire = 6.5; // distance entre les yeux (en cm)
float Z0 = 2.0; // position de la face avant du cube dans l'espace
float DZREF= 2.0; // distance caractéristique permettant de différencier un étirement d'un trou
float pixel_cm= 0.03; // largeur d'un pixel en cm

float factor=(distance_inter_oculaire/DECRAN)* (P/256.0)/pixel_cm;
float offset=(distance_inter_oculaire/DECRAN)*Z0/pixel_cm;

//couleur du pixel généré
vec4 ColPx;

float getDepth(sampler2D depth, float pc_x, float tc_y, float scaler)
{
	 vec2 position =vec2(pc_x/scaler, tc_y);
	 vec4 pixelValue = texture2D(depth, position);
	 float d = pixelValue.r;

	 //Linéarisation de la Depth
	 // d = 2.0 * ZNEAR / (ZFAR + ZNEAR - d*(ZFAR-ZNEAR) );
	 // d *= lineardepthtrigger;
	 // if(d > 1.0) d = 1.0;
	 return d*255.0;
}
float generateSecondViewPixel(sampler2D depth, float ALPHA,float l){
	pc_DELTA=pc_DELTA/ALPHA;
	float zTheorique = -pc_DELTA;
	int f = 1;
	float pc_x = (gl_TexCoord[0].s)*width;
	float tc_y = gl_TexCoord[0].t;
	float k = pc_x- l*pc_DELTA;
	float kmin= pc_x - pc_DELTA;
	float kmax = pc_x + pc_DELTA ;
	float lastk;
	float zimg,zimgp,ztemp, ztemp2, ztempref, ztempref2;
	float zref,zrefp,lastz ;
	float kp ;
	kp = k-l ;
	float abs2;
	float scaler = width-1.0;
	

	vec2 position;
	vec4 pixelValue;
	vec4 pixelValuePrec;

	if(k<0.0){ k=0.0; zTheorique = -pc_x ;} 
	if (kmin<0.0){ kmin=0.0;}
	if(kp<0.0) {kp = 0.0 ; }
	if(kmax >= width){ kmax = width-1.0 ;}
	if (kp >= width){ kp = width-1.0 ;}
	if (k>=width) { k = width-1.0 ; zTheorique = pc_x - width ;}

	float XSecondViewPixel=gl_TexCoord[0].s;




	while (k>=kmin && k<=kmax ){
	 
	 zimg = getDepth(depth, k, tc_y, scaler);
	 zimgp = getDepth(depth, kp, tc_y, scaler);

	 zref  = (factor*zimg + offset) / ALPHA;
	 zrefp = (factor*zimgp + offset) / ALPHA;

	 if ((zTheorique-zref<0.5)&&(zTheorique-zref > -0.5)) {
		return k/scaler;

	} 
	else if ((zTheorique > zref)&&(f==1)){
	 abs2 = zref - zrefp;
	 if ((abs2 < DZREF)&&(abs2 > -DZREF)) {
		return k/scaler;

		
	}
}

if(zTheorique < zref) f = 1; else f= 0;
if(zTheorique < zref) {
	lastz = (zref-zTheorique); 
	lastk = k;

	ztemp = getDepth(depth, k, tc_y, scaler);
	ztempref = factor*ztemp/ALPHA + offset/ALPHA;
	}

	zTheorique = zTheorique+1.0;
	k=k + l;
	kp=k - l;

}

	// On est dans un trou


ztemp2 = getDepth(depth, (lastk-l*lastz), tc_y, scaler);
ztempref2  = factor*ztemp2/ALPHA + offset/ALPHA;

float zdiff;
if (ztempref2 < ztempref) zdiff = ztempref - ztempref2;
else zdiff = ztempref2 - ztempref;

if(zdiff>4.0){
 XSecondViewPixel=lastk/scaler;}
 else{ 
	XSecondViewPixel=(lastk-l*lastz)/scaler;}
	
	
	return  XSecondViewPixel;
}

void selectColor(in int texID, out vec4 color){
	vec2 position2; 
	float XSecondViewPixel;
	float coef = 3.0;

	if(texID == 4) {

		XSecondViewPixel = generateSecondViewPixel(depth0, coef,1.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color = ColPx;
	}
	if(texID == 3) {
	
		XSecondViewPixel = generateSecondViewPixel(depth0,2.0*coef,1.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color = ColPx;
	}
		if(texID == 2){ color = texture2D(tex0, gl_TexCoord[0].st);}
		if(texID == 1) {

		XSecondViewPixel = generateSecondViewPixel(depth0,2.0*coef,-1.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color =  ColPx;
	}
	if(texID == 0) {
	
		XSecondViewPixel = generateSecondViewPixel(depth0,1.0*coef,-1.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color = ColPx;
	}

}


void main()
{
	/*int x = int(gl_FragCoord.x + 0.5);
	int y = int(gl_FragCoord.y + 0.5);
	int modulox = int(mod(x,nbviews));
	int moduloy = int(mod(y,nbviews));

	int viewLine;

	if (leftInterlacing)
		viewLine = nbviews - 1 - moduloy ;
	else
		viewLine = moduloy;
	int viewPix = int(mod(viewLine + 3 * modulox , nbviews));
	int viewR = viewPix - nbviews*(viewPix/nbviews) ;
	int viewG = viewPix + 1 - nbviews*((viewPix +1)/nbviews);
	int viewB = viewPix + 2 - nbviews*((viewPix +2)/nbviews);

	if(viewR >= nbviews) viewR -= nbviews;
	if(viewG >= nbviews) viewG -= nbviews;
	if(viewB >= nbviews) viewB -= nbviews;

	vec4 colR, colG, colB ;*/
	int x = int(gl_FragCoord.x + 0.5);
	int y = int(gl_FragCoord.y + 0.5) ;
	int modulox = x/5;
	int moduloy = y/5;
	modulox = x - 5 * modulox;
	moduloy = y - 5 * moduloy;

	int viewLine = 4 - moduloy ;
	int viewPix = viewLine + 3 * modulox ;
	int viewR = viewPix - 5*(viewPix/5) ;
	int viewG = viewPix + 1 - 5*((viewPix +1)/5);
	int viewB = viewPix + 2 - 5*((viewPix +2)/5);



	vec4 colR, colG, colB ;




/*
	float pitch = 5.0 + 1.0  - mod(gl_FragCoord.y , 5.0);
	int col = int( mod(pitch + 3.0 * (gl_FragCoord.x), 5.0 ) );
	int viewR = int(col);
	int viewG = int(col) + 1;
	int viewB = int(col) + 2;
	if (viewG >= 5) viewG -= 5;
	if (viewB >= 5) viewB -= 5;
*/



 //test direct texture sampling
 // colR = texture2D(depth0, gl_TexCoord[0].st);
 // float d = 2.0 * ZNEAR / (ZFAR + ZNEAR - colR.r*(ZFAR-ZNEAR) );
 // d *= lineardepthtrigger;
 // if(d > 1.0) d = 1.0;
 // gl_FragColor = vec4(d,d,d,1.0);

 //test single view 
	/*vec2 position2; 
	float XSecondViewPixel;
	XSecondViewPixel = generateSecondViewPixel(depth0,2.0,1.0);
	position2 = vec2(XSecondViewPixel,gl_TexCoord[0].t);
	ColPx = texture2D(tex0, position2);
	gl_FragColor = ColPx;*/



	// viewB = 1;
	// viewR = 2;
	// viewG = 2;
	selectColor(viewR, colR) ; 
	gl_FragColor.r = colR.r ;
	selectColor(viewG, colG) ; 
	gl_FragColor.g = colG.g ;
	selectColor(viewB, colB) ; 
	gl_FragColor.b = colB.b ;


}


