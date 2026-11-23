#version 330

#define MAX_LIGHTS 32

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec4 baseColor;

// Keep these values in sync with the C++ LIGHT_TYPE enum:
// POINT_LIGHT = 0, SPOT_LIGHT = 1, DIRECTIONAL_LIGHT = 2.
// Directional lights behave like sunlight: no position-based attenuation and no range cutoff.
uniform int lightCount;
uniform int lightTypes[MAX_LIGHTS];
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightDirections[MAX_LIGHTS];
uniform vec4 lightColors[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];
uniform float lightRanges[MAX_LIGHTS];
uniform float spotAngles[MAX_LIGHTS];

out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);

    // Preserve transparent decal pixels.
    if (texColor.a < 0.01)
        discard;

    vec3 normal = normalize(gl_FrontFacing ? fragNormal : -fragNormal);
    vec3 lighting = vec3(0.0);

    int count = clamp(lightCount, 0, MAX_LIGHTS);

    for (int i = 0; i < count; ++i)
    {
        vec3 lightContribution = vec3(0.0);

        if (lightTypes[i] == 0) // POINT_LIGHT
        {
            vec3 toLight = lightPositions[i] - fragPosition;
            float distanceToLight = length(toLight);
            float range = max(lightRanges[i], 0.0001);
            vec3 directionToLight = distanceToLight > 0.0001
                ? toLight / distanceToLight
                : normal;

            // Matches the ambient floor used by the original point shader.
            float diffuse = max(dot(normal, directionToLight), 0.15);
            float attenuation = 1.0 - clamp(distanceToLight / range, 0.0, 1.0);

            lightContribution = lightColors[i].rgb * diffuse
                              * lightIntensities[i] * attenuation;
        }
        else if (lightTypes[i] == 1) // SPOT_LIGHT
        {
            vec3 toLight = lightPositions[i] - fragPosition;
            float distanceToLight = length(toLight);
            float range = max(lightRanges[i], 0.0001);
            vec3 directionToLight = distanceToLight > 0.0001
                ? toLight / distanceToLight
                : normal;

            float diffuse = max(dot(normal, directionToLight), 0.0);
            // lightDirections points outward from the spotlight, as in the old shader.
            float spot = dot(normalize(lightDirections[i]), -directionToLight);
            float cutoff = cos(radians(clamp(spotAngles[i], 0.0, 179.0)));
            float spotFactor = spot >= cutoff ? 1.0 : 0.0;
            float attenuation = 1.0 - clamp(distanceToLight / range, 0.0, 1.0);

            lightContribution = lightColors[i].rgb * diffuse
                              * lightIntensities[i] * attenuation * spotFactor;
        }
        else if (lightTypes[i] == 2) // DIRECTIONAL_LIGHT (sun)
        {
            // lightDirections[i] is the direction the rays travel.
            // The opposite vector points from the surface toward the sun.
            // Deliberately do not use lightPositions[i] or lightRanges[i] here:
            // sunlight illuminates the whole scene without distance attenuation.
            vec3 directionToLight = normalize(-lightDirections[i]);
            float diffuse = max(dot(normal, directionToLight), 0.0);

            lightContribution = lightColors[i].rgb * diffuse * lightIntensities[i];
        }

        lighting += lightContribution;
    }

    vec3 color = texColor.rgb * baseColor.rgb * lighting;
    finalColor = vec4(color, texColor.a * baseColor.a);
}