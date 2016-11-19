#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>
#include <cmath>
#define TETRAHEDRON 1
#define HEXAHEDRON 2
#define OCTAHEDRON 3
#define DODECAHEDRON 4
#define ICOSAHEDRON 5



using namespace std;

class myCoordinates {
public:
	GLfloat x;
	GLfloat y;
	GLfloat z;
	myCoordinates(){
		
	}
	myCoordinates(GLfloat Cx, GLfloat Cy, GLfloat Cz) {
		x=Cx;
		y=Cy;
		z=Cz;
	}
	
	myCoordinates(const myCoordinates & c) {
		x=c.x;
		y=c.y;
		z=c.z;
	}
	~myCoordinates() {
		
	};
	void operator=(const myCoordinates & a) {
		this->x+=a.x;
		this->y+=a.y;
		this->z+=a.z;
	}
	void set(GLfloat x, GLfloat y, GLfloat z) {
		this->x=x;
		this->y=y;
		this->z=z;
	}
	friend ostream & operator <<(ostream & s, const myCoordinates & z) {
		s << "[" << z.x << ", " << z.y << ", " << z.z << "]"; 
		return s;
	}
};

myCoordinates sub(myCoordinates a, myCoordinates b) {
    	myCoordinates ret;
    	ret.x=a.x-b.x;
    	ret.y=a.y-b.y;
    	ret.z=a.z-b.z;
    	return ret;
}

myCoordinates crossProduct(myCoordinates u, myCoordinates v) {
	myCoordinates ret;
	ret.x = u.y*v.z - u.z*v.y;
	ret.y = u.z*v.x - u.x*v.z;
	ret.z = u.x*v.y - u.y*v.x;
	return ret;
}

myCoordinates normalize(myCoordinates v) {
	myCoordinates ret;
	GLfloat mod=sqrt(pow(v.x,2) + pow(v.y,2) + pow(v.z,2));
	ret.x=v.x/mod;
	ret.y=v.y/mod;
	ret.z=v.z/mod;
	return ret;
}

GLfloat determinant(vector<vector<GLfloat> > m) {
	if (m.size()==1) {
		return m[0][0];
	}
	GLfloat sum=0;
	for (int i=0; i<m.size(); i++) {
		vector<vector<GLfloat> > m2=m;
		m2.erase(m2.begin()+i);
		for (int j=0; j<m2.size(); j++) {
			m2[j].erase(m2[j].begin());
		}
		sum+=m[i][0]*pow(-1,i+2)*determinant(m2);
	}
	return sum;
}

vector<vector<GLfloat> > transpose(vector<vector<GLfloat> > m) {
	vector<vector<GLfloat> > ret(m[0].size());
	for (int i=0; i<ret.size(); i++) {
		ret[i].resize(m.size());
		for (int j=0; j<m.size(); j++) {
			ret[i][j]=m[j][i];
		}
	}
	return ret;
}

vector<vector<GLfloat> > inverse(vector<vector<GLfloat> > m) {
	vector<vector<GLfloat> > c(m.size());
	for (int i=0; i<c.size(); i++) {
		c[i].resize(m.size());
		for (int j=0; j<c.size(); j++) {
			vector<vector<GLfloat> > m2=m;
			m2.erase(m2.begin()+i);
			for (int k=0; k<m2.size(); k++) {
				m2[k].erase(m2[k].begin()+j);
			}
			cout << m2.size() << " " << m2[0].size() << endl;
			c[i][j]=determinant(m2)*pow(-1,i+j+2);
		}
		
	}
	vector<vector<GLfloat> > ret;
	ret=transpose(c);
	GLfloat det=determinant(m)
	;for (int i=0; i<ret.size(); i++) {
		for (int j=0; j<ret.size(); j++) {
			ret[i][j]/=det;
		}
	}
	return ret;
}

