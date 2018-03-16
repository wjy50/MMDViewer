attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aUV;
attribute vec4 aBones;
attribute vec4 aWeights;

uniform vec3 uSunPosition;
uniform mat4 uViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uSunMat;
uniform mat4 uBoneMats[-*d-];

varying vec3 vPosition;
varying vec3 vNormal;
varying vec2 vUV;
varying vec3 vSunPosition;
varying vec3 vPositionInSun;

void main()
{
    vec4 position4=vec4(aPosition,1.0);
    vec4 normal4=vec4(aNormal,0.0);
    vec4 position=vec4(0.0);
    vec4 normal=vec4(0.0);
    vSunPosition=(uViewMat*vec4(uSunPosition,0.0)).xyz;
    vUV=aUV;
    for(int i=0,b;i < 4 && (b=int(aBones[i])) >= 0;i++)
    {
        position+=uBoneMats[b]*position4*aWeights[i];
        normal+=uBoneMats[b]*normal4*aWeights[i];
    }
    position4=uViewMat*position;
    vPosition=(position4).xyz;
    vNormal=(uViewMat*normal).xyz;
    vPositionInSun=(uSunMat*position).xyz;
    gl_Position=uProjectionMat*position4;
}