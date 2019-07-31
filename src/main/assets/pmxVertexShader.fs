attribute vec3 aNormal;
attribute vec3 aPosition;
attribute vec2 aUV;
attribute vec4 aBones;
attribute vec4 aWeights;

uniform vec3 uSunPosition;
uniform mat4 uViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uSunMat;
uniform mat4 uBoneMats[-*d-];

varying vec3 vNormal;
varying vec2 vUV;
varying vec3 vSunPosition;
varying vec3 vPositionInSun;
varying vec3 vPosition;

void main()
{
    vec4 position4 = vec4(aPosition, 1.0);
    vec4 normal4 = vec4(aNormal, 0.0);
    vec4 position = vec4(0.0);
    vec4 normal = vec4(0.0);
    position += uBoneMats[int(aBones[0])] * position4 * aWeights[0];
    normal += uBoneMats[int(aBones[0])] * normal4 * aWeights[0];
    if (aBones[1] >= 0.0) {
        position += uBoneMats[int(aBones[1])] * position4 * aWeights[1];
        normal += uBoneMats[int(aBones[1])] * normal4 * aWeights[1];
    }
    if (aBones[2] >= 0.0) {
        position += uBoneMats[int(aBones[2])] * position4 * aWeights[2];
        normal += uBoneMats[int(aBones[2])] * normal4 * aWeights[2];
    }
    if (aBones[3] >= 0.0) {
        position += uBoneMats[int(aBones[3])] * position4 * aWeights[3];
        normal += uBoneMats[int(aBones[3])] * normal4 * aWeights[3];
    }
    vec4 positionInView = uViewMat * position;
    vec4 proj = uProjectionMat * positionInView;
    vPosition = positionInView.xyz;
    vNormal = (uViewMat * normal).xyz;
    vPositionInSun = (uSunMat * position).xyz;
    vSunPosition = (uViewMat * vec4(uSunPosition, 0.0)).xyz;
    vUV = aUV;
    gl_Position = proj;
}