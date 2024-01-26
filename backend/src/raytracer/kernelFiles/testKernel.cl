#define EPSILON 1e-6f
#define NORM 0.57735f
#define airNI 1.0f
#define fogScatterIntensity 0.0f

//Data structs used as containers
typedef struct {
    // ambient color (unrealistic, should be disabled in the pathtracer)
    float3 Ka;
    // diffuse color
    float3 Kd;
    // specular color
    float3 Ks;
    // emissive color
    float3 Ke;
    //Transparency
    float3 Tr;
    //shininess (how perfect are the reflections)
    float Ns;
    //transparency
    float d;
    //Refractive index
    float Ni;

    // id for maps:
    int texture_id;

    // subsurface coefficient (also has to be set in the editor)
    float scatteringIntensity;
    float scatteringRadius;
}Material;
typedef struct {
    int materialID;
    float3 v1, v2, v3;
    float3 vn;
    //float2 uv1, uv2, uv3;
}Triangle;
typedef struct {
    int materialID;
    float3 position;
    float radius;
}Sphere;
typedef struct {
    float3 position;
    float3 lookDirection;
    float3 upVec;
    float fieldOfView;
    int width;
    int height;
}Camera;
typedef struct {
    float3 position;
    float3 Ke;
}PointLight;
typedef struct {
    int width;
    int height;
    int offset;
}Texture;

typedef struct {
    float3 orig;
    float3 dir;
} Ray;




//Utility functions

// Generate a random float between 0 and 1
float generateRandom(uint *seed0, uint *seed1) {
    // Combine two uints to form a ulong
    ulong combined_seed = ((ulong)(*seed0) << 32) | (ulong)(*seed1);

    // PCG Random Number Generation (inlined)
    combined_seed = combined_seed * 6364136223846793005ULL + 1442695040888963407ULL;
    uint xorshifted = ((combined_seed >> 18u) ^ combined_seed) >> 27u;
    uint rot = combined_seed >> 59u;
    ulong randInt = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));

    // Split the combined seed back into two uints
    *seed0 = (uint)(combined_seed >> 32);
    *seed1 = (uint)(combined_seed & 0xFFFFFFFF);

    // Convert to float between 0 and 1
    return (randInt & 0xFFFFFF) / (float)0x1000000;
}



float schlickFresnel(float cosTheta, float R0, float eta1, float eta2) {
    float reflectance;

    // Check for total internal reflection
    float sinThetaI = sqrt(1.0f - cosTheta * cosTheta);
    float sinThetaT = (eta1 / eta2) * sinThetaI;

    if (sinThetaT * sinThetaT > 1.0f) {
        return 1.0f; // Total internal reflection
    }

    float oneMinusCosTheta = 1.0f - cosTheta;

    reflectance = R0 + (1.0f - R0) * pow(oneMinusCosTheta, 5);

    return reflectance;
}

