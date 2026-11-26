#version 450
#extension GL_GOOGLE_include_directive : require

// Sampling functions from Retro Media Lab C# (osd-support), Veldrid.
// See THIRD-PARTY-NOTICES.md for xBR, HQx and ScaleFX.
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform SamplingParameters {
    vec4 Output;
    ivec4 Settings;
    vec4 Adjustments;
    vec4 Corrections;
} Parameters;

layout(binding = 1) uniform sampler2D PointSampler;
layout(binding = 2) uniform sampler2D LinearSampler;

vec3 filterPointAt(ivec2 coordinate)
{
    ivec2 size=textureSize(PointSampler,0);
    return texelFetch(PointSampler,
        clamp(coordinate,ivec2(0),size-1),0).rgb;
}
vec3 nearestSample(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    return filterPointAt(ivec2(floor(uv*vec2(size))));
}

vec3 filterLinearAt(ivec2 coordinate)
{
    ivec2 size=textureSize(PointSampler,0);
    return texelFetch(PointSampler,
        clamp(coordinate,ivec2(0),size-1),0).rgb;
}
vec3 linearSampleCompact(vec2 uv)
{
    vec2 size=vec2(textureSize(PointSampler,0));
    vec2 pixel=uv*size-.5;
    ivec2 base=ivec2(floor(pixel));
    vec2 fraction=fract(pixel);
    vec3 top=mix(filterLinearAt(base),filterLinearAt(base+ivec2(1,0)),fraction.x);
    vec3 bottom=mix(filterLinearAt(base+ivec2(0,1)),filterLinearAt(base+ivec2(1,1)),fraction.x);
    return mix(top,bottom,fraction.y);
}

vec3 sharpBilinearSampleCompact(vec2 uv)
{
    vec2 size=vec2(textureSize(LinearSampler,0));
    vec2 pixel=uv*size-.5,base=floor(pixel),fraction=fract(pixel);
    vec2 scale=max(Parameters.Output.xy/size,vec2(1.0));
    vec2 sharp=clamp((fraction-.5)*scale*1.35+.5,0.0,1.0);
    vec2 mapped=(base+sharp+.5)/size;
    vec3 color=linearSampleCompact(mapped);
    vec2 stepSize=1.0/size;
    vec3 blur=(linearSampleCompact(mapped+vec2(stepSize.x,0.0))
        +linearSampleCompact(mapped-vec2(stepSize.x,0.0))
        +linearSampleCompact(mapped+vec2(0.0,stepSize.y))
        +linearSampleCompact(mapped-vec2(0.0,stepSize.y)))*.25;
    return clamp(color+(color-blur)*.28,0.0,1.0);
}

float filterCubicWeight(float distance)
{
    const float a=-.5;
    float value=abs(distance);
    if(value<=1.0)return (a+2.0)*value*value*value-(a+3.0)*value*value+1.0;
    if(value<2.0)return a*value*value*value-5.0*a*value*value+8.0*a*value-4.0*a;
    return 0.0;
}
vec3 filterNeighborhood(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    ivec2 p=ivec2(floor(uv*vec2(size)));
    return (filterPointAt(p+ivec2(-1,0))+filterPointAt(p+ivec2(1,0))
        +filterPointAt(p+ivec2(0,-1))+filterPointAt(p+ivec2(0,1)))*.25;
}
vec3 bicubicSampleCompact(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    vec2 pixel=uv*vec2(size)-.5;
    ivec2 base=ivec2(floor(pixel));
    vec3 sum=vec3(0.0);float weightSum=0.0;
    for(int y=-1;y<=2;y++)
    {
        float wy=filterCubicWeight(pixel.y-float(base.y+y));
        for(int x=-1;x<=2;x++)
        {
            float weight=wy*filterCubicWeight(pixel.x-float(base.x+x));
            sum+=filterPointAt(base+ivec2(x,y))*weight;weightSum+=weight;
        }
    }
    return clamp(sum/max(weightSum,.0001),0.0,1.0);
}

