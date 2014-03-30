///////////////////////////////////////////////////////////////////
// FILE:      main.cpp
// CONTAINS:  Modified version of main.cpp from Assignment 2, for demoing skinning.
///////////////////////////////////////////////////////////////////

#include "defs.h"
#include "TriangleMesh.h"
#include "MeshAnimation.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

#define Bone MeshAnimation::TBone

// Camera parameters
int windowWidth = 1024;         // window width (pixels)
int windowHeight = 768;         // window height (pixels)
float cameraFovy = 35.0;        // field of view  in the y-direction (degrees)
float cameraZnear = 1.0;        // near clipping plane (distance from camera)
float cameraZfar = 100.0;       // far clipping plane (distance from camera)
float cameraR = 30;             // camera position: distance to center (OpenGL units)
float cameraTheta = 0.25*3.14;  // camera position: angle theta (radians)
float cameraPhi = 0.25*3.14;    // camera position: angle phi(radians)
float cameraCenter[] = {0,0,0}; // where the camera is looking at (OpenGL units)
float cameraUp[] = {0,1,0};     // up vector of the camera (OpenGL units)

// Rendering parameters
TriangleMesh::MeshDrawStyle meshDrawStyle = TriangleMesh::SOLID;

// Animation time parameters
bool paused = false;   // is the animation paused?
float currentTime = 0; // current time of the animation (seconds)
//float maxTime = 3.0;   // maximum time (seconds). 'time' loops from 0 to maxTime
float deltaTime = 0.0; // elapsed time since last update (seconds)
int elapsedTime = 0;   // elapsed time since program started (milliseconds)
float dtScale = 1.0;   // scaling factor for timesteps in physical simulation

// Animation mode 
int mode = 0; 				// bind pose
int animation_id = 0;	// first animation clip

// My mesh and skeleton
TriangleMesh mesh;
MeshAnimation animation;
string sceneFile;
string skeletonFile;

bool linearBlend = false;
int closestMeshIndex = -1;
std::vector<float> weights;

extern void initScene();
extern void updateScene();
extern void loadScene();
Vector3 getBoneHead(int boneId);
Vector3 getBoneTail(int boneId);
extern void computeVertexBoneWeight();
extern float closestDistance(Vector3 boneStart, Vector3 boneEnd, Vector3 vertex);
extern float computeDistance(Vector3 p1, Vector3 p2);
extern void computeDeformedMesh();
extern Vector3 convertToWorldCoordinateFromBoneCoordinate(Vector3 boneVector, MeshAnimation::TBone &bone);
extern Vector3 convertToBoneCoordinateFromWorldCoordinate(Vector3 worldVector, MeshAnimation::TBone &bone);

///////////////////////////////////////////////////////////////////
// FUNC:  init()
// DOES:  initializes default light source and surface material
///////////////////////////////////////////////////////////////////

void init(void) 
{
   glClearColor (0.8f, 0.8f, 0.8f, 1.0);
   glShadeModel (GL_SMOOTH);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_COLOR_MATERIAL);
   glEnable(GL_NORMALIZE);

   initScene();
   updateScene();
}

///////////////////////////////////////////////////////////////////
// FUNC: initScene()
// DOES: initialize the scene
///////////////////////////////////////////////////////////////////

void initScene()
{
  currentTime = 0;     // reset time

  loadScene();         // read scene description
	
  computeVertexBoneWeight(); // compute weight of bones on each vertex

  switch(mode) {   // mode-specific initialization
  case 0:
    break;
  case 1:
    break;
  case 2:
    break;
  case 3:
		break;
  case 4:
    break;
  }
}

///////////////////////////////////////////////////////////////////
// FUNC: loadScene()
// DOES: read in mesh and skeleton
///////////////////////////////////////////////////////////////////

void loadScene()
{
// TODO: remove to automate this
//    ifstream myfile(sceneFile.c_str());
//    if (!myfile.is_open()) {
//        cerr << "Unable open scene: " << sceneFile << endl;
//        return;
//    }
//    mesh.readFromOBJ(sceneFile.c_str());
    mesh.readFromOBJ("meshes/simplebear.obj");
	
	// read in mesh skeleton
	animation.LoadSkeletonXML("skeletons/org_mapped.skeleton.xml");
    
    //    ifstream myanimationfile(skeletonFile.c_str());
    //    if (!myanimationfile.is_open()) {
    //        cerr << "Unable open skeleton: " << skeletonFile << endl;
    //        return;
    //    }
    // animation.LoadSkeletonXML(skeletonFile.c_str());
	
}