float3 wavelengthToRgb(float len) {

    float3 rgb_out;
    // Constants, Arrays, and Matrix for sRGB and D65 illuminant
    const int LEN_MIN = 380, LEN_MAX = 780, LEN_STEP = 5;

    const float X[] = {0.000160, 0.000662, 0.002362, 0.007242, 0.019110, 0.043400, 0.084736, 0.140638, 0.204492, 0.264737,
                       0.314679, 0.357719, 0.383734, 0.386726, 0.370702, 0.342957, 0.302273, 0.254085, 0.195618, 0.132349,
                       0.080507, 0.041072, 0.016172, 0.005132, 0.003816, 0.015444, 0.037465, 0.071358, 0.117749, 0.172953,
                       0.236491, 0.304213, 0.376772, 0.451584, 0.529826, 0.616053, 0.705224, 0.793832, 0.878655, 0.951162,
                       1.014160, 1.074300, 1.118520, 1.134300, 1.123990, 1.089100, 1.030480, 0.950740, 0.856297, 0.754930,
                       0.647467, 0.535110, 0.431567, 0.343690, 0.268329, 0.204300, 0.152568, 0.112210, 0.081261, 0.057930,
                       0.040851, 0.028623, 0.019941, 0.013842, 0.009577, 0.006605, 0.004553, 0.003145, 0.002175, 0.001506,
                       0.001045, 0.000727, 0.000508, 0.000356, 0.000251, 0.000178, 0.000126, 0.000090, 0.000065, 0.000046,
                       0.000033};
    const float Y[] = {0.000017, 0.000072, 0.000253, 0.000769, 0.002004, 0.004509, 0.008756, 0.014456, 0.021391, 0.029497,
                       0.038676, 0.049602, 0.062077, 0.074704, 0.089456, 0.106256, 0.128201, 0.152761, 0.185190, 0.219940,
                       0.253589, 0.297665, 0.339133, 0.395379, 0.460777, 0.531360, 0.606741, 0.685660, 0.761757, 0.823330,
                       0.875211, 0.923810, 0.961988, 0.982200, 0.991761, 0.999110, 0.997340, 0.982380, 0.955552, 0.915175,
                       0.868934, 0.825623, 0.777405, 0.720353, 0.658341, 0.593878, 0.527963, 0.461834, 0.398057, 0.339554,
                       0.283493, 0.228254, 0.179828, 0.140211, 0.107633, 0.081187, 0.060281, 0.044096, 0.031800, 0.022602,
                       0.015905, 0.011130, 0.007749, 0.005375, 0.003718, 0.002565, 0.001768, 0.001222, 0.000846, 0.000586,
                       0.000407, 0.000284, 0.000199, 0.000140, 0.000098, 0.000070, 0.000050, 0.000036, 0.000025, 0.000018,
                       0.000013 };
    const float Z[] = {0.000705, 0.002928, 0.010482, 0.032344, 0.086011, 0.197120, 0.389366, 0.656760, 0.972542, 1.282500,
                       1.553480, 1.798500, 1.967280, 2.027300, 1.994800, 1.900700, 1.745370, 1.554900, 1.317560, 1.030200,
                       0.772125, 0.570060, 0.415254, 0.302356, 0.218502, 0.159249, 0.112044, 0.082248, 0.060709, 0.043050,
                       0.030451, 0.020584, 0.013676, 0.007918, 0.003988, 0.001091, 0.000000, 0.000000, 0.000000, 0.000000,
                       0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                       0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                       0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                       0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                       0.000000};

    const float MATRIX_SRGB_D65[9] = {             3.2404542, -1.5371385, -0.4985314,
                                                  -0.9692660,  1.8760108,  0.0415560,
                                                   0.0556434, -0.2040259,  1.0572252};

    if(len < LEN_MIN || len > LEN_MAX) {
        rgb_out.x = 0;
        rgb_out.y = 0;
        rgb_out.z = 0;
        return rgb_out;
    }

    len -= LEN_MIN;
    int index = (int)floor(len / LEN_STEP);
    float offset = len - LEN_STEP * index;

    // Interpolate function inlined
    float x = X[index] + offset * (X[index + 1] - X[index]) / LEN_STEP;
    float y = Y[index] + offset * (Y[index + 1] - Y[index]) / LEN_STEP;
    float z = Z[index] + offset * (Z[index + 1] - Z[index]) / LEN_STEP;

    // Matrix multiplication
    float r = MATRIX_SRGB_D65[0]*x + MATRIX_SRGB_D65[1]*y + MATRIX_SRGB_D65[2]*z;
    float g = MATRIX_SRGB_D65[3]*x + MATRIX_SRGB_D65[4]*y + MATRIX_SRGB_D65[5]*z;
    float b = MATRIX_SRGB_D65[6]*x + MATRIX_SRGB_D65[7]*y + MATRIX_SRGB_D65[8]*z;

    /*// Gamma correction inlined
    r = (r <= 0.0031308f) ? 12.92f * r : (1.055f) * pow(r, 1.0f / 2.4f) - 0.055f;
    g = (g <= 0.0031308f) ? 12.92f * g : (1.055f) * pow(g, 1.0f / 2.4f) - 0.055f;
    b = (b <= 0.0031308f) ? 12.92f * b : (1.055f) * pow(b, 1.0f / 2.4f) - 0.055f;*/

    // Clip function inlined
    rgb_out.x = (r < 0) ? 0 : (r > 1) ? 1 : r;
    rgb_out.y = (g < 0) ? 0 : (g > 1) ? 1 : g;
    rgb_out.z = (b < 0) ? 0 : (b > 1) ? 1 : b;

    rgb_out = rgb_out* (float3)(1.7968,1.9422,2.5632);

    return rgb_out;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

float3 sampleHemisphereCosineWeighted(
    uint *m_seed0,
    uint *m_seed1,
    float3 normalVector,
    float3 sampleVector,
    float coneSize)
{
    float u1 = generateRandom(m_seed0,m_seed1);
    float u2 = generateRandom(m_seed0,m_seed1);

    float phi = 2.0f * 3.1415f * u1; // The constant M_PI might require the inclusion of some OpenCL header or a definition.
    float cosTheta = sqrt(1.0f - u2 * coneSize);
    float sinTheta = sqrt(u2 * coneSize);

    float3 sample;
    sample.x = cos(phi) * sinTheta;
    sample.y = sin(phi) * sinTheta;
    sample.z = cosTheta;

    float3 worldUp = (float3)(0.0f, 1.0f, 0.0f);
    if (fabs(dot(sampleVector, worldUp)) > 0.99f) { // FIXED: m_dot -> dot
        worldUp = (float3)(1.0f, 0.0f, 0.0f);
    }
    float3 tangent = normalize(cross(worldUp, sampleVector));
    float3 bitangent = normalize(cross(sampleVector, tangent));

    sample = sample.x * tangent + sample.y * bitangent + sample.z * sampleVector;

    //Check for vectors that reach below the surface and if the do, mirror them up
    if(dot(sample,normalVector)<EPSILON){
        sample = (-sample) - 2.0f * dot(sampleVector, -sample) * sampleVector;
    }

    return sample;
}

float pdfCosHemisphere(float3 sampled_direction, float3 sampleVector, float coneFraction) {
    float cosTheta = dot(normalize(sampled_direction), normalize(sampleVector));

    return cosTheta;
}


float3 sampleEqualWeighted(uint *seed0, uint *seed1) {
    float theta = 2.0f * M_PI_F * generateRandom(seed0, seed1);  // azimuthal angle, [0, 2π]
    float phi = acos(1.0f - 2.0f * generateRandom(seed0, seed1));  // polar angle, [0, π]

    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return (float3)(x, y, z);
}




float3 sampleReflection(
    uint *m_seed0,
    uint *m_seed1,
    Material hitMaterial,
    float3 normalVector,
    float3 reflectionVector
    )
{
    float coneSize = 1.0f - hitMaterial.Ns;
    return sampleHemisphereCosineWeighted(m_seed0, m_seed1, normalVector, reflectionVector, coneSize);

}


float3 sampleRefraction(
    uint *m_seed0,
    uint *m_seed1,
    float3 normalVector,
    float3 incidentVector,
    float n1,
    float n2,
    float waveLength
    )
{
    // Constants for the Cauchy equation for glass
    float A = n2;
    float B = 0.005f;

    // Current ray's wavelength (in micrometers)
    float lambda = waveLength / 1000.0f; // Assuming waveLength is in nanometers, convert to micrometers

    // Calculate adjusted refractive index using the Cauchy equation
    lambda = - lambda;

    lambda += 0.380f + 0.780f;

    float adjusted_n2 = A + (B / (lambda * lambda));

    // Calculate the refraction index ratio
    float eta = n1 / adjusted_n2;

    // Compute the cosine of the angle between the incident vector and the normal
    float cosThetaI = -dot(normalVector, incidentVector);
    // Compute the sine squared of the angle between the refracted vector and the normal
    float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);

    // Total Internal Reflection
    if (sin2ThetaT > 1.0f)
        return (float3)(0.0f, 0.0f, 0.0f);

    // Compute the cosine of the angle between the refracted vector and the normal
    float cosThetaT = sqrt(1.0f - sin2ThetaT);

    // Compute the refracted vector
    float3 refractedVector = eta * incidentVector + (eta * cosThetaI - cosThetaT) * normalVector;

    return refractedVector;
}


