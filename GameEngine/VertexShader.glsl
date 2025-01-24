// Vertex Shader source code
const char* vertexShaderSource = R"(
    #version 330 core
    in vec3 aPos;
    in vec3 vNormal;
    in vec3 aCol;
    in vec2 aUV;

    uniform mat4 model; // Model matrix
    uniform mat4 camMatrix;
    
    out vec3 FragPos; // Pass the position to the fragment shader
    out vec3 FragNormal;
    out vec3 FragCol;
    out vec2 FragUV;

    out vec3 FragTangent;    // Tangent in world space
    out vec3 FragBitangent;  // Bitangent in world space

    // Attributes for the neighboring vertices of the triangle (required for tangent computation)
    in vec3 v1;  // Position of vertex 1
    in vec3 v2;  // Position of vertex 2
    in vec2 uv1; // Texture coordinate of vertex 1
    in vec2 uv2; // Texture coordinate of vertex 2
    
    uniform bool bIsMovingTexture;
    uniform float deltaTime;

    void main() {
    if(bIsMovingTexture){
       FragUV += aUV * 100.0f * deltaTime;
    }
    else{
        FragUV = aUV;
    }
       FragCol = aCol;
       FragPos = vec3(model * vec4(aPos, 1.0));         // Transform position to world space
       FragNormal = mat3(transpose(inverse(model))) * vNormal;  // Transform normal to world space

       // Calculate edge vectors in model space
        vec3 edge1 = v1 - aPos;
        vec3 edge2 = v2 - aPos;

        // Calculate delta UV coordinates
        vec2 deltaUV1 = uv1 - aUV;
        vec2 deltaUV2 = uv2 - aUV;

        // Calculate the tangent and bitangent vectors
        float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
        vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

        // Transform tangent and bitangent to world space
        FragTangent = normalize(mat3(model) * tangent);
        FragBitangent = normalize(mat3(model) * bitangent);

       gl_Position = camMatrix * vec4(FragPos, 1.0);    // Compute final position
    }
)";