///////////////////////////////////////////////////////////////////
// FUNC: getBoneHead
// DOES: compute head position of bone in world coordinate
///////////////////////////////////////////////////////////////////

Vector3 getBoneHead(int boneId)
{
	vec3f head = animation.bones[boneId].matrix*vec3f(0,0,0);
	return Vector3(head[0],head[1],head[2]);
}

///////////////////////////////////////////////////////////////////
// FUNC: getBoneTail
// DOES: compute tail position of bone.
// 			 If bone has child, use child's head as the tail,
//			 otherwise use "pos" to fake it.
///////////////////////////////////////////////////////////////////

Vector3 getBoneTail(int boneId)
{
	Bone bone = animation.bones[boneId];
	if(bone.childs.size() > 0){
		return getBoneHead(bone.childs[0]);
	}
	else{
		vec3f tail = animation.bones[boneId].matrix*vec3f(bone.pos[0], bone.pos[1], bone.pos[2]);
		return Vector3(tail[0],tail[1],tail[2]);
	}
}

///////////////////////////////////////////////////////////////////
// FUNC: computeVertexBoneWeight()
// DOES: compute weight of bones on each mesh vertex
///////////////////////////////////////////////////////////////////

void computeVertexBoneWeight()
{
    //for (int i = 0; i < animation.bones.size(); i++) {
    //MeshAnimation::TBone b = animation.bones.at(i);
    int index = animation.GetBoneIndexOf("Bone.001_L.002");
    MeshAnimation::TBone b = animation.bones.at(index);
    
    
        cout << b.name << endl;

    Vector3 boneHead = getBoneHead(index);
    boneHead.print("Head coordinate is ");
    Vector3 boneTail = getBoneTail(index);
    boneTail.print("Tail coordinate is ");
    
    if (!linearBlend) {
        
        //double closestDist = 20;
        for (int i=0; i < mesh.vertices.size(); i++) {
            Vector3 meshV = mesh.vertices.at(i);
            float dist = closestDistance(boneHead, boneTail, meshV);
            
            cout << "Distance " << dist << endl;
            
            weights.push_back(dist);
        }
        //cout << "Closest distance is " << closestDist << endl;
    }
//    }
    
    // Here for testing purposes
//    Vector3 bone1 = Vector3(1, 1, 0);
//    Vector3 bone2 = Vector3(2, 1, 0);
//    Vector3 vertex1 = Vector3(1.5, 1, 0);
//    
//    Vector3 vertex2 = Vector3(1, 2, 0);
//    Vector3 vertex3 = Vector3(2, 2, 0);
//    
//    Vector3 vertex4 = Vector3(1, -1, 0);
//    Vector3 vertex5 = Vector3(1, 1, 5);
//    
//    
//    cout << closestDistance(bone1, bone2, vertex1) << endl;
//    cout << closestDistance(bone1, bone2, vertex2) << endl;
//    cout << closestDistance(bone1, bone2, vertex3) << endl;
//    cout << closestDistance(bone1, bone2, vertex4) << endl;
//    cout << closestDistance(bone1, bone2, vertex5) << endl;
}

///////////////////////////////////////////////////////////////////
// FUNC: convertWorldCoordinateFromBoneCoordinate()
// DOES: Given a boneVector in bone space, compute the coordinate in world space
///////////////////////////////////////////////////////////////////

Vector3 convertToWorldCoordinateFromBoneCoordinate(Vector3 boneVector, MeshAnimation::TBone &bone) {
    vec3f boneVec = vec3f(boneVector[0], boneVector[1], boneVector[2]);
    vec3f result = bone.matrix * boneVec;
    
    return Vector3(result[0],result[1],result[2]);
}

///////////////////////////////////////////////////////////////////
// FUNC: convertBoneCoordinateFromWorldCoordinate()
// DOES: Given a vector in world space, compute the coordinate to bone space
///////////////////////////////////////////////////////////////////

Vector3 convertToBoneCoordinateFromWorldCoordinate(Vector3 worldVector, MeshAnimation::TBone &bone) {
    vec3f worldVec = vec3f(worldVector[0], worldVector[1], worldVector[2]);
    vec3f result = bone.invbindmatrix * worldVec;
    
    return Vector3(result[0],result[1],result[2]);
}

