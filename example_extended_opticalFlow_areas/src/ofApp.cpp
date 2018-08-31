#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::addRoiAverage(float x, float y, float w, float h) {
	ofRectangle	myNewRoi;
	myNewRoi.set(x, y, w, h);
	myAvgFlowRois.push_back(myNewRoi); 
}
//
////--------------------------------------------------------------
void ofApp::removeRoiAverage(int id) {

	myAvgFlowRois.pop_back(); // TODIO use id
}

//--------------------------------------------------------------
void ofApp::setup(){
	
	// ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_NOTICE);
	
	densityWidth = 1280;
	densityHeight = 720;
	// process all but the density on 16th resolution
	flowWidth = densityWidth / 4;
	flowHeight = densityHeight / 4;
	windowWidth = ofGetWindowWidth();
	windowHeight = ofGetWindowHeight();
	
	opticalFlow.setup(flowWidth, flowHeight);

	averageFlow.setup(32, 32, FT_VELOCITY);
	//averageFlow.setRoi(.2, .2, .2, .2);

	//Setup several Averages.
	addRoiAverage(0.1, 0.1, 0.25, 0.25);
	addRoiAverage(0.3, 0.5, 0.2, 0.3);
	addRoiAverage(0.6, 0.7, 0.15, 0.2);

	flows.push_back(&opticalFlow);
	flows.push_back(&averageFlow);

	for (auto flow : flows) { flow->setVisualizationFieldSize(glm::vec2(flowWidth / 2, flowHeight / 2)); }

	simpleCam.setup(densityWidth, densityHeight, true);
	cameraFbo.allocate(densityWidth, densityHeight);
	ftUtil::zero(cameraFbo);
	
	lastTime = ofGetElapsedTimef();
	
	setupGui();
}

//--------------------------------------------------------------
void ofApp::setupGui() {
	
	gui.setup("settings");
	gui.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));
	gui.setDefaultFillColor(ofColor(160, 160, 160, 160));
	gui.add(guiFPS.set("average FPS", 0, 0, 60));
	gui.add(guiMinFPS.set("minimum FPS", 0, 0, 60));
	gui.add(toggleFullScreen.set("fullscreen (F)", false));
	toggleFullScreen.addListener(this, &ofApp::toggleFullScreenListener);
	gui.add(toggleGuiDraw.set("show gui (G)", true));
	gui.add(toggleCameraDraw.set("draw camera (C)", true));
	gui.add(toggleMouseDraw.set("draw mouse (M)", true));
	gui.add(toggleParticleDraw.set("draw particles (P)", true));
	gui.add(toggleAverageDraw.set("draw average (A)", true));
	toggleParticleDraw.addListener(this, &ofApp::toggleParticleDrawListener);
	gui.add(toggleReset.set("reset (R)", false));
	toggleReset.addListener(this, &ofApp::toggleResetListener);
	
	visualizationParameters.setName("visualization");
	visualizationParameters.add(visualizationMode.set("mode", FLUID_DEN, INPUT_FOR_DEN, FLUID_DEN));
	visualizationParameters.add(visualizationName.set("name", "fluidFlow Density"));
	visualizationParameters.add(visualizationScale.set("scale", 1, 0.1, 10.0));
	visualizationParameters.add(toggleVisualizationScalar.set("show scalar", false));
	visualizationMode.addListener(this, &ofApp::visualizationModeListener);
	toggleVisualizationScalar.addListener(this, &ofApp::toggleVisualizationScalarListener);
	visualizationScale.addListener(this, &ofApp::visualizationScaleListener);
	
	bool s = true;
	switchGuiColor(s = !s);
	gui.add(visualizationParameters);
	for (auto flow : flows) {
		switchGuiColor(s = !s);
		gui.add(flow->getParameters());
	}
	
	if (!ofFile("settings.xml")) { gui.saveToFile("settings.xml"); }
	gui.loadFromFile("settings.xml");
	
//	gui.minimizeAll();
	minimizeGui(&gui);
	
	toggleGuiDraw = true;
}

//--------------------------------------------------------------
void ofApp::switchGuiColor(bool _switch) {
	ofColor guiHeaderColor[2];
	guiHeaderColor[0].set(160, 160, 80, 200);
	guiHeaderColor[1].set(80, 160, 160, 200);
	ofColor guiFillColor[2];
	guiFillColor[0].set(160, 160, 80, 200);
	guiFillColor[1].set(80, 160, 160, 200);
	
	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[_switch]);
	gui.setDefaultFillColor(guiFillColor[_switch]);
}

