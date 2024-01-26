//
// Created by m on 23.11.2023.
//


//Check if and how the ray could be scattered before the next intersection
bool evaluateVolume(struct SceneData *bulkData, struct Ray *ray, struct HitInformation *hitInformation, float3 *accumEmission,
                    float3 *accumBRDF, int *subsurfaceIteration) {
    //This checks two cases: the ray is inside an object or the ray could possibly hit a smoke particle
    //First we do a random check - this is a universal step for both cases
    float randCheck = generateRandom(&(bulkData->seed0), &(bulkData->seed1));
    if (!hitInformation->insideMaterial) {
        //The ray isnt inside an object. To sample the fog (if there is any), we take a random point on the ray and set this to be the current hit point
        //In this case, as we didnt hit an object, we dont do the BDRF.
        //Do we hit a fog particle?
        if (randCheck < (FOGINTENSITY * length(hitInformation->hitPoint - ray->orig)) / FOGDISTANCE) {
            ray->orig = FOGDISTANCE * (randCheck / FOGINTENSITY) + ray->orig;
            //Tell the sampleLights function to not use a angel factor at all. set the type to fog scattering
            hitInformation->brdfType = 4;
            return true;
        }
        return false;
    } else {
        //Subsurface
        if(ray->hit.t/hitInformation->currentMaterial.Ka.y > randCheck && *subsurfaceIteration < (int)(bulkData->bounces/2)){
            float moveDistance = ray->hit.t * randCheck;
            if (moveDistance > hitInformation->currentMaterial.Ka.y) {
                moveDistance = hitInformation->currentMaterial.Ka.y;
            }
            ray->orig = ray->orig + ray->dir * moveDistance;
            ray->dir = normalize(sampleEqualWeighted(&(bulkData->seed0), &(bulkData->seed1)));
            *accumBRDF *= hitInformation->currentMaterial.Tf * hitInformation->currentMaterial.Ka.x;
            (*subsurfaceIteration)++;
            return true;
        }
        return false;
    }
    return false;
}