///////////////////////////////////////////////////////////////////
// FUNC: closestPoint()
// DOES: Given a bone and a vertex, returns shortest distance from point to the bone segment
///////////////////////////////////////////////////////////////////
float closestDistance(Vector3 boneStart, Vector3 boneEnd, Vector3 vertex) {
    // Use this formula
    Vector3 v = boneEnd - boneStart;
    Vector3 w = vertex - boneStart;
    
    double c1 = w.dot(v);
    if (c1 <= 0) {
        return computeDistance(vertex, boneStart);
    }
    double c2 = v.dot(v);
    if (c2 <= c1) {
        return computeDistance(vertex, boneEnd);
    }
    
    double c = c1 / c2;
    Vector3 closestPoint = boneStart + (c * v);
    return computeDistance(vertex, closestPoint);
}

// Distance function between two points
float computeDistance(Vector3 p1, Vector3 p2) {
    float x = pow(p2[0] - p1[0], 2);
    float y = pow(p2[1] - p1[1], 2);
    float z = pow(p2[2] - p1[2], 2);
    return sqrt(x + y + z);
}

///////////////////////////////////////////////////////////////////
// FUNC: computeDeformedMesh()
// DOES: compute new location of all vertices based on the current skeleton
//			 using linear blend skinning
///////////////////////////////////////////////////////////////////

void computeDeformedMesh()
{ 
  // mesh size
	int meshSize = mesh.vertices.size();
    
	// compute and update coords of mesh vertices based on bone positions
    int index = animation.GetBoneIndexOf("Bone.001_L.002");
    MeshAnimation::TBone b = animation.bones.at(index);
    
    for (int i = 0; i < mesh.vertices.size(); i++) {
        Vector3 meshV = mesh.vertices.at(i);
        Vector3 meshInBone = convertToBoneCoordinateFromWorldCoordinate(meshV, b);
        meshInBone.print("MeshInBone is ");
        float normalizedWeight = 1 / sqrt(pow(weights[i], 2));
        Vector3 pDeformed = normalizedWeight * convertToWorldCoordinateFromBoneCoordinate(meshInBone, b);
        
        pDeformed.print("New Point is ");
        
        mesh.vertices.at(i) = pDeformed;
    }

}

///////////////////////////////////////////////////////////////////
// FUNC: updateScene()
// DOES: update the location of all objects/vertices in the scene, as a function of Time
///////////////////////////////////////////////////////////////////

void updateScene()
{
  switch(mode) {
  case 0:					// no motion
    break;
  case 1:   
		animation.SetPose(animation_id, currentTime);		// set skeleton pose
		computeDeformedMesh();     							// now compute the deformed mesh
    break;
  case 2:        
    break;
  case 3:
  case 4:
    break;
  }

}

///////////////////////////////////////////////////////////////////
// FUNC: drawScene()
// DOES: draw the scene as updated by the function updateScene()
///////////////////////////////////////////////////////////////////

void drawScene()
{
  // Draw ground plane
  glColor3f(0.5,0.5,0.9);
  glNormal3f(0,1,0);
  glBegin(GL_POLYGON);
  {
    glVertex3f(-10,-0.101,-10);
    glVertex3f(-10,-0.101,10);
    glVertex3f(10,-0.101,10);
    glVertex3f(10,-0.101,-10);
  }
  glEnd();

  // Draw mesh
  glColor3f(0.7,0.5,0.1);   // brownish color
  mesh.draw(meshDrawStyle);
	
	// Draw skeleton
	//animation.DrawSkeleton();
	glColor3f(1.0, 0.0, 0.0);
	for (int i = 0; i < animation.bones.size(); i++){
		Vector3 head = getBoneHead(i);
		Vector3 tail = getBoneTail(i);
		
		glBegin(GL_LINES);
		glVertex3f(head[0], head[1], head[2]);
		glVertex3f(tail[0], tail[1], tail[2]);
		glEnd();
	}
	
}

///////////////////////////////////////////////////////////////////
// FUNC:  displayCallback()
// DOES:  clears screen, draws the world, swaps buffers
///////////////////////////////////////////////////////////////////