float3 sampleDiffuse(
    uint *m_seed0,
    uint *m_seed1,
    float3 normalVector
    )
{
    return sampleHemisphereCosineWeighted(m_seed0, m_seed1, normalVector, normalVector, 1.0f);
}








//NEW function for easier and clearer processing of probabilities
//Returns a direction vector and modifies a color vector to be used by the kernel

float3 determineRayBehaviour(Material hitMaterial, Material currentMaterial, float3 normal, float3 incidentVector, float3* colorVector, uint *m_seed0, uint *m_seed1, bool* insideMaterial, int id, float waveLength, float* pdf){
    normal = normalize(normal);
    incidentVector = normalize(incidentVector);

    //Get the fresnel coefficient first
    float l_cosTheta;

    l_cosTheta = dot(normal, -incidentVector);


    float R0 = pow((currentMaterial.Ni - hitMaterial.Ni)/(currentMaterial.Ni + hitMaterial.Ni), 2); //e.g. 0.04 for air/glas

    float fresnel = schlickFresnel(l_cosTheta, R0, currentMaterial.Ni, hitMaterial.Ni);
    fresnel = clamp(fresnel, 0.0f, 1.0f);

    //Calculate the transparency factor in this point:
    float refractiveAmount = (1.0f-fresnel) * hitMaterial.d;

    //Get a random value:
    float randCheck = generateRandom(m_seed0, m_seed1);

    // Check if ray will be refracted
    if(randCheck < refractiveAmount)
    {
        *pdf = 1.0f;
        float n1 = *insideMaterial ? hitMaterial.Ni : currentMaterial.Ni;
        float n2 = *insideMaterial ? currentMaterial.Ni : hitMaterial.Ni;


        float3 refractedVector = sampleRefraction(m_seed0, m_seed1, normal, incidentVector, n1, n2, waveLength);
        // Check for Total Internal Reflection
        if(length(refractedVector) == 0.0f) {
            // Handle TIR as reflection
            float3 reflectionVector = incidentVector - 2.0f * dot(normal, incidentVector) * normal;
            return sampleReflection(m_seed0, m_seed1, hitMaterial, normal, reflectionVector);
        }
        *insideMaterial = !(*insideMaterial);
        return refractedVector;
    }


    //if the ray is not refracted, calculate the chance to reflect
    float reflectiveAmount = NORM *  length(hitMaterial.Ks);

    /*if(randCheck < reflectiveAmount){
        float3 reflectionVector = incidentVector - 2.0f * dot(normal, incidentVector) * normal;
        float3 reflectionSample = sampleReflection(m_seed0,m_seed1, hitMaterial, normal, reflectionVector);
        float pdfValue = pdfCosHemisphere(reflectionSample,reflectionVector,1.0f-hitMaterial.Ns);
        *pdf = pdfValue;
        return reflectionSample;
    }*/
    float3 diffuseSample = sampleDiffuse(m_seed0,m_seed1, normal);
    float pdfValue = pdfCosHemisphere(diffuseSample, normal, 1.0f);
    *pdf = pdfValue;
    return diffuseSample;


}



