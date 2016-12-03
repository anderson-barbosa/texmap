#include "myClasses.h"
#include <stdio.h>
#include <climits>
#include <stdarg.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/GLAux.h>

#define BFS 10
#define DFS 11
 
double rotate_y=0; 
double rotate_x=0;
int sourceFace=0;
int height;
int width;
bool open=false;
GLfloat angle=0;
GLfloat pAngle; 
int type=TETRAHEDRON;
int searchType=BFS;
char * imageFile = (char *)"bananas.bmp";
int NumFaces=4;
vector<vector<GLfloat> > texCoordMatrix;
vector<vector<pair<GLfloat, GLfloat> > > texCoord;
bool update=true;
bool faceChange=true;
bool showEditor=false;
vector<pair<int, int> > movingVertices;
int max_x, max_y = -3;
int min_x, min_y = 3;
int dx, dy = 0;
vector<vector<vector<GLfloat> > > facesModelview(NumFaces);
vector<vector<vector<GLfloat> > > facesProjection(NumFaces);


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

vector<vector<GLfloat> > getModelview() {
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
	return modelview;
}

vector<vector<GLfloat> > getProjection() {
	vector<vector<GLfloat> > projection(4);
	GLfloat pj[16];
	glGetFloatv(GL_PROJECTION_MATRIX, pj);
	for (int j=0; j<4; j++) {
		projection[j].resize(4);
		projection[j][0]=pj[0+j*4];
		projection[j][1]=pj[1+j*4];
		projection[j][2]=pj[2+j*4];
		projection[j][3]=pj[3+j*4];
	}
	projection=transpose(projection);
	return projection;
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
		max_x = ceil(max(texCoord[i][j].first, (float)max_x));
		max_y = ceil(max(texCoord[i][j].second, (float)max_y));
		min_x = floor(min(texCoord[i][j].first, (float)min_x));
		min_y = floor(min(texCoord[i][j].second, (float)min_y));
	}
}

void initializeTexCoord() {
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
}

void drawPolyhedron() {
	Polyhedron polyhedron(type);
	pAngle=polyhedron.getAngle();
	if (searchType==BFS){
		polyhedron.bfs(sourceFace);
	} else if (searchType==DFS) {
		polyhedron.dfs(sourceFace);
	}
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  	glLoadIdentity();
  	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();
    glOrtho(-3,3,-3,3,-3,3);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
  	glRotatef( rotate_x, 1.0, 0.0, 0.0 );
 	glRotatef( rotate_y, 0.0, 1.0, 0.0 );
	vector<vector<pair<myCoordinates, myCoordinates> > > transformations=polyhedron.getTransformations();
	if (update && faceChange) {
		max_x = 3;
		max_y =  3;
		min_x = -3;
		min_y = -3;
		texCoordMatrix=updateTexCoord(sourceFace, polyhedron);
		updateFaceTexCoord(sourceFace, polyhedron);
	}
	facesModelview[sourceFace]=getModelview();
	facesProjection[sourceFace]=getProjection();
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
				glRotated(pAngle, vecr.x, vecr.y, vecr.z);
				glTranslatef(-vect.x, -vect.y, -vect.z);
			}
			if (update && faceChange) {
				updateFaceTexCoord(i, polyhedron);
			}
		glPopMatrix();
		glPushMatrix();
			for (int j=transformations[i].size()-1; j>=0; j--) {
				myCoordinates vect=transformations[i][j].first;
				glTranslatef(vect.x, vect.y, vect.z);
				myCoordinates vecr=transformations[i][j].second;
				glRotated(angle, vecr.x, vecr.y, vecr.z);
				glTranslatef(-vect.x, -vect.y, -vect.z);
			}
			facesModelview[i]=getModelview();
			facesProjection[i]=getProjection();
			polyhedron.drawFace(i, texCoord[i]);
		glPopMatrix();
	}
	if (update) {
		update=false;
		if (faceChange) {
			faceChange=false;
		}
		max_x = max(max(max_x, max_y), 3);
		max_y = max(max(max_x, max_y), 3);
		min_x = min(min(min_x, min_y),-3);
		min_y = min(min(min_x, min_y),-3);
		
		max_x = max(abs(min_x),max_x);
		max_y = max_x;
		min_x = -max_x;
		min_y = -max_y;
	}
	if (angle<pAngle && open) {
		angle+=pAngle/30.0f;
		if (angle>=pAngle) {
			angle=pAngle;
		}
	}
	if (angle>0 && !open) {
		angle-=pAngle/30.0f;
		if (angle<0) {
			angle=0;
		}
	}
	glPopMatrix();
}

