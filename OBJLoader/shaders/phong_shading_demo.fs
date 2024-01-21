#version 330 core

in vec3 iPosView;
in vec3 iNormalView;
in vec2 iTexCoord;

uniform sampler2D mapKd;
uniform mat4 viewMatrix;
uniform mat4 worldMatrix;
uniform float Ns;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightIntensity;
uniform float spotLightCutOffDeg;
uniform float spotLightTotalWidthDeg;


out vec4 FragColor;

vec3 Diffuse(vec3 Kd, vec3 I, vec3 N, vec3 lightDir)
{
    return Kd * I * max(0, dot(N, lightDir));
}

vec3 Specular(vec3 Ks, vec3 I, vec3 N, vec3 vH)
{
    return Ks * I * max(0, pow(dot(N, vH), 32.0));
}

void main()
{
    // 2D Texture.
    vec3 texColor = texture2D(mapKd, iTexCoord).rgb;
    // Ambient Light.
    // Ambient Light.
    vec3 ambient = Ka * ambientLight;
    // Directional Light.
    vec3 vsLightDir = (viewMatrix * worldMatrix * vec4(-dirLightDir, 0.0)).xyz;
    vsLightDir = normalize(vsLightDir);
    vec3 vH = normalize(-iPosView + vsLightDir);
    // Diffuse.
    vec3 diffuse = Diffuse(Kd, dirLightRadiance, iNormalView, vsLightDir);
    // Specular.
    vec3 specular = Specular(Ks, dirLightRadiance, iNormalView, vH);
    vec3 dirLight = diffuse + specular;

    // Point light.
    vec4 tmpPos = viewMatrix * worldMatrix * vec4(pointLightPos, 0.0);
    vec3 vsLightPos = tmpPos.xyz / tmpPos.w;
    vsLightDir = normalize(vsLightPos - iPosView);
    float distSurfaceToLight = distance(vsLightPos, iPosView);
    float attenuation = 1.0f / (distSurfaceToLight * distSurfaceToLight);
    vec3 radiance = pointLightIntensity * attenuation;
    // Diffuse.
    diffuse = Diffuse(Kd, radiance, iNormalView, vsLightDir);
    // Specular.
    vH = normalize(-iPosView + vsLightDir);
    specular = Specular(Ks, radiance, iNormalView, vH);
    vec3 pointLight = diffuse + specular;

    // Spot light.
    tmpPos = viewMatrix * vec4(spotLightPos, 1.0);
    vsLightPos = tmpPos.xyz / tmpPos.w;
    vsLightDir = normalize(vsLightPos - iPosView);
    distSurfaceToLight = distance(vsLightPos, iPosView);
    attenuation = 1.0f / (distSurfaceToLight * distSurfaceToLight);
    if(cos(radians(spotLightTotalWidthDeg)) < dot(vsLightDir, iNormalView)){
        if(cos(radians(spotLightCutOffDeg)) > dot(vsLightDir, iNormalView)){
            float lambertian = ((dot(vsLightDir, iNormalView) - cos(radians(spotLightTotalWidthDeg))) / (cos(radians(spotLightCutOffDeg)) - cos(radians(spotLightTotalWidthDeg))));
            attenuation = attenuation * lambertian;
        }
    }
    else{
       attenuation = 0;
    }
    radiance = spotLightIntensity * attenuation;
    // Diffuse.
    diffuse = Diffuse(Kd, radiance, iNormalView, vsLightDir);
    // Specular.
    vH = normalize(-iPosView + vsLightDir);
    specular = Specular(Ks, radiance, iNormalView, vH);
    vec3 spotLight = diffuse + specular;

    FragColor = vec4(texColor , 1.0);
}
