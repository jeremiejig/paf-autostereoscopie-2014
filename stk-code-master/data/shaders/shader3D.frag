uniform int nbviews;

// Tells the orientation of the screen parallax barriers
// 1 if it's towards the left, else -1
uniform bool leftInterlacing; 

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;


void getTextureSample(in int texID, out vec4 color) {

  if(texID == 0 ) color = texture2D(tex0, gl_TexCoord[0].st);
  if(texID == 1 ) color = texture2D(tex1, gl_TexCoord[0].st);
  if(texID == 2 ) color = texture2D(tex2, gl_TexCoord[0].st);
  if(texID == 3 ) color = texture2D(tex3, gl_TexCoord[0].st);
  if(texID == 4 ) color = texture2D(tex4, gl_TexCoord[0].st);
  if(texID == 5 ) color = texture2D(tex5, gl_TexCoord[0].st);
  if(texID == 6 ) color = texture2D(tex6, gl_TexCoord[0].st);
  if(texID == 7 ) color = texture2D(tex7, gl_TexCoord[0].st);

}

void main()
{
  int x = int(gl_FragCoord.x - 0.5);
  int y = int(gl_FragCoord.y - 0.5);
  int moduloy = int(mod(y,nbviews));

  int viewLine;

  if (leftInterlacing)
    viewLine = nbviews - 1 - moduloy ;
  else
    viewLine = moduloy;
  int viewPix = viewLine + 3 * x;
  int viewR = int(mod(viewPix,nbviews));
  int viewG = int(mod(viewPix + 1,nbviews));
  int viewB = int(mod(viewPix + 2,nbviews));

  vec4 colR, colG, colB ;

  getTextureSample(viewR, colR) ;
  getTextureSample(viewG, colG) ;
  getTextureSample(viewB, colB) ;
  gl_FragColor.r = colR.r ;
  gl_FragColor.g = colG.g ;
  gl_FragColor.b = colB.b ;
}

