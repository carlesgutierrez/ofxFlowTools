/*  ************************************************************************************
 *
 *  ftParticleFlow
 *
 *  Created by Matthias Oostrik on 03/16.14.
 *  Copyright 2014 http://www.MatthiasOostrik.com All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *  OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  ************************************************************************************ */

#include "ftParticleFlow.h"

namespace flowTools {
	
	ftParticleFlow::ftParticleFlow(){
		parameters.setName("particle flow");
		parameters.add(bIsActive.set("active", true));
		parameters.add(speed.set("speed", 20, 0, 100));
		parameters.add(cellSize.set("cell size", 1.25, 0.0, 2.0));
		parameters.add(birthChance.set("birth chance", 0.5, 0, 1));
		parameters.add(birthVelocityChance.set("birth velocity chance", 0.5, 0, 1));
		parameters.add(lifeSpan.set("lifespan", 5, 0, 10));
		parameters.add(lifeSpanSpread.set("lifespan spread", .25, 0, 1));
		parameters.add(mass.set("mass", 1.3, 0, 2));
		parameters.add(massSpread.set("mass spread", .2, 0, 1));
		parameters.add(size.set("size", 2, 0, 10));
		parameters.add(sizeSpread.set("size spread", .75, 0, 1));
		parameters.add(twinkleSpeed.set("twinkle speed", 11, 0, 20));
		parameters.add(gravity.set("gravity", ofVec2f(0, 0), ofVec2f(-10,-10), ofVec2f(10,10)));
	}
	
	void ftParticleFlow::setup(int _simulationWidth, int _simulationHeight, int _numParticlesX, int _numParticlesY) {
		simulationWidth = _simulationWidth;
		simulationHeight = _simulationHeight;
		numParticlesX = _numParticlesX;
		numParticlesY = _numParticlesY;
		numParticles = (numParticlesX * numParticlesY);
				
		particleMesh.setMode(OF_PRIMITIVE_POINTS);
		for(int x = 0; x < _numParticlesX; x++){
			for(int y = 0; y < _numParticlesY; y++){
				particleMesh.addVertex(ofVec3f(x,y));
				particleMesh.addTexCoord(ofVec2f(x, y));
			}
		}
		
		int internalFormatVelocity = GL_RG32F;
		
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);  // Why?
		
		
		particleAgeLifespanMassSizeSwapBuffer.allocate(numParticlesX, numParticlesY, GL_RGBA32F, GL_NEAREST);
		particleAgeLifespanMassSizeSwapBuffer.black();
		particlePositionSwapBuffer.allocate(numParticlesX, numParticlesY, internalFormatVelocity, GL_NEAREST);
		particlePositionSwapBuffer.black();
		initPositionShader.update(*particlePositionSwapBuffer.getBuffer());
		particlePositionSwapBuffer.swap();
		particleHomeBuffer.allocate(numParticlesX, numParticlesY, internalFormatVelocity);
		particleHomeBuffer.black();
		initPositionShader.update(particleHomeBuffer);
		
		fluidVelocitySwapBuffer.allocate(simulationWidth, simulationHeight, internalFormatVelocity);
		fluidVelocitySwapBuffer.black();
		flowVelocitySwapBuffer.allocate(simulationWidth, simulationHeight, internalFormatVelocity);
		flowVelocitySwapBuffer.black();
		densitySwapBuffer.allocate(simulationWidth, simulationHeight, GL_RGBA32F);
		densitySwapBuffer.black();
		obstacleBuffer.allocate(simulationWidth, simulationHeight, GL_RGB); // GL_RED??
		obstacleBuffer.black();
		
		ofPopStyle();
		
	}
	
	void ftParticleFlow::update(float _deltaTime) {
		float time = ofGetElapsedTimef();
		if (_deltaTime != 0)
			deltaTime = _deltaTime;
		else
			deltaTime = time - lastTime;
		lastTime = time;
		timeStep = deltaTime * speed.get();
		
		if (bIsActive.get()) {
			
			ofPushStyle();
			ofEnableBlendMode(OF_BLENDMODE_DISABLED);
			
			
			particleAgeLifespanMassSizeSwapBuffer.swap();
			ALMSParticleShader.update(*particleAgeLifespanMassSizeSwapBuffer.getBuffer(),
									   particleAgeLifespanMassSizeSwapBuffer.getBackTexture(),
									   particlePositionSwapBuffer.getTexture(),
									   flowVelocitySwapBuffer.getTexture(),
									   densitySwapBuffer.getTexture(),
									   obstacleBuffer.getTexture(),
									   deltaTime,
									   birthChance.get(),
									   birthVelocityChance.get(),
									   lifeSpan.get(), lifeSpanSpread.get(),
									   mass.get(), massSpread.get(),
									   size.get(), sizeSpread.get());
			
			particlePositionSwapBuffer.swap();
			moveParticleShader.update(*particlePositionSwapBuffer.getBuffer(),
									  particlePositionSwapBuffer.getBackTexture(),
									  particleAgeLifespanMassSizeSwapBuffer.getTexture(),
									  fluidVelocitySwapBuffer.getTexture(),
									  particleHomeBuffer.getTexture(),
									  timeStep,
									  cellSize.get(),
									  gravity);
			
			ofPopStyle();
	 
			flowVelocitySwapBuffer.black();
			fluidVelocitySwapBuffer.black();
			densitySwapBuffer.black();
			obstacleBuffer.black();
		}
	}
	
	void ftParticleFlow::draw(int _x, int _y, int _width, int _height, ofBlendMode _blendmode) {
		ofPushStyle();
		ofPushView();
		ofEnableBlendMode(_blendmode);
		ofTranslate(_x, _y);
		ofScale(_width / numParticlesX, _height / numParticlesY);
		drawParticleShader.update(particleMesh, numParticles, particlePositionSwapBuffer.getTexture(), particleAgeLifespanMassSizeSwapBuffer.getTexture(), twinkleSpeed.get());
		
		ofPopView();
		ofPopStyle();
	}

	void ftParticleFlow::addFlowVelocity(ofTexture & _tex, float _strength) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		flowVelocitySwapBuffer.swap();
		addShader.update(*flowVelocitySwapBuffer.getBuffer(),
						 flowVelocitySwapBuffer.getBackTexture(),
						 _tex,
						 _strength);
		ofPopStyle();
	}
	
	void ftParticleFlow::addFluidVelocity (ofTexture& _tex, float _strength) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		fluidVelocitySwapBuffer.swap();
		addShader.update(*fluidVelocitySwapBuffer.getBuffer(),
						 fluidVelocitySwapBuffer.getBackTexture(),
						 _tex,
						 _strength);
		ofPopStyle();
	}
	
	void ftParticleFlow::setObstacle (ofTexture& _tex) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		obstacleBuffer.black();
		obstacleBuffer.begin();
		_tex.draw(0,0,simulationWidth,simulationHeight);
		obstacleBuffer.end();
		ofPopStyle();
	}
}