float filterColorDistance(vec3 a,vec3 b){return dot(abs(a-b),vec3(.299,.587,.114));}
vec3 xbrCompactSample(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    vec2 position=uv*vec2(size);ivec2 p=ivec2(floor(position));vec2 f=fract(position);
    vec3 c=filterPointAt(p),h=filterPointAt(p+ivec2(f.x<.5?-1:1,0));
    vec3 v=filterPointAt(p+ivec2(0,f.y<.5?-1:1));
    vec3 d=filterPointAt(p+ivec2(f.x<.5?-1:1,f.y<.5?-1:1));
    float corner=pow(clamp(1.0-2.0*length(f-vec2(f.x<.5?0.0:1.0,f.y<.5?0.0:1.0)),0.0,1.0),1.4);
    float diagonal=filterColorDistance(h,v)<filterColorDistance(c,d)?1.0:0.35;
    vec3 edge=filterColorDistance(c,h)<filterColorDistance(c,v)?h:v;
    return mix(c,mix(edge,(h+v+d)/3.0,.35),corner*.52*diagonal);
}

vec3 xbrzCompactSample(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    vec2 position=uv*vec2(size);ivec2 p=ivec2(floor(position));vec2 f=fract(position);
    ivec2 direction=ivec2(f.x<.5?-1:1,f.y<.5?-1:1);
    vec3 c=filterPointAt(p),h=filterPointAt(p+ivec2(direction.x,0));
    vec3 v=filterPointAt(p+ivec2(0,direction.y)),d=filterPointAt(p+direction);
    vec3 h2=filterPointAt(p+ivec2(direction.x*2,0));
    vec3 v2=filterPointAt(p+ivec2(0,direction.y*2));
    vec2 cornerPosition=vec2(direction.x<0?0.0:1.0,direction.y<0?0.0:1.0);
    float corner=clamp(1.15-1.6*length(f-cornerPosition),0.0,1.0);
    float edgeAgreement=1.0-clamp(abs(filterColorDistance(c,h)-filterColorDistance(c,v))*2.5,0.0,1.0);
    vec3 smoothed=(c*2.0+h*2.0+v*2.0+d+h2+v2)/9.0;
    return mix(c,smoothed,corner*(.58+.22*edgeAgreement));
}

vec3 hqxCompactSample(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    vec2 position=uv*vec2(size);ivec2 p=ivec2(floor(position));
    vec3 c=filterPointAt(p),l=filterPointAt(p+ivec2(-1,0)),r=filterPointAt(p+ivec2(1,0));
    vec3 u=filterPointAt(p+ivec2(0,-1)),d=filterPointAt(p+ivec2(0,1));
    float horizontal=filterColorDistance(l,r),vertical=filterColorDistance(u,d);
    vec3 direction=horizontal<vertical?(l+r)*.5:(u+d)*.5;
    float edge=clamp(min(horizontal,vertical)*3.0,0.0,1.0);
    vec3 highQuality=mix(linearSampleCompact(uv),direction,.32*(1.0-edge));
    return clamp(mix(highQuality,c,.18*edge),0.0,1.0);
}

vec3 scaleFxCompactSample(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    vec2 position=uv*vec2(size);ivec2 p=ivec2(floor(position));vec2 f=fract(position);
    vec3 c=filterPointAt(p),l=filterPointAt(p+ivec2(-1,0)),r=filterPointAt(p+ivec2(1,0));
    vec3 u=filterPointAt(p+ivec2(0,-1)),d=filterPointAt(p+ivec2(0,1));
    vec3 horizontal=f.x<.5?l:r,vertical=f.y<.5?u:d;
    float dh=filterColorDistance(c,horizontal),dv=filterColorDistance(c,vertical);
    vec3 candidate=dh<dv?horizontal:vertical;
    float corner=clamp((abs(f.x-.5)+abs(f.y-.5)-.28)*1.9,0.0,1.0);
    return mix(c,candidate,corner*clamp(1.0-min(dh,dv)*2.0,.18,.72));
}

