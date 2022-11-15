
//--------------------------------------------------------------
//  Student Name:  Raul Cisneros
//  Date: 12/2/2021


#include "ofApp.h"
#include "Util.h"



//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
void ofApp::setup() {

	// disable rectangular textures
	ofDisableArbTex();

	// load the shader
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif

	// load all needed game assests
	text.loadFont("fonts/Oswald-Light.ttf", 15);
	ofLoadImage(particleTex, "images/dot.png");
	missionFailed.load("sounds/missionFailed.mp3");
	explosionSound.load("sounds/explosion.mp3");
	rockets.load("sounds/rocket.mp3");
	skybox->load();
	
	// Sets up values for terrain and display
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bTerrainSelected = true;
	
	//set easyCam
	cam.setPosition(ofVec3f(200, 100, 200));
	cam.lookAt(ofVec3f(0, 0, 0));
	cam.setDistance(100);
	cam.setNearClip(.1);
	cam.setFov(65.5);

	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();
	ofEnableLighting();

	theCam = &cam;

	//sets up all other usable cameras
	setUpCameras();

	// setup rudimentary lighting 
	lightSetUp();
	initLightingAndMaterials();

	//loads and creates moon envirorment
	moon.loadModel("geo/moon-houdini.obj");
	moon.setScaleNormalization(false);
	boundingBox = Octree::meshBounds(moon.getMesh(0));

	// create sliders for testing
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(keyPos.setup("KeyLight Position", ofVec3f(40, 15, 20), ofVec3f(-500, -500, -500), ofVec3f(500, 500, 500)));
	gui.add(fillPos.setup("FillLight Position", ofVec3f(-85, 25, 80), ofVec3f(-500, -500, -500), ofVec3f(500, 500, 500)));
	gui.add(rimPos.setup("RimLight Position", ofVec3f(0, 10, -60), ofVec3f(-500, -500, -500), ofVec3f(500, 500, 500)));
	bHide = false;
	
	//sets up all emitters for game
	setEmitters();

	//  Create Octree for testing.
	octree.create(moon.getMesh(0), 20);
	//creates lander
	landerSetUp(lander2);
	// creates landing zone
	landingZone = Box(Vector3(-24, -1, -18), Vector3(21, 16, 27));
}

//--------------------------------------------------------------
// incrementally update scene (animation)

