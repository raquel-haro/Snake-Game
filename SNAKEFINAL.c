/*gcc –o text text.c glut32.lib –lopengl32 –lglu32*/

#include <windows.h>  //must be included to work in windows.
#include <math.h>     //provides sin, cos, etc.
#include <time.h>     //provides timing functions
#include <stdlib.h>   //provides several useful functions.
#include "glut.h"  //connects to glut.h 
#include <stdio.h>
//////////////////////////////////////////////////////////////////////////////////////////////

// Used for the texture
#define NUMTEXTURES 4
GLuint texture[NUMTEXTURES];

/* Image type - contains height, width, and data */
typedef struct {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

///////////////////////////////////////////////////////////////////////////////////////////////
float CamX=-7;
float CamY=-3.7;

int z = 0;
int allowSpace = 1;

int Player1Score=0;
int Player2Score=0;



int xPos1 =10;
int yPos1 = 20;

int xPos2 =165;
int yPos2 = 20;


int xChange1 =0;
int yChange1 = 0;

int xChange2 =0;
int yChange2 = 0;



int drawArray[179][91] = {0};

int gameTrue = 0;

//Function Prototypes
int  delayMS(int delayTime); //Creates a delay for the cycles of the renderScene function
void pressKey(int key, int xx, int yy); //Enables the user to use the arrows keys to move
void processNormalKeys(unsigned char key, int xx, int yy); //Processes the keys pressed 
void changeSize(int w, int h); //Enables changes in the width and height of the window size
void renderScene(void); //Generates the images seen on the screen
void drawString(void *font, char *string, float red, float blue, float green, float xstart, float ystart, float zstart);
void arrayFunc(int i,int j);
void drawGrid();
void checkGame();
void toStartScreen();
void toEndScreen();
void ClearScreen();

//////////////////////////////////TEXTURE FUNCTION PROTOTYPES\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void drawCube(float x, float y, float z,int TextNum,float xV,float yV);
static unsigned int getint(FILE* fp);
static unsigned int getshort(FILE* fp);
int ImageLoad(char* filename, Image* image);
void LoadGLTexture();
void InitScene(void);

//////////////////////////////////////////////=\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

/////////////////////////////////////TEXTURE FUNCTIONS\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void drawCube (float x, float y, float z,int TextNum,float xV,float yV) {

  glEnable(GL_TEXTURE_2D);		/* Enable Texture Mapping */

  glPushMatrix();
  
  glTranslatef(x, y, z);



  // bind texture
  glBindTexture(GL_TEXTURE_2D, texture[TextNum]);

  // add in material (for lighting)
  static GLfloat brownish[] = {1, 1, 1, 1.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, brownish);


  glBegin(GL_QUADS);
  /* Front Face */
  glNormal3f( 0.0f, 0.0f, 1.0f);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-xV, -yV,  1.0f);
  glTexCoord2f(1.0f, 0.0f); glVertex3f( xV, -yV,  1.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f( xV,  yV,  1.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-xV,  yV,  1.0f);
  glEnd();

  glPopMatrix();

  glDisable(GL_TEXTURE_2D);		/* Enable Texture Mapping */
  
}

static unsigned int getint(FILE* fp)
{
  int c, c1, c2, c3;

  /* get 4 bytes */
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE* fp)
{
  int c, c1;
  
  /* get 2 bytes */
  c = getc(fp);  
  c1 = getc(fp);

  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

int ImageLoad(char *filename, Image *image) 
{
  FILE *file;
  unsigned long size;                 /* size of the image in bytes. */
  unsigned long i;                    /* standard counter. */
  unsigned short int planes;          /* number of planes in image (must be 1)  */
  unsigned short int bpp;             /* number of bits per pixel (must be 24) */
  char temp;                          /* used to convert bgr to rgb color. */

  /* make sure the file is there. */
  if ((file = fopen(filename, "rb"))==NULL) {
    printf("File Not Found : %s\n",filename);
    return 0;
  }
    
  /* seek through the bmp header, up to the width/height: */
  fseek(file, 18, SEEK_CUR);

  /* No 100% errorchecking anymore!!! */

  /* read the width */
  image->sizeX = getint (file);
  printf("Width of %s: %lu\n", filename, image->sizeX);
    
  /* read the height  */
  image->sizeY = getint (file);
  printf("Height of %s: %lu\n", filename, image->sizeY);
    
  /* calculate the size (assuming 24 bits or 3 bytes per pixel). */
  size = image->sizeX * image->sizeY * 3;

  /* read the planes */
  planes = getshort(file);
  if (planes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, planes);
    return 0;
  }

  /* read the bpp */
  bpp = getshort(file);
  if (bpp != 24) {
    printf("Bpp from %s is not 24: %u\n", filename, bpp);
    return 0;
  }
	
  /* seek past the rest of the bitmap header. */
  fseek(file, 24, SEEK_CUR);

  /* read the data.  */
  image->data = (char *) malloc(size);
  if (image->data == NULL) {
    printf("Error allocating memory for color-corrected image data");
    return 0;	
  }

  if ((i = fread(image->data, size, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }

  for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb) 
    temp = image->data[i];
    image->data[i] = image->data[i+1];
    image->data[i+1] = temp;
  }

  /* we're done. */
  return 1;
}

