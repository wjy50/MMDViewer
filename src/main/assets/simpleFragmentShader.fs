precision lowp float;

uniform vec3 uSunLightStrength;
uniform vec3 uAmbient;
uniform vec4 uDiffuse;
uniform vec3 uSpecular;
uniform float uShininess;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec3 vSunPosition;

void sunLight(in vec3 position,in vec3 normal,in vec3 sunPosition,
              inout vec4 diffuse,inout vec3 specular)
{
    vec3 halfVec=normalize(sunPosition-position);
    float nDotViewPosition=max(dot(normal,sunPosition),0.0);
    diffuse=uDiffuse*nDotViewPosition;
    float nDotViewHalfVec=max(dot(normal,halfVec),0.0);
    float powerFactor=pow(nDotViewHalfVec,uShininess);
    specular=uSpecular*powerFactor;
}

void main()
{
    vec3 position=normalize(vPosition);
    vec3 normal=normalize(vNormal);
    vec3 sunPosition=normalize(vSunPosition);
    vec4 diffuse;
    vec3 specular;
    sunLight(position,normal,sunPosition,diffuse,specular);
    gl_FragColor=vec4((uAmbient+diffuse.rgb+specular)*uSunLightStrength,uDiffuse.a);
}