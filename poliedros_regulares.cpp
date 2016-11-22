#include "myClasses.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/GLAux.h>
 
double rotate_y=0; 
double rotate_x=0;
int sourceFace=0;
int height;
bool open=false;
GLfloat angle=0;
GLfloat pAngle;
GLfloat colors[][3]={{1.0, 0.0, 0.0},    // vermelho
					{0.0, 1.0, 0.0},     // verde
					{0.0, 0.0, 1.0},     // azul
					{1.0, 1.0, 1.0},     // branco
					{1.0, 0.0, 1.0},     // magenta
					{0.0, 1.0, 1.0},	 // ciano
					{1.0, 0.5, 1.0},     // coral
					{0.65, 0.16, 0.16},  // marrom
					{0.6, 0.41, 0.2},    // amarelo esverdiado
					{0.0, 0.5, 1.0},     // azul ardósia
					{0.5, 0.0, 1.0},     // roxo
					{0.32, 0.5, 0.46},   // verde cobre
					{0.75, 0.75, 0.75},  // cinza
					{1.0, 1.0, 0.0},     // amarelo
					{1.0, 0.75, 0.40},   // rosa 
					{0.72, 0.45, 0.2},   // cobre escuro 
					{0.81, 0.5, 0.2},    // ouro
					{0.5, 1.0, 0.0},     // verde claro
					{0, 1.0, 0.5},       // verde claro
					{0.55, 0.14, 0.14}}; // escarlata    
int type=TETRAHEDRON;
int NumFaces=4;
vector<vector<GLfloat> > texCoordMatrix;
bool update=false;


GLuint textureID;
AUX_RGBImageRec *myPixelArray; 

AUX_RGBImageRec *LoadBMP(char *Filename){
	FILE *File=NULL;
	if (!Filename) {
		return NULL;         
	}
	File=fopen(Filename,"r");	
	if (File)	// Se o arquivo existe
	{
		fclose(File);			        
		return auxDIBImageLoad(Filename);//Retorna a imagem
	}
	return NULL;			
}

vector<vector<GLfloat> > updateTexCoord(int i, Polyhedron ph) {
	vector<myCoordinates> vertices=ph.vertices;
	vector<vector<int> > faces=ph.faces;
	myCoordinates u=sub(vertices[faces[i][1]],vertices[faces[i][0]]);
	myCoordinates v=sub(vertices[faces[i][2]],vertices[faces[i][1]]);
	myCoordinates n=normalize(crossProduct(u,v));
	GLfloat mv[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv);
	vector<vector<GLfloat> > modelview(4);
	for (int j=0; j<4; j++) {
		modelview[j].resize(4);
		modelview[j][0]=mv[0+j*4];
		modelview[j][1]=mv[1+j*4];
		modelview[j][2]=mv[2+j*4];
		modelview[j][3]=mv[3+j*4];
	}
	modelview=transpose(modelview);
	vector<vector<GLfloat> > newn=normalize(multMatrix(modelview,coToVe(n,0)));
	vector<vector<GLfloat> > v1=multMatrix(modelview, coToVe(u,0));
	vector<vector<GLfloat> > nv=coToVe(crossProduct(veToCo(newn),veToCo(v1)),0);
	vector<vector<GLfloat> > p=multMatrix(modelview,coToVe(vertices[faces[i][0]],1));
	vector<vector<GLfloat> > marray[]={v1, nv, newn, p};
	vector<vector<GLfloat> > m(4);
	for (int k=0; k<4; k++) {
		m[k].resize(4);
		for (int l=0; l<4; l++) {
			m[k][l]=marray[l][k][0];
		}
	}
	m=inverse(m);
	return m;
}


void display(){
//	cout << update << endl;
	Polyhedron polyhedron(type);
	pAngle=polyhedron.getAngle();
	if (open) {
	 polyhedron.bfs(sourceFace);
	}
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  	glLoadIdentity();
  	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
    glOrtho(-3,3,-3,3,-3,3);
    glMatrixMode(GL_MODELVIEW);
  	glRotatef( rotate_x, 1.0, 0.0, 0.0 );
 	glRotatef( rotate_y, 0.0, 1.0, 0.0 );
	if (open) {
		vector<vector<pair<myCoordinates, myCoordinates> > > transformations=polyhedron.getTransformations();
		if (update) {
			texCoordMatrix=updateTexCoord(sourceFace, polyhedron);
			update=false;
		}
		polyhedron.drawFace(sourceFace, update, texCoordMatrix);
		for (int i=0; i<NumFaces; i++) {
			if (i==sourceFace) {
				continue;
			}
			glPushMatrix();
				for (int j=transformations[i].size()-1; j>=0; j--) {
					myCoordinates vect=transformations[i][j].first;
					glTranslatef(vect.x, vect.y, vect.z);
					myCoordinates vecr=transformations[i][j].second;
					glRotated(angle, vecr.x, vecr.y, vecr.z);
					glTranslatef(-vect.x, -vect.y, -vect.z);
				}
				polyhedron.drawFace(i, update, texCoordMatrix);
			glPopMatrix();
		}
		if (angle<pAngle) {
			angle+=pAngle/30.0f;
			if (angle>=pAngle) {
				angle=pAngle;
				update=true;
			}
		}
	
	} else {
		for (int i=0; i<NumFaces; i++) {
			polyhedron.drawFace(i, update, texCoordMatrix);
		}
 
	}
    glFlush();
    glutSwapBuffers();
 
}

