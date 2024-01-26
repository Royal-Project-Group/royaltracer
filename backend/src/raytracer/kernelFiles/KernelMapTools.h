//
// Created by m on 23.11.2023.
//
// Edited by d on 13.12.2023.


// returns the u, v and w value for the barycentric coordinates
void calculateMapCoordinates(struct SceneData* bulkData, struct Ray* ray, struct HitInformation* hitInformation) {

    int instPrimIndex = ray->hit.instPrim;
    struct TriEX triEx = bulkData->triEx[instPrimIndex];

    float3 hitPointOffset = hitInformation->hitPoint - bulkData->triBase[instPrimIndex].v1;

    // Precompute dot products
    float dotV2V0 = dot(hitPointOffset, triEx.vv0);
    float dotV2V1 = dot(hitPointOffset, triEx.vv1);

    // Calculate barycentric coordinates
    float v = (triEx.d11 * dotV2V0 - triEx.d01 * dotV2V1) * triEx.invDenom;
    float w = (triEx.d00 * dotV2V1 - triEx.d01 * dotV2V0) * triEx.invDenom;
    float u = 1.0f - v - w;

    // Interpolate texture coordinates
    float2 interpolatedTex = triEx.uv1 + v * (triEx.uv2 - triEx.uv1) + w * (triEx.uv3 - triEx.uv1);

    ray->hit.pTex = interpolatedTex;
}



float getTex(float x, float y) {
    // Multiply both by 8 and map to an integer
    int xN = (int)(x * 4.0);
    int yN = (int)(y * 4.0);

    // Check if the sum of xN and yN is even or odd
    if ((xN + yN) % 2 == 0) {
        // Even sum: return one color
        return 0.1f;  // You can use 0.0f for black and 1.0f for white if you prefer.
    } else {
        // Odd sum: return the other color
        return 1.0f;
    }
}

void normalMap(struct SceneData* bulkData, struct Ray* ray, struct HitInformation* hitInformation){
    int normalMapId = hitInformation->hitMaterial.normalMap_id;
    if(normalMapId != -1){

        float3 normal = hitInformation->hitNormal;

        int normalWidth = bulkData->normalMaps[normalMapId].width;
        int normalHeight = bulkData->normalMaps[normalMapId].height;
        int offset = bulkData->normalMaps[normalMapId].offset;

        int xN = (int)(ray->hit.pTex.x * normalWidth);
        int yN = (int)(ray->hit.pTex.y * normalHeight);

        int index = (xN + yN * normalWidth) % (normalWidth*normalHeight);

        float3 normalMapVector = bulkData->normalMapData[index + offset];
        normalMapVector = normalize(normalMapVector * 2.0f - (float3)(1.0f, 1.0f, 1.0f));

        normalMapVector = normalize(normalMapVector);
        float3 tangentNormal = (float3){0.0f, 0.0f, 1.0f};

        // printf("Width: %d, Height: %d, Offset: %d, xN: %d, yN: %d\n", textureWidth, textureHeight, offset, xN, yN);

        float3 angleOfRotation = cross(normal, tangentNormal);
        float angle = acos(dot(normal, tangentNormal) / (sqrt(dot(normal, normal)) * sqrt(dot(tangentNormal, tangentNormal))));


        float3 KxV = cross(angleOfRotation, normalMapVector); // normalMapVector is the normal from the normal map
        float3 KxKxV = cross(angleOfRotation, KxV);
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);

        // Apply Rodrigues' rotation formula
        float3 rotatedVector = {
                normalMapVector.x * cosAngle + KxV.x * sinAngle + KxKxV.x * (1 - cosAngle),
                normalMapVector.y * cosAngle + KxV.y * sinAngle + KxKxV.y * (1 - cosAngle),
                normalMapVector.z * cosAngle + KxV.z * sinAngle + KxKxV.z * (1 - cosAngle)
        };

        hitInformation->hitNormal =  normalize(rotatedVector);
    }
}

void bumpMap(struct SceneData* bulkData, struct Ray* ray, struct HitInformation* hitInformation){
    int bumpMapId = hitInformation->hitMaterial.bumpMap_id;
    if(bumpMapId != -1){
        float3 normal = hitInformation->hitNormal;
        int bumWidth = bulkData->bumpMaps[bumpMapId].width;
        int bumHeight = bulkData->bumpMaps[bumpMapId].height;
        int offset = bulkData->bumpMaps[bumpMapId].offset;

        float stepWidth = 1.0f / bumWidth;
        float stepHeight = 1.0f / bumHeight;

        int xN = (int)(ray->hit.pTex.x * bumWidth);
        int yN = (int)(ray->hit.pTex.y * bumHeight);

        int xNRight = (xN + 1) % bumWidth;
        int yNUp = (yN + 1) % bumHeight;

        int index = (xN + yN * bumWidth) % (bumWidth*bumHeight);
        int indexRight = (xNRight + yN * bumWidth) % (bumWidth*bumHeight);
        int indexUp = (xN + yNUp * bumWidth) % (bumWidth*bumHeight);

        // Directly access the bump map values at the required points
        float heightCenter = bulkData->bumpMapData[index + offset];
        float heightRight = bulkData->bumpMapData[indexRight + offset];
        float heightUp = bulkData->bumpMapData[indexUp + offset];

        // Calculate the derivatives of the height field
        float dU = heightRight - heightCenter;
        float dV = heightUp - heightCenter;

        // calculate the tangent and bitangent vectors
        float3 tangent = normalize(cross(normal, (float3)(0.0f, 0.0f, 1.0f)));

        // check if the tangent is close to zero
        if (length(tangent) < 0.1f) {
            tangent = normalize(cross(normal, (float3)(0.0f, 1.0f, 0.0f)));
        }

        float3 bitangent = normalize(cross(normal, tangent));

        float bumpIntensity = 1.0f;
        hitInformation->hitNormal += (dU * tangent + dV * bitangent) * bumpIntensity;

    }
}

float3 getDiffuseMap(struct SceneData* bulkData, struct Ray* ray, struct HitInformation* hitInformation){
    float3 col = {1.0, 1.0, 1.0};
    int texture_id = hitInformation->hitMaterial.texture_id;
    if(  texture_id != -1){
        // Get the width and height of the texture
        int width = bulkData->texture[texture_id].width;
        int height = bulkData->texture[texture_id].height;
        int offset = bulkData->texture[texture_id].offset;

        int xN = (int)(ray->hit.pTex.x * width);
        int yN = (int)(ray->hit.pTex.y * height);

        int index = (xN + yN * width ) % (width*height);
        col = bulkData->textureData[index + offset];

    }

    return col;
}