bool filterSimilar(vec3 a,vec3 b){return filterColorDistance(a,b)<.035;}
vec3 scaleNxCompactSample(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    vec2 position=uv*vec2(size);ivec2 p=ivec2(floor(position));vec2 f=fract(position);
    vec3 c=filterPointAt(p),l=filterPointAt(p+ivec2(-1,0)),r=filterPointAt(p+ivec2(1,0));
    vec3 u=filterPointAt(p+ivec2(0,-1)),d=filterPointAt(p+ivec2(0,1));
    vec3 result=c;
    if(!filterSimilar(u,d)&&!filterSimilar(l,r))
    {
        if(f.x<.5&&f.y<.5&&filterSimilar(l,u))result=(l+u)*.5;
        else if(f.x>=.5&&f.y<.5&&filterSimilar(u,r))result=(u+r)*.5;
        else if(f.x<.5&&f.y>=.5&&filterSimilar(l,d))result=(l+d)*.5;
        else if(f.x>=.5&&f.y>=.5&&filterSimilar(d,r))result=(d+r)*.5;
    }
    return result;
}

vec3 sabrCompactSample(vec2 uv)
{
    ivec2 size=textureSize(PointSampler,0);
    vec2 position=uv*vec2(size);ivec2 p=ivec2(floor(position));vec2 f=fract(position);
    vec3 c=filterPointAt(p),l=filterPointAt(p+ivec2(-1,0)),r=filterPointAt(p+ivec2(1,0));
    vec3 u=filterPointAt(p+ivec2(0,-1)),d=filterPointAt(p+ivec2(0,1));
    vec3 nw=filterPointAt(p+ivec2(-1,-1)),ne=filterPointAt(p+ivec2(1,-1));
    vec3 sw=filterPointAt(p+ivec2(-1,1)),se=filterPointAt(p+ivec2(1,1));
    float slash=filterColorDistance(nw,se)+filterColorDistance(l,d)+filterColorDistance(u,r);
    float backslash=filterColorDistance(ne,sw)+filterColorDistance(r,d)+filterColorDistance(u,l);
    vec3 diagonal=slash<backslash?mix(nw,se,(f.x+f.y)*.5):mix(ne,sw,(1.0-f.x+f.y)*.5);
    float edge=clamp(abs(slash-backslash)*2.0,0.12,.68);
    return clamp(mix(linearSampleCompact(uv),diagonal,edge*.48),0.0,1.0);
}

vec3 hq2xSampleCompact(vec2 uv){return mix(hqxCompactSample(uv),linearSampleCompact(uv),.06);}

vec3 hq3xSampleCompact(vec2 uv){return mix(hqxCompactSample(uv),linearSampleCompact(uv),.18);}

vec3 hq4xSampleCompact(vec2 uv){return mix(hqxCompactSample(uv),linearSampleCompact(uv),.32);}

vec3 twoXSaiSampleCompact(vec2 uv){ivec2 size=textureSize(PointSampler,0);vec2 p=uv*vec2(size),f=fract(p);ivec2 b=ivec2(floor(p)),d=ivec2(f.x<.5?-1:1,f.y<.5?-1:1);vec3 c=filterPointAt(b),h=filterPointAt(b+ivec2(d.x,0)),v=filterPointAt(b+ivec2(0,d.y));vec3 t=filterColorDistance(h,v)<.035?h:(h+v)*.5;float q=clamp(1.0-1.7*length(f-vec2(d.x<0?0.0:1.0,d.y<0?0.0:1.0)),0.0,1.0);return mix(c,t,q*.45);}

vec3 superTwoXSaiSampleCompact(vec2 uv){return mix(twoXSaiSampleCompact(uv),linearSampleCompact(uv),.18);}

vec3 superEagleSampleCompact(vec2 uv){vec3 a=twoXSaiSampleCompact(uv),b=linearSampleCompact(uv);return clamp(a+(a-b)*.22,0.0,1.0);}

