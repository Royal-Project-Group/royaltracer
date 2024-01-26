//
// Created by m on 25.11.2023.
//

#ifndef ROYAL_TRACER_KERNELTRACE_H
#define ROYAL_TRACER_KERNELTRACE_H

//Return color vector
void tracePath(struct SceneData* bulkData, struct Ray* ray, float3* accumEmission, float3* accumBRDF) {

    //Setup the hitinformation struct
    struct HitInformation hitInformation;
    hitInformation.insideMaterial = false;

    struct Material airMaterial;
    airMaterial.Ni = 1.0f;
    bulkData->airMaterial = airMaterial;
    hitInformation.currentMaterial = bulkData->airMaterial;
    int subsurfaceIteration = 0;


    for (int j = 0; j < bulkData->bounces; j++) {

        int hitID = -1;
        float hitDist = FLT_MAX;
        ray->hit.t = FLT_MAX;

        //Get the intersection object
        BVHIntersect(ray, 0, bulkData->triBase, bulkData->bvhNodes, bulkData->triIdx, bulkData->triCount);
        if (ray->hit.t != FLT_MAX) {
            hitDist = ray->hit.t;
            hitID = ray->hit.instPrim;
            //Fill the hitinformation struct
            hitInformation.hitPoint = ray->orig + ray->dir * hitDist;
            hitInformation.hitNormal = bulkData->triBase[hitID].vn;
            hitInformation.hitMaterial = bulkData->materials[bulkData->triEx[hitID].materialID];
            hitInformation.hitObjectId = bulkData->triEx[hitID].objectID;
            hitInformation.bounce = j;

            //If the hit normal faces the same direction as the ray, switch it
            if(dot(hitInformation.hitNormal, ray->dir) > 0){
                hitInformation.hitNormal *= -1.0f;
            }


            //Fill the additional data for the denoiser
            if(j==0){
                bulkData->albedoOut[bulkData->pixelID] = (float4)(hitInformation.hitMaterial.Kd.x, hitInformation.hitMaterial.Kd.y, hitInformation.hitMaterial.Kd.z, 0.0f);
                bulkData->normalOut[bulkData->pixelID] = (float4)(hitInformation.hitNormal.x, hitInformation.hitNormal.y, hitInformation.hitNormal.z, 0.0f);
            }

            //Check if we do scattering or
            if(!evaluateVolume(bulkData, ray, &hitInformation, accumEmission, accumBRDF, &subsurfaceIteration)){
                //Do the BRDF evaluation
                evaluateBRDF(bulkData, ray, &hitInformation, accumEmission, accumBRDF,j);

                //NEE (Shadow rays)
                *accumEmission += (sampleLights(bulkData, &hitInformation, ray) + hitInformation.hitMaterial.Ke) * (*accumBRDF);
            }
            else{
                //NEE (Shadow rays)
                *accumEmission += sampleLights(bulkData, &hitInformation, ray) * (*accumBRDF);
            }

            //We have to apply the pdf to the accumBRDF to account for the importance sampling done
            (*accumBRDF) *= hitInformation.pdf;


        }
        else {
            //Background emission, possibly add a skybox here or set the value based on a variable
            *accumEmission += ((float3)(20.0f, 20.0f, 20.0f)) * (*accumBRDF);

            //Fill the additional data for the denoiser
            if(j==0){
                bulkData->albedoOut[bulkData->pixelID] = (float4)(20.0f, 20.0f, 20.0f, 0.0f);
            }
            return;
        }
        hitID = -1;

        // Russian roulette termination
        if (j > 3 && !hitInformation.insideMaterial) { // Start applying Russian roulette after a few bounces
            float luminance = 0.2126f * accumBRDF->x + 0.7152f * accumBRDF->y + 0.0722f * accumBRDF->z;
            float terminationProbability = fmaxf(0.1f, 1.0f - luminance);

            if (generateRandom(&(bulkData->seed0), &(bulkData->seed1)) < terminationProbability) {
                break; // Terminate the path
            } else {
                *accumBRDF /= (1.0f - terminationProbability); // Adjust BRDF if continuing
            }
        }

    }
}

#endif //ROYAL_TRACER_KERNELTRACE_H
