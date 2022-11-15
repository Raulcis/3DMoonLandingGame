#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include <glm/gtx/intersect.hpp>
#include "Particle.h"
#include "ParticleEmitter.h"
#include "CubeMap.h"

class SkyBox{
public:

	CubeMap* cubeMap = new CubeMap();
	ofShader* cubeshader = new ofShader();

	//loads skybox information
	// Written by Raul cisneros
	void SkyBox::load() {
		cubeshader->load("skybox");
		cubeMap->loadImages("images/x.png", "images/y.png", "images/z.png", "images/nex.png", "images/ney.png", "images/nez.png");

	}

	void SkyBox::draw() {

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


		glEnable(GL_TEXTURE_CUBE_MAP_ARB);

		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		cubeshader->begin();
		//glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		cubeMap->bind();

		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		cubeshader->setUniform1i("EnvMap", 0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();


		Draw_Skybox(0,-5, 0, 600, 600, 600);

		cubeshader->end();

		glDisable(GL_TEXTURE_CUBE_MAP_ARB);

		glDisable(GL_DEPTH_TEST);
		glPopMatrix();

	}

	void SkyBox::Draw_Skybox(float x, float y, float z, float width, float height, float length) {

		// Center the Skybox around the given x,y,z position
		x = x - width / 2;
		y = y - height / 2;
		z = z - length / 2;

		// Draw Front side

		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);
		glEnd();

		// Draw Back side

		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
		glEnd();

		// Draw Left side

		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
		glEnd();

		// Draw Right side

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
		glEnd();

		// Draw Up side

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
		glEnd();

		// Draw Down side

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y, z + length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z);
		glEnd();
	}
};
class LunarLander {
public:
	LunarLander() {  };
	LunarLander(string model);
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 addedThrust;
	glm::vec3 gravity;
	ofVec3f impulseForce;
	ofVec3f forces;
	float rotation;
	float turningVel;
	float turningAcc;
	float thrust;
	float damping;
	float fuel;
	bool landerSelected;
	bool landerSpawned;
	bool landed = false;
	ofxAssimpModelLoader Model;
	Box BoundingBox;

	// Functions
	void integrate();
	void integrateTurning();
	void setPos(glm::vec3 newPos);
	void setRot() { if (LanderSpawned()) Model.setRotation(0, this->rotation, 0.0, 1.0, 0.0);}
	bool LanderSelected() { return landerSelected; }
	bool LanderSpawned() { return landerSpawned; }
	Box LanderBoundingBox() { if (LanderSpawned()) return BoundingBox; }
	ofxAssimpModelLoader getModel() { if (LanderSpawned()) return Model; }
	glm::vec3 getPos() { return position; }
	void addForces() { forces = gravity + addedThrust + impulseForce; }
	//keeps bounding box updated
	void updateBoundingBox() {
		if (LanderSpawned()) {
			glm::vec3 min = getModel().getSceneMin() + getPos();
			glm::vec3 max = getModel().getSceneMax() + getPos();

			BoundingBox = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}
	
};


class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent2(ofDragInfo dragInfo);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void drawAxis(ofVec3f);
	void initLightingAndMaterials();
	void savePicture();
	void toggleWireframeMode();
	void togglePointsDisplay();
	void toggleSelectTerrain();
	void setCameraTarget();
	bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point);
	bool raySelectWithOctree(ofVec3f& pointRet);
	glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p, glm::vec3 n);
	void lightSetUp();
	void setUpCameras();
	void checkCollisions();
	bool hitDetection(ofVec3f&);
	void UI();
	void gameOverScreen();
	void startScreen();
	void landerSetUp(LunarLander* lander);
	void setEmitters();
	void ofApp::loadVbo();

	ofEasyCam cam;

	//octree variables
	ofxAssimpModelLoader moon, lander;
	ofLight light;
	Box boundingBox, landerBounds;
	Box testBox;
	vector<Box> colBoxList;
	bool bLanderSelected = false;
	Octree octree;
	TreeNode selectedNode;
	glm::vec3 mouseDownPos, mouseLastPos;
	bool bInDrag = false;
	ofxIntSlider numLevels;
	
	ofxPanel gui;

	//octree variables
	bool bAltKeyDown;
	bool bCtrlKeyDown;
	bool bWireframe;
	bool bDisplayPoints;
	bool bPointSelected;
	bool bHide;
	bool pointSelected = false;
	bool bDisplayLeafNodes = false;
	bool bDisplayOctree = false;
	bool bDisplayBBoxes = false;
	bool bLanderLoaded;
	bool bTerrainSelected;
	ofVec3f selectedPoint;
	ofVec3f intersectPoint;
	vector<Box> bboxList;
	const float selectionRange = 4.0;


	//creates sky box and lander for game
	SkyBox* skybox = new SkyBox();
	LunarLander* lander2 = new LunarLander("geo/lander.obj");

	// all game variables
	ofLight keyLight, fillLight, rimLight;
	ofxVec3Slider keyPos; 
	ofxVec3Slider fillPos;
	ofxVec3Slider rimPos;
	Box landingZone;
	bool gameStarted = false;
	bool gameOver = false;
	bool inBounds = false;
	bool showNearest;
	bool exploded;
	bool altitudeOn = true;
	bool sound = false;
	bool pickStartPos = false;
	ofCamera* theCam;
	ofCamera bottom;
	ofCamera follow;
	ofCamera front;
	ofCamera ground;
	ofCamera trailer;
	float altitude;
	string fuelIndicator;
	string altitudeIndicator;
	ofTrueTypeFont text;
	ParticleEmitter emitter;
	ParticleEmitter explosion;
	ofSoundPlayer missionFailed;
	ofSoundPlayer explosionSound;
	ofSoundPlayer rockets;
	int fuel;
	int altitudeRounded;

	// textures
	ofTexture particleTex;
	// shaders
	ofVbo vbo;
	ofShader shader;
};