vector<vector<GLfloat> > multMatrix(vector<vector<GLfloat> > a, vector<vector<GLfloat> > b) {
	vector<vector<GLfloat> > ret(a.size());
	for (int i=0; i<a.size(); i++) {
		ret[i].resize(b[0].size());
		for(int j=0; j<b[0].size(); j++) {
			GLfloat sum=0;
			for (int k=0; k<b.size(); k++) {
				sum+=a[i][k]*b[k][j];
			}
			ret[i][j]=sum;
		}
	}
	return ret;
}

class Polyhedron {
public:
	vector<myCoordinates> vertices;
	vector<vector<int> > faces;
	vector<vector<int> > edges;
	vector<vector<int> > spanningTree;
	vector<vector<pair<GLdouble, GLdouble> > > texCoord;
	GLfloat angle;
	
	int isInVector(int v1, int v2, vector<vector<int> > v) {
		for (int i=0; i<v.size(); i++) {
			if ((v1==v[i][0] && v2==v[i][1]) || (v2==v[i][0] && v1==v[i][1])) {
				return i;
			} 
		}
		return -1;
	}
	bool isInVector(int v1, vector<int> v) {
		for (int i=0; i<v.size(); i++) {
			if (v1==v[i]) {
				return true;
			} 
		}
		return false;
	}
	int edgePosition(int f1, int f2, vector<vector<int> > v) {
		for (int i=0; i<v.size(); i++) {
			if ((f1==v[i][2] && f2==v[i][3]) || (f2==v[i][2] && f1==v[i][3])) {
				return i;
			} 
		}
		return -1;
	}
	
	int previous(vector<int> v, int i) {
		if (i==0) {
			return v.back();
		} else {
			return v[i-1];
		}
	}
	
	
	void generateEdges() {
		for (int i=0; i<faces.size(); i++) {
			for (int j=0; j<faces[i].size(); j++) {
				if (j==faces[i].size()-1){
					int test=isInVector(faces[i][j],faces[i][0], edges);
					if (test==-1){
						int e[]={faces[i][j],faces[i][0],i,-1};
						vector<int> edge(e,e+sizeof(e)/sizeof(int));
						edges.push_back(edge);
					} else {
						edges[test][3]=i;
					}
				} else {
					int test=isInVector(faces[i][j], faces[i][j+1], edges);
					if (test==-1) {
						int e[]={faces[i][j],faces[i][j+1],i,-1};
						vector<int> edge(e,e+sizeof(e)/sizeof(int));
						edges.push_back(edge);
					} else {
						edges[test][3]=i;
					}
					
				}
			}
		}
	}
	