void displayCallback(void)
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // clear draw buffer
  glMatrixMode(GL_MODELVIEW);  // setup ModelView matrix for the desired camera position
  glLoadIdentity();
  float eye [] = 	{ 
		cameraCenter[0] + cameraR * std::sin(cameraTheta) * std::sin(cameraPhi),
		cameraCenter[1] + cameraR * std::cos(cameraTheta),
		cameraCenter[2] + cameraR * std::sin(cameraTheta) * std::cos(cameraPhi)  	};
  gluLookAt(eye[0],eye[1],eye[2],cameraCenter[0],cameraCenter[1],cameraCenter[2],cameraUp[0],cameraUp[1], cameraUp[2]);
  drawScene();        // draw scene
  glutSwapBuffers();  // swap buffers
}

///////////////////////////////////////////////////////////////////
// FUNC:  keyCallback()
// DOES:  handles glut keyboard events
///////////////////////////////////////////////////////////////////

void keyCallback(unsigned char key, int x, int y)
{
  unsigned int i,n;

  if( (key >= '0') && (key <= '9') ) {
    mode = (int) key - (int) '0';
    initScene();
    updateScene();
  }
  switch(key) {
  case 'q': 
    exit(0);
    break;
  case 'r':
    currentTime = 0; // set global time to zero
    paused = false;  // run animation
    break;
  case ' ':
    paused = !paused;
    break;
  case 'm':
    ++meshDrawStyle;   // cycle through mesh draw styles
    break;
  case 'p':
    mesh.print();
    break;
  default:
    break;
  }
  glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////
// FUNC:  specialKeyCallback()
// DOES:  called for non-ASCII key strokes, e.g., arrow keys
///////////////////////////////////////////////////////////////////

void specialKeyCallback(int key, int x, int y)
{
  switch(key) {
  case GLUT_KEY_RIGHT: 
    cameraPhi += 3.14 / 16;
    break;
  case GLUT_KEY_LEFT: 
    cameraPhi -= 3.14 / 16;
    break;
  case GLUT_KEY_UP: 
    cameraTheta -= 3.14 / 16;
    if(cameraTheta < 0.01)
      cameraTheta = 0.01;
    break;
  case GLUT_KEY_DOWN: 
    cameraTheta += 3.14 / 16;
    if(cameraTheta > 3.14)
      cameraTheta = 3.14;
    break;
  case GLUT_KEY_PAGE_UP: 
    cameraR /= 1.2;
    break;
  case GLUT_KEY_PAGE_DOWN: 
    cameraR *= 1.2;
    break;
  default:
    break;
  }
  glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////
// FUNC:  myReshape()
// DOES:  this is called if the on-screen window is resized
///////////////////////////////////////////////////////////////////

void myReshape(int w, int h)
{
   windowWidth = w;
   windowHeight = h;
   glViewport(0, 0, w, h);
   glMatrixMode(GL_PROJECTION); 
   glLoadIdentity();
   gluPerspective(cameraFovy, (GLfloat) w/h, 1.0, 10000.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

///////////////////////////////////////////////////////////////////
// FUNC:  idleCallback()
// DOES:  glut calls this when idle;  used for animation
///////////////////////////////////////////////////////////////////

void idleCallback()
{
  if(paused)    {
    elapsedTime = glutGet(GLUT_ELAPSED_TIME);        // Update elapsed time
    return;
  }
  int newElapsedTime = glutGet(GLUT_ELAPSED_TIME);  // Compute new time and update elapsed time
  deltaTime = 0.001*dtScale*(newElapsedTime - elapsedTime);
  elapsedTime = newElapsedTime;
  currentTime += deltaTime;
  //if (currentTime>=maxTime)   currentTime = 0.0;
  updateScene();         // update scene
  glutPostRedisplay();   // draw scene
}

///////////////////////////////////////////////////////////////////
// FUNC:  main()
// DOES:  intializes glut window, calls init(), then hands over control to glut
///////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    if (argc==3) {
        sceneFile = argv[1];
        skeletonFile = argv[2];
    }
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutCreateWindow("CPSC 426 Jan 2014:  Assmt 2");
   glutReshapeFunc(myReshape);
   glutDisplayFunc(displayCallback);
   glutKeyboardFunc(keyCallback);
   glutSpecialFunc(specialKeyCallback);
   glutIdleFunc(idleCallback);
   init();
   glutMainLoop();
   return 0;
}



