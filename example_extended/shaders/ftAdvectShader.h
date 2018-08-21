
#pragma once

#include "ofMain.h"
#include "ftShader.h"

namespace flowTools {
	
	class ftAdvectShader : public ftShader {
	public:
		ftAdvectShader() {
			bInitialized = true;
			if (ofIsGLProgrammableRenderer()) { glThree(); } else { glTwo(); }
			string shaderName = "ftAdvectShader";
			if (bInitialized) { ofLogVerbose(shaderName + " initialized"); }
			else { ofLogWarning(shaderName + " failed to initialize"); }
			load("tempShader/ftVertexShader.vert", "tempShader/" + shaderName + ".frag");
		}
		
	protected:
		void glTwo() {
			fragmentShader = GLSL120(
									 uniform sampler2DRect Backbuffer;
									 uniform sampler2DRect Obstacle;
									 uniform sampler2DRect Velocity;
									 
									 uniform float TimeStep;
									 uniform float Dissipation;
									 uniform float InverseCellSize;
									 uniform vec2	Scale;
									 
									 void main(){
										 vec2 st = gl_TexCoord[0].st;
										 vec2 st2 = st * Scale;
										 
										 float inverseSolid = 1.0 - ceil(texture2DRect(Obstacle, st2).x - 0.5);
										 
										 vec2 u = texture2DRect(Velocity, st2).rg / Scale;
										 vec2 coord =  st - TimeStep * InverseCellSize * u;
										 
										 gl_FragColor = Dissipation * texture2DRect(Backbuffer, coord) * inverseSolid;
									 }
									 );
			
			bInitialized *= setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
			bInitialized *= linkProgram();
		}
		
		void glThree() {
			fragmentShader = GLSL150(
									 uniform sampler2DRect Backbuffer;
									 uniform sampler2DRect Obstacle;
									 uniform sampler2DRect Velocity;
									 
									 uniform float TimeStep;
									 uniform float Dissipation;
									 uniform float InverseCellSize;
									 uniform vec2	Scale;
									 
									 in vec2 texCoordVarying;
									 out vec4 fragColor;
									 
									 
									 vec2 bilerp(sampler2DRect d, vec2 p)
									 {
										 vec4 ij; // i0, j0, i1, j1
										 ij.xy = floor(p - 0.5) + 0.5;
										 ij.zw = ij.xy + 1.0;
										 
										 vec4 uv = ij;
										 vec2 d11 = texture(d, uv.xy).xy;
										 vec2 d21 = texture(d, uv.zy).xy;
										 vec2 d12 = texture(d, uv.xw).xy;
										 vec2 d22 = texture(d, uv.zw).xy;
										 
										 vec2 a = p - ij.xy;
										 
										 return mix(mix(d11, d21, a.x), mix(d12, d22, a.x), a.y);
									 }
									 
									 void main(){
										 vec2 st = texCoordVarying;
										 vec2 st2 = st * Scale;
										 
										 float inverseSolid = 1.0 - ceil(texture(Obstacle, st2).x - 0.5);
										 
										 vec2 u = texture(Velocity, st2).rg / Scale;
										 vec2 coord =  st - TimeStep * InverseCellSize * u;
										 
										 vec4 vel = vec4(bilerp(Backbuffer, coord), 0.0, 0.0);
										 fragColor = Dissipation * vel * inverseSolid;
									 }
									 );
			
			bInitialized *= setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
			bInitialized *= setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
			bInitialized *= bindDefaults();
			bInitialized *= linkProgram();
		}
		
	public:
		void update(ofFbo& _fbo, ofTexture& _backTex,  ofTexture& _velTex, ofTexture& _obsTex, float _timeStep, float _dissipation, float _cellSize){
			_fbo.begin();
			begin();
			setUniform1f("TimeStep", _timeStep);
			setUniform1f("Dissipation", _dissipation);
			setUniform1f("InverseCellSize", 1.0 / _cellSize);
			setUniform2f("Scale", _velTex.getWidth() / _fbo.getWidth(), _velTex.getHeight()/ _fbo.getHeight());
			setUniformTexture("Backbuffer", _backTex, 0);
			setUniformTexture("Velocity", _velTex, 1);
			setUniformTexture("Obstacle", _obsTex, 2);
			renderFrame(_fbo.getWidth(), _fbo.getHeight());
			end();
			_fbo.end();
		}
	};
}


