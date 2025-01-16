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

uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

float ShadowCalculation(vec4 fragPosLightSpace);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    //Add directional lighting.
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    FragColor = vec4(result, 0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(viewDir + lightDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,
    TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1,
    TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1,
    TexCoords));
    float shadow = ShadowCalculation(FragPosLightSpace);
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.005 * (1.0 - dot(Normal, -dirLight.direction)), 0.0005);
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}