	Polyhedron (int type) {
		switch (type)
		{
			case TETRAHEDRON:
				{
					texCoord.resize(4);
					for (int i=0; i<4; i++) {
						texCoord[i].resize(3);
					}
					angle = 180-acos(1.0/3)*180.0/M_PI;
					vertices.push_back(myCoordinates(0.5, 0.5, 0.5));
					vertices.push_back(myCoordinates( -0.5,  -0.5, 0.5));
					vertices.push_back(myCoordinates(-0.5,  0.5, -0.5));
					vertices.push_back(myCoordinates(0.5,  -0.5, -0.5));
					
					int f[][3]={{2,  1,  0},
					   {0,  3,  2},
					   {1,  3,  0},
					   {2,  3,  1}};
					for (int i=0; i<4; i++) {
						vector<int> face(f[i], f[i]+sizeof(f[i])/sizeof(int));
						faces.push_back(face);
					}
				}
				generateEdges();
				break;
			case HEXAHEDRON:
				{
					texCoord.resize(6);
					for (int i=0; i<6; i++) {
						texCoord[i].resize(4);
					}
					angle = 90;
					vertices.resize(8);
					vertices[0].set(0.5, -0.5, 0.5);
					vertices[1].set(0.5, 0.5, 0.5);
					vertices[2].set(-0.5, 0.5, 0.5);
					vertices[3].set(-0.5, -0.5, 0.5);
					vertices[4].set(0.5, -0.5, -0.5);
					vertices[5].set(0.5, 0.5, -0.5);
					vertices[6].set(-0.5, 0.5, -0.5);
					vertices[7].set(-0.5, -0.5, -0.5);
					
					int f[][4]={{0,1,2,3},
					   {7,6,5,4},
					   {4,5,1,0},
					   {3,2,6,7},
					   {1,5,6,2},
					   {4,0,3,7}};
					for (int i=0; i<6; i++) {
						vector<int> face(f[i], f[i]+sizeof(f[i])/sizeof(int));
						faces.push_back(face);
					}
					generateEdges();
					break;
				}
			case OCTAHEDRON:
				{
					texCoord.resize(8);
					for (int i=0; i<8; i++) {
						texCoord[i].resize(3);
					}
					angle = 180-acos(-1.0/3)*180/M_PI;
					vertices.push_back(myCoordinates(1.0, 0.0, 0.0));
					vertices.push_back(myCoordinates( -1.0,  0.0, 0.0));
					vertices.push_back(myCoordinates(0.0,  1.0, 0.0));
					vertices.push_back(myCoordinates(0.0,  -1.0, 0.0));
					vertices.push_back(myCoordinates(0.0, 0.0, 1.0));
					vertices.push_back(myCoordinates( 0.0,  0.0, -1.0));
					
					int f[][3]={{0,  2,  4},
					   {0,  4,  3},
					   {0,  3,  5},
					   {0,  5,  2},
					   {1,  2,  5},
					   {1,  5,  3},
					   {1,  3,  4},
					   {1,  4,  2}};
					for (int i=0; i<8; i++) {
						vector<int> face(f[i], f[i]+sizeof(f[i])/sizeof(int));
						faces.push_back(face);
					}
					generateEdges();
					break;
					
				}
			case DODECAHEDRON:
				{
					texCoord.resize(12);
					for (int i=0; i<12; i++) {
						texCoord[i].resize(5);
					}
					angle = 180-acos(-1.0/sqrt(5))*180/M_PI;
					GLfloat t=(1.0 + sqrt(5.0)) / 2.0; 
					GLfloat r=1.0/t;
					vertices.push_back(myCoordinates(- 0.5, - 0.5, - 0.5)); 
					vertices.push_back(myCoordinates(- 0.5, - 0.5,  0.5));
					vertices.push_back(myCoordinates(- 0.5,  0.5, - 0.5));
					vertices.push_back(myCoordinates(- 0.5,  0.5,  0.5));
					vertices.push_back(myCoordinates(0.5, - 0.5, - 0.5));  
					vertices.push_back(myCoordinates(0.5, - 0.5,  0.5));
					vertices.push_back(myCoordinates(0.5,  0.5, - 0.5));   
					vertices.push_back(myCoordinates( 0.5,  0.5,  0.5));
					vertices.push_back(myCoordinates(0, - r/2, - t/2));  
					vertices.push_back(myCoordinates(0, - r/2,  t/2));
					vertices.push_back(myCoordinates(0,  r/2, - t/2));    
					vertices.push_back(myCoordinates(0,  r/2,  t/2));
					vertices.push_back(myCoordinates(- r/2, - t/2,  0)); 
					vertices.push_back(myCoordinates(- r/2,  t/2,  0));
					vertices.push_back(myCoordinates(r/2, - t/2,  0 ));  
					vertices.push_back(myCoordinates(r/2,  t/2,  0));
					vertices.push_back(myCoordinates(- t/2,  0, - r/2));  
					vertices.push_back(myCoordinates(t/2,  0, - r/2));
					vertices.push_back(myCoordinates(- t/2,  0,  r/2));  
					vertices.push_back(myCoordinates(t/2,  0,  r/2));
					
					int f[][5]={{3, 11,  7, 15, 13},
						{7, 19, 17, 6, 15},
						{17,  4,  8, 10, 6},
						{8,  0, 16,  2, 10},
						{0, 12,  1, 18, 16},
						{6, 10,  2, 13, 15},
						{2, 16, 18, 3, 13},
						{18,  1, 9, 11,  3},
						{4, 14, 12, 0, 8},
						{11, 9, 5, 19, 7},
						{19, 5, 14, 4, 17},
						{1, 12, 14, 5,  9}};
					
					for (int i=0; i<12; i++) {
						vector<int> face(f[i], f[i]+sizeof(f[i])/sizeof(int));
						faces.push_back(face);
					}
					
					generateEdges();
					break;
					
				}
			case ICOSAHEDRON:
				{
					texCoord.resize(20);
					for (int i=0; i<20; i++) {
						texCoord[i].resize(3);
					}
					GLfloat t=(1.0 + sqrt(5.0)) / 2.0; 
					GLfloat r=1.0/t;
					angle = 180-acos(-sqrt(5)/3.0)*180/M_PI;
					vertices.push_back(myCoordinates(-0.5,  t/2,  0));
					vertices.push_back(myCoordinates(0.5,  t/2,  0));
					vertices.push_back(myCoordinates(-0.5, -t/2,  0));
					vertices.push_back(myCoordinates(0.5, -t/2,  0));
					vertices.push_back(myCoordinates(0, -0.5,  t/2));
					vertices.push_back(myCoordinates(0,  0.5,  t/2));
					vertices.push_back(myCoordinates(0, -0.5, -t/2));
					vertices.push_back(myCoordinates(0,  0.5, -t/2));
					vertices.push_back(myCoordinates(t/2,  0, -0.5));
					vertices.push_back(myCoordinates(t/2,  0,  0.5));
					vertices.push_back(myCoordinates(-t/2,  0, -0.5));
					vertices.push_back(myCoordinates(-t/2,  0,  0.5));
					
					int f[][3]={{0, 11, 5},
						{0, 5, 1},
						{0, 1, 7},
						{0, 7, 10},
						{0, 10, 11},
						{1, 5, 9},
						{5, 11, 4},
						{11, 10, 2},
						{10, 7, 6},
						{7, 1, 8},
						{3, 9, 4},
						{3, 4, 2},
						{3, 2, 6},
						{3, 6, 8},
						{3, 8, 9},
						{4, 9, 5},
						{2, 4, 11},
						{6, 2, 10},
						{8, 6, 7},
						{9, 8, 1}};
					for (int i=0; i<20; i++) {
						vector<int> face(f[i], f[i]+sizeof(f[i])/sizeof(int));
						faces.push_back(face);
					}
					generateEdges();
					break;
					break;
					
				}
			default:
				break;
		}
	
	}
	void drawFace(int i, bool open) {
		GLfloat color[] = {0.0, 0.0, 1.0, 1.0};
		GLfloat white[]  ={1.0, 1.0, 1.0, 1.0};
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100);
		myCoordinates u=sub(vertices[faces[i][1]],vertices[faces[i][0]]);
		myCoordinates v=sub(vertices[faces[i][2]],vertices[faces[i][1]]);
		myCoordinates n=normalize(crossProduct(u,v));
//		GLfloat mv[16];
//		glGetFloatv(GL_MODELVIEW_MATRIX, mv);
//		vector<vector<GLfloat> > modelview(4);
//		for (int j=0; j<4; j++) {
//			modelview[j].resize(4);
//			modelview[j][0]=mv[0+j*4];
//			modelview[j][1]=mv[1+j*4];
//			modelview[j][2]=mv[2+j*4];
//			modelview[j][3]=mv[3+j*4];
//		}
//		vector<vector<GLfloat> > newn=normalize(multMatrix(modelview,n));
//		vector<vector<GLfloat> > v1=multMatriz(modelview, u);
//		vector<vector<GLfloat> > nv=multMatrix(newn,v1);
//		vector<vector<GLfloat> > p=multM
		
