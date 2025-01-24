const char* fragmentShaderSource = R"(
    #version 330 core
    #define MAX_LIGHTS 30

    struct sLights {
        vec3 position;      // position for point lights
        vec4 color;
        vec4 ambient;
        vec4 diffuse;       // diffuse color
        vec4 specular;      // specular color (w = specular power)
        vec4 atten;         // (constant, linear, quadratic, cutoff)
        vec4 direction;     // for directional lights/spot lights
        vec4 param1;        // x = light type, y = inner angle, z = outer angle
        vec4 param2;        // x = on/off
    };

    in vec3 FragPos;    // Incoming fragment position from vertex shader
    in vec3 FragNormal; // Incoming fragment normal from vertex shader
    in vec3 FragCol;
    in vec2 FragUV;
    in vec3 FragTangent;
    in vec3 FragBitangent;

    uniform int numberOfLights;  // Pass the number of active lights
    uniform sLights pLights[MAX_LIGHTS];  // Assuming you have a maximum of 30 lights
    uniform vec3 camLocation;   // Camera position

    uniform bool bIsVisible;
    
    uniform samplerCube cubeMap;
    uniform bool bUseCubeMap;

    uniform bool bIsStensil;

    uniform bool bIsTransparent;
    uniform float transparencyIndex;

    uniform bool bIsRefractive;
    uniform float refractiveIndex;
    uniform bool bIsReflective;
    uniform float reflectiveIndex;

    uniform bool bIsCubeMap;

    uniform bool bUseTexture;
    uniform int numberOfTextures;
    uniform sampler2D textureSamplers[192];    // Max number of texture units is 192

    out vec4 FragColor;

    // Calculating attenuation
    float CalculateAttenuation(sLights light, vec3 fragPos) {
        float distance = length(light.position.xyz - fragPos);
        return 1.0 / (light.atten.x + light.atten.y * distance + light.atten.z * (distance * distance));
    }
    
    // Calculating diffuse lighting
    vec3 CalculateDiffuse(sLights light, vec3 norm, vec3 lightDir) {
        float diff = max(dot(norm, lightDir), 0.0);
        return diff * light.color.rgb * light.diffuse.rgb;
    }

    // Calculating specular lighting
    vec3 CalculateSpecular(sLights light, vec3 norm, vec3 lightDir, vec3 viewDir) {
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), light.specular.w); // specular power in w component
        return spec * light.specular.rgb * light.color.rgb;
    }

    void main() {
        if(!bIsVisible) {
            return; // If object is not visible, discard the fragment
        }

        vec3 finalColor = vec3(0.0);
        vec3 viewDir = normalize(camLocation - FragPos);
        vec3 textureNormal = texture(textureSamplers[1], FragUV).rgb;
        float roughnessFromRoughnessMap = 0.0;

        if(numberOfTextures >= 3) {
            roughnessFromRoughnessMap = texture(textureSamplers[2], FragUV).r;
        }
        else {
            roughnessFromRoughnessMap = 1.0;
        }
        
        // for heightMap
        float heightFromHeightMap = texture(textureSamplers[3], FragUV).r - 0.5;
        vec2 displacedUV = FragUV + heightFromHeightMap * 0.05 * normalize(viewDir).xy;

        // Calculating light direction
        vec3 lightDir;
        vec3 norm; // Use the normal map if available


        // Compute the TBN matrix for transforming normal map data
        vec3 T = normalize(FragTangent);
        vec3 B = normalize(FragBitangent);
        vec3 N = normalize(FragNormal);
        mat3 TBN = mat3(T, B, N);

        // Calculate the normal from the normal map if available
        norm = N; // Default to interpolated normal
        if (numberOfTextures >= 2) {
            vec3 normalFromMap = texture(textureSamplers[1], FragUV).rgb * 2.0 - 1.0; // Convert from [0,1] to [-1,1]
            norm = normalize(TBN * normalFromMap); // Transform to world space
        }
        //if(numberOfTextures >= 2) {
        //    norm = normalize(textureNormal);
        //}
        //else {
        //    norm = normalize(FragNormal);
        //}

        vec3 result = vec3(0.0);

        // for metallicMap
        float metallic;

        if(numberOfTextures >= 5) {
            metallic = texture(textureSamplers[4], FragUV).r;
        }
        else {
            metallic = 0.0;
        }

        vec3 reflection = reflect(viewDir, norm);
        vec3 envReflection = texture(cubeMap, reflection).rgb;

        // Loop over all the lights
        for (int i = 0; i < numberOfLights; i++) {
            if (pLights[i].param1.x == 0.0) { // Point light
                lightDir = normalize(pLights[i].position.xyz - FragPos);
            } else if (pLights[i].param1.x == 1.0) { // Directional light
                lightDir = normalize(-pLights[i].direction.xyz);
            }

            // Calculate lighting components
            float attenuation = CalculateAttenuation(pLights[i], FragPos);
            vec3 diffuse = vec3(0.0);
            
            if(numberOfTextures >= 5) {
                diffuse = CalculateDiffuse(pLights[i], norm, lightDir) * (1.0 - metallic);
            }
            else {
                diffuse = CalculateDiffuse(pLights[i], norm, lightDir);
            }
            
            vec3 viewDir = normalize(camLocation - FragPos);
            vec3 specular = vec3(0.0);

            if(numberOfTextures >= 3) {
                specular = CalculateSpecular(pLights[i], norm, lightDir, viewDir) * (1.0 - roughnessFromRoughnessMap);
            }
            else {
                specular = CalculateSpecular(pLights[i], norm, lightDir, viewDir);
            }

            // Accumulate lighting results
            result += (diffuse + specular) * attenuation;
        }

        // Apply texture if available
        vec3 diffuseTexture = vec3(0.0);

        if(bIsStensil) {
            float stencilColour = texture( textureSamplers[0], FragUV ).r;
		    if ( stencilColour < 0.5f )
		    {
			    discard;
		    }
            FragColor = vec4(stencilColour, 0.0, 0.0, 1.0);
            return;
        }

        if(numberOfTextures >= 4) {
            diffuseTexture = texture(textureSamplers[0], displacedUV).rgb;
        }
        else {
            diffuseTexture = texture(textureSamplers[0], FragUV).rgb;
        }

        finalColor += result * diffuseTexture.rgb * (roughnessFromRoughnessMap);

        if(numberOfTextures >= 5){
            finalColor += envReflection * (metallic);
            //finalColor = mix(finalColor, envReflection, metallic);
        }

        // If cubeMap is used, reflect the environment
        if(bIsCubeMap) {
            if(bUseCubeMap) {
                finalColor = texture(cubeMap, FragNormal).rgb;
            }
        }

        // Handle transparency
        //if(bIsTransparent) {
        //    finalColor = vec4(finalColor, transparencyIndex);
        //}

        // Reflection and refraction logic
        vec3 eyeToVertexRay = normalize(camLocation - FragPos);
        vec3 reflectRay;
        vec3 reflectColour;
        vec3 refractRay;
        vec3 refractColour;

        // Reflection logic
        if(bIsReflective) {
            reflectRay = reflect(eyeToVertexRay, norm);
            reflectColour = texture(cubeMap, reflectRay).rgb;
            finalColor += reflectColour * reflectiveIndex;
        }

        // Refraction logic
        if(bIsRefractive) {
            refractRay = refract(eyeToVertexRay, norm, refractiveIndex);
            refractColour = texture(cubeMap, refractRay).rgb;
            finalColor += refractColour;
        }

        // Combine reflection and refraction
        if(bIsReflective && bIsRefractive) {
            float fresnel = pow(1.0 - dot(viewDir, norm), 3.0);
            finalColor = mix(refractColour, reflectColour, fresnel) * refractiveIndex;
        }

        // Final fragment color
        //FragColor = vec4(FragUV, 0.0, 1.0);

        if(bIsTransparent) {
            FragColor = vec4(finalColor, transparencyIndex);
            return;
        }
        FragColor = vec4(finalColor, 1.0);
    }
)";