void ofApp::update() {

	if (gameOver == false){

		// Update positioning of lights
		keyLight.setPosition(keyPos);
		fillLight.setPosition(fillPos);
		rimLight.setPosition(rimPos);

		// Updates position of lander2 bounding box
     	lander2->updateBoundingBox();

		// Checks if lander2 is in bounds of valid landing area
		if (lander2->BoundingBox.overlap(landingZone)) {
			inBounds = true;
		}
		else {
			inBounds = false;
		}

		if (gameStarted == true) {
			// Adds Impulse Force for ground collision
			checkCollisions();

			//physics movement of ship
			lander2->integrate();

			//physics rotation of ship
			lander2->integrateTurning();
		}

		// Update cameras position relavtive to lander
		if (lander2->LanderSpawned()) {
			bottom.setPosition(lander2->getPos());
			front.setPosition(ofVec3f(lander2->getPos().x, lander2->getPos().y + 5, lander2->getPos().z - 5));
			follow.setPosition(ofVec3f(lander2->getPos().x, lander2->getPos().y, lander2->getPos().z + 40));
			follow.lookAt(ofVec3f(lander2->getPos().x, lander2->getPos().y, lander2->getPos().z));
			ground.lookAt(ofVec3f(lander2->getPos().x, lander2->getPos().y, lander2->getPos().z));
			trailer.lookAt(ofVec3f(lander2->getPos().x, lander2->getPos().y, lander2->getPos().z));
			trailer.setPosition(lander2->getPos() + glm::vec3(-15, 0, 10));
		}

		// Updates the altitude variable
		ofVec3f p;
		hitDetection(p);
		altitude = lander2->getPos().y - p.y;

		//updates exhaust emitter
		emitter.setPosition(ofVec3f(lander2->getPos().x, lander2->getPos().y + 2.5, lander2->getPos().z));
		emitter.setOneShot(true);
		emitter.setVelocity(ofVec3f(0, -25, 0));
		emitter.update();

		//updates explosion emitter
		explosion.setPosition(ofVec3f(lander2->getPos().x, lander2->getPos().y + 2.5, lander2->getPos().z));
		explosion.setOneShot(true);
		explosion.setVelocity(ofVec3f(0, -25, 0));
		explosion.update();

		// Checks if game is over
		if (lander2->landed)
			gameOver = true;
		if (exploded)
			lander2->thrust = 0;
	}
}
//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(ofColor::black);
	
	// show start screen
	if (gameOver == false && pickStartPos == false) {
		startScreen();
	}
	else if (gameOver != true ){

		// draws gui
		glDepthMask(false);
		if (!bHide) gui.draw();
		glDepthMask(true);

		theCam->begin();
		ofFill();
		// draws skybox background
		skybox->draw();
		ofPushMatrix();

	//	keyLight.draw();
	//	fillLight.draw();
	//	rimLight.draw();

		ofSetColor(ofColor::red);
		ofNoFill();

		//draws box around landing zone
		Octree::drawBox(landingZone);

		if (bWireframe) {                    // wireframe mode  (include axis)
			ofDisableLighting();
			ofSetColor(ofColor::slateGray);
			moon.drawWireframe();
			if (lander2->LanderSpawned()) {
				lander2->getModel().drawWireframe();
				if (!bTerrainSelected) drawAxis(lander2->getPos());
			}
			if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
		}
		else {
			ofEnableLighting();              // shaded mode
			moon.drawFaces();
			ofMesh mesh;
			if (lander2->landerSpawned == true) {
				lander2->getModel().drawFaces();
				if (!bTerrainSelected) drawAxis(lander2->getPos());
				if (bDisplayBBoxes) {
					ofNoFill();
					ofSetColor(ofColor::white);
					for (int i = 0; i < lander2->getModel().getNumMeshes(); i++) {
						ofPushMatrix();
						ofMultMatrix(lander2->getModel().getModelMatrix());
						ofRotate(-90, 1, 0, 0);
						Octree::drawBox(bboxList[i]);
						ofPopMatrix();
					}
				}

				if (lander2->LanderSelected()) {
					ofVec3f min = lander2->getModel().getSceneMin() + lander2->getPos();
					ofVec3f max = lander2->getModel().getSceneMax() + lander2->getPos();

					Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
					ofSetColor(ofColor::white);
					Octree::drawBox(bounds);

					// draw colliding boxes
					ofSetColor(ofColor::blue);
					for (int i = 0; i < colBoxList.size(); i++) {
						Octree::drawBox(colBoxList[i]);
					}
				}
			}
		}
	//	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

		if (bDisplayPoints) {                // display points as an option    
			glPointSize(3);
			ofSetColor(ofColor::green);
			moon.drawVertices();
		}

		// highlight selected point (draw sphere around selected point)
		//
		if (bPointSelected) {
			ofSetColor(ofColor::blue);
			ofDrawSphere(selectedPoint, .1);
		}


		// recursively draw octree
		//
		ofDisableLighting();
		int level = 0;
		//	ofNoFill();

		if (bDisplayLeafNodes) {
			octree.drawLeafNodes(octree.root);
			cout << "num leaf: " << octree.numLeaf << endl;
		}
		else if (bDisplayOctree) {
			ofNoFill();
			ofSetColor(ofColor::white);
			//	cout << "levels" << numLevels << endl;
			octree.draw(numLevels, 0);
		}

		// if point selected, draw a sphere
		if (pointSelected) {
			ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
			ofVec3f d = p - cam.getPosition();
			ofSetColor(ofColor::lightGreen);
			//	ofDrawSphere(p, .02 * d.length());
		}

			// Draws Emitter for exhaust
		loadVbo();
		glDepthMask(GL_FALSE);
		ofSetColor(ofColor::red);
		
		// makes everything look glowy
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofEnablePointSprites();
		
		// begin shader
		shader.begin();
		
		// draw exhaust particle emitter
		particleTex.bind();
		vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
		particleTex.unbind();
		
		// end drawing
		shader.end();
		ofDisablePointSprites();
		ofDisableBlendMode();
		ofEnableAlphaBlending();
		glDepthMask(GL_TRUE);

		// Draws Emitter for explosion
		explosion.draw();

		ofPopMatrix();
		theCam->end();
		
		// draws user interface
		UI();

	}else if (gameOver == true) {
		//draws game over screen
		gameOverScreen();
	}
}

