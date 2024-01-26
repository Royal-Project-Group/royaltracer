#define EPSILON 3e-6f

typedef struct {
    int materialID;
    int objectID;
    float3 v1, v2, v3;
    float3 centroid;
    float3 vn;
    float2 uv1, uv2, uv3;

    // paramaters that can be precalulated for each triangle for map coordinates
    float3 vv0, vv1;
    float d00, d01, d11, invDenom;
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
    float3 orig;
    float3 dir;
} Ray;

// kernel responsible for transforming the triangles
__kernel void gpuKernel(
        __global Triangle* triangles,
        __global float4* transformation,
        __global Triangle* output,
        int d_triCount){

        int id = get_global_id(0);

        Triangle result;
        float3 vertices[3] = {triangles[id].v1, triangles[id].v2, triangles[id].v3};

        for(int i = 0; i < 3; i++)
        {
            float4 vertex = {vertices[i].x, vertices[i].y, vertices[i].z, 1.0f};

            // apply transformation

            float4 erg;
            erg.x = vertex.x * transformation[0].x + vertex.y * transformation[0].y + vertex.z * transformation[0].z + vertex.w * transformation[0].w;
            erg.y = vertex.x * transformation[1].x + vertex.y * transformation[1].y + vertex.z * transformation[1].z + vertex.w * transformation[1].w;
            erg.z = vertex.x * transformation[2].x + vertex.y * transformation[2].y + vertex.z * transformation[2].z + vertex.w * transformation[2].w;
            erg.w = vertex.x * transformation[3].x + vertex.y * transformation[3].y + vertex.z * transformation[3].z + vertex.w * transformation[3].w;

            float inverseW = 1.0f / erg.w;
            erg.x *= inverseW;
            erg.y *= inverseW;
            erg.z *= inverseW;

            vertices[i] = (float3){erg.x, erg.y, erg.z};
        }

        // keep certain values from the original triangle
        result.materialID = triangles[id].materialID;

        result.v1 = vertices[0];
        result.v2 = vertices[1];
        result.v3 = vertices[2];

        float2 uvTriangle[3] = {triangles[id].uv1, triangles[id].uv2, triangles[id].uv3};

        // iterate over all uv coordinates

        result.uv1 = uvTriangle[0];
        result.uv2 = uvTriangle[1];
        result.uv3 = uvTriangle[2];

        // calculate normal
        float3 edge1 = (float3){result.v2.x - result.v1.x, result.v2.y - result.v1.y, result.v2.z - result.v1.z};
        float3 edge2 = (float3){result.v3.x - result.v1.x, result.v3.y - result.v1.y, result.v3.z - result.v1.z};

        float3 normal;
        normal.x = 1.0f * (edge1.y * edge2.z - edge1.z * edge2.y);
        normal.y = 1.0f * (edge1.z * edge2.x - edge1.x * edge2.z);
        normal.z = 1.0f * (edge1.x * edge2.y - edge1.y * edge2.x);

        // normalize the result
        float inverseLength = 1.0f/sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        normal.x *= inverseLength;
        normal.y *= inverseLength;
        normal.z *= inverseLength;

        result.vn = normal;


        // calculate stuff for map coordinates
        result.vv0 = result.v2 - result.v1;
        result.vv1 = result.v3 - result.v1;

        result.d00 = dot(result.vv0, result.vv0);
        result.d01 = dot(result.vv0, result.vv1);
        result.d11 = dot(result.vv1, result.vv1);

        result.invDenom = 1.0f / (result.d00 * result.d11 - result.d01 * result.d01);


        //printf("Result: %f, %f, %f \n", result.v1.x, result.v1.y, result.v1.z);

        output[id] = result;
}
