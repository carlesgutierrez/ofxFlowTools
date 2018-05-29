
#pragma once

#include "ofMain.h"
#include "ftDisplayScalarShader.h"

namespace flowTools {
	
	class ftDisplayScalar {
	public:
		
		void	setup(int _width, int _height){
			width = _width;
			height = _height;
			displayScalarBuffer.allocate(width, height);
			displayScalarBuffer.black();
			parameters.setName("display scalar");
			parameters.add(scale.set("scale", 1, 0, 10));
		};
		
		void	setSource(ofTexture& tex)	{ floatTexture = &tex; }
		void	setScale(float _value)		{ scale.set(_value); }
		
		float	getScale()	{ return scale.get(); }
		int		getWidth()	{ return width; }
		int		getHeight()	{ return height; }
		
		ofTexture& getTexture() {return displayScalarBuffer.getTexture();}
		void	draw(int _x, int _y) {draw(_x, _y, width, height);}
		void	draw(int _x, int _y, int _width, int _height, ofBlendMode _blendmode = OF_BLENDMODE_DISABLED) {
			displayScalarBuffer.black();
			displayScalarShader.update(displayScalarBuffer, *floatTexture, scale.get());
			
			ofPushStyle();
			ofEnableBlendMode(_blendmode);
			displayScalarBuffer.draw(_x, _y, _width, _height);
			ofPopStyle();
		};
		
		ofParameterGroup parameters;
	protected:
		ofParameter<float>	scale;
		int		width;
		int		height;
		ftFbo	displayScalarBuffer;
		ofTexture* floatTexture;
		ftDisplayScalarShader displayScalarShader;
		
	};
}

