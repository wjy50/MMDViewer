attribute vec3 aPosition;
attribute vec4 aBones;
attribute vec4 aWeights;

uniform mat4 uSunMat;
uniform mat4 uBoneMats[-*d-];

void main()
{
    vec4 position4 = vec4(aPosition, 1.0);
    vec4 position = vec4(0.0);
    for(int i = 0, b; i < 4 && (b = int(aBones[i])) >= 0; ++i) {
        position += uBoneMats[b] * position4 * aWeights[i];
    }
    gl_Position = uSunMat * position;
}