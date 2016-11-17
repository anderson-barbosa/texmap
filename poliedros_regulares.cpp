#include "myClasses.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
 
double rotate_y=0; 
double rotate_x=0;
int sourceFace;
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
int signal=1;


void display(){
	Polyhedron polyhedron(type);
	pAngle=polyhedron.getAngle();
	if (open) {
	 polyhedron.bfs(sourceFace);
	}
	if (type==TETRAHEDRON) {
	signal=-1;
	} else {
	signal=1;
	}
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  	glLoadIdentity();
    glOrtho(-1,1,-1,1,-4,4) ;
  	glRotatef( rotate_x, 1.0, 0.0, 0.0 );
 	glRotatef( rotate_y, 0.0, 1.0, 0.0 );
	
	
	if (open) {
		vector<vector<pair<myCoordinates, myCoordinates> > > transformations=polyhedron.getTransformations();
		for (int i=0; i<NumFaces; i++) {
			if (i==sourceFace) {
				polyhedron.drawFace(i,colors[i][0], colors[i][1], colors[i][2]);
				continue;
			}
			glPushMatrix();
				for (int j=transformations[i].size()-1; j>=0; j--) {
					myCoordinates vect=transformations[i][j].first;
					glTranslatef(vect.x, vect.y, vect.z);
					myCoordinates vecr=transformations[i][j].second;
					glRotated(angle, signal*vecr.x, signal*vecr.y, signal*vecr.z);
					glTranslatef(-vect.x, -vect.y, -vect.z);
				}
				polyhedron.drawFace(i,colors[i][0], colors[i][1], colors[i][2]);
			glPopMatrix();
		}

		if (angle<pAngle) {
			angle+=pAngle/30.0f;
			if (angle>pAngle) {
				angle=pAngle;
			}
		}
	
	} else {
		for (int i=0; i<NumFaces; i++) {
			polyhedron.drawFace(i,colors[i][0], colors[i][1], colors[i][2]);
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
			} else if (i!=-1){
				open=true;
				sourceFace=i;
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
  glutMainLoop();
  return 0;
 
}
