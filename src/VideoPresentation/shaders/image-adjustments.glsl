// Image adjustment functions from Retro Media Lab C# (osd-support).

float srgbToLinear(float value)
{
    value=clamp(value,0.0,1.0);
    return value<=.04045 ? value/12.92 : pow((value+.055)/1.055,2.4);
}

float linearToSrgb(float value)
{
    value=clamp(value,0.0,1.0);
    return value<=.0031308 ? value*12.92 : 1.055*pow(value,1.0/2.4)-.055;
}

vec3 videoBrightnessParameter(vec3 color,float adjustment)
{
    return mix(color,vec3(adjustment<0.0?0.0:1.0),abs(adjustment));
}

vec3 videoContrastParameter(vec3 color,float factor)
{
    if(factor==1.0) return color;
    const float middleGray=.21586;
    return clamp(vec3(middleGray)+(color-vec3(middleGray))*factor,0.0,1.0);
}

float linearToSrgb(float value);
vec3 videoGammaParameter(vec3 color,float curve)
{
    if(curve==0.0) return color;
    float luminance=dot(color,vec3(0.2126,0.7152,0.0722));
    float signal=linearToSrgb(luminance);
    const float blackRoot=pow(.001,1.0/2.4);
    float corrected=curve<0.0?pow(mix(blackRoot,1.0,signal),2.4):pow(signal,curve);
    if(corrected==luminance) return color;
    float target=corrected>luminance?1.0:0.0;
    float amount=(corrected-luminance)/(target-luminance);
    return mix(color,vec3(target),amount);
}

vec3 videoSaturationParameter(vec3 color,float factor)
{ float luminance=dot(color,vec3(0.2126,0.7152,0.0722)); return clamp(vec3(luminance)+(color-vec3(luminance))*factor,0.0,1.0); }

vec3 videoSharpnessParameter(vec3 center,vec3 average,float setting)
{ float strength=abs(setting); return clamp(setting>=0.0?center+(center-average)*strength:mix(center,average,strength),0.0,1.0); }

vec3 restoreHostSource(vec3 filtered, vec2 uv, vec2 sourceSize, float deband, float details);

vec3 adjustLinear(vec3 color, vec4 parameters)
{
    color = videoContrastParameter(color, parameters.y);
    color = videoBrightnessParameter(color, parameters.x);
    color = videoGammaParameter(color, parameters.z);
    return videoSaturationParameter(color, parameters.w);
}

vec3 linearSourceSample(vec2 coordinates)
{
    vec3 encoded = sourceColor(coordinates);
    return vec3(srgbToLinear(encoded.r), srgbToLinear(encoded.g), srgbToLinear(encoded.b));
}

vec3 adjustedLinearSample(vec2 coordinates, vec4 parameters)
{
    return adjustLinear(linearSourceSample(coordinates), parameters);
}

vec3 sharpenAdjustedSample(vec3 color, vec2 coordinates, vec2 outputSize, vec4 parameters, float sharpness)
{
    if (abs(sharpness) <= 0.0001) return color;
    vec2 stepSize = 1.0 / max(outputSize, vec2(1.0));
    vec3 sum = color;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
            if (x != 0 || y != 0)
                sum += adjustedLinearSample(coordinates + vec2(float(x), float(y)) * stepSize, parameters);
    float weight = 9.0;
    if (sharpness < 0.0) {
        vec2 dx = vec2(2.0 * stepSize.x, 0.0);
        vec2 dy = vec2(0.0, 2.0 * stepSize.y);
        sum += (adjustedLinearSample(coordinates - dx, parameters) +
            adjustedLinearSample(coordinates + dx, parameters) +
            adjustedLinearSample(coordinates - dy, parameters) +
            adjustedLinearSample(coordinates + dy, parameters)) * 0.5;
        weight = 11.0;
    }
    return videoSharpnessParameter(color, sum / weight, sharpness);
}

vec3 processImageAdjustments(vec2 coordinates, vec2 outputSize, vec2 sourceSize, vec4 parameters, vec3 corrections)
{
    if (all(equal(parameters, vec4(0.0, 1.0, 0.0, 1.0))) && all(equal(corrections, vec3(0.0))))
        return sourceColor(coordinates);
    vec3 color = restoreHostSource(linearSourceSample(coordinates), coordinates, sourceSize, corrections.y, corrections.z);
    color = adjustLinear(color, parameters);
    color = sharpenAdjustedSample(color, coordinates, outputSize, parameters, corrections.x);
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
