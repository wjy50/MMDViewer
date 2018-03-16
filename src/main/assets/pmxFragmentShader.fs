precision lowp float;

uniform vec3 uSunLightStrength;
uniform vec3 uAmbient;
uniform vec4 uDiffuse;
uniform vec4 uSpecular;
uniform sampler2D uSamplers[3];//uSamplers[0] == uTexture && uSamplers[1] == uSphereMap
uniform ivec3 uTextureModes;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec2 vUV;
varying vec3 vSunPosition;
varying vec3 vPositionInSun;

void sunLight(in vec3 position,in vec3 normal,in vec3 sunPosition,
              inout vec4 diffuse,inout vec3 specular,in float nDotView)
{
    float nDotViewPosition=max(nDotView,0.01);
    float nDotViewHalfVec=max(dot(normal,normalize(sunPosition-position)),0.01);
    diffuse=uDiffuse*nDotViewPosition;
    specular=uSpecular.rgb*pow(nDotViewHalfVec,uSpecular.a);
}

vec2 sphereCoordinate(vec3 viewVec,vec3 normal)
{
    vec3 r=reflect(viewVec,normal)+vec3(0.0,0.0,1.0);
    //vec3 r2=r*r;
    r/=(2.0*length(r));
    mat3 m=mat3(1.0,0.0,0.0,0.0,-1.0,0.0,0.5,0.5,0.0);
    return (m*vec3(r.xy,1.0)).xy;
}

void main()
{
    vec3 position=normalize(vPosition);
    vec3 normal=normalize(vNormal);
    vec3 sunPosition=normalize(vSunPosition);
    vec4 textureColor= uTextureModes.x > 0 ? texture2D(uSamplers[0],vUV) : vec4(1.0);
    if(uTextureModes.y == 1)textureColor.rgb+=texture2D(uSamplers[1],sphereCoordinate(position,normal)).rgb;
    else if(uTextureModes.y != 0)textureColor*=texture2D(uSamplers[1],sphereCoordinate(position,normal));
    if(textureColor.a < 0.01)discard;
    vec4 diffuse;
    vec3 specular;
    float biasDot=dot(normal,sunPosition);
    if(biasDot >= 0.0)
    {
        sunLight(position,normal,sunPosition,diffuse,specular,biasDot);
        float shadowFactor=1.0-step(texture2D(uSamplers[2],vPositionInSun.xy).z,vPositionInSun.z+0.0016*(biasDot-1.25));
        diffuse*=shadowFactor;
        specular*=shadowFactor;
    }
    gl_FragColor=textureColor*vec4((uAmbient+diffuse.rgb+specular)*uSunLightStrength,uDiffuse.a);
}