void drawEditor() {
	glClearColor(0,0,0,1);	
	glDisable(GL_LIGHTING);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPushMatrix();
	glLoadIdentity();
	glScalef(3/(float)max_x, 3/(float)max_y, 1.0f);
//	glMultMatrix()
	for (int i=0; i<2*max_x; i++) {
		for (int j=0; j<2*max_y; j++) {
			glBegin(GL_POLYGON);
				glTexCoord2d(0.0f, 0.0f); glVertex3f(i-max_x,j-max_x,-1);
				glTexCoord2d(1.0f, 0.0f); glVertex3f(i-max_x+1,j-max_x,-1);
				glTexCoord2d(1.0f, 1.0f); glVertex3f(i-max_x+1,j-max_x+1,-1);
				glTexCoord2d(0.0f, 1.0f); glVertex3f(i-max_x,j-max_x+1,-1);	
			glEnd();
		}
	}
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0f, 0.0f, 1.0f);
	glLineWidth(3);
	for (int i=0; i<NumFaces; i++) {
		glBegin(GL_LINE_LOOP);
			for (int j=0; j<texCoord[i].size(); j++) {
				glVertex2f(texCoord[i][j].first, texCoord[i][j].second);
			}
		glEnd();	
	}
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}




void display(){
	int x, y, w, h;
	if (showEditor) {
		if (width>=2*height) {
			dx = floor((width-2*height)/2.0);
			dy = 0;
		//	w=2*height;
			h=height;
		} else {
			dx = 0;
			dy = floor((height-width/2)/2.0);
			w=width;
			h=w/2;
		}
		glViewport(dx+h,dy,h,h);
		drawPolyhedron();
		glViewport(dx,dy,h, h);
		drawEditor(); 
	} else {
		glViewport(max((width-height)/2,0), max(0, (height-width)/2), min(width,height), min(width, height));
		drawPolyhedron();
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
	width=w;
//	glViewport(210, 0, h, h);
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

void initializeTexture() {
	myPixelArray = LoadBMP(imageFile);
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
	sourceFace = 0;
	facesModelview.resize(NumFaces);
	facesProjection.resize(NumFaces);
	open=false;
	angle=0.0f;
	faceChange=true;
	initializeTexCoord();
	glutPostRedisplay();
}

void searchMenu(int option) {
	switch (option){
		case 0:
			searchType=BFS;
			break;
		case 1:
			searchType=DFS;
			break;
	}
	open=false;
	angle=0.0f;
	faceChange=true;
	glutPostRedisplay();
}

void textureMenu(int option) {
	switch(option) {
		case 0:
			imageFile=(char *)"bananas.bmp";
			break;
		case 1:
			imageFile=(char *)"arvore-de-natal.bmp";
			break;
		case 2:
			imageFile=(char *)"papai-noel.bmp";
			break;
	}
	initializeTexture();
	glutPostRedisplay();
}

void mainMenu(int option){
	if (!option && !showEditor) {
		showEditor=true;
	} else if(!option && showEditor) {
		showEditor=false;
	}
	glutPostRedisplay();
}

void createMenu() {
	int menu, submenu1, submenu2, submenu3;
	
	submenu1 = glutCreateMenu(polyhedronMenu);
	glutAddMenuEntry("Tetraedro",0);
	glutAddMenuEntry("Hexaedro",1);
	glutAddMenuEntry("Octaedro",2);
	glutAddMenuEntry("Dodecaedro",3);
	glutAddMenuEntry("Icosaedro",4);
	
	submenu2 = glutCreateMenu(searchMenu);
	glutAddMenuEntry("BFS", 0);
	glutAddMenuEntry("DFS", 1);
	
	submenu3 = glutCreateMenu(textureMenu);
	glutAddMenuEntry("Bananas", 0);
	glutAddMenuEntry("Árvore de Natal", 1);
	glutAddMenuEntry("Papai Noel",2);
	
	menu = glutCreateMenu(mainMenu);
	glutAddSubMenu("Poliedro",submenu1);
	glutAddSubMenu("Tipo de busca",submenu2);
	glutAddSubMenu("Textura", submenu3);
	glutAddMenuEntry("Mostrar/Esconder editor", 0);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

vector<pair<int, int> > getVertex(int x, int y) {
	vector<pair<int, int> > ret;
	for (int i=0; i<texCoord.size(); i++) {
		for (int j=0; j<texCoord[i].size(); j++) {
			int vx=(texCoord[i][j].first*(3.0/max_x)+3)*height/6.0+dx;
			int vy=(-texCoord[i][j].second*(3.0/max_y)+3)*height/6.0+dy;
		//	cout << vx << " " << vy << endl;
			if (vx<=x+5 && vx>=x-5 && vy<=y+5 && vy>=y-5) {
		//		cout << "OK" << endl;
				ret.push_back(make_pair(i, j));
			}
		}
	}
	return ret;
}

pair<GLfloat, GLfloat> windowCoordinates(myCoordinates c, int face) {
	int h=min(height, width/2);
	vector<vector<GLfloat> > modelview=facesModelview[face];
	vector<vector<GLfloat> > projection=facesProjection[face];
	vector<vector<GLfloat> > vertex=coToVe(c, 1);
	vertex=multMatrix(modelview, vertex);
	vertex=multMatrix(projection, vertex);
//	printm(modelview);
//	printm(projection);
	for (int i=0; i<4; i++) {
		vertex[i][0]/=vertex[3][0];
	}
	pair<GLfloat, GLfloat> ret=make_pair(vertex[0][0], vertex[1][0]);
	ret.first=(showEditor ? dx+h+(ret.first+1)*height/2.0 : max((width-height)/2,0)+(ret.first+1)*min(width,height)/2.0);
	ret.second=(showEditor ? height-(dy+(ret.second+1)*height/2.0) : height-(max(0, (height-width)/2)+(ret.second+1)*min(width, height)/2.0));
//	glViewport(max((width-height)/2,0), max(0, (height-width)/2), min(width,height), min(width, height));
	return ret;
	
	
}

int index(int x, int y) {
	Polyhedron p(type);
	vector<myCoordinates> vertices=p.vertices;
	vector<vector<int> > faces=p.faces;
	pair<GLfloat, GLfloat> wc;
//	vector<int> vfaces;
	for (int i=0; i<NumFaces; i++) {
		int sum=0;
		for (int j=0; j<p.faces[i].size(); j++) {
			int next=(j==faces[i].size()-1 ? 0 : j+1);
			wc=windowCoordinates (vertices[faces[i][j]], i);
			vector<vector<GLfloat> > p = coToVe(myCoordinates(wc.first, wc.second, 0),1);
			wc=windowCoordinates (vertices[faces[i][next]], i);
			vector<vector<GLfloat> > q = coToVe(myCoordinates(wc.first, wc.second, 0),1);
			vector<vector<GLfloat> > r = coToVe(myCoordinates(x,y,0),1);
		//	cout << "OK" << endl;
			sum+=Orientation3p(p,q,r);
		}
		if (sum==-faces[i].size()) {
			return i;
		}
	}
	return -1;
}

void myMouse(int b, int s, int x, int y) {
	if (b==GLUT_LEFT_BUTTON) {
		if (!showEditor || (x>height)) {
			if (s==GLUT_UP) {
			//	cout << windowCoordinates(myCoordinates(0.5, 0.5, 0.5),0).first << " " << windowCoordinates(myCoordinates(0.5, 0.5, 0.5),0).second << endl;
			//	cout << "Real: " << x << " " << y << endl;
			//    cout << index(x,y) << endl;
				if (open) {
					open=false;
					glutPostRedisplay();
				} else {
					int i=index(x,y);
					open=(i>=0);
					update=true;
					faceChange=(i!=sourceFace);
					sourceFace=(i>=0 ? i : sourceFace);
					glutPostRedisplay();
				}
			}
		}
		else {
			vector<pair<int, int> > v=getVertex(x, y);
			if (v.size()>0 && s==GLUT_DOWN) {
				movingVertices=v;
			} else {
				movingVertices.clear();
			//	cout << x << " " << y << endl;
			}
		}
	}
}

void myMotion(int x, int y) {
	
	for (int i=0; i<movingVertices.size(); i++) {
		cout << "OK" << endl;
	//	cout << x << " " << y << endl;
		GLfloat new_x=(max_x/3.0)*(((x-dx)*6)/(float)height-3);
		GLfloat new_y=-(max_y/3.0)*(((y-dy)*6)/(float)height-3);
		cout << "Vertice " << new_x << " " << new_y << endl;
	    cout << max_x << " " << min_x << " " << max_y << " " << min_y << endl;
		texCoord[movingVertices[i].first][movingVertices[i].second].first=(max(min(new_x, (float)max_x), (float)min_x));
		texCoord[movingVertices[i].first][movingVertices[i].second].second=(max(min(new_y, (float)max_y), (float)min_y));
		glutPostRedisplay();
	}
}



void myKeyboard(unsigned char key, int x, int y ) {
	if (key=='1'){
		type=TETRAHEDRON;
		open=false;
		NumFaces=4;
		angle=0.0f;
		faceChange=true;
		sourceFace = 0;
	} else if (key=='2') {
		type=HEXAHEDRON;
		open=false;
		NumFaces=6;
		angle=0.0f;
		faceChange=true;
		sourceFace = 0;
	} else if (key=='3') {
		type=OCTAHEDRON;
		open=false;
		NumFaces=8;
		angle=0.0f;
		faceChange=true;
		sourceFace = 0;
	} else if (key=='4') {
		type=DODECAHEDRON;
		open=false;
		NumFaces=12;
		angle=0.0f;
		faceChange=true;
		sourceFace = 0;
	} else if (key=='5') {
		type=ICOSAHEDRON;
		open=false;
		NumFaces=20;
		angle=0.0f;
		faceChange=true;
		sourceFace = 0;
	}
	initializeTexCoord();
	facesModelview.resize(NumFaces);
	facesProjection.resize(NumFaces);
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
	initializeTexture();
	initializeTexCoord();
	createMenu();
}
 
int main(int argc, char* argv[]){
 
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(1080,540);
  glutCreateWindow("Poliedros regulares");
  glEnable(GL_DEPTH_TEST);
  glutDisplayFunc(display);
  glutSpecialFunc(specialKeys);
  glutMouseFunc(myMouse);
  glutMotionFunc(myMotion);
  glutReshapeFunc(myReshape);
  glutTimerFunc(50, myTimer, 0);
  glutKeyboardFunc(myKeyboard);
  initializations();
  glutMainLoop();
  return 0;
 
}