vec3 epxScale2xSampleCompact(vec2 uv){ivec2 size=textureSize(PointSampler,0);vec2 p=uv*vec2(size),f=floor(clamp(fract(p)*2.0,0.0,1.0));ivec2 b=ivec2(floor(p));vec3 u=filterPointAt(b+ivec2(0,-1)),l=filterPointAt(b+ivec2(-1,0)),c=filterPointAt(b),r=filterPointAt(b+ivec2(1,0)),d=filterPointAt(b+ivec2(0,1));if(filterColorDistance(u,d)>=.035&&filterColorDistance(l,r)>=.035){if(f.x<.5&&f.y<.5&&filterColorDistance(l,u)<.035)c=l;else if(f.x>.5&&f.y<.5&&filterColorDistance(u,r)<.035)c=r;else if(f.x<.5&&f.y>.5&&filterColorDistance(l,d)<.035)c=l;else if(f.x>.5&&f.y>.5&&filterColorDistance(d,r)<.035)c=r;}return c;}

float jinc2J1(float v){float h=v*.5,t=h,s=t;for(int i=1;i<12;i++){t*=-(h*h)/(float(i)*float(i+1));s+=t;}return s;}
float jinc2Kernel(float v){return abs(v)<.0001?.5:jinc2J1(v)/v;}
vec3 jinc2SampleCompact(vec2 uv){ivec2 size=textureSize(PointSampler,0);vec2 p=uv*vec2(size)-.5;ivec2 b=ivec2(floor(p));vec3 s=vec3(0.0);float ws=0.0;for(int y=-1;y<=2;y++)for(int x=-1;x<=2;x++){ivec2 q=b+ivec2(x,y);float d=length(p-vec2(q));if(d<2.0){float w=jinc2Kernel(3.14159265*d)*jinc2Kernel(1.57079633*d);s+=filterPointAt(q)*w;ws+=w;}}return clamp(s/max(ws,.0001),0.0,1.0);}

float lanczosSinc(float v){return abs(v)<.0001?1.0:sin(3.14159265*v)/(3.14159265*v);}
vec3 lanczosSampleCompact(vec2 uv){ivec2 size=textureSize(PointSampler,0);vec2 p=uv*vec2(size)-.5;ivec2 b=ivec2(floor(p));vec3 s=vec3(0.0);float ws=0.0;for(int y=-2;y<=3;y++)for(int x=-2;x<=3;x++){ivec2 q=b+ivec2(x,y);vec2 d=p-vec2(q);float w=lanczosSinc(d.x)*lanczosSinc(d.x/3.0)*lanczosSinc(d.y)*lanczosSinc(d.y/3.0);s+=filterPointAt(q)*w;ws+=w;}return clamp(s/max(ws,.0001),0.0,1.0);}

vec3 sourceColor(vec2 uv) {
    switch (Parameters.Settings.x) {
    case 0: return nearestSample(uv);
    case 1: return linearSampleCompact(uv);
    case 2: return sharpBilinearSampleCompact(uv);
    case 3: return bicubicSampleCompact(uv);
    case 4: return xbrCompactSample(uv);
    case 5: return xbrzCompactSample(uv);
    case 6: return hqxCompactSample(uv);
    case 7: return scaleFxCompactSample(uv);
    case 8: return scaleNxCompactSample(uv);
    case 9: return sabrCompactSample(uv);
    case 10: return hq2xSampleCompact(uv);
    case 11: return hq3xSampleCompact(uv);
    case 12: return hq4xSampleCompact(uv);
    case 13: return twoXSaiSampleCompact(uv);
    case 14: return superTwoXSaiSampleCompact(uv);
    case 15: return superEagleSampleCompact(uv);
    case 16: return epxScale2xSampleCompact(uv);
    case 17: return jinc2SampleCompact(uv);
    case 18: return lanczosSampleCompact(uv);
    default: return nearestSample(uv);
    }
}


vec3 sourcePoint(vec2 coordinates) { return nearestSample(coordinates); }

#include "image-adjustments.glsl"
#include "image-restoration.glsl"

void main() {
    color = Parameters.Settings.z == 0
        ? vec4(processImageAdjustments(uv, Parameters.Output.xy, vec2(textureSize(PointSampler, 0)),
            Parameters.Adjustments, Parameters.Corrections.xyz), 1.0)
        : texture(PointSampler, uv);
}
