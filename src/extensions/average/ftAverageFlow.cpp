#include "ftAverageFlow.h"

namespace flowTools {
	
	int ftAverageFlow::areaCount = 0;
	
	//--------------------------------------------------------------
	void ftAverageFlow::setup(int _width, int _height, ftFlowForceType _type) {
		areaCount++;
		type = _type;
		GLint internalFormat = ftUtil::getInternalFormatFromType(type);
		ftFlow::allocate(_width, _height, internalFormat);
		roiFbo.allocate(_width, _height, internalFormat);
		ftUtil::zero(roiFbo);
		numChannels = ftUtil::getNumChannelsFromInternalFormat(internalFormat);
		
		direction.clear();
		direction.resize(numChannels, 0);
		components.clear();
		components.resize(numChannels, 0);
		prevComponents.clear();
		prevComponents.resize(numChannels, 0);
		
		inputPixels.allocate(inputWidth, inputHeight, numChannels);
		magnitudes.resize(inputWidth * inputHeight, 0);
		
		roi = ofRectangle(0,0,1,1);
		
		meanMagnitude = 0;
		normalizedMagnitude = 0;
		prevNormalizedMagnitude = 0;
		stdevMagnitude = 0;
		
		magnitudeColor = ofFloatColor(1, 1, 1, 1.);
		componentColors.push_back(ofFloatColor(.6, 1, .2, 1.));	// light green
		componentColors.push_back(ofFloatColor(.4, .8, 1, 1.));	// blue
		componentColors.push_back(ofFloatColor(.2, 1, .6, 1.));	// dark green
		componentColors.push_back(ofFloatColor(.8, .4, 1, 1.));	// purple
		outputFbo.allocate(16, 16);
		overlayFbo.allocate(16, 16);
		bUpdateVisualizer = false;
		
		string name = "average " + ftFlowForceNames[type];
		if (areaCount > 1) name += " " + ofToString(areaCount - 1);
		parameters.setName(name);
		parameters.add(pNormalizationMax.set("normalization", .025, .01, .1));
		parameters.add(pNormalizedMagnitude.set("magnitude", 0, 0, 1));
//		parameters.add(pStdevMagnitude.set("stdev mag", 0, 0, 1));
				
		pComponents.resize(numChannels);
		pDirection.resize(numChannels);
		if (numChannels > 1) {
			componentParameters.setName("components");
			for (int i=0; i<numChannels; i++) {
				componentParameters.add(pComponents[i].set(getComponentName(i), 0, -1, 1));
			}
			parameters.add(componentParameters);
		} else {
			parameters.add(pComponents[0].set(getComponentName(0), 0, -1, 1));
			parameters.add(pDirection[0].set(getComponentName(0), 0, -1, 1));
		}
		if (type == FT_VELOCITY_SPLIT) {
			parameters.add(pHighComponentBoost.set("boost directionality", 0, 0, 5));
		}	else { pHighComponentBoost.set(0); }
		
		roiParameters.setName("region of interest");
		pRoi.resize(4);
		roiParameters.add(pRoi[0].set("x", 0, 0, 1));
		roiParameters.add(pRoi[1].set("y", 0, 0, 1));
		roiParameters.add(pRoi[2].set("width", 1, 0, 1));
		roiParameters.add(pRoi[3].set("height", 1, 0, 1));
		for (int i=0; i<4; i++) {
			pRoi[i].addListener(this, &ftAverageFlow::pRoiListener);
		}
		parameters.add(roiParameters);
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::setInput(ofTexture &_tex){
		resetInput();
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		ftUtil::roi(inputFbo, _tex, roi);
		//reset subImages if sizes are differentor not allocated
		if (!roiFloatImageCV.bAllocated 
			/*|| roiFloatImageCV.getWidth() != roiColorImageCV.getWidth()
			|| roiFloatImageCV.getHeight() != inputPixels.getHeight()*/) {

			roiColorImageCV.allocate(inputWidth, inputWidth);
			auxImagePixes.allocate(inputWidth, outputHeight, OF_IMAGE_COLOR);

			roiFloatImageCV.allocate(outputWidth, outputHeight/*_tex.getWidth(), _tex.getHeight()*/); //_tex size not working. seems that 32x32 has been defined and that I what we have to use.

			cout << "Allocate images: " << endl;
			cout << "roiColorImageCV w= " << ofToString(roiColorImageCV.getWidth(), 0) + " h= " + ofToString(roiColorImageCV.getHeight(), 0) << endl;
			cout << "roiFloatImageCV w= " << ofToString(roiFloatImageCV.getWidth(), 0) + " h= " + ofToString(roiFloatImageCV.getHeight(), 0) << endl;
			cout << "auxImagePixes w= " << ofToString(auxImagePixes.getWidth(), 0) + " h= " + ofToString(auxImagePixes.getHeight(), 0) << endl;

		}
		ofPopStyle();
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::addInput(ofTexture &_tex, float _strength) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		ftUtil::zero(roiFbo);
		ftUtil::roi(roiFbo, _tex, roi);
		ofPopStyle();
		ftFlow::addInput(roiFbo.getTexture(), _strength);
	}

	//--------------------------------------------------------------
	void ftAverageFlow::addSubImages(ofRectangle subImageRect, int idSubImage) {
		subRoisVector.push_back(subImageRect);
	}

	//--------------------------------------------------------------
	void ftAverageFlow::removeSubImages(int idSubImage) {
		//TODO remove specific subRegion
		subRoisVector.pop_back();
	}

	//--------------------------------------------------------------
	void ftAverageFlow::update() {

		//Works pero peta cuando  UpdateRoi
		//ftUtil::toPixels(inputFbo, inputPixels);
		//roiFloatImageCV.setFromPixels(inputPixels);

		//ftUtil::toPixels(inputFbo, inputPixels);

		//1
		//inputFbo.readToPixels(roiColorImageCV.getPixels());
		//roiFloatImageCV = roiColorImageCV;

		//2
		inputFbo.readToPixels(auxImagePixes);
		roiColorImageCV.setFromPixels(auxImagePixes);
		roiFloatImageCV = roiColorImageCV;

		//roiFloatImageCV.setFromPixels(inputFbo.getP);

		//auxImagePixes.setFromPixels(inputPixels);
		//roiFloatImageCV.set ;
		//roiFloatImageCV.updateTexture();
		//Change here to use several ROI's reading just subImage
		//for (int i = 0; i < subRoisVector.size(); i++) {
		//	ofVec4f resultsRoi = updateForThisRoi(subRoisVector[i]);
		//}

	}
	//--------------------------------------------------------------
	ofVec4f ftAverageFlow::updateForThisRoi(ofRectangle _roi) {

		ofVec4f averageFlow_Analisys_Roi = ofVec4f(0, 0, 0, 0); //component 0, component 1, pNormalizedMagnitude, pStdevMagnitude 
		//Change here to use several ROI's reading just subImage
		roiFloatImageCV.setROI(ofRectangle(_roi.x*roiFloatImageCV.getWidth(), _roi.y*roiFloatImageCV.getHeight(), _roi.width*roiFloatImageCV.getWidth(), _roi.height*roiFloatImageCV.getHeight()));
		//roiFloatImageCV.updateTexture();

		//ofFloatPixels roiPixels = roiFloatImageCV.getPixels();
		//ofFloatPixels floatPixelData = roiFloatImageCV.getRoiFloatPixelsRef();

		//float* floatPixelData = inputPixels.getData();
		float* floatPixelData = roiFloatImageCV.getRoiPixelsAsFloats();
		
		vector<float> totalVelocity;
		totalVelocity.resize(numChannels, 0);
		
		int numPixels = roiFloatImageCV.getROI().getWidth() * roiFloatImageCV.getROI().getHeight();
		for (int i=0; i<numPixels; i++) {
			float mag = 0;
			for (int j=0; j<numChannels; j++) {
				float vel = floatPixelData[i * numChannels + j];
				totalVelocity[j] += vel;
				mag += vel * vel;
			}
			magnitudes[i] = sqrt(mag);
		}
		getMeanStDev(magnitudes, meanMagnitude, stdevMagnitude);
		
		normalizedMagnitude = meanMagnitude / pNormalizationMax.get();
		normalizedMagnitude = ofClamp(normalizedMagnitude, 0, 1);
		
		float totalMagnitude = 0;
		for (auto tv : totalVelocity) { totalMagnitude += tv * tv; }
		totalMagnitude = sqrt(totalMagnitude);
		
		for (int i=0; i<numChannels; i++) {
			direction[i] = totalVelocity[i] / totalMagnitude;
			components[i] = direction[i] * normalizedMagnitude;
		}
		
		// normalize to highest component and apply boost
		if (pHighComponentBoost.get() > 0 && numChannels > 1) {
			float highVelocity = 0;
			float P = 1;
			for (int i=0; i<numChannels; i++) {
				if (fabs(components[i]) > highVelocity) {
					highVelocity = fabs(components[i]);
					if (components[i] < 0) P = -1;
				}
			}
			for (int i=0; i<numChannels; i++) {
				components[i] /= highVelocity;
				components[i] = powf(fabs(components[i]), pHighComponentBoost.get()) * P;
				components[i] *= highVelocity;
			}
		}
		
		// use only 2 decimals
		for (int i=0; i<numChannels; i++) {
			pComponents[i] = int(components[i] * 100) / 100.0;
			pDirection[i] = int(direction[i] * 100) / 100.0;
		}
		pNormalizedMagnitude.set(int(normalizedMagnitude * 100) / 100.0);
		pStdevMagnitude.set(int(stdevMagnitude * 100) / 100.0);
		
		bUpdateVisualizer = true;

		//Return Desired data Updated
		for (int i = 0; i<numChannels; i++) {
			if (i == 0) {
				averageFlow_Analisys_Roi.x = components[i];
			}
			else if (i == 1) {
				averageFlow_Analisys_Roi.y = components[i];
			}
		}

		averageFlow_Analisys_Roi.z = pNormalizedMagnitude;
		averageFlow_Analisys_Roi.w = pStdevMagnitude;

		return averageFlow_Analisys_Roi;
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::drawOutput(int _x, int _y, int _w, int _h) {
		int x = _x + roi.x * _w;
		int y = _y + roi.y * _h;
		int w = roi.width * _w;
		int h = roi.height * _h;
		
		drawVisualizer(x, y, w, h);
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::drawROI(int _x, int _y, int _w, int _h) {
		int x = _x + roi.x * _w;
		int y = _y + roi.y * _h;
		int w = roi.width * _w;
		int h = roi.height * _h;
		
		drawBackground(x, y, w, h);
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::drawVisualizer(int _x, int _y, int _w, int _h) {
		ofPushStyle();

		drawBackground(_x, _y, _w, _h);
		drawGraph(_x, _y, _w, _h);

		ofSetColor(ofColor::red);
		roiFloatImageCV.draw(ofRectangle(_x, _y, _w, _h));

		ofSetColor(ofColor::white);
		//Update and Draw those results sub ROIS	- > TODO separate update from Draw
		ofFill();
		for (int i = 0; i < subRoisVector.size(); i++) {
			ofVec4f resultsRoi = updateForThisRoi(subRoisVector[i]);
			ofDrawBitmapStringHighlight("x["+ ofToString(resultsRoi.x,3) + "] y[" +ofToString(resultsRoi.y, 3) + "]" + " magnitude norm -> "+ofToString(resultsRoi.z, 3) + " pStdevMagnitude -> " + ofToString(resultsRoi.w, 3), _x, _y + 40*i);
		}

		ofNoFill();
		ofSetLineWidth(1);
		ofSetColor(ofColor::red);
		for (int i = 0; i < subRoisVector.size(); i++) {
			ofDrawRectangle(ofRectangle(_x + subRoisVector[i].x*_w, _y + subRoisVector[i].y*_h, subRoisVector[i].width*_w, subRoisVector[i].height*_h) );
			//ofDrawBitmapStringHighlight("x[" + ofToString(resultsRoi.x, 0) + "] y[" + ofToString(resultsRoi.y, 0) + "]" + " magnitude norm -> " + ofToString(resultsRoi.z, 0) + " pStdevMagnitude -> " + ofToString(resultsRoi.w, 0), ofGetMouseX(), ofGetMouseY() + 50 * i);
		}

		ofPopStyle();

	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::drawBackground(int _x, int _y, int _w, int _h) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(0, 0, 0, 63);
		ofDrawRectangle(_x, _y, _w, _h);
		ofNoFill();
		ofSetColor(0, 0, 0, 255);
		ofDrawRectangle(_x-1, _y-1, _w+2, _h+2);
		ofDrawBitmapStringHighlight(parameters.getName(), _x + _w * .5 - (parameters.getName().length() * 4), _y + 16);
		ofPopStyle();
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::drawGraph(int _x, int _y, int _w, int _h) {
		if (bUpdateVisualizer) {
			if (outputFbo.getWidth() != _w || outputFbo.getHeight() != _h) {
				outputFbo.allocate(_w, _h);
				ftUtil::zero(outputFbo);
				createGraphOverlay(_w, _h);
			}
			
			// graph
			ofPushStyle();
			outputFbo.swap();
			outputFbo.begin();
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			ofClear(0, 0, 0, 0);
			ofEnableBlendMode(OF_BLENDMODE_DISABLED);
			ofSetColor(255, 255, 255, 255);
			outputFbo.getBackTexture().draw(-4, 0);
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			
			int halfH = _h * .5;
			if (type == FT_VELOCITY_SPLIT) { halfH = (_h); }
			
			ofSetColor(magnitudeColor);
			ofDrawLine(_w - 4, (1 - prevNormalizedMagnitude) * halfH, _w, (1 - normalizedMagnitude) * halfH);
			ofDrawLine(_w - 4, 1 + (1 - prevNormalizedMagnitude) * halfH, _w, 1 + (1 - normalizedMagnitude) * halfH);
			prevNormalizedMagnitude = normalizedMagnitude;
			for (int i=0; i<numChannels; i++) {
				ofSetColor(componentColors[i]);
				ofDrawLine(_w - 4, (1 - prevComponents[i]) * halfH, _w, (1 - getComponent(i)) * halfH);
				prevComponents[i] = getComponent(i);
			}
			outputFbo.end();
			outputFbo.draw(_x, _y, _w, _h);
			
			// overlay
			overlayFbo.draw(_x, _y, _w, _h);
			
			ofPopStyle();
		}
		bUpdateVisualizer = false;
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::createGraphOverlay(int _w, int _h) {
		overlayFbo.allocate(_w, _h);
		ftUtil::zero(overlayFbo);
		
		overlayFbo.begin();
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255, 255, 255, 255);
		int yStep = 16;
		if (type != FT_VELOCITY_SPLIT) {
			ofDrawBitmapString("1",  _w - 10, yStep);
			ofDrawBitmapString("0",  _w - 10, (_h * 0.5) + yStep);
			ofDrawBitmapString("-1", _w - 18, _h - yStep * .5);
		} else {
			ofDrawBitmapString("1", _w - 10, yStep);
			ofDrawBitmapString("0", _w - 10, _h - yStep * .5);
		}
		
		int yOffset = yStep;
		ofSetColor(magnitudeColor);
		ofDrawBitmapString("magnitude",5, yOffset);
		yOffset += yStep;
		
		for (int i=0; i<numChannels; i++) {
			ofSetColor(componentColors[i]);
			ofDrawBitmapString(getComponentName(i), 5, yOffset);
			yOffset += yStep;
		}
		ofPopStyle();
		
		overlayFbo.end();
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::setRoi(ofRectangle _rect) {
		float x = _rect.x;
		float y = _rect.y;
		float maxW = 1.0 - x;
		float maxH = 1.0 - y;
		float w = min(_rect.width, maxW);
		float h = min(_rect.height, maxH);
		
		roi = ofRectangle(x, y, w, h);
		
		if (pRoi[0] != x) { pRoi[0].set(x); }
		if (pRoi[1] != y) { pRoi[1].set(y); }
		if (pRoi[2].getMax() != maxW) { pRoi[2].setMax(maxW); pRoi[2].set(w); }
		if (pRoi[3].getMax() != maxH) { pRoi[3].setMax(maxH); pRoi[3].set(h); }
		if (pRoi[2] != w) { pRoi[2].set(w); }
		if (pRoi[3] != h) { pRoi[3].set(h); }
	}
	
	//--------------------------------------------------------------
	void ftAverageFlow::getMeanStDev(vector<float> &_v, float &_mean, float &_stDev) {
		float mean = accumulate(_v.begin(), _v.end(), 0.0) / (float)_v.size();
		std::vector<float> diff(_v.size());
		std::transform(_v.begin(), _v.end(), diff.begin(), std::bind2nd(std::minus<float>(), mean));
		float sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
		float stDev = std::sqrt(sq_sum / _v.size());
		
		_mean = mean;
		_stDev = stDev;
	}
	
	//--------------------------------------------------------------
	string ftAverageFlow::getComponentName(int _index)  {
		vector<string> componentNames;
		switch (type) {
			case FT_VELOCITY:
			case FT_VELOCITY_NORM:
				componentNames = {"x", "y"};
				break;
			case FT_VELOCITY_SPLIT:
				componentNames = {"right", "down", "left", "up"};
				break;
			case FT_DENSITY:
				componentNames = {"red", "green", "blue", "alpha"};
				break;
			case FT_PRESSURE:
				componentNames = {"pressure"};
				break;
			case FT_TEMPERATURE:
				componentNames = {"temperature"};
				break;
			default:
				componentNames = {"unknown 0", "unknown 1", "unknown 2", "unknown 3"};
				break;
		}
		
		if (_index < componentNames.size()) {
			return componentNames[_index];
		}
		return "unknown";
		
	}
}