bool solveQuadratic(const float a, const float b, const float c, float* x0, float* x1) {
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;

    float q = (b > 0) ? -0.5f * (b + sqrt(discr)) : -0.5f * (b - sqrt(discr));
    *x0 = q / a;
    *x1 = c / q;
    if (*x0 > *x1) {
        float tmp = *x0;
        *x0 = *x1;
        *x1 = tmp;
    }

    return true;
}

bool intersectSph(
    Ray* ray,
    __global Sphere* sphere,
    float* t,
    bool recognizeBackfaces
    )
{
    float t0, t1;

    float3 L = sphere->position - ray->orig;
    float tca = dot(L, ray->dir);

    float d2 = dot(L, L) - tca * tca;
    if (d2 > sphere->radius * sphere->radius) return false;

    float thc = sqrt(sphere->radius * sphere->radius - d2);
    t0 = tca - thc;
    t1 = tca + thc;

    // Analytic solution
    L = ray->orig - sphere->position;
    float a = dot(ray->dir, ray->dir);
    float b = 2 * dot(ray->dir, L);
    float c = dot(L, L) - sphere->radius * sphere->radius;
    if (!solveQuadratic(a, b, c, &t0, &t1)) return false;

    // Modify this part to consider backfaces if the boolean is true
    if (t0 < EPSILON) {
        t0 = t1;
        if (t0 < EPSILON) return false;
    }

    *t = t0;

    return true;
}

bool intersectTri(
    Ray* ray,
    const __global Triangle* tri,
    float* t,
    bool recognizeBackfaces
)
{
    float3 edge1 = tri->v2 - tri->v1;
    float3 edge2 = tri->v3 - tri->v1;
    float3 h = cross(ray->dir, edge2);
    float a = dot(edge1, h);

    if(dot(tri->vn, -ray->dir) < 0.0f && !recognizeBackfaces){
        return false;
    }

    if(fabs(a) < EPSILON)
        return false;

    float f = 1.0f / a;
    float3 s = ray->orig - tri->v1;
    float u = f * dot(s, h);

    if(u < 0.0f - EPSILON || u > 1.0f + EPSILON)
        return false;

    float3 q = cross(s, edge1);
    float v = f * dot(ray->dir, q);

    if(v < 0.0f - EPSILON || u + v > 1.0f + EPSILON)
        return false;

    float s_t = f * dot(edge2, q);

    if(s_t > EPSILON) {
        *t = s_t;
        return true;
    }
    return false;
}






//We have to do seperate checks for triangles and spheres.

//Sphere Check: returns the index of the sphere, -1 if none is hit
int sphereIntersection(
    __global Sphere* spheres,
    int sphCount,
    Ray* ray,
    float* s_t,
    bool hitBackfaces
    )
{
    float closestDistance = FLT_MAX;
    int hitID = -1;
    float t;
    for (int i = 0; i < sphCount; i++) {
        if (intersectSph(ray, &spheres[i], &t,hitBackfaces)) { // FIXED: Added missing arguments
            if (t < closestDistance) {
                closestDistance = t;
                hitID = i;
            }
        }
    }
    *s_t = closestDistance; // FIXED: Dereferenced s_t to assign value
    //if(*s_t<FLT_MAX)
    //printf("Distance: %f at hit: %d \n", *s_t, hitID);
    return hitID;
}