// load vertex buffer in preparation for rendering
void ofApp::loadVbo() {
	if (emitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter.sys->particles.size(); i++) {
		points.push_back(emitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(5));
	}
	// upload the data to the vbo
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {
	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {
	switch (key) {
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'r':
		//sets values to begin game again 
		if (gameOver) {
			exploded = false;
			gameOver = false;
			inBounds = false;
			showNearest = false;
			gameStarted=false;
			pickStartPos = false;
			lander2->Model.loadModel("geo/lander.obj");
			lander2->Model.setScaleNormalization(false);
			lander2->thrust = 25.0;
			lander2->acceleration = glm::vec3(0, 0, 0);
			lander2->turningVel = 0;
			lander2->velocity = glm::vec3(0, 0, 0);
			lander2->rotation = 0;
			lander2->setPos(ofVec3f(-50, 30, -50));
			lander2->fuel = 200;
			lander2->landed = false;
			lander2->landerSelected = false;
		}
		break;
	case 's':
		savePicture();
		break;
	case 'T':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'd':
		// activate default view
		theCam = &cam;
		break;
	case 't':
		// activate bottom cam view
		theCam = &bottom;
		break;
	case 'f':
		// activate cam that follows lander
		theCam = &follow;
		break;
	case 'o':
		// activate front camera view
		theCam = &front;
		break;
	case 'g':
		// activate ground camera that follows lander
		theCam = &ground;
		break;
	case '1':
		// activate camera that follows lander
		theCam = &trailer;
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case ' ':
		if (lander2->fuel > 0 && exploded == false) {
			// Play rockets sound effect
			if (sound == false) {
				rockets.play();
				sound = true;
			}
			// Starts rocket emitter
			if (!emitter.started)
				emitter.start();
			lander2->addedThrust = lander2->thrust * glm::vec3(0, 1, 0);
			// Decreases fuel
			lander2->fuel--;
		}
		// Start game
		if (gameStarted == false) {
			gameStarted = true;
		}
		//checks to see if personal position is selected
		if (pickStartPos == false) {
			pickStartPos = true;
		}
		break;
	case OF_KEY_LEFT:
		// Move forward relative to Z-Axis 
		lander2->addedThrust = lander2->thrust * glm::vec3(0, 0, 1);
		break;
	case OF_KEY_RIGHT:
		// Move backward relative to Z-Axis 
		lander2->addedThrust = lander2->thrust * glm::vec3(0, 0,-1);
		break;
	case OF_KEY_UP:
		// Move forward relative to X-Axis 
		lander2->addedThrust = lander2->thrust * glm::vec3(1, 0, 0);
		break;
	case OF_KEY_DOWN:
		// Move backward relative to X-Axis 
		lander2->addedThrust = lander2->thrust * glm::vec3(-1, 0, 0);
		break;
	case 'z':
		// rotates left relative to Y-Axis 
		lander2->turningAcc = lander2->thrust * -3;
		break;
	case 'x':
		// rotates right relative to Y-Axis 
		lander2->turningAcc = lander2->thrust * 3;
		break;
	case 'm':
		// togles altitude  
		altitudeOn = !altitudeOn;
		break;
	case '0':
		//makes cam look at lander
		cam.lookAt(ofVec3f(lander2->getPos().x, lander2->getPos().y, lander2->getPos().z));
		break;
	case '2':
		// turn on personal position selection
		pickStartPos = true;
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {

	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {

	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	case OF_KEY_LEFT:
		// Stop applying thrust force
		lander2->addedThrust = lander2->thrust * glm::vec3(0, 0, 0);
		break;
	case OF_KEY_RIGHT:
		// Stop applying thrust force
		lander2->addedThrust = lander2->thrust * glm::vec3(0, 0, 0);
		break;
	case OF_KEY_UP:
		// Stop applying thrust force	
		lander2->addedThrust = lander2->thrust * glm::vec3(0,0,0);
		break;
	case OF_KEY_DOWN:
		// Stop applying thrust force
		lander2->addedThrust = lander2->thrust * glm::vec3(0,0,0);
	case ' ':
		// Stop applying thrust force and stops sound
		lander2->addedThrust = glm::vec3(0, 0, 0);
		rockets.stop();
		sound = false;
		break;
	case 'z':
		// stop rotation acceleration 
		lander2->turningAcc = lander2->turningAcc = 0;
		break;
	case 'x':
		// stop rotation acceleration 
		lander2->turningAcc = lander2->turningAcc = 0;
		break;
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {


}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (lander2->LanderSpawned()) {
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander2->getModel().getSceneMin() + lander2->getModel().getPosition();
		ofVec3f max = lander2->getModel().getSceneMax() + lander2->getModel().getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			lander2->landerSelected = true;
			ofPoint currentPos = lander2->getModel().getPosition();
			mouseDownPos = getMousePointOnPlane(currentPos, cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			lander2->landerSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f& pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = theCam->screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - theCam->getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag && lander2->LanderSpawned()) {

		glm::vec3 lander2Pos = lander2->getPos();

		glm::vec3 mousePos = getMousePointOnPlane(lander2Pos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;

		lander2Pos += delta;
		lander2->setPos(lander2Pos);
		mouseLastPos = mousePos;

		ofVec3f min = lander2->getModel().getSceneMin() + lander2->getModel().getPosition();
		ofVec3f max = lander2->getModel().getSceneMax() + lander2->getModel().getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
	

		/*if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}*/


	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{ 5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
}

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}


// Written by Raul cisneros
void ofApp::checkCollisions()
{
	// Checks if lander2 collides with ground
	colBoxList.clear();
	if (octree.intersect(lander2->BoundingBox, octree.root, colBoxList) && lander2->velocity.y < 0) {
		ofVec3f norm = ofVec3f(0, 1, 0);
		ofVec3f vel = lander2->velocity;

		// Sets lander2's impulse force
		lander2->impulseForce = 60 * (1.85) * ((-vel.dot(norm)) * norm);
		// Checks if lander2 is in bounds and below specific impulse force
		if (lander2->impulseForce.y < 500 && lander2->impulseForce.y > 0) {
			// sets values if the lander landed
			if (gameOver == false) {
				lander2->thrust = 0;
				lander2->landed = true;
				if (inBounds == false && exploded == false) {
					missionFailed.play();
				}
			}
		}
		// Checks if lander2's impulse force is above specific value
		else if (lander2->impulseForce.y > 700) {
			// if it is then it explodes 
			if (!explosion.started) {
				lander2->Model.clear();
				explosionSound.play();
				explosion.start();
				exploded = true;
				missionFailed.play();
			}
		}
	}
}

// Written by Raul cisneros
void ofApp::UI(){
	ofSetColor(ofColor::white);
	
	// Displays fuel amount
	fuel = std::round(lander2->fuel);
	fuelIndicator += "Fuel remaining: " + std::to_string(fuel);
	text.drawString(fuelIndicator, ofGetWindowWidth() - 225, 50);
	fuelIndicator.clear();

	// Displays altitude
	if (altitudeOn == true){
		altitudeRounded = std::round(altitude);
		altitudeIndicator += "Altitude: " + std::to_string(altitudeRounded);
		text.drawString(altitudeIndicator, ofGetWindowWidth() - 190, 75);
		altitudeIndicator.clear();
	}
}

// Written by Raul cisneros
void ofApp::gameOverScreen() {
	ofSetColor(ofColor::white);
		string Text, Text2;
		
		// depending on the ending of the game it will display the corresponding msg 
		if (exploded) {
			Text = "You lose! The ship landed too hard and exploded!";
			text.drawString(Text, ofGetWindowWidth() / 2 - text.stringWidth(Text) / 2, ofGetWindowHeight() / 2 - 20);
			Text2 = "Push R to try again";
			text.drawString(Text2, ofGetWindowWidth() / 2 - text.stringWidth(Text2) / 2, ofGetWindowHeight() / 2 + 20);
		}
		else if (inBounds) {
			Text = "You win! Perfect landing!";
			text.drawString(Text, ofGetWindowWidth() / 2 - text.stringWidth(Text) / 2, ofGetWindowHeight() / 2 - 20);
			Text2 = "Push R to try again";
			text.drawString(Text2, ofGetWindowWidth() / 2 - text.stringWidth(Text2) / 2, ofGetWindowHeight() / 2 + 20);
		}
		else if (!inBounds) {
			Text = "You lose! You landed in the wrong area!";
			text.drawString(Text, ofGetWindowWidth() / 2 - text.stringWidth(Text) / 2, ofGetWindowHeight() / 2 - 20);
			Text2 = "Push R to try again";
			text.drawString(Text2, ofGetWindowWidth() / 2 - text.stringWidth(Text2) / 2, ofGetWindowHeight() / 2 + 20);
	}
}

// Written by Raul cisneros
void ofApp::startScreen(){
	ofSetColor(ofColor::white);
	string Text, Text2, Text3, Text4;
	//displays controlers and cameras options
		Text = ("Press the spacebar to begin");
		text.drawString(Text, ofGetWindowWidth() / 2 - text.stringWidth(Text) / 2, ofGetWindowHeight() / 2 - 35);
		Text2 = ("Move with arrow keys and activate rockets with Spacebar, press z to rotate lander left, press x to rotate lander right");
		text.drawString(Text2, ofGetWindowWidth() / 2 - text.stringWidth(Text2) / 2, ofGetWindowHeight() / 2);
		Text3 = ("press d for default view, press t to view below the lander,press f to follow the lander with camera, press 0 to make camera look at lander");
		text.drawString(Text3, ofGetWindowWidth() / 2 - text.stringWidth(Text3) / 2, ofGetWindowHeight() / 2 + 35);
		Text4 = ("press o for view in front of lander, press g for ground view to lander, press m to turn on/off altitude sensor display, press c to enable freecam");
		text.drawString(Text4, ofGetWindowWidth() / 2 - text.stringWidth(Text4) / 2, ofGetWindowHeight() / 2 + 70);
}
//--------------------------------------------------------------
void ofApp::dragEvent2(ofDragInfo dragInfo) {

}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
	
}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

//Sets up all three lights
// Written by Raul cisneros
void ofApp::lightSetUp(){
	keyLight.setup();
	keyLight.enable();
	keyLight.setAreaLight(1,1);
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	keyLight.setSpecularColor(ofFloatColor(1, 1, 1));
	keyLight.rotate(45, ofVec3f(0, 1, 0));
	keyLight.rotate(-45, ofVec3f(1, 0, 0));
	keyLight.setPosition(keyPos);

	fillLight.setup();
	fillLight.enable();
	fillLight.setSpotlight();
	fillLight.setScale(.05);
	fillLight.setSpotlightCutOff(15);
	fillLight.setAttenuation(2, .001, .001);
	fillLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
	fillLight.rotate(-10, ofVec3f(1, 0, 0));
	fillLight.rotate(-45, ofVec3f(0, 1, 0));
	fillLight.setPosition(fillPos);

	rimLight.setup();
	rimLight.enable();
	rimLight.setSpotlight();
	rimLight.setScale(.05);
	rimLight.setSpotlightCutOff(30);
	rimLight.setAttenuation(.2, .001, .001);
	rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
	rimLight.rotate(180, ofVec3f(0, 1, 0));
	rimLight.setPosition(rimPos);
}

// Written by Raul cisneros
void ofApp::setUpCameras(){

	// Shows perspective from side view of lander2
	follow.setNearClip(.1);
	follow.setFov(65.5);

	// Looks right below lander2
	bottom.setPosition(lander2->getPos());
	bottom.lookAt(ofVec3f(lander2->getPos().x, -1, lander2->getPos().z));
	bottom.setNearClip(.1);
	bottom.setFov(65.5);

	// Shows lander2 from position on ground near landing zone
	ground.setPosition(0, 5, 50);
	ground.setNearClip(.1);
	ground.setFov(65.5);

	// Shows front of lander2
	front.lookAt(glm::vec3(0, 0, -5));
	front.setNearClip(.1);
	front.setFov(65.5);

	//follows lander 
	trailer.setPosition(lander2->getPos() + glm::vec3(40,0,10));
	trailer.setNearClip(.1);
	trailer.setFov(65.5);

}

// shots ray from camera and uses octree to detect a hit
// Written by Raul cisneros
bool ofApp::hitDetection(ofVec3f& point)
{

	ofVec3f lander2ScreenPos(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2);
	ofVec3f rayPoint = bottom.screenToWorld(lander2ScreenPos);
	ofVec3f rayDir = rayPoint - bottom.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		point = octree.mesh.getVertex(selectedNode.points[0]);
		ofSetColor(ofColor::green);
		ofDrawLine(lander2->getPos(), point);
	}
	return pointSelected;
}

// Written by Raul cisneros
void ofApp::setEmitters() {
	// Sets Emitter for the landers rocket
	emitter.setRate(2.5);
	emitter.setLifespan(0.25);
	emitter.setParticleRadius(0.05);
	emitter.setEmitterType(DiscEmitter);
	emitter.setGroupSize(250);

	// Sets up Explosion Emitter
	explosion.setRate(2.5);
	explosion.setLifespan(1.0);
	explosion.setParticleRadius(0.05);
	explosion.setEmitterType(RadialEmitter);
	explosion.setGroupSize(1000);
}
//sets lander values
// Written by Raul cisneros
void ofApp::landerSetUp(LunarLander* lander2) {

	lander2->thrust = 25.0;
	lander2->gravity = ofVec3f(0, -8.0, 0);
	lander2->damping = 0.99;
	lander2->acceleration = glm::vec3(0, 0, 0);
	lander2->turningVel = 0;
	lander2->velocity = glm::vec3(0, 0, 0);
	lander2->turningVel = 0;
	lander2->turningAcc = 0;
	lander2->rotation = 0;
	lander2->setPos(ofVec3f(-30, 50, -30));
	lander2->fuel = 200;
	lander2->landerSelected = false;

}

//creates lander
// Written by Raul cisneros
LunarLander::LunarLander(string model){
	// loads ship model
	if (Model.loadModel(model))
		landerSpawned = true;
	Model.setScaleNormalization(false);

	// sets up bounding box
	ofVec3f min = getModel().getSceneMin() + getPos();
	ofVec3f max = getModel().getSceneMax() + getPos();
	BoundingBox = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

//physics of lander
// Written by Raul cisneros
void LunarLander::integrate()
{
		// update position
		this->setPos(this->getPos() + this->velocity * 1.0 / 60.0);
		// adds forces
		addForces();
		ofVec3f accel = acceleration + forces;
		// update velocity
		this->velocity = this->velocity + accel * (1.0 / 60.0);
		// multiply velocity by damping factor
		this->velocity = this->velocity * this->damping;
		impulseForce.set(0, 0, 0);
		forces.set(0, 0, 0);
	
}

//turning physics
// Written by Raul cisneros
void LunarLander::integrateTurning(){
	// update rotation
	this->rotation = this->rotation + this->turningVel * 1.0 / 60.0;
	this->setRot();
	// update velocity 
	this->turningVel = this->turningVel + this->turningAcc * (1.0 / 60.0);
	// multiply velocity by damping factor
	this->turningVel = this->turningVel * this->damping;
}
//sets position
// Written by Raul cisneros
void LunarLander::setPos(glm::vec3 newPos)
{
	if (LanderSpawned()) {
		position = newPos;
		Model.setPosition(newPos.x, newPos.y, newPos.z);
	}
}