void LoadGLTexture(GLuint texture, char* filename)
{
  // Load color map
  // Load the texture BMP file
  Image *TextureImage;

  // Allocate space for the image
  TextureImage = (Image *) malloc(sizeof(Image));
  if (TextureImage == NULL) {
    printf("Error allocating space for image");
    exit(0);
  }
  
  // Name of texture goes here!
  if (!ImageLoad(filename, TextureImage)) {
    exit(1);
  }        
  
  //Bind Textures: (and apply filtering)
  /* Create MipMapped Texture */
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage->sizeX, TextureImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
}

void InitScene(void)
{
  //Takes care of Initializing OpenGL parameters
  // 1 -- Setup textures (allocate space)
  glGenTextures(NUMTEXTURES, texture);
  // 2 -- Load the textures into OpenGL's memory
  LoadGLTexture(texture[0], "Texture1.bmp");
  LoadGLTexture(texture[1], "Texture2.bmp");
  LoadGLTexture(texture[2], "Texture3.bmp");
  LoadGLTexture(texture[3], "Texture4.bmp");
  
  //glEnable(GL_TEXTURE_2D);		/* Enable Texture Mapping */
  glShadeModel(GL_SMOOTH);		/* Enable Smooth Shading */

  //glClearColor(backColor[0], backColor[1], backColor[2], backColor[3]); // Set the background color

  glClearDepth(1.0f);			/* Depth Buffer Setup */
  glEnable(GL_DEPTH_TEST);		/* Enables Depth Testing */
  glDepthFunc(GL_LEQUAL);			/* The Type Of Depth Testing To Do */
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	/* Really Nice Perspective Calculations */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int delayMS(int delayTime)
{
    time_t timerStart=clock();   			//Record the start time.
	while(clock()-timerStart<delayTime) {}  //Wait until delayTime ticks have passed.
}
void toStartScreen()
{
	CamX=-7;
	CamY=-3.7;
	gameTrue=0;
	allowSpace = 1;
}

void toEndScreen()
{
	CamX=-7;
	CamY=0;
	gameTrue=0;
	allowSpace = 0;
}


void ClearScreen()
{
	int i,j;
	for(i=0;i<181;i++){
		for(j=0;j<92;j++){
			drawArray[i][j] = 0;
		}
	}
	xPos1 = 10;
	yPos1 = 20;
	xChange1 =0;
	yChange1 = 0;
	
	xPos2 = 160;
	yPos2 = 20;
	xChange2 =0;
	yChange2 = 0;
}


void changeSize(int w, int h) 
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)  h = 1;
	float ratio =  w * 1.0 / h;
	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);
	// Reset Matrix
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);
	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void drawString(void *font, char *string, float red, float green, float blue, float xstart, float ystart, float zstart)
{
	char *charPtr;
	//glLoadIdentity(); 
	glPushMatrix();
	glScalef(0.002, 0.002, 0.002);
	glColor3f(red,green,blue);
	glLineWidth(3);
	//glMatrixMode(GL_MODELVIEW);
	glTranslatef(xstart,ystart,zstart); //Position your text
			
	for (charPtr=string; *charPtr; charPtr++) 
	{
		glutStrokeCharacter(font, *charPtr);
	}
	glPopMatrix();
}