void myTimer(int id) {
	glutPostRedisplay();
	glutTimerFunc(50, myTimer, 0);
}
 
void myReshape(int w, int h) {
	height=h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-3.0, 3.0, -3.0, 3.0);
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}

void specialKeys( int key, int x, int y ) {
 
	if (key == GLUT_KEY_RIGHT) {
		rotate_y += 5;
	} else if (key == GLUT_KEY_LEFT){
	   rotate_y -= 5;
	} else if (key == GLUT_KEY_UP) {
	   rotate_x += 5;
	} else if (key == GLUT_KEY_DOWN) {
	   rotate_x -= 5;
	}
  	glutPostRedisplay();
 
}

int index(float r, float g, float b) {
	for (int i=0; i<20; i++) {
		float cr, cg, cb;
		cr=round(r*100.0f)/100.0f;
		cg=round(g*100.0f)/100.0f;
		cb=round(b*100.0f)/100.0f;
		if (colors[i][0]==cr && colors[i][1]==cg && colors[i][2]==cb) {
			return i;
		}
	}
	return -1;
}

void myMouse(int b, int s, int x, int y) {
	if (b==GLUT_LEFT_BUTTON) {
		if (s==GLUT_UP) {
			GLubyte color[3];
			float colori[3];
			glReadPixels(x,height-y,1,1,GL_RGB,GL_UNSIGNED_BYTE, color);
			colori[0]=(int)color[0]/255.0f;
			colori[1]=(int)color[1]/255.0f;
			colori[2]=(int)color[2]/255.0f;
			int i=index(colori[0], colori[1], colori[2]);
			if (open) {
				open=false;
				angle=0.0f;
				glutPostRedisplay();
			} else {
				open=true;
				glutPostRedisplay();
			}
		}
	}
}

void myKeyboard(unsigned char key, int x, int y ) {
	if (key=='1'){
		type=TETRAHEDRON;
		open=false;
		NumFaces=4;
		angle=0.0f;
	} else if (key=='2') {
		type=HEXAHEDRON;
		open=false;
		NumFaces=6;
		angle=0.0f;
	} else if (key=='3') {
		type=OCTAHEDRON;
		open=false;
		NumFaces=8;
		angle=0.0f;
	} else if (key=='4') {
		type=DODECAHEDRON;
		open=false;
		NumFaces=12;
		angle=0.0f;
	} else if (key=='5') {
		type=ICOSAHEDRON;
		open=false;
		NumFaces=20;
		angle=0.0f;
	}
	
	glutPostRedisplay();
}

void initializations() {
	glClearColor(0.0, 0.0, 0.0, 1.0); // intentionally background
	glEnable(GL_NORMALIZE); // normalize normal vectors
	glShadeModel(GL_SMOOTH); // do smooth shading
	glEnable(GL_LIGHTING); // enable lighting
	// ambient light (red)
	GLfloat ambientIntensity[4] = {0.8, 0.8, 0.8, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientIntensity);
	// set up light 0 properties
	GLfloat lt0Intensity[4] = {1.5, 1.5, 1.5, 1.0}; // white
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lt0Intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lt0Intensity);
	GLfloat lt0Position[4] = {2.0, 4.0, 5.0, 1.0}; // location
	glLightfv(GL_LIGHT0, GL_POSITION, lt0Position);
	// attenuation params (a,b,c)
	glLightf (GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0);
	glLightf (GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
	glLightf (GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.1);
	glEnable(GL_LIGHT0);
	
	
	myPixelArray = LoadBMP((char *)"image.bmp");
	GLuint textureID; // the ID of this texture
	glGenTextures(1, &textureID); // assign texture ID
	glBindTexture(GL_TEXTURE_2D, textureID); // make this the active texture
	//
	// ... input image nRows x nCols into RGB array myPixelArray
	//
	glTexImage2D(GL_TEXTURE_2D, 0, 3, myPixelArray->sizeX, myPixelArray->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, myPixelArray->data);
	// generate mipmaps (see below)
//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, nCols, nRows, GL_RGB,	GL_UNSIGNED_BYTE, myPixelArray);
	
	glEnable(GL_TEXTURE_2D); // enable texturing
	glBindTexture(GL_TEXTURE_2D, textureID); // select the active texture
	// (use GL_REPLACE below for skyboxes)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// repeat texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// reasonable filter choices
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	
	
	GLfloat tcArray[][4] = { {1.0f, 0.0f, 0.0f, 0.5f},
							 {0.0f, 1.0f, 0.0f, 0.5f},
							 {0.0f, 0.0f, 1.0f, 0.0f},
							 {0.0f, 0.0f, 0.0f, 1.0f},
							};
	texCoordMatrix.resize(4);
	for (int i=0; i<4; i++) {
		texCoordMatrix[i].resize(4);
		for (int j=0; j<4; j++) {
			texCoordMatrix[i][j]=tcArray[i][j];
		}
		
	}
	
}
 
int main(int argc, char* argv[]){
 
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500,500);
  glutCreateWindow("Poliedros regulares");
  glEnable(GL_DEPTH_TEST);
  glutDisplayFunc(display);
  glutSpecialFunc(specialKeys);
  glutMouseFunc(myMouse);
  glutReshapeFunc(myReshape);
  glutTimerFunc(50, myTimer, 0);
  glutKeyboardFunc(myKeyboard);
  initializations();
  glutMainLoop();
  return 0;
 
}
