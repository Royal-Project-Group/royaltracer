//
// Created by m on 02.12.2023.
//

#define MAX_LIGHTS 1000

#ifndef ROYAL_TRACER_KERNELLIGHTS_H
#define ROYAL_TRACER_KERNELLIGHTS_H


float3 sampleLights(
        struct SceneData* bulkData,
        struct HitInformation* hitInfo,
                struct Ray* ray
        ) {

    float3 weightedIntensity = (float3)(0.0f, 0.0f, 0.0f);
    // Artificial radius factor for the light sources
    float artificialRadius = 0.5f;

    //FOR NOW: NO SHADING WHEN REFLECTING! LATER: IMPLEMENTATION OF A "VIRTUAL" LIGHT DIAMETER
    /*if(hitInfo->brdfType == 1){
        return weightedIntensity;
    }*/

    float totalWeight = 0;
    float probability[MAX_LIGHTS];
    float cumulativeProbability[MAX_LIGHTS];

    // Step 1: Calculate weights for each light and total weight
    for (int i = 0; i < bulkData->poiCount; i++) {
        float distance = length(hitInfo->hitPoint - bulkData->pointLights[i].position);
        float weight = 1.0f / (distance * distance);
        totalWeight += weight;
    }

    // Step 2: Calculate normalized probabilities
    for (int i = 0; i < bulkData->poiCount; i++) {
        float distance = length(hitInfo->hitPoint - bulkData->pointLights[i].position);
        float weight = 1.0f / (distance * distance);
        probability[i] = weight / totalWeight;

        // Creating cumulative distribution for easier selection
        cumulativeProbability[i] = (i == 0) ? probability[i] : cumulativeProbability[i - 1] + probability[i];
    }

    // Step 3: Select a light based on probability
    float rand = generateRandom(&(bulkData->seed0), &(bulkData->seed1));
    int selectedLightIndex = -1;
    for (int i = 0; i < bulkData->poiCount; i++) {
        if (rand < cumulativeProbability[i]) {
            selectedLightIndex = i;
            break;
        }
    }

    // Get the probability inverse
    float invProb = (1.0f / probability[selectedLightIndex]);



    //Iterate through all simple light sources and check if there is a intersection with an object between the hitpoint and the light source
    //for (int i = 0; i < bulkData->poiCount; i++) {
        {
            //Ray for next event estimation
            struct Ray lightRay;
            struct Ray shadowRay;
            lightRay.orig = hitInfo->hitPoint;
            shadowRay.orig = hitInfo->hitPoint;
            lightRay.dir = bulkData->pointLights[selectedLightIndex].position - hitInfo->hitPoint;
            lightRay.hit.t = FLT_MAX;
            shadowRay.hit.t = FLT_MAX;
            float lDir = length(lightRay.dir);
            lightRay.dir = normalize(lightRay.dir);
            shadowRay.dir = normalize(lightRay.dir);

            shadowRay.dir = sampleHemisphereCosineWeighted(&(bulkData->seed0), &(bulkData->seed1), lightRay.dir,
                                                           lightRay.dir, 0.001f);
            BVHIntersect(&shadowRay, 0, bulkData->triBase, bulkData->bvhNodes, bulkData->triIdx, bulkData->triCount);


            if (shadowRay.hit.t > lDir) {
                // Calculate the distance factor
                float distanceFactor = 1 / (lDir * lDir * 12.566f);

                if (hitInfo->brdfType == 0 && !hitInfo->insideMaterial) { // Diffuse interaction
                    float angleFactor = dot(lightRay.dir, normalize(hitInfo->hitNormal));
                    weightedIntensity += (angleFactor * distanceFactor) * bulkData->pointLights[selectedLightIndex].Ke;
                } else if (hitInfo->brdfType == 1 && !hitInfo->insideMaterial) { // Specular interaction
                    // Calculate the specular highlight based on the angle between the reflected ray and the light source
                    float angleFactor = fabs(dot(normalize(lightRay.dir), normalize(ray->dir)));
                    // Determine if the light is within the virtual size of the light source
                    float radiusThreshold = atan(artificialRadius / lDir);
                    float angleBetweenReflectedAndLightDir = acos(angleFactor + EPSILON);
                    float specularIntensity = 0.0f;
                    if (angleBetweenReflectedAndLightDir < radiusThreshold) {
                        specularIntensity =
                                ((radiusThreshold - angleBetweenReflectedAndLightDir) / radiusThreshold) /
                                artificialRadius;

                        float roughnessFactor = 1.0f - hitInfo->hitMaterial.Ns;
                        specularIntensity *= pow(1.0f - angleFactor, roughnessFactor);
                    }

                    weightedIntensity += specularIntensity * distanceFactor * bulkData->pointLights[selectedLightIndex].Ke;
                } else if (hitInfo->brdfType == 2 && !hitInfo->insideMaterial) { //Refraction
                    //If a refraction occurs, it depends on how the event circumstances are.
                    //Only when exiting a material, the light should have an effect on the ray
                    float angleFactor = fabs(dot(normalize(lightRay.dir), normalize(ray->dir)));
                    float radiusThreshold = atan(artificialRadius / lDir);
                    float angleBetweenReflectedAndLightDir = acos(angleFactor);
                    float specularIntensity = 0.0f;
                    if (angleBetweenReflectedAndLightDir < radiusThreshold) {
                        specularIntensity =
                                ((radiusThreshold - angleBetweenReflectedAndLightDir) / radiusThreshold) /
                                artificialRadius;

                        float roughnessFactor = 1.0f - hitInfo->hitMaterial.Ns;
                        specularIntensity *= pow(1.0f - angleFactor, roughnessFactor);
                    }
                    weightedIntensity += specularIntensity * distanceFactor * bulkData->pointLights[selectedLightIndex].Ke;
                } else if (hitInfo->brdfType == 4 && !hitInfo->insideMaterial) {
                    weightedIntensity += distanceFactor * bulkData->pointLights[selectedLightIndex].Ke;
                }
            }
        }

        for (int i = 0; i < bulkData->dirCount; i++) {
            //Ray for next event estimation
            struct Ray lightRay;
            struct Ray shadowRay;
            lightRay.orig = hitInfo->hitPoint;
            shadowRay.orig = hitInfo->hitPoint;
            lightRay.dir = -bulkData->directionalLights[i].direction;
            lightRay.hit.t = FLT_MAX;
            shadowRay.hit.t = FLT_MAX;
            float lDir = FLT_MAX;//length(lightRay.dir);

            lightRay.dir = normalize(lightRay.dir);
            shadowRay.dir = normalize(lightRay.dir);

            shadowRay.dir = sampleHemisphereCosineWeighted(&(bulkData->seed0), &(bulkData->seed1), lightRay.dir,
                                                           lightRay.dir, 0.001f);
            BVHIntersect(&shadowRay, 0, bulkData->triBase, bulkData->bvhNodes, bulkData->triIdx, bulkData->triCount);


            if (!(shadowRay.hit.t < lDir)) {
                // Calculate the distance factor

                if (hitInfo->brdfType == 0 && !hitInfo->insideMaterial) { // Diffuse interaction
                    // Calculate the angle factor
                    float angleFactor = dot(normalize(lightRay.dir), normalize(hitInfo->hitNormal));

                    weightedIntensity += angleFactor * bulkData->directionalLights[i].Ke;
                } else if (hitInfo->brdfType == 1 && !hitInfo->insideMaterial) { // Specular interaction
                    // Calculate the specular highlight based on the angle between the reflected ray and the light source
                    float angleFactor = fabs(dot(normalize(lightRay.dir), normalize(ray->dir)));
                    // Determine if the light is within the virtual size of the light source
                    float radiusThreshold = atan(artificialRadius / lDir);
                    float angleBetweenReflectedAndLightDir = acos(angleFactor);
                    float specularIntensity = 0.0f;
                    if (angleBetweenReflectedAndLightDir < radiusThreshold) {
                        specularIntensity = ((radiusThreshold - angleBetweenReflectedAndLightDir) / radiusThreshold) /
                                            artificialRadius;

                        float roughnessFactor = 1.0f - hitInfo->hitMaterial.Ns;
                        specularIntensity *= pow(1.0f - angleFactor, roughnessFactor);
                    }

                    weightedIntensity += specularIntensity * bulkData->directionalLights[i].Ke;
                } else if (hitInfo->brdfType == 2 && !hitInfo->insideMaterial) {
                    //If a refraction occurs, it depends on how the event circumstances are.
                    //Only when exiting a material, the light should have an effect on the ray
                    float angleFactor = fabs(dot(normalize(lightRay.dir), normalize(ray->dir)));
                    float radiusThreshold = atan(artificialRadius / lDir);
                    float angleBetweenReflectedAndLightDir = acos(angleFactor);
                    float specularIntensity = 0.0f;
                    if (angleBetweenReflectedAndLightDir < radiusThreshold) {
                        specularIntensity = ((radiusThreshold - angleBetweenReflectedAndLightDir) / radiusThreshold) /
                                            artificialRadius;

                        float roughnessFactor = 1.0f - hitInfo->hitMaterial.Ns;
                        specularIntensity *= pow(1.0f - angleFactor, roughnessFactor);
                    }
                    weightedIntensity += specularIntensity * bulkData->directionalLights[i].Ke;
                } else if (hitInfo->brdfType == 4 && !hitInfo->insideMaterial) {
                    weightedIntensity += bulkData->directionalLights[i].Ke;
                }
            }
        }

        weightedIntensity = (float3)(fabs(weightedIntensity.x), fabs(weightedIntensity.y), fabs(weightedIntensity.z));
        return weightedIntensity;
    }
#endif //ROYAL_TRACER_KERNELLIGHTS_H