//Triangle Check: returns the index of the sphere, -1 if none is hit
int triangleIntersection(
    __global Triangle* triangles,
    int triCount,
    Ray* ray,
    float* s_t,
    bool hitBackfaces
    )
{
    //Store the intersect value
    float closestDistance = FLT_MAX;
    int hitID = -1;

    //Get the initial impact point
    float t;
    for (int i = 0; i < triCount; i++) {
        if (intersectTri(ray, &triangles[i], &t,hitBackfaces)) {
            if (t < closestDistance) {
                closestDistance = t;
                hitID=i;
            }
        }
    }
    *s_t = closestDistance;
    return hitID;
}




//Function for checking all lights directly...
//On each sample point (bounce), check all the lights we hit (Point and Directional). Every light the ray can hit, add its lighting strength based on the distance to the light
float3 sampleSimpleLights(
       __global Triangle* triangles,
       __global Sphere* spheres,
       __global PointLight* pointLights,
       /*__global DirectionalLights* dirLights,*/
       Material hitMaterial,
       float3 hitNormal,
       int triCount,
       int sphCount,
       int poiCount,
       /*int dirCount,*/
       float3 hitPoint
    )
    {

    float3 weightedIntensity = (float3)(0.0f,0.0f,0.0f);

    //Iterate through all simple light sources and check if there is a intersection with an object between the hitpoint and the light source
    for(int i=0;i<poiCount;i++){
                float hitDistSph = FLT_MAX;
                float hitDistTri = FLT_MAX;
                //to calculate the distance
                float t;



                Ray ray;
                ray.orig= hitPoint;
                ray.dir = pointLights[i].position - hitPoint;
                float lDir = length(ray.dir);

                ray.dir = normalize(ray.dir);


                //Do the sphere intersection
                int hitIDSph = sphereIntersection(spheres, sphCount, &ray, &hitDistSph, false);
                int hitIDTri = triangleIntersection(triangles, triCount, &ray, &hitDistTri, false);

                if(hitDistSph + EPSILON > lDir && hitDistTri + EPSILON > lDir){
                    // Calculate the distance factor
                    float distanceFactor = 1 / (length(lDir) * length(lDir) * 4.0f * 3.1415f);

                    // Calculate the angle factor
                    float angleFactor = dot(ray.dir, normalize(hitNormal));

                    weightedIntensity += (angleFactor * distanceFactor) * pointLights[i].Ke;
                }
    }

    /*//Iterate through all simple light sources and check if there is a intersection with an object between the hitpoint and the light source
    for(int i=0;i<poiCount;i++){
                float hitDistSph = FLT_MAX;
                float hitDistTri = FLT_MAX;
                //to calculate the distance
                float t;



                Ray ray;
                ray.orig= hitPoint;
                ray.dir = -1.0f * dirLights[i].direction;


                //Do the sphere intersection
                int hitIDSph = sphereIntersection(spheres, sphCount, &ray, &hitDistSph, false);
                int hitIDTri = triangleIntersection(triangles, triCount, &ray, &hitDistTri, false);

                if(!(hitIDSph != -1 || hitIDTri != -1)){
                    // No distance Factor here

                    // Calculate the angle factor
                    float angleFactor = dot(normalize(ray.dir), normalize(hitNormal));

                    weightedIntensity += angleFactor * pointLights[i].Ke;
                }
    }*/


    weightedIntensity = (float3)(fabs(weightedIntensity.x), fabs(weightedIntensity.y), fabs(weightedIntensity.z));
    return weightedIntensity;
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
// returns the u, v and w value for the barycentric coordinates
float3 getUVOfHitPandTri(Triangle hitTriangle, float3 P){
    float3 a = hitTriangle.v1;
    float3 b = hitTriangle.v2;
    float3 c = hitTriangle.v3;

    float3 v0 = b-a;
    float3 v1 = c-a;
    float3 v2 = P-a;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00*d11 -d01*d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    float3 erg = {u, v, w};
    return erg;
}


//Return color vector
float3 tracePath(
        __global Triangle* triangles,
        __global Sphere* spheres,
        __global PointLight* pointLights,
        __global Material* materials,
        __global Texture* textures,
        __global float3* texturedata,
        int triCount,
        int sphCount,
        int poiCount,
        int matCount,
        int texCount,
        int bounces,
        float3 r_dir,
        float3 r_orig,
        uint* seed0,
        uint* seed1,
        __global float4* albedoOut,
        __global float4* normalOut,
        int id,
        float waveLength
)
{

    //Output value
    float3 accum_color = (float3)(0.0f,0.0f,0.0f);
    //Color mask
    //
    float3 mask = (float3)(1.0f,1.0f,1.0f);


    float3 dir=normalize(r_dir);
    float3 orig=r_orig;

    //The current object the ray bounces of from: stop rebouncing
    int currentObj=-1;
    int hitType=-1; //Save the type of the object hit, e.g. sphere(0), triangle(1)
    Material airMaterial;
    airMaterial.Ni = 1.0f;
    Material currentMaterial = airMaterial;
    bool isRefracting = false;
    int subsurfaceIteration = 0;
    float P_term = 0.3;  // termination probability russian roulette

    for (int j = 0; j < 1; j++){
        float hitDistSph = FLT_MAX;
        float hitDistTri = FLT_MAX;
        //to calculate the distance
        float t;

        Ray ray;
        ray.orig= orig;
        ray.dir = dir;

        //Do the sphere intersection
        int hitIDSph = sphereIntersection(spheres, sphCount, &ray, &hitDistSph, isRefracting);
        int hitIDTri = triangleIntersection(triangles, triCount, &ray, &hitDistTri, isRefracting);

        //Check which one is closer
        if(hitIDSph != -1 || hitIDTri != -1){
            //printf("hit bounce: %d\n", j);
            if(hitDistSph<=hitDistTri){
                hitType=0;
                currentObj = hitIDSph;
                t=hitDistSph;
            }
            else{
                hitType=1;
                //printf("HitTri: %d\n",hitIDTri);
                currentObj = hitIDTri;
                t=hitDistTri;
            }
        }
        float3 normal;
        Material material;
        float3 hitPos = orig + t * dir;


        if(hitType==-1){
            accum_color += mask * (float3)(0.0f, 0.0f, 0.0f);  // Here mask will affect the color added
            return accum_color;
        }
        else if(hitType==0){
            Sphere hitSphere = spheres[currentObj];
            normal = normalize(hitPos - hitSphere.position);
            material = materials[hitSphere.materialID];
        }
        else if(hitType==1){
            Triangle hitTriangle = triangles[currentObj];
            normal = normalize(hitTriangle.vn);
            material = materials[hitTriangle.materialID];
        }
        float3 colorVector;
        float3 dir_temp = dir;
        bool temp_isRefracting = isRefracting;
        int normalizeID = 1; // Assuming 1 is the default value


        //rotate the normal vector in case incident and normal looking in the same direction (backface!)
        if (dot(normal, dir) > 0.0f) {
            normal = -normal;
        }


        // Generate a random value for the scattering check
        float randomScatter = generateRandom(seed0, seed1);

        //Get the length of the hitVector
        float hitLength = length(dir * t);

        // Check for scattering condition
        if (randomScatter < fogScatterIntensity && !isRefracting && j<1) {
            // Update the ray's origin based on scattering
            orig = orig + dir * (t-(EPSILON*2.0f)) * (randomScatter/(fogScatterIntensity+EPSILON*4.0f));

            // Randomize the ray's direction
            dir = normalize(sampleEqualWeighted(seed0, seed1));
            mask *= 0.5f;
        }
        else if(isRefracting && hitLength/currentMaterial.scatteringRadius > randomScatter && subsurfaceIteration < (int)(bounces/2)){
            //Check the subsurfaceScattering
            //Limit the scattering to make it terminate. limit the maximum steps to half of the bounces
            //The longer the normal ray would have been, the more likely it is to subsurface scatter
            orig = orig + dir * t * randomScatter;
            dir = normalize(sampleEqualWeighted(seed0, seed1));
            mask *= currentMaterial.Tr * currentMaterial.scatteringIntensity;
            subsurfaceIteration++;

        }
        else{
            subsurfaceIteration = 0;
            float pdf;
            // Update 'isRefracting' and 'normalizeID' within the function
            dir = determineRayBehaviour(material, currentMaterial, normal, dir, &colorVector, seed0, seed1, &isRefracting, id, waveLength, &pdf);

            // Update currentMaterial based on refraction status
            if (!temp_isRefracting && isRefracting) {
                currentMaterial = material;
                hitPos = hitPos + t *  (EPSILON * normalize(dir));

                //change the color according to the picked wavelength
                float3 rgbWL = wavelengthToRgb(waveLength);
                //printf("wl: %f, R: %f, G: %f, B: %f\n", waveLength, rgbWL.x, rgbWL.y, rgbWL.z);
                //if there is a refraction, apply the random wavelength factor
                float3 wl_factor = (float3)(rgbWL.x * (1/NORM),rgbWL.y * (1/NORM),rgbWL.z* (1/NORM));
                mask*= wl_factor;
            }
            else if (temp_isRefracting && !isRefracting) {
                currentMaterial = airMaterial;
                hitPos = hitPos + t * (EPSILON * normalize(dir));
            }
            else{
                hitPos = hitPos - t * (EPSILON * normalize(dir_temp));
            }

            // Update hitPos based on the new direction
            orig = hitPos;

        if(hitType==1 ){
            Triangle hitTriangle = triangles[currentObj];
            Material mat = materials[hitTriangle.materialID];

            float3 bary = getUVOfHitPandTri(hitTriangle, hitPos);

            float2 t0 = {0.01, 0.01};
            float2 t1 = {0.99, 0.01};
            float2 t2 = {0.99, 0.99};

            float2 pTex = t0 + bary.y*(t1-t0) + bary.z*(t2-t0);


            if(pTex.x >= 0.0f && pTex.x <= 1.0f &&pTex.y >= 0.0f && pTex.y <= 1.0f){

                int textureWidth = textures[mat.texture_id].width;
                int textureHeight = textures[mat.texture_id].height;
                int offset = textures[mat.texture_id].offset;

                int xN = (int)(pTex.x * textureWidth);
                int yN = (int)(pTex.y * textureHeight);

                // printf("Width: %d, Height: %d, Offset: %d, xN: %d, yN: %d\n", textureWidth, textureHeight, offset, xN, yN);

                float3 col = texturedata[xN* textureWidth + yN + offset];

                mask *= col;

                       /*   float blackWhite = getTex(pTex.x, pTex.y);
                            mask.y *= blackWhite;
                            mask.z *= blackWhite;*/

            }
        }

            // Update the accumulated brightness
            mask *= material.Kd;  // Modulate the mask with the material color

            //pdf
            mask *= pdf;

            // Get the color and emission vector from the object that was hit
            accum_color += (material.Ke + sampleSimpleLights(triangles, spheres, pointLights,material,normal, triCount,sphCount,  poiCount, hitPos)) * mask;
		}

        // Russian Roulette termination
        float r = generateRandom(seed0, seed1);
        if (r < P_term && j > 3) {
            return accum_color;  // Terminate the ray
        }
    }
    accum_color /= (1 - P_term);
    return accum_color;
}


//Return color vector
void tracePictureMaps(
       __global Triangle* triangles,
       __global Sphere* spheres,
       __global Material* materials,
       int triCount,
       int sphCount,
       int matCount,
       float3 r_dir,
       float3 r_orig,
       __global float4* albedoOut,
       __global float4* normalOut,
       int id
    )
{
    float3 mask = (float3)(1.0f,1.0f,1.0f);

    float3 dir=normalize(r_dir);
    float3 orig=r_orig;

    //The current object the ray bounces of from
    int currentObj=-1;
    int hitType=-1; //Save the type of the object hit, e.g. sphere(0), triangle(1)

    float hitDistSph = FLT_MAX;
    float hitDistTri = FLT_MAX;
    //to calculate the distance
    float t;

    Ray ray;
    ray.orig= orig;
    ray.dir = dir;



    //Do the sphere intersection
    int hitIDSph = sphereIntersection(spheres, sphCount, &ray, &hitDistSph, false);
    int hitIDTri = triangleIntersection(triangles, triCount, &ray, &hitDistTri, false);

    //Check which one is closer
    if(hitIDSph != -1 || hitIDTri != -1){
        //printf("hit bounce: %d\n", j);
        if(hitDistSph<=hitDistTri){
            hitType=0;
            currentObj = hitIDSph;
            t=hitDistSph;
        }
        else{
            hitType=1;
            //printf("HitTri: %d\n",hitIDTri);
            currentObj = hitIDTri;
            t=hitDistTri;
        }
    }
    float3 normal;
    Material material;
    float3 hitPos = orig + t * dir;

    if(hitType==-1){
        material.Kd = (float3)(0.0f, 0.0f, 0.0f);  // Here mask will affect the color added
    }
    else if(hitType==0){
        Sphere hitSphere = spheres[currentObj];
        normal = normalize(hitPos - hitSphere.position);
        material = materials[hitSphere.materialID];
    }
    else if(hitType==1){
        Triangle hitTriangle = triangles[currentObj];
        normal = normalize(hitTriangle.vn);
        material = materials[hitTriangle.materialID];
    }

    albedoOut[id] = (float4)(material.Kd.x,material.Kd.y, material.Kd.z, id);
    normalOut[id] = (float4)(normal.x, normal.y, normal.z, id);

}




float sample_hero_wavelength(float min_wavelength, float max_wavelength, uint *seed0, uint *seed1) {
    float rand_val = generateRandom(seed0,seed1);  // Generate a random value between 0 and 1
    return min_wavelength + rand_val * (max_wavelength - min_wavelength);
}

inline float gaussian_weight(float hero_wavelength, float target_wavelength, float sigma) {
    float diff = target_wavelength - hero_wavelength;
    return exp(-0.5f * diff * diff / (sigma * sigma)) / (sigma * sqrt(2.0f * 3.1415f));
}

float3 distribute_radiance(float3 hero_emission, float hero_wavelength, float min_wavelength, float max_wavelength, int num_wavelengths, float sigma) {
    float3 distributed_emission = (float3)(0.0f, 0.0f, 0.0f);
    float wavelength_step = (max_wavelength - min_wavelength) / (num_wavelengths - 1);

    for (int i = 0; i < num_wavelengths; i++) {
        float target_wavelength = min_wavelength + i * wavelength_step;
        float weight = gaussian_weight(hero_wavelength, target_wavelength, sigma);
        distributed_emission += hero_emission * weight;
    }

    return distributed_emission;
}



__kernel void gpuKernel(
        __global Triangle* triangles,
        __global Sphere* spheres,
        __global PointLight* pointLights,
        __global Material* materials,
        __global Texture* textures,
        __global float3* texturedata,
        __global Camera* cam,
        __global float4* pixelOut,
        __global float4* albedoOut,
        __global float4* normalOut,
        int triCount,
        int sphCount,
        int poiCount,
        int matCount,
        int texCount,
        int bounces,
        int samples
       )
{
    int id = get_global_id(0);
    //printf("OUT: ID: %d\n", id);
    //Get the pixel coordinate
    int x = id % cam->width;
    int y = id / cam->width;



    //Generate the seeds for the random generator
    uint seed0 = x;
    uint seed1 = y;

    // Derive the right vector from lookDirection and upVec
    float3 right = normalize(cross(cam->lookDirection, cam->upVec));
    float3 up = normalize(cam->upVec);
    float foV = cam->fieldOfView * (-3.1415f/180);

    // Calculate the pixel size based on FOV and width
    float pixelSize = 2 * tan(foV * 0.5f) / cam->width;

    float3 screenCenter = cam->position + normalize(cam->lookDirection);

    float3 pixelPos = screenCenter + (((x - cam->width * 0.5f) * pixelSize * right) + ((y - cam->height * 0.5f) * pixelSize * up));

    //First direction, origin is cam position
    float3 dir = normalize(pixelPos - cam->position);
    float3 orig = cam->position;

        //debug
        //printf("OUT: %f, %f, %f\n", dir.x, dir.y, dir.z);

    float3 accum_emission = (float3) (0.0f, 0.0f, 0.0f);

    float min_wavelength = 380.0f;
    float max_wavelength = 780.0f;
    int num_wavelengths = 4;  // C

    // Calculate p for uniform sampling
    float p_uniform = 1.0f / (max_wavelength - min_wavelength);

    for (int i = 0; i < 50; i++) {
        seed0+=i;
        seed1+=i;
        // Sample hero wavelength using your function
        float hero_wavelength = sample_hero_wavelength(min_wavelength, max_wavelength, &seed0, &seed1);

        // p for hero wavelength is uniform
        float p_hero = p_uniform;

float3 hero_emission = tracePath(triangles, spheres, pointLights, materials, textures, texturedata, triCount, sphCount, poiCount, matCount, texCount, bounces, dir, orig, &seed0, &seed1, albedoOut, normalOut, id, hero_wavelength);;

/*// Initialize path contribution with the hero emission
float3 path_contribution = hero_emission;

// Calculate the sum of p for all wavelengths,
// in this case, it would be 4 * p_uniform for uniform sampling
float sum_p = num_wavelengths * p_uniform;

// Loop for other wavelengths
for (int j = 0; j < num_wavelengths; j++) {
    // Calculate wavelength based on hero wavelength
    float wavelength = fmod(hero_wavelength - min_wavelength + ((float)j / num_wavelengths) * (max_wavelength - min_wavelength),
                            max_wavelength - min_wavelength) + min_wavelength;

    // p for this wavelength is also uniform
    float p_j = p_uniform;

    // Calculate MIS weight
    float w_j = sum_p / (num_wavelengths * p_hero);

    // Estimate emission for this wavelength based on hero emission
    float3 estimated_emission = hero_emission;  // Replace with better estimation logic if available

    // Weight the estimated emission
    float3 weighted_estimated_emission = w_j * estimated_emission;

    // Accumulate weighted estimated emission to the path contribution
    path_contribution += weighted_estimated_emission;
}*/

        // Accumulate the path contribution to the overall emission
        accum_emission += hero_emission;
    }

    // Divide by the number of samples to get the average
    accum_emission /= 50.0f;

    //normalize the output
    //Every value that is higher then 255, cut that off.
    if(accum_emission.x>255.0f)
        accum_emission.x= 255.0f;
    if(accum_emission.y>255.0f)
        accum_emission.y= 255.0f;
    if(accum_emission.z>255.0f)
        accum_emission.z= 255.0f;

    tracePictureMaps(triangles, spheres, materials, triCount, sphCount, matCount, dir, orig, albedoOut, normalOut, id);

    pixelOut[id]=(float4)(accum_emission.x, accum_emission.y, accum_emission.z, id);
}