//Evaluate an intersection with a surface
void evaluateBRDF(struct SceneData *bulkData, struct Ray *ray, struct HitInformation *hitInformation, float3 *accumEmission,
             float3 *accumBRDF, int bounce) {
    //Get the BRDF for the material hit.
    //The hierarchy is: fresnel -> refraction -> reflection -> diffuse for the surface
    //For the volume: absorption -> scattering -> emission

    // calculate texture coordinates
    calculateMapCoordinates(bulkData, ray, hitInformation);
    normalMap(bulkData, ray, hitInformation);
    bumpMap(bulkData, ray, hitInformation);

    //Diffuse mask
    float3 diffuseMask = getDiffuseMap(bulkData, ray, hitInformation);

    //ADD NORMALMAP/BUMP CALL HERE (IMPLEMENT IN MAPTOOLS AS WELL)
    //To do this, use the information stored in hitInformation and bulkData to access the map data
    //This follows the concept of a pipeline -> you transfer the data by changing the information in hitInformation
    //e.g. if you want to change the normal vector, modify the "hitNormal" variable
    //As bump maps might be not suited for refraction, consider moving their processing to  "skipRefraction:"
    //Calculate the fresnel term
    float l_cosTheta = dot(hitInformation->hitNormal, -ray->dir);
    float R0 = pow((hitInformation->currentMaterial.Ni - hitInformation->hitMaterial.Ni) /
                   (hitInformation->currentMaterial.Ni + hitInformation->hitMaterial.Ni), 2); //e.g. 0.04 for air/glas
    float fresnel = schlickFresnel(l_cosTheta, R0, hitInformation->currentMaterial.Ni, hitInformation->hitMaterial.Ni);
    fresnel = clamp(fresnel, 0.0f, 1.0f);

    //Now that we have the fresnel term, we can calculate the reflection and refraction
    //the fresnel term describes the ratio of reflection and refraction
    float refractiveAmount = (1.0f - fresnel) * hitInformation->hitMaterial.d;
    float reflectiveAmount = hitInformation->hitMaterial.Ks.x * fresnel;
    if (reflectiveAmount < hitInformation->hitMaterial.Ks.y) {
        reflectiveAmount = hitInformation->hitMaterial.Ks.y;
    }

    //Generate a random number to decide the further path
    float randCheck = generateRandom(&(bulkData->seed0), &(bulkData->seed1));

    //DEBUG______________________________________________________________________________________________________________________
    /*if (reflectiveAmount > 0.0f){
        printf("Reflective amount: %f\n", reflectiveAmount);
        printf("randCheck: %f\n", randCheck);
    }*/
    //DEBUG______________________________________________________________________________________________________________________


    //Check the refraction first (if there is any)
    if (randCheck < refractiveAmount) {
        hitInformation->brdfType = 2;

        //If the ray is inside the material and about to exit, we have to set the currentMaterial to air
        if (hitInformation->insideMaterial && hitInformation->hitObjectId == hitInformation->currentObjectId) {
            hitInformation->currentMaterial = bulkData->airMaterial;
        }

        float n1 = hitInformation->insideMaterial ? hitInformation->hitMaterial.Ni : hitInformation->currentMaterial.Ni;
        float n2 = hitInformation->insideMaterial ? hitInformation->currentMaterial.Ni : hitInformation->hitMaterial.Ni;

        // Calculate the refraction index ratio
        float eta = n1 / n2;

        // Compute the cosine of the angle between the incident vector and the normal
        float cosThetaI = -dot(hitInformation->hitNormal, ray->dir);
        // Compute the sine squared of the angle between the refracted vector and the normal
        float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);

        // Total Internal Reflection
        if (sin2ThetaT > 1.0f)
            goto skipRefraction;

        // Compute the cosine of the angle between the refracted vector and the normal
        float cosThetaT = sqrt(1.0f - sin2ThetaT);

        // Compute the refracted vector
        float3 refractedVector = eta * ray->dir + (eta * cosThetaI - cosThetaT) * hitInformation->hitNormal;
        hitInformation->pdf = 1.0f;

        //Correct hitPoint... here, we set the hitpoint behind the surface as we "penetrate" it
        hitInformation->hitPoint += ray->dir * EPSILON;
        ray->orig = hitInformation->hitPoint;

        //The ray enters a translucent material
        if (!hitInformation->insideMaterial) {
            ray->dir = refractedVector;
            hitInformation->insideMaterial = true;
            hitInformation->currentMaterial = hitInformation->hitMaterial;
            hitInformation->currentObjectId = hitInformation->hitObjectId;
            return;
        }
            //The ray already is inside a translucent material. This means we are either in
            //another translucent material, we hit an internal face of a given object, or we
            //Hit the surface of the object again (exiting it)
        else {
            hitInformation->insideMaterial = false;
            return;
        }
    }

    skipRefraction:


    //Check the reflection
    if (randCheck < reflectiveAmount) {
        hitInformation->brdfType = 1;
        if (hitInformation->hitMaterial.Ns == 1.0f) {
            //Mirror reflection
            ray->dir = ray->dir - 2.0f * dot(hitInformation->hitNormal, ray->dir) * hitInformation->hitNormal;
            hitInformation->pdf = 1.0f;
            //Correct hitPoint
            hitInformation->hitPoint -= ray->dir * EPSILON;
            ray->orig = hitInformation->hitPoint;
            *accumBRDF *= hitInformation->hitMaterial.Tf;
            return;
        }

        //Correct hitPoint
        hitInformation->hitPoint -= ray->dir * EPSILON;
        ray->orig = hitInformation->hitPoint;
        ray->dir = sampleHemisphereCosineWeighted(&(bulkData->seed0), &(bulkData->seed1), ray->dir - 2.0f *
                                                                                                     dot(hitInformation->hitNormal,
                                                                                                         ray->dir) *
                                                                                                     hitInformation->hitNormal,
                                                  hitInformation->hitNormal, (1.0f - hitInformation->hitMaterial.Ns));
        hitInformation->pdf = 1.0f;
        *accumBRDF *= hitInformation->hitMaterial.Tf;
        return;
    }

    hitInformation->brdfType = 0;
    //Get the new ray direction (currently only diffuse, later done by BRDF)
    ray->dir = sampleHemisphereCosineWeighted(&(bulkData->seed0), &(bulkData->seed1), hitInformation->hitNormal,
                                              hitInformation->hitNormal, 1.0f);
    //REMOVE PDF LATER
    hitInformation->pdf = 1.0f;
    if (bounce > 0)
        hitInformation->pdf = dot(normalize(ray->dir), normalize(hitInformation->hitNormal));

    //Correct hitPoint
    hitInformation->hitPoint -= ray->dir * EPSILON;
    ray->orig = hitInformation->hitPoint;


    *accumBRDF *= hitInformation->hitMaterial.Kd * diffuseMask;
}