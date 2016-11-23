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
int type=TETRAHEDRON;
int NumFaces=4;
vector<vector<GLfloat> > texCoordMatrix;
vector<vector<pair<GLfloat, GLfloat> > > texCoord;
bool update=false;
bool change=true;


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

void updateFaceTexCoord(int i, Polyhedron p) {
	vector<myCoordinates> vertices=p.vertices;
	vector<int> face=p.faces[i];
	vector<vector<GLfloat> > modelview(4);
	GLfloat mv[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv);
	for (int j=0; j<4; j++) {
		modelview[j].resize(4);
		modelview[j][0]=mv[0+j*4];
		modelview[j][1]=mv[1+j*4];
		modelview[j][2]=mv[2+j*4];
		modelview[j][3]=mv[3+j*4];
	}
	modelview=transpose(modelview);
	for (int j=0; j<face.size(); j++) {
		vector<vector<GLfloat> > tc = multMatrix(texCoordMatrix, multMatrix(modelview, coToVe(vertices[face[j]],1)));
		texCoord[i][j].first=tc[0][0];
		texCoord[i][j].second=tc[1][0];
	}
}

void initializeTexCoord(Polyhedron p) {
	texCoord.resize(NumFaces);
	switch (NumFaces) {
		case 4:
		case 8:
		case 20:
			{
				for (int i=0; i<texCoord.size(); i++) {
					texCoord[i].resize(3);
				}
				break;
			}
		case 6:
			{
				for (int i=0; i<texCoord.size(); i++) {
					texCoord[i].resize(4);
				}
				break;
			}
		case 12:
			{
				for (int i=0; i<texCoord.size(); i++) {
					texCoord[i].resize(5);
				}
				break;
			}
		default:
			break;
	}
	vector<myCoordinates> vertices=p.vertices;
	vector<vector<int> > faces=p.faces;
	for (int i=0; i<texCoord.size(); i++) {
		for (int j=0; j<texCoord[i].size(); j++) {
			texCoord[i][j]=make_pair(vertices[faces[i][j]].x+0.5,vertices[faces[i][j]].y+0.5);
		}
	}
}

void display(){
//	cout << update << endl;
	Polyhedron polyhedron(type);
	pAngle=polyhedron.getAngle();
	polyhedron.bfs(sourceFace);
	if (change) {
		initializeTexCoord(polyhedron);
		change=false;
	}
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  	glLoadIdentity();
  	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
    glOrtho(-3,3,-3,3,-3,3);
    glMatrixMode(GL_MODELVIEW);
  	glRotatef( rotate_x, 1.0, 0.0, 0.0 );
 	glRotatef( rotate_y, 0.0, 1.0, 0.0 );
	vector<vector<pair<myCoordinates, myCoordinates> > > transformations=polyhedron.getTransformations();
	if (update) {
		texCoordMatrix=updateTexCoord(sourceFace, polyhedron);
		updateFaceTexCoord(sourceFace, polyhedron);
	}
	polyhedron.drawFace(sourceFace, texCoord[sourceFace]);
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
			if (update) {
				updateFaceTexCoord(i, polyhedron);
			}
			polyhedron.drawFace(i, texCoord[i]);
		glPopMatrix();
	}
	if (update) {
		update=false;
	}
	if (angle<pAngle && open) {
		angle+=pAngle/30.0f;
		if (angle>=pAngle) {
			angle=pAngle;
			update=true;
		}
	}
	if (angle>0 && !open) {
		angle-=pAngle/30.0f;
		if (angle<0) {
			angle=0;
		}
	}
    glFlush();
    glutSwapBuffers();
 
}

void myTimer(int id) {
	glutPostRedisplay();
	glutTimerFunc(25, myTimer, 0);
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

void polyhedronMenu(int option) {
	switch (option) {
		case 0:
			type = TETRAHEDRON;
			NumFaces = 4;
			break;
		case 1:
			type = HEXAHEDRON;
			NumFaces = 6;
			break;
		case 2:
			type = OCTAHEDRON;
			NumFaces = 8;
			break;
		case 3:
			type = DODECAHEDRON;
			NumFaces = 12;
			break;
		case 4:
			type = ICOSAHEDRON;
			NumFaces = 20;
			break;
	}
	open=false;
	angle=0.0f;
	change=true;
}

void mainMenu(int option){

}

void createMenu() {
	int menu, submenu1;
	
	submenu1 = glutCreateMenu(polyhedronMenu);
	glutAddMenuEntry("Tetraedro",0);
	glutAddMenuEntry("Hexaedro",1);
	glutAddMenuEntry("Octaedro",2);
	glutAddMenuEntry("Dodecaedro",3);
	glutAddMenuEntry("Icosaedro",4);
	
	menu = glutCreateMenu(mainMenu);
	glutAddSubMenu("Poliedro",submenu1);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void myMouse(int b, int s, int x, int y) {
	if (b==GLUT_LEFT_BUTTON) {
		if (s==GLUT_UP) {
			if (open) {
				open=false;
				glutPostRedisplay();
			} else {
				open=true;
				update=false;
				glutPostRedisplay();
			}
		}
	}
	if (b==GLUT_RIGHT_BUTTON) {
		if (s==GLUT_UP) {
			createMenu();
			glutPostRedisplay();
		}
	}
}

void myKeyboard(unsigned char key, int x, int y ) {
	if (key=='1'){
		type=TETRAHEDRON;
		open=false;
		NumFaces=4;
		angle=0.0f;
		change=true;
	} else if (key=='2') {
		type=HEXAHEDRON;
		open=false;
		NumFaces=6;
		angle=0.0f;
		change=true;
	} else if (key=='3') {
		type=OCTAHEDRON;
		open=false;
		NumFaces=8;
		angle=0.0f;
		change=true;
	} else if (key=='4') {
		type=DODECAHEDRON;
		open=false;
		NumFaces=12;
		angle=0.0f;
		change=true;
	} else if (key=='5') {
		type=ICOSAHEDRON;
		open=false;
		NumFaces=20;
		angle=0.0f;
		change=true;
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
