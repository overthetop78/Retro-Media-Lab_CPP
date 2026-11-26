#pragma once

namespace RetroMediaLab::VideoPresentation::OpenGlSamplingShader {

// GLSL sampling functions from Retro Media Lab C# (osd-support).
// Attribution for xBR, HQx and ScaleFX is in THIRD-PARTY-NOTICES.md.
inline constexpr char Vertex[] = R"glsl(
VERTEX_IN vec2 Position;
VERTEX_OUT vec2 TextureCoordinate;

void main() {
    gl_Position = vec4(Position, 0.0, 1.0);
    TextureCoordinate = vec2(Position.x + 1.0, 1.0 - Position.y) * 0.5;
}
)glsl";

inline constexpr const char* FragmentParts[] = {R"glsl(
uniform sampler2D Source;
uniform vec4 Processing;
uniform vec4 Output;
uniform vec4 Adjustments;
uniform vec2 Restoration;
FRAGMENT_IN vec2 TextureCoordinate;
DECLARE_COLOR

float cubicWeight(float distance)
{
    const float a = -0.5;
    float v = abs(distance);
    if (v <= 1.0) return (a + 2.0) * v * v * v - (a + 3.0) * v * v + 1.0;
    if (v < 2.0) return a * v * v * v - 5.0 * a * v * v + 8.0 * a * v - 4.0 * a;
    return 0.0;
}

vec4 bicubicSample(vec2 uv)
{
    vec2 pixel = uv * Processing.zw - 0.5;
    vec2 base = floor(pixel);
    vec4 sum = vec4(0.0);
    float weightSum = 0.0;
    for (int y = -1; y <= 2; y++)
    {
        float wy = cubicWeight(pixel.y - (base.y + float(y)));
        for (int x = -1; x <= 2; x++)
        {
            float weight = wy * cubicWeight(pixel.x - (base.x + float(x)));
            vec2 coordinate = clamp(base + vec2(float(x), float(y)),
                vec2(0.0), Processing.zw - 1.0);
            sum += texture2D(Source, (coordinate + 0.5) / Processing.zw) * weight;
            weightSum += weight;
        }
    }
    return clamp(sum / max(weightSum, 0.0001), 0.0, 1.0);
}

vec4 pointSample(vec2 uv)
{
    vec2 coordinate = clamp(floor(uv * Processing.zw), vec2(0.0), Processing.zw - 1.0);
    return texture2D(Source, (coordinate + 0.5) / Processing.zw);
}

vec4 linearSample(vec2 uv)
{
    return texture2D(Source, clamp(uv, 0.5 / Processing.zw, 1.0 - 0.5 / Processing.zw));
}

vec4 sharpBilinearSample(vec2 uv)
{
    vec2 pixel = uv * Processing.zw - 0.5;
    vec2 base = floor(pixel);
    vec2 fraction = pixel - base;
    vec2 scale = max(Output.xy / Processing.zw, vec2(0.0001));
    vec2 sharp = clamp((fraction - 0.5) * scale + 0.5, 0.0, 1.0);
    return linearSample((base + sharp + 0.5) / Processing.zw);
}

float jinc2J1(float v){float h=v*.5,t=h,s=t;for(int i=1;i<12;i++){t*=-(h*h)/(float(i)*float(i+1));s+=t;}return s;}
float jinc2Kernel(float v){return abs(v)<.0001?.5:jinc2J1(v)/v;}
vec4 jinc2Sample(vec2 uv){vec2 p=uv*Processing.zw-.5,b=floor(p);vec4 s=vec4(0.0);float ws=0.0;for(int y=-1;y<=2;y++)for(int x=-1;x<=2;x++){vec2 q=b+vec2(float(x),float(y));float d=length(p-q);if(d<2.0){float w=jinc2Kernel(3.14159265*d)*jinc2Kernel(1.57079633*d);q=clamp(q,vec2(0.0),Processing.zw-1.0);s+=texture2D(Source,(q+.5)/Processing.zw)*w;ws+=w;}}return clamp(s/max(ws,.0001),0.0,1.0);}

float lanczosSinc(float v){return abs(v)<.0001?1.0:sin(3.14159265*v)/(3.14159265*v);}
vec4 lanczosSample(vec2 uv){vec2 p=uv*Processing.zw-.5,b=floor(p);vec4 s=vec4(0.0);float ws=0.0;for(int y=-2;y<=3;y++)for(int x=-2;x<=3;x++){vec2 q=b+vec2(float(x),float(y)),d=p-q;float w=lanczosSinc(d.x)*lanczosSinc(d.x/3.0)*lanczosSinc(d.y)*lanczosSinc(d.y/3.0);q=clamp(q,vec2(0.0),Processing.zw-1.0);s+=texture2D(Source,(q+.5)/Processing.zw)*w;ws+=w;}return clamp(s/max(ws,.0001),0.0,1.0);}

)glsl", R"glsl(
vec3 xbrPointAt(vec2 center, vec2 offset)
{
    vec2 coordinate = clamp(center + offset, vec2(0.0), Processing.zw - 1.0);
    return texture2D(Source, (coordinate + 0.5) / Processing.zw).rgb;
}

float xbrLuma(vec3 color)
{
    return 48.0 * dot(color, vec3(0.299, 0.587, 0.114));
}

float xbrDistance(float a, float b, float c, float d,
    float e, float f, float g, float h)
{
    return abs(a - b) + abs(a - c) + abs(d - e) + abs(d - f)
        + 4.0 * abs(g - h);
}

void xbrSelect(float e, float f, float h, float c, float g, float i,
    float h5, float f4, float d, float i5, float i4, float b,
    vec3 fColor, vec3 hColor, float toward,
    inout float bestBlend, inout vec3 bestColor)
{
    if (abs(e - f) <= 0.000001 || abs(e - h) <= 0.000001) return;
    if (xbrDistance(e, c, g, i, h5, f4, h, f)
        >= xbrDistance(h, d, i5, f, i4, b, e, i)) return;
    float blend = smoothstep(1.1, 1.9, toward);
    if (blend <= bestBlend) return;
    bestBlend = blend;
    bestColor = abs(e - f) <= abs(e - h) ? fColor : hColor;
}

vec4 xbrSample(vec2 uv)
{
    vec2 position = uv * Processing.zw;
    vec2 center = floor(position);
    vec2 fraction = fract(position);
    vec3 A1 = xbrPointAt(center, vec2(-1.0, -2.0));
    vec3 B1 = xbrPointAt(center, vec2(0.0, -2.0));
    vec3 C1 = xbrPointAt(center, vec2(1.0, -2.0));
    vec3 A0 = xbrPointAt(center, vec2(-2.0, -1.0));
    vec3 A = xbrPointAt(center, vec2(-1.0, -1.0));
    vec3 B = xbrPointAt(center, vec2(0.0, -1.0));
    vec3 C = xbrPointAt(center, vec2(1.0, -1.0));
    vec3 C4 = xbrPointAt(center, vec2(2.0, -1.0));
    vec3 D0 = xbrPointAt(center, vec2(-2.0, 0.0));
    vec3 D = xbrPointAt(center, vec2(-1.0, 0.0));
    vec3 color = xbrPointAt(center, vec2(0.0));
    vec3 F = xbrPointAt(center, vec2(1.0, 0.0));
    vec3 F4 = xbrPointAt(center, vec2(2.0, 0.0));
    vec3 G0 = xbrPointAt(center, vec2(-2.0, 1.0));
    vec3 G = xbrPointAt(center, vec2(-1.0, 1.0));
    vec3 H = xbrPointAt(center, vec2(0.0, 1.0));
    vec3 I = xbrPointAt(center, vec2(1.0, 1.0));
    vec3 I4 = xbrPointAt(center, vec2(2.0, 1.0));
    vec3 G5 = xbrPointAt(center, vec2(-1.0, 2.0));
    vec3 H5 = xbrPointAt(center, vec2(0.0, 2.0));
    vec3 I5 = xbrPointAt(center, vec2(1.0, 2.0));
    float a1 = xbrLuma(A1), b1 = xbrLuma(B1), c1 = xbrLuma(C1);
    float a0 = xbrLuma(A0), a = xbrLuma(A), b = xbrLuma(B);
    float c = xbrLuma(C), c4 = xbrLuma(C4), d0 = xbrLuma(D0);
    float d = xbrLuma(D), e = xbrLuma(color), f = xbrLuma(F);
    float f4 = xbrLuma(F4), g0 = xbrLuma(G0), g = xbrLuma(G);
    float h = xbrLuma(H), i = xbrLuma(I), i4 = xbrLuma(I4);
    float g5 = xbrLuma(G5), h5 = xbrLuma(H5), i5 = xbrLuma(I5);
    vec3 candidate = color;
    float blend = 0.0;
    xbrSelect(e,f,h,c,g,i,h5,f4,d,i5,i4,b,F,H,
        fraction.x + fraction.y, blend, candidate);
    xbrSelect(e,f,b,i,a,c,b1,f4,d,c1,c4,h,F,B,
        fraction.x + 1.0 - fraction.y, blend, candidate);
    xbrSelect(e,d,b,g,c,a,b1,d0,f,a1,a0,h,D,B,
        2.0 - fraction.x - fraction.y, blend, candidate);
    xbrSelect(e,d,h,a,i,g,h5,d0,f,g5,g0,b,D,H,
        1.0 - fraction.x + fraction.y, blend, candidate);
    return vec4(mix(color, candidate, blend), 1.0);
}

vec3 hqxPointAt(vec2 center, vec2 offset)
{
    vec2 coordinate = clamp(center + offset, vec2(0.0), Processing.zw - 1.0);
    return texture2D(Source, (coordinate + 0.5) / Processing.zw).rgb;
}

bool hqxDifferent(vec3 first, vec3 second)
{
    vec3 difference = first - second;
    return abs(0.25 * (difference.r + difference.g + difference.b)) > 0.018
        || abs(0.25 * (difference.r - difference.b)) > 0.002
        || abs(-0.125 * difference.r + 0.25 * difference.g
            - 0.125 * difference.b) > 0.005;
}

bool hqxPattern(float pattern, float mask, float expected)
{
    float bit = 1.0;
    for (int index = 0; index < 8; index++)
    {
        float maskBit = mod(floor(mask / bit), 2.0);
        if (maskBit > 0.5)
        {
            float actualBit = mod(floor(pattern / bit), 2.0);
            float expectedBit = mod(floor(expected / bit), 2.0);
            if (abs(actualBit - expectedBit) > 0.5) return false;
        }
        bit *= 2.0;
    }
    return true;
}

vec3 hqxMix(vec3 first, float firstWeight, vec3 second, float secondWeight)
{
    return (first * firstWeight + second * secondWeight)
        / (firstWeight + secondWeight);
}

vec3 hqxMix(vec3 first, float firstWeight, vec3 second, float secondWeight,
    vec3 third, float thirdWeight)
{
    return (first * firstWeight + second * secondWeight + third * thirdWeight)
        / (firstWeight + secondWeight + thirdWeight);
}

vec4 hqxSample(vec2 uv)
{
    vec2 position = uv * Processing.zw;
    vec2 center = floor(position);
    vec2 direction = vec2(fract(position.x) > 0.5 ? -1.0 : 1.0,
        fract(position.y) > 0.5 ? -1.0 : 1.0);
    vec3 w0 = hqxPointAt(center, vec2(-direction.x, -direction.y));
    vec3 w1 = hqxPointAt(center, vec2(0.0, -direction.y));
    vec3 w2 = hqxPointAt(center, vec2(direction.x, -direction.y));
    vec3 w3 = hqxPointAt(center, vec2(-direction.x, 0.0));
    vec3 w4 = hqxPointAt(center, vec2(0.0));
    vec3 w5 = hqxPointAt(center, vec2(direction.x, 0.0));
    vec3 w6 = hqxPointAt(center, vec2(-direction.x, direction.y));
    vec3 w7 = hqxPointAt(center, vec2(0.0, direction.y));
    vec3 w8 = hqxPointAt(center, direction);
    float pattern = 0.0;
    if (hqxDifferent(w0, w4)) pattern += 1.0;
    if (hqxDifferent(w1, w4)) pattern += 2.0;
    if (hqxDifferent(w2, w4)) pattern += 4.0;
    if (hqxDifferent(w3, w4)) pattern += 8.0;
    if (hqxDifferent(w5, w4)) pattern += 16.0;
    if (hqxDifferent(w6, w4)) pattern += 32.0;
    if (hqxDifferent(w7, w4)) pattern += 64.0;
    if (hqxDifferent(w8, w4)) pattern += 128.0;
    vec3 color;
    if ((hqxPattern(pattern,191.0,55.0) || hqxPattern(pattern,219.0,19.0))
        && hqxDifferent(w1,w5)) color = hqxMix(w4,3.0,w3,1.0);
    else if ((hqxPattern(pattern,219.0,73.0) || hqxPattern(pattern,239.0,109.0))
        && hqxDifferent(w7,w3)) color = hqxMix(w4,3.0,w1,1.0);
    else if ((hqxPattern(pattern,11.0,11.0) || hqxPattern(pattern,254.0,74.0)
        || hqxPattern(pattern,254.0,26.0)) && hqxDifferent(w3,w1)) color = w4;
    else if ((hqxPattern(pattern,111.0,42.0) || hqxPattern(pattern,91.0,10.0)
        || hqxPattern(pattern,191.0,58.0) || hqxPattern(pattern,223.0,90.0)
        || hqxPattern(pattern,159.0,138.0) || hqxPattern(pattern,207.0,138.0)
        || hqxPattern(pattern,239.0,78.0) || hqxPattern(pattern,63.0,14.0)
        || hqxPattern(pattern,251.0,90.0) || hqxPattern(pattern,187.0,138.0)
        || hqxPattern(pattern,127.0,90.0) || hqxPattern(pattern,175.0,138.0)
        || hqxPattern(pattern,235.0,138.0)) && hqxDifferent(w3,w1))
        color = hqxMix(w4,3.0,w0,1.0);
    else if (hqxPattern(pattern,11.0,8.0)) color = hqxMix(w4,2.0,w0,1.0,w1,1.0);
    else if (hqxPattern(pattern,11.0,2.0)) color = hqxMix(w4,2.0,w0,1.0,w3,1.0);
    else if (hqxPattern(pattern,47.0,47.0)) color = hqxMix(w4,4.0,w3,1.0,w1,1.0);
    else if (hqxPattern(pattern,191.0,55.0) || hqxPattern(pattern,219.0,19.0))
        color = hqxMix(w4,5.0,w1,2.0,w3,1.0);
    else if (hqxPattern(pattern,219.0,73.0) || hqxPattern(pattern,239.0,109.0))
        color = hqxMix(w4,5.0,w3,2.0,w1,1.0);
    else if (hqxPattern(pattern,27.0,3.0) || hqxPattern(pattern,79.0,67.0)
        || hqxPattern(pattern,139.0,131.0) || hqxPattern(pattern,107.0,67.0))
        color = hqxMix(w4,3.0,w3,1.0);
    else if (hqxPattern(pattern,75.0,9.0) || hqxPattern(pattern,139.0,137.0)
        || hqxPattern(pattern,31.0,25.0) || hqxPattern(pattern,59.0,25.0))
        color = hqxMix(w4,3.0,w1,1.0);
    else if (hqxPattern(pattern,126.0,42.0) || hqxPattern(pattern,239.0,171.0)
        || hqxPattern(pattern,191.0,143.0) || hqxPattern(pattern,126.0,14.0))
        color = hqxMix(w4,2.0,w3,3.0,w1,3.0);
    else if (hqxPattern(pattern,251.0,106.0) || hqxPattern(pattern,111.0,110.0)
        || hqxPattern(pattern,63.0,62.0) || hqxPattern(pattern,251.0,250.0)
        || hqxPattern(pattern,223.0,222.0) || hqxPattern(pattern,223.0,30.0))
        color = hqxMix(w4,3.0,w0,1.0);
    else if (hqxPattern(pattern,10.0,0.0) || hqxPattern(pattern,79.0,75.0)
        || hqxPattern(pattern,159.0,27.0) || hqxPattern(pattern,47.0,11.0)
        || hqxPattern(pattern,190.0,10.0) || hqxPattern(pattern,238.0,10.0)
        || hqxPattern(pattern,126.0,10.0) || hqxPattern(pattern,235.0,75.0)
        || hqxPattern(pattern,59.0,27.0))
        color = hqxMix(w4,2.0,w3,1.0,w1,1.0);
    else color = hqxMix(w4,6.0,w3,1.0,w1,1.0);
    return vec4(color, 1.0);
}

)glsl", R"glsl(
float advancedColorDistance(vec3 first, vec3 second)
{
    vec3 d = first - second;
    return 48.0 * abs(dot(d, vec3(0.299, 0.587, 0.114)))
        + 7.0 * abs(dot(d, vec3(-0.168736, -0.331264, 0.5)))
        + 6.0 * abs(dot(d, vec3(0.5, -0.418688, -0.081312)));
}

bool advancedSame(vec3 first, vec3 second)
{
    return advancedColorDistance(first, second) < 0.55;
}

vec4 xbrzSample(vec2 uv)
{
    vec3 center = pointSample(uv).rgb;
    vec3 filtered = xbrSample(uv).rgb;
    float edge = clamp(advancedColorDistance(center, filtered) / 8.0, 0.0, 1.0);
    return vec4(mix(filtered, center, 0.18 * (1.0 - edge)), 1.0);
}

vec4 hq2xSample(vec2 uv){return mix(hqxSample(uv),linearSample(uv),.06);}

vec4 hq3xSample(vec2 uv){return mix(hqxSample(uv),linearSample(uv),.18);}

vec4 hq4xSample(vec2 uv){return mix(hqxSample(uv),linearSample(uv),.32);}

vec4 twoXSaiSample(vec2 uv){vec2 p=uv*Processing.zw,f=fract(p),b=floor(p),d=vec2(f.x<.5?-1.0:1.0,f.y<.5?-1.0:1.0);vec3 c=xbrPointAt(b,vec2(0.0)),h=xbrPointAt(b,vec2(d.x,0.0)),v=xbrPointAt(b,vec2(0.0,d.y));vec3 t=advancedSame(h,v)?h:(h+v)*.5;float q=clamp(1.0-1.7*length(f-vec2(d.x<0.0?0.0:1.0,d.y<0.0?0.0:1.0)),0.0,1.0);return vec4(mix(c,t,q*.45),1.0);}

vec4 superTwoXSaiSample(vec2 uv){vec4 a=twoXSaiSample(uv);return mix(a,linearSample(uv),.18);}

vec4 superEagleSample(vec2 uv){vec4 a=twoXSaiSample(uv),b=linearSample(uv);return vec4(clamp(a.rgb+(a.rgb-b.rgb)*.22,0.0,1.0),1.0);}

vec4 epxScale2xSample(vec2 uv){vec2 p=uv*Processing.zw,b=floor(p),f=floor(clamp(fract(p)*2.0,0.0,1.0));vec3 u=xbrPointAt(b,vec2(0.0,-1.0)),l=xbrPointAt(b,vec2(-1.0,0.0)),c=xbrPointAt(b,vec2(0.0)),r=xbrPointAt(b,vec2(1.0,0.0)),d=xbrPointAt(b,vec2(0.0,1.0));if(!advancedSame(u,d)&&!advancedSame(l,r)){if(f.x<.5&&f.y<.5&&advancedSame(l,u))c=l;else if(f.x>.5&&f.y<.5&&advancedSame(u,r))c=r;else if(f.x<.5&&f.y>.5&&advancedSame(l,d))c=l;else if(f.x>.5&&f.y>.5&&advancedSame(d,r))c=r;}return vec4(c,1.0);}

)glsl", R"glsl(
vec4 scaleFxSample(vec2 uv)
{
    vec2 position = uv * Processing.zw;
    vec2 pixel = floor(position);
    vec2 cell = floor(clamp(fract(position) * 3.0, 0.0, 2.0));
    vec3 up = xbrPointAt(pixel, vec2(0.0, -1.0));
    vec3 left = xbrPointAt(pixel, vec2(-1.0, 0.0));
    vec3 center = xbrPointAt(pixel, vec2(0.0));
    vec3 right = xbrPointAt(pixel, vec2(1.0, 0.0));
    vec3 down = xbrPointAt(pixel, vec2(0.0, 1.0));
    if (cell.x < 0.5 && cell.y < 0.5 && advancedSame(left, up)
        && !advancedSame(center, left)) return vec4(left, 1.0);
    if (cell.x > 1.5 && cell.y < 0.5 && advancedSame(up, right)
        && !advancedSame(center, right)) return vec4(right, 1.0);
    if (cell.x < 0.5 && cell.y > 1.5 && advancedSame(left, down)
        && !advancedSame(center, left)) return vec4(left, 1.0);
    if (cell.x > 1.5 && cell.y > 1.5 && advancedSame(down, right)
        && !advancedSame(center, right)) return vec4(right, 1.0);
    return vec4(center, 1.0);
}

vec4 scaleNxSample(vec2 uv)
{
    vec2 position = uv * Processing.zw;
    vec2 pixel = floor(position);
    vec2 cell = floor(clamp(fract(position) * 3.0, 0.0, 2.0));
    vec3 up = xbrPointAt(pixel, vec2(0.0, -1.0));
    vec3 left = xbrPointAt(pixel, vec2(-1.0, 0.0));
    vec3 center = xbrPointAt(pixel, vec2(0.0));
    vec3 right = xbrPointAt(pixel, vec2(1.0, 0.0));
    vec3 down = xbrPointAt(pixel, vec2(0.0, 1.0));
    if (advancedSame(up, down) || advancedSame(left, right))
        return vec4(center, 1.0);
    if (cell.x < 0.5 && cell.y < 0.5 && advancedSame(left, up))
        return vec4(left, 1.0);
    if (cell.x > 1.5 && cell.y < 0.5 && advancedSame(up, right))
        return vec4(right, 1.0);
    if (cell.x < 0.5 && cell.y > 1.5 && advancedSame(left, down))
        return vec4(left, 1.0);
    if (cell.x > 1.5 && cell.y > 1.5 && advancedSame(down, right))
        return vec4(right, 1.0);
    return vec4(center, 1.0);
}

vec4 sabrSample(vec2 uv)
{
    vec2 position = uv * Processing.zw;
    vec2 pixel = floor(position);
    vec2 fraction = fract(position);
    vec3 center = xbrPointAt(pixel, vec2(0.0));
    vec3 horizontal = xbrPointAt(pixel,
        vec2(fraction.x < 0.5 ? -1.0 : 1.0, 0.0));
    vec3 vertical = xbrPointAt(pixel,
        vec2(0.0, fraction.y < 0.5 ? -1.0 : 1.0));
    float proximity = clamp(1.25 - 1.5
        * (abs(fraction.x - 0.5) + abs(fraction.y - 0.5)), 0.0, 1.0);
    float coherence = 1.0 - clamp(
        advancedColorDistance(horizontal, vertical) / 18.0, 0.0, 1.0);
    float contrast = clamp(min(advancedColorDistance(center, horizontal),
        advancedColorDistance(center, vertical)) / 8.0, 0.0, 1.0);
    return vec4(mix(center, (horizontal + vertical) * 0.5,
        0.78 * proximity * coherence * contrast), 1.0);
}

)glsl", R"glsl(
vec4 sampleConfigured(vec2 uv)
{
    int sampling = int(Processing.y + 0.5);
    if (sampling == 0) return pointSample(uv);
    if (sampling == 1) return linearSample(uv);
    if (sampling == 2) return sharpBilinearSample(uv);
    if (sampling == 3) return bicubicSample(uv);
    if (sampling == 4) return xbrSample(uv);
    if (sampling == 5) return xbrzSample(uv);
    if (sampling == 6) return hqxSample(uv);
    if (sampling == 7) return scaleFxSample(uv);
    if (sampling == 8) return scaleNxSample(uv);
    if (sampling == 9) return sabrSample(uv);
    if (sampling == 10) return hq2xSample(uv);
    if (sampling == 11) return hq3xSample(uv);
    if (sampling == 12) return hq4xSample(uv);
    if (sampling == 13) return twoXSaiSample(uv);
    if (sampling == 14) return superTwoXSaiSample(uv);
    if (sampling == 15) return superEagleSample(uv);
    if (sampling == 16) return epxScale2xSample(uv);
    if (sampling == 17) return jinc2Sample(uv);
    if (sampling == 18) return lanczosSample(uv);
    return pointSample(uv);
}

vec3 sourceColor(vec2 coordinates) { return sampleConfigured(coordinates).rgb; }
vec3 sourcePoint(vec2 coordinates) { return pointSample(coordinates).rgb; }
)glsl"};

inline constexpr char FragmentMain[] = R"glsl(
void main() {
    FRAGMENT_COLOR = vec4(processImageAdjustments(TextureCoordinate, Output.xy, Processing.zw, Adjustments,
        vec3(Processing.x, Restoration)), 1.0);
}
)glsl";

} // namespace RetroMediaLab::VideoPresentation::OpenGlSamplingShader
