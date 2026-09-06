#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform sampler2D texture0;

uniform vec3 lightPosition;
uniform vec4 lightColor;
uniform float lightIntensity;
uniform float lightRange;

uniform vec4 baseColor;

out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);

    vec3 toLight = lightPosition - fragPosition;

    float distance = length(toLight);
    vec3 lightDirection = normalize(toLight);

    vec3 normal = gl_FrontFacing ? fragNormal : -fragNormal;
    float diffuse = max(dot(normal, lightDirection), 0.0);

    float attenuation = 1.0 - clamp(distance / lightRange, 0.0, 1.0);

    vec3 lighting = lightColor.rgb * diffuse * lightIntensity * attenuation;
    vec3 color = texColor.rgb * baseColor.rgb * lighting;

    finalColor = vec4(color, texColor.a * baseColor.a);
}