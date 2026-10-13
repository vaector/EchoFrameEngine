#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

// Only fed by DrawMeshInstanced.
in mat4 instanceTransform;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform int useInstancing;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main()
{
    if (useInstancing == 1)
    {
        // Instanced: mvp is view-projection, instanceTransform is the model matrix.
        vec4 worldPosition = instanceTransform * vec4(vertexPosition, 1.0);
        mat3 normalTransform = transpose(inverse(mat3(instanceTransform)));

        fragPosition = worldPosition.xyz;
        fragNormal = normalize(normalTransform * vertexNormal);
        gl_Position = mvp * worldPosition;
    }
    else
    {
        // Normal draw: same as the original shader.
        fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
        fragNormal = normalize(mat3(matNormal) * vertexNormal);
        gl_Position = mvp * vec4(vertexPosition, 1.0);
    }

    fragTexCoord = vertexTexCoord;
}