		glBegin(GL_POLYGON);
			for (int j=0; j<faces[i].size(); j++) {
//				vector<vector<GLfloat> > m(4);
//				for (int k=0; k<4; k++) {
//					m[k].resize(4)
//					switch (k) {
//						case 0:
//							{
//								m[k][0]=u.x;
//								m[k][1]=v1.x;
//								m[k][2]=n.x;
//								m[k][3]=vertices[face]
//							}
//					}
//				}
//				vector<vector<GLfloat> > tc=multMatrix(m,tv);
//				texCoord[i][j]=make_pair(tc[0], tc[1]);
//				GLdouble s = texCoord[i][j].first
//				GLdouble t = texCoord[i][j].second
				glNormal3f(n.x, n.y, n.z);
				glTexCoord2d(vertices[faces[i][j]].x+0.5,vertices[faces[i][j]].y+0.5);
				glVertex3f(vertices[faces[i][j]].x, vertices[faces[i][j]].y, vertices[faces[i][j]].z);
			}
		glEnd();
	}
	void bfs(int sourceFace) {
		vector<bool> marking(faces.size());
		vector<vector<int> > adjacencyVector(faces.size());
		spanningTree.resize(faces.size());
		for (int i=0; i<faces.size(); i++){
			marking[i]=0;
			spanningTree[i].clear();
		}
		queue<int> vqueue;
		marking[sourceFace]=1;
		vqueue.push(sourceFace);
		for (int i=0; i<edges.size(); i++) {
			if (!isInVector(edges[i][2], adjacencyVector[edges[i][3]])) {
				adjacencyVector[edges[i][3]].push_back(edges[i][2]);
			}
			if (!isInVector(edges[i][3], adjacencyVector[edges[i][2]])) {
				adjacencyVector[edges[i][2]].push_back(edges[i][3]);
			}
		}
		while(!vqueue.empty()){
			int v=vqueue.front();
			vqueue.pop();
			for (int i=0; i<adjacencyVector[v].size(); i++){
				int x=adjacencyVector[v][i];
				if (marking[x]==0){
					marking[x]=1;
					spanningTree[x].clear();
					spanningTree[x].insert(spanningTree[x].begin(),spanningTree[v].begin(), spanningTree[v].end());
					spanningTree[x].push_back(v);
					vqueue.push(x);
				}
			}
		}
		for (int i=0; i<spanningTree.size(); i++) {
			reverse(spanningTree[i].begin(), spanningTree[i].end());
		}
	}
	pair<myCoordinates, myCoordinates> getEdge(int face1, int face2) {
		int i=edgePosition(face1, face2, edges);
		for (int j=0; j<faces[face1].size(); j++) {
			if (faces[face1][j]==edges[i][0]) {
				if (previous(faces[face1], j)==edges[i][1]) {
					return make_pair(vertices[edges[i][0]], vertices[edges[i][1]]);
				} else {
					return make_pair(vertices[edges[i][1]], vertices[edges[i][0]]);
				}
			}
		}
	}
	vector<vector<pair<myCoordinates, myCoordinates> > > getTransformations () {
		vector<vector<pair<myCoordinates, myCoordinates> > > ret(faces.size());
		int c=1;
		bool b=true;
		while (b) {
			b=false;
			for (int i=0; i<spanningTree.size(); i++) {
				 if (spanningTree[i].size()==c) {
					pair<myCoordinates, myCoordinates> edge=getEdge(spanningTree[i][0], i);
					pair<myCoordinates, myCoordinates> p(edge.first,sub(edge.second,edge.first));
					ret[i].push_back(make_pair(edge.first, sub(edge.second, edge.first)));
					if (c>1) {
						ret[i].insert(ret[i].end(), ret[spanningTree[i][0]].begin(), ret[spanningTree[i][0]].end());
					}
					b=true;	
				}
			}
			c+=1;
		}
		return ret;
	}
	GLfloat getAngle() {
		return angle;
	}
};