//--------------------------------------------------------------
void ofApp::minimizeGui(ofxGuiGroup* _group) {
	for (int i=0; i< _group->getNumControls(); i++) {
		ofxGuiGroup * subGroup  = dynamic_cast<ofxGuiGroup*>(_group->getControl(i));
		if (subGroup) {
			minimizeGui(subGroup);
			_group->minimizeAll();
		}
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	float dt = 1.0 / max(ofGetFrameRate(), 1.f); // more smooth as 'real' deltaTime.
	
	simpleCam.update();
	if (simpleCam.isFrameNew()) {
		cameraFbo.begin();
		simpleCam.draw(cameraFbo.getWidth(), 0, -cameraFbo.getWidth(), cameraFbo.getHeight());  // draw flipped
		cameraFbo.end();
		
		opticalFlow.setInput(cameraFbo.getTexture());
	}


	opticalFlow.update();
	
	//for (int i = 0; i < myAverageFlowVector.size(); i++) {
	/*averageFlow.setInput(opticalFlow.getVelocity());*/
	//averageFlow.update();
	//}

}

//--------------------------------------------------------------
void ofApp::draw(){
	
	ofClear(0,0);
	
	ofPushStyle();
	if (toggleCameraDraw.get()) {
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		cameraFbo.draw(0, 0, windowWidth, windowHeight);
	}

	
	////averageFlow.drawOutput(averageFlow.getRoi().x*ofGetWidth(), averageFlow.getRoi().y*ofGetHeight(), averageFlow.getRoi().width*ofGetWidth(), averageFlow.getRoi().height*ofGetHeight());
	//ofDrawBitmapStringHighlight("getStDevMagnitude -> " + ofToString(averageFlow.getStDevMagnitude(), 3), 700, 30);
	//ofDrawBitmapStringHighlight("getMeanMagnitude -> " + ofToString(averageFlow.getMeanMagnitude(), 3), 700, 60);

	if (toggleAverageDraw) {
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		for (int i = 0; i < myAvgFlowRois.size(); i++) {
			averageFlow.setInput(opticalFlow.getVelocity());
			averageFlow.setRoi(myAvgFlowRois[i]);
			averageFlow.update();
				
			averageFlow.drawInput(myAvgFlowRois[i].x*densityWidth, myAvgFlowRois[i].y*densityHeight, myAvgFlowRois[i].width*densityWidth, myAvgFlowRois[i].height*densityHeight);
			averageFlow.drawVisualizer(myAvgFlowRois[i].x*densityWidth, myAvgFlowRois[i].y*densityHeight, myAvgFlowRois[i].width*densityWidth, myAvgFlowRois[i].height*densityHeight);

			ofDrawBitmapStringHighlight("getStDevMagnitude -> " + ofToString(averageFlow.getStDevMagnitude(), 3), myAvgFlowRois[i].x*densityWidth, myAvgFlowRois[i].y*densityHeight - 20);
			ofDrawBitmapStringHighlight("getComponent 0 -> " + ofToString(averageFlow.getComponent(0), 3), myAvgFlowRois[i].x*densityWidth, myAvgFlowRois[i].y*densityHeight - 60);
			ofDrawBitmapStringHighlight("getComponent 1 -> " + ofToString(averageFlow.getComponent(1), 3), myAvgFlowRois[i].x*densityWidth, myAvgFlowRois[i].y*densityHeight - 40);

		}
	}
	
	if (toggleGuiDraw) {
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		drawGui();
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawGui() {
	guiFPS = (int)(ofGetFrameRate() + 0.5);
	
	// calculate minimum fps
	float deltaTime = ofGetElapsedTimef() - lastTime;
	lastTime = ofGetElapsedTimef();
	deltaTimeDeque.push_back(deltaTime);
	
	while (deltaTimeDeque.size() > guiFPS.get())
		deltaTimeDeque.pop_front();
	
	float longestTime = 0;
	for (int i=0; i<deltaTimeDeque.size(); i++){
		if (deltaTimeDeque[i] > longestTime)
			longestTime = deltaTimeDeque[i];
	}
	
	guiMinFPS.set(1.0 / longestTime);
	
	
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	gui.draw();
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
		default: break;
		case '1': visualizationMode.set(INPUT_FOR_DEN); break;
		case '2': visualizationMode.set(INPUT_FOR_VEL); break;
		case '3': visualizationMode.set(FLOW_VEL); break;
		case '4': visualizationMode.set(BRIDGE_VEL); break;
		case '5': visualizationMode.set(BRIDGE_DEN); break;
		case '6': visualizationMode.set(FLUID_VORT); break;
		case '7': visualizationMode.set(FLUID_TMP); break;
		case '8': visualizationMode.set(FLUID_PRS); break;
		case '9': visualizationMode.set(FLUID_VEL); break;
		case '0': visualizationMode.set(FLUID_DEN); break;
		case 'G':toggleGuiDraw = !toggleGuiDraw; break;
		case 'F': toggleFullScreen.set(!toggleFullScreen.get()); break;
		case 'C': toggleCameraDraw.set(!toggleCameraDraw.get()); break;
		case 'M': toggleMouseDraw.set(!toggleMouseDraw.get()); break;
		case 'R': toggleReset.set(!toggleReset.get()); break;
		case 'P': toggleParticleDraw.set(!toggleParticleDraw.get()); break;
		case 'A': toggleAverageDraw.set(!toggleAverageDraw.get()); break;
			break;
	}
}

//--------------------------------------------------------------
void ofApp::toggleResetListener(bool& _value) {
	if (_value) {
		for (auto flow : flows) { flow->reset(); }
		fluidFlow.addObstacle(flowToolsLogo.getTexture());
	}
	_value = false;
}
