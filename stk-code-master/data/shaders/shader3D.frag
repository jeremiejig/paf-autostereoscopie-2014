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
  int x = int(gl_FragCoord.x + 0.5);
  int y = int(gl_FragCoord.y + 0.5) ;
  int modulox = x/nbviews;
  int moduloy = y/nbviews;
  modulox = x - nbviews * modulox;
  moduloy = y - nbviews * moduloy;

  int viewLine;

  if (leftInterlacing)
    viewLine = nbviews - 1 - moduloy ;
  else
    viewLine = moduloy;
  int viewPix = viewLine + 3 * modulox ;
  int viewR = viewPix - nbviews*(viewPix/nbviews) ;
  int viewG = viewPix + 1 - nbviews*((viewPix +1)/nbviews);
  int viewB = viewPix + 2 - nbviews*((viewPix +2)/nbviews);

  vec4 colR, colG, colB ;

  getTextureSample(viewR, colR) ;
  gl_FragColor.r = colR.r ;
  getTextureSample(viewG, colG) ;
  gl_FragColor.g = colG.g ;
  getTextureSample(viewB, colB) ;
  gl_FragColor.b = colB.b ;
}

