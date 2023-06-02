#version 460 core

out vec4 FragColor;

in vec4 vPosition;

uniform float g;
uniform float g2;

uniform vec3 v3CameraPos;
uniform vec3 v3LightPos;
uniform vec3 v3InvWaveLength;
uniform float fCameraHeight;
uniform float fCameraHeight2;
uniform float fOuterRadius;
uniform float fOuterRadius2;
uniform float fInnerRadius;
uniform float fInnerRadius2;
uniform float fKrESun;
uniform float fKmESun;
uniform float fKr4PI;
uniform float fKm4PI;
uniform float fScale;
uniform float fScaleDepth;
uniform float fScaleOverScaleDepth;
uniform int Samples;
uniform float alpha;

float scale(float fCos) 
{ 
	float x = 1.0 - fCos;  
	return(fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25)))));  
} 

void main(void) 
{ 

	vec3 v3Direction;  
	vec3 PrimaryColor;  
	vec3 SecondaryColor;  


    //Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	vec3 v3Pos = vPosition.xyz;  
	vec3 v3Ray = v3Pos - v3CameraPos;  
	float fFar = length(v3Ray);  
	v3Ray /= fFar;  

	//Calculate the ray's starting position, then calculate its scattering offset
	vec3 v3Start = v3CameraPos;  
	float fHeight = length(v3Start);  
	float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));  
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;  
	float fStartOffset = fDepth * scale(fStartAngle);  

	//Initialize the scattering loop variables
	float fSampleLength = fFar / Samples;  
	float fScaledLength = fSampleLength * fScale;  
	vec3 v3SampleRay = v3Ray * fSampleLength;  
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;  

	//Now loop through the sample rays
	vec3 v3FrontColor = vec3(0.0);  
	for (int i = 0; i < Samples; i++) 
	{ 
		float fHeight = length(v3SamplePoint);  
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));  
		float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;  
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;  
		float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));  
		vec3 v3Attenuate = exp(-fScatter * (v3InvWaveLength * fKr4PI + fKm4PI));  
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);  
		v3SamplePoint += v3SampleRay;  
	} 

	//Finally, scale the Mie and Rayleight colors and
	SecondaryColor = v3FrontColor * fKmESun;  
	PrimaryColor = v3FrontColor * (v3InvWaveLength * fKrESun);  
	v3Direction = v3CameraPos - v3Pos;  

	float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);  
	float fRayleighPhase = 1.0 + fCos * fCos;  
	float fMeiPhase = (1.0 - g2) / (2.0 + g2) * (1.0 + fCos * fCos) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);  
	vec3 Color = 1.0 - exp(-1.5 * (fRayleighPhase * PrimaryColor + fMeiPhase * SecondaryColor));  
	//FragColor = vec4(pow(Color, vec3(1.0 / 2.2)), 1.0);   // Gamma correction
	FragColor = vec4(Color, alpha);  
}
