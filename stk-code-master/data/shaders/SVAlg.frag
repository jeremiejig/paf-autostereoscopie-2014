uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;

uniform sampler2D depth0;
uniform sampler2D depth1;
uniform sampler2D depth2;
uniform sampler2D depth3;


//define constants for second image interpolation
float pc_DELTA = 64.0; //64
float DECRAN = 80.0; // Distance écran - utilisateur (en cm)
float P = 10.0; //plage de perception de la profondeur (en centimètre ~ profondeur du cube)
float distance_inter_oculaire = 4.5; // distance entre les yeux (en cm)
float Z0 = 0.0; // position de la face avant du cube dans l'espace
int HAUTEUR_IMAGE= 900; // hauteur de l’image
int LARGEUR_IMAGE= 1440; // largeur de l’image
float DZREF= 2.0; // distance caractéristique permettant de différencier un étirement d'un trou
float pixel_cm= 0.03; // largeur d'un pixel en cm

//nouveaux parametres
float ALPHA=3.0;
float l=1.0;

vec4 ColPx;
float factor=(distance_inter_oculaire/DECRAN)* (P/256.0)/pixel_cm;
float offset=(distance_inter_oculaire/DECRAN)*Z0/pixel_cm;


void selectColor(in int texID, out vec4 color){
	vec2 position2; 
	float XSecondViewPixel;

	if(texID == 0) {
		ALPHA=1.25;
		l=1.0;
		XSecondViewPixel = generateSecondViewPixel(depth0,1440.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color = ColPx;}

	else if(texID == 1) {
		ALPHA=1.75;
		l=1.0;
		XSecondViewPixel = generateSecondViewPixel(depth0,1440.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color = ColPx;}

	else if(texID == 2) {
		ALPHA=2.0;
		l=1.0;
		XSecondViewPixel = generateSecondViewPixel(depth0,1440.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color = ColPx;}

	if(texID == 3) {
		ALPHA=2.25;
		l=1.0;
		XSecondViewPixel = generateSecondViewPixel(depth0,1440.0);
		position2 = vec2(XSecondViewPixel ,gl_TexCoord[0].t);
		ColPx = texture2D(tex0, position2);
		color = ColPx;}
	
	if(texID == 4)
		color = texture2D(tex0, gl_TexCoord[0].st);
}


float generateSecondViewPixel(sampler2D depth, float width){
	pc_DELTA=pc_DELTA/ALPHA;
	float zTheorique = -pc_DELTA;
	int f = 1;
	float k = gl_TexCoord[0].s*width-pc_DELTA;
	float kmax = gl_TexCoord[0].s*width+pc_DELTA ;
	float lastk;
	float zimg,zimgp,ztemp, ztemp2, ztempref, ztempref2;
	float zref,zrefp,lastz ;
	float kp ;
	kp = k-l ;
	float kmin;
	float abs2;
	int ok = 0;

	vec2 position;
	vec4 pixelValue;
	vec4 pixelValuePrec;

	if(k < 0.0)		{k=0.0 ; zTheorique=-gl_TexCoord[0].s*width;} 
	if(kmin < 0.0)	{kmin=0.0 ;}
	if(kp < 0.0) 	{kp = 0.0 ;}
	if(kmax >= width){kmax = width-1.0 ;}
	if(kp >= width)	{kp = width-1.0 ;}
	if(k >= width) 	{k = width-1.0 ; zTheorique = gl_TexCoord[0].s*width-width ;}

	float XSecondViewPixel=gl_TexCoord[0].s;

	while (k>=kmin && k<kmax ){
        position =vec2(k/width,gl_TexCoord[0].t);
        pixelValue = texture2D(depth, position);
        position = vec2(kp/width,gl_TexCoord[0].t);
        pixelValuePrec =texture2D(depth, position);
        zimg=pixelValue.r*256.0;
        zimgp=pixelValuePrec.r*256.0;
        //zimg=0.0;
        //zimgp=0.0;
                
        zref  = factor*zimg/ALPHA + offset/ALPHA;
        zrefp = factor*zimgp/ALPHA + offset/ALPHA;

	    if (abs(zTheorique-zref)<0.5){
            
		XSecondViewPixel = k/width;
        ok=1;
      	break;
		} 

		else if ((zTheorique > zref)&&(f==1)){
			abs2 = abs(zref - zrefp);

		    if (abs2 < DZREF){
				XSecondViewPixel= k/width;
				ok=1;
		        break;
			}
		}

		if(zTheorique < zref) f = 1; else f = 0;

		if(zTheorique < zref) {
			lastz = abs(zref-zTheorique) ; 
			lastk = k;
			position = vec2(kp/width,gl_TexCoord[0].t);
			pixelValue =texture2D(depth, position);
			ztemp=pixelValue.r;
			ztempref = factor*ztemp/ALPHA + offset/ALPHA;
		}

		zTheorique = zTheorique+1.0;
		k=k+l;
		kp=k-l;

	}

	// On est dans un trou
       
	if(ok==0){
		position = vec2(lastk-l*lastz,gl_TexCoord[0].t);
		pixelValue =texture2D(depth, position);
		ztemp2=pixelValue.r;
		ztempref2  = factor*ztemp2/ALPHA + offset/ALPHA;

        
		if(abs(ztempref2-ztempref) > 4.0)
			XSecondViewPixel=lastk/width;
		else
			XSecondViewPixel=(lastk-lastz)/width;
	}

	return  XSecondViewPixel;
}


void main()
{
	int x = int(gl_FragCoord.x + 0.5);
	int y = int(gl_FragCoord.y + 0.5);
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

	selectColor(viewR, colR) ; 
	gl_FragColor.r = colR.r ;
	selectColor(viewG, colG) ; 
	gl_FragColor.g = colG.g ;
	selectColor(viewB, colB) ; 
	gl_FragColor.b = colB.b ;

	//gl_FragColor = ColPx;
}