void drawGrid() 
{
	int row = 0;
    int col = 0;
	float xx=0, yy=0;
	float size=0.05;
	glPushMatrix();

    for (row = 1; row <=180 ; row++) {
	  for(col=1; col<=91; col++)
	    {
        if(drawArray[row][col]==0)
		{
			glColor3f(0,0,0);
        }
		if(drawArray[row][col]==1)
		{
			glColor3f(0.0f, 1.0f, 0.0f);
		}
		if(drawArray[row][col]==2){
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		xx = (((float)row)*size)-4.5;
		yy = (2.3-((float)col)*size);
		
        glBegin(GL_QUADS);		         // Draw a Quadrangle
	      glVertex3f(xx,      yy,      0.0f); // Top left
	      glVertex3f(xx+size, yy,      0.0f); // top right
	      glVertex3f(xx+size, yy-size, 0.0f); // bottom right
	      glVertex3f(xx,      yy-size, 0.0f); //bottom left
        glEnd();					     // Finished Square (Quadrangle)
	  }
    }
	glPopMatrix();

}  

void renderScene(void) 
{
	//You may change the following line to adjust the background:
	glClearColor(0.0f, 0.0f, 0.0f, 0); //Set background to blue
	//Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	//Reset transformations.
	glLoadIdentity(); 
	//Set the camera view.
	gluLookAt(CamX, CamY, 6.0f,  //Location of your eye
			  CamX, CamY, 0.0f,  //Location you are looking at
			  0.0f, 1.0f, 0.0f); //Direction of "up"
	

	
	//SCENCE 2 (the game part)
	
	drawCube(-7,-3.7,0.5,0,4.5,2.25);
	drawCube(20,20,-3,1,8,4);
	if(z==1){
		drawCube(-7,0,0.5,2,4.5,2.25);
	}
	else{
		drawCube(-7,0,0.5,3,4.5,2.25);
	}
	
	xPos1+=xChange1;
	if (xPos1 >= 181){
		Player2Score++;
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
	}
	if (xPos1 <= 0){
		Player2Score++;
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
		}
	yPos1+=yChange1;
	if (yPos1 >= 92){
		Player2Score++;
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
		
	}
	if (yPos1 <= 0){
		Player2Score++;
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
	}
	xPos2+=xChange2;
	if ((xPos2 >= 181)){
		Player1Score++;
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
		
	}
	if (xPos2 <= 0){
		Player1Score++;
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
	}
	yPos2+=yChange2;
	if (yPos2 >= 92){
		Player1Score++;
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
	}
	if (yPos2 <= 0){
		Player1Score++;	
		gameTrue = 0;
		//toEndScreen();
		ClearScreen();
	}
	
	if(gameTrue){
		if (drawArray[xPos1][yPos1]==0){
			drawArray[xPos1][yPos1]=1;
		}
		else{
			Player2Score++;
			ClearScreen();
		}	
	}
	
	if(gameTrue){
		if (drawArray[xPos2][yPos2]==0){
			drawArray[xPos2][yPos2]=2;
		}
		else{
			Player1Score++;
			ClearScreen();
		}	
	}
	
	if(Player1Score==5){
		z=1;
		toEndScreen();
	}
	if(Player2Score==5){
		z=0;
		toEndScreen();
	}

	
	glPushMatrix();
	glTranslatef(20,20,-2);
	drawGrid();
	glPopMatrix();
	delayMS(30);
	
	glPushMatrix();
	glTranslatef(17.30,21,2);
	switch (Player1Score) {
		case 0:
			drawString(GLUT_STROKE_ROMAN, "0", 1,0,0, 0,0,0);
			break;
		case 1:
			drawString(GLUT_STROKE_ROMAN, "1", 1,0,0, 0,0,0);
			break;
		case 2:
			drawString(GLUT_STROKE_ROMAN, "2", 1,0,0, 0,0,0);
			break;
		case 3:
			drawString(GLUT_STROKE_ROMAN, "3", 1,0,0, 0,0,0);
			break;
		case 4:
			drawString(GLUT_STROKE_ROMAN, "4", 1,0,0, 0,0,0);
			break;
		case 5:
			drawString(GLUT_STROKE_ROMAN, "5", 1,0,0, 0,0,0);
			break;
	}
	glPopMatrix();
	//player 2 score
	glPushMatrix();
	glTranslatef(22.6,21,2);
	switch (Player2Score) {
		case 0:
			drawString(GLUT_STROKE_ROMAN, "0", 1,0,0, 0,0,0);
			break;
		case 1:
			drawString(GLUT_STROKE_ROMAN, "1",1,0,0, 0,0,0);
			break;
		case 2:
			drawString(GLUT_STROKE_ROMAN, "2", 1,0,0, 0,0,0);
			break;
		case 3:
			drawString(GLUT_STROKE_ROMAN, "3", 1,0,0, 0,0,0);
			break;
		case 4:
			drawString(GLUT_STROKE_ROMAN, "4", 1,0,0, 0,0,0);
			break;
		case 5:
			drawString(GLUT_STROKE_ROMAN, "5", 1,0,0, 0,0,0);
			break;
	}
	glPopMatrix();

	
	drawString(GLUT_STROKE_ROMAN, "GAME OVER", 1,0,0, -3900,100,0);
	if(z == 1){
		drawString(GLUT_STROKE_ROMAN, "Player 1 Wins", 1,1,1, -3940,-200,0);
		drawString(GLUT_STROKE_ROMAN, "Press F3 to restart", 1,1,1, -4100,-400,0);
	}
	else{
		drawString(GLUT_STROKE_ROMAN, "Player 2 Wins", 1,1,1, -3940,-200,0);
		drawString(GLUT_STROKE_ROMAN, "Press F3 to restart", 1,1,1, -4120,-400,0);
	}
	
	// THIS IS SCENE 1 (start screen)
	drawString(GLUT_STROKE_ROMAN, "Press Space to start", 1,1,1, -4150,-1600,0);
	drawString(GLUT_STROKE_ROMAN, "Player 1: use wasd to move", 1,1,1, -4410, -2200,0);
	drawString(GLUT_STROKE_ROMAN, "Player 2: use arrow keys to move", 1,1,1, -4575,-2400,0);


	glutSwapBuffers();	
	
	
} 
void specialKeys(int key, int xx, int yy) 
{
       switch (key) {
             case GLUT_KEY_F2 :
				CamX = -7.0;
				CamY=0.0;
				break;
			 case GLUT_KEY_F3 : 
				CamX =-7.0;
				CamY =-3.7;
				gameTrue = 0;
				ClearScreen();
				Player1Score=0;
				Player2Score=0;
				allowSpace=1;
				break;
			case GLUT_KEY_F5:
				z=1;
				break;
			case GLUT_KEY_F6:
				z=0;
				break;
				
				
			case GLUT_KEY_UP : 
				if(gameTrue){
					xChange2=0;
					yChange2=-1;
				}
				break;
             case GLUT_KEY_DOWN :
				if(gameTrue){
					xChange2=0;
					yChange2=1;
				}
				break;
			 case GLUT_KEY_LEFT : 
				if(gameTrue){
				xChange2=-1;
				yChange2= 0;
				}
				break;
			 case GLUT_KEY_RIGHT :
				if(gameTrue){
				xChange2=1;
				yChange2= 0;
				}
				break;
       }
}
void keyboardKeys(unsigned char key,int x, int y)
{
       switch (key) {
            case 32 : 
				if((gameTrue==0)&&(allowSpace)){
					CamX = 20;
					CamY=20;
					xChange1=1;
					xChange2=-1;
					gameTrue = 1;
				}
				break;
				
				
			case 119: //W
				if(gameTrue){
					xChange1=0;
					yChange1=-1;
					//printf("W\n");
					
				}
				break;
			case 115: //s
				if(gameTrue){
					xChange1 = 0;
					yChange1 = 1;
				//	printf("S\n");
				}
				break;
			case 97: //a
				if(gameTrue){
					xChange1 = -1;
					yChange1 = 0;
					//printf("A\n");
				}
				break;
			case 100: //d
				if(gameTrue){
					xChange1 = 1;
					yChange1 = 0;
					//printf("D\n");
				}
				break;
			case 27 : //ESC
				exit(0);
				break;
		
		}
}


int main(int argc, char **argv) 
{	
	srand(time(NULL));

	// Init GLUT and Create Window
	glutInit(&argc, argv); //Allows input arguments to initialize OpenGL.
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0,0); //Create window at this location.
	glutInitWindowSize(1420,768); //Default window size.
	glutCreateWindow("Displaying Text in OpenGL");  //Window title.

	// Register Callbacks
	glutDisplayFunc(renderScene);  //When you need to display, call renderScene().
	glutReshapeFunc(changeSize);   //When you need to reshape, call changeSize().
    glutIdleFunc(renderScene);
	glutSpecialFunc(specialKeys);
	glutKeyboardFunc(keyboardKeys);
	// OpenGL Init
	glEnable(GL_DEPTH_TEST);  //Enables objects to appear in 3D
	InitScene();
	// Enter GLUT Event Processing Cycle
	glutMainLoop();  //Enter an infinite loop to draw the picture.

	return 1;
}