//
// Created by m on 23.11.2023.
//

float fmaxf(float a, float b) {
    return (a > b) ? a : b;
}


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


float3 sampleHemisphereCosineWeighted(
        uint *seed0,
        uint *seed1,
        float3 sampleVector,
        float3 normalVector,
        float coneSize)
{
    float u1 = generateRandom(seed0,seed1);
    float u2 = generateRandom(seed0,seed1);

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


float3 sampleEqualWeighted(uint *seed0, uint *seed1) {
    float theta = 2.0f * M_PI_F * generateRandom(seed0, seed1);  // azimuthal angle, [0, 2π]
    float phi = acos(1.0f - 2.0f * generateRandom(seed0, seed1));  // polar angle, [0, π]

    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return (float3)(x, y, z);
}


float schlickFresnel(float cosTheta, float R0, float eta1, float eta2) {
    float reflectance;

    // Check for total internal reflection
    float sinThetaI = native_sqrt(1.0f - cosTheta * cosTheta);
    float sinThetaT = (eta1 / eta2) * sinThetaI;

    if (sinThetaT * sinThetaT > 1.0f) {
        return 1.0f; // Total internal reflection
    }

    float oneMinusCosTheta = 1.0f - cosTheta;

    reflectance = R0 + (1.0f - R0) * pow(oneMinusCosTheta, 5);

    return reflectance;
}