#version 330 core

struct DirLight {
vec3 direction;
vec3 ambient;
vec3 diffuse;
vec3 specular;
};
uniform DirLight dirLight;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};
uniform Material material;

uniform sampler2D shadowMap;

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec4 FragPosLightSpace;
in vec3 TangentLightDirection;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 lightDir);

float ShadowCalculation(vec4 fragPosLightSpace);

void main()
{
    vec3 norm = texture(material.texture_normal1, TexCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);

    //Add directional lighting.
    vec3 result = CalcDirLight(dirLight, norm, TangentViewPos, TangentLightDirection);

    FragColor = vec4(result, 0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 lightDir)
{
    vec3 lightRelativeOrigin = normalize(-lightDir);
    // diffuse shading
    float diff = max(dot(normal, lightRelativeOrigin), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(viewDir + lightRelativeOrigin);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,
    TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1,
    TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1,
    TexCoords)) * vec3(texture(material.texture_diffuse1, TexCoords)) * 2;
    float shadow = ShadowCalculation(FragPosLightSpace);
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.001 * (1.0 - dot(Normal, -dirLight.direction)), 0.0001);
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}