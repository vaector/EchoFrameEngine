#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform sampler2D texture0;

uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform float lightIntensity;

uniform vec4 baseColor;

out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);

    // Ignore transparent parts of the decal.
    if (texColor.a < 0.01)
        discard;

    vec3 direction = normalize(-lightDirection);

    vec3 normal = gl_FrontFacing ? fragNormal : -fragNormal;

    float diffuse = max(dot(normal, direction), 0.0);

    vec3 lighting =
        lightColor.rgb *
        diffuse *
        lightIntensity;

    vec3 color =
        texColor.rgb *
        baseColor.rgb *
        lighting;

    finalColor = vec4(
        color,
        texColor.a * baseColor.a
    );
}