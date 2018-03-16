attribute vec3 aPosition;
attribute vec3 aNormal;

uniform vec3 uSunPosition;
uniform mat4 uModelMat;
uniform mat4 uViewMat;
uniform mat4 uProjectionMat;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec3 vSunPosition;

void main()
{
    mat4 mvMat=uViewMat*uModelMat;
    vPosition=(mvMat*vec4(aPosition,1.0)).xyz;
    vSunPosition=(uViewMat*vec4(uSunPosition,0.0)).xyz;
    vNormal=(mvMat*vec4(aNormal,0.0)).xyz;
    gl_Position=uProjectionMat*mvMat*vec4(aPosition,1.0);
}