//
// Created by m on 23.11.2023.
//


void IntersectTri(struct Ray* ray, __global struct TriBASE* tri, uint instPrim )
{
    //Backface culling
    /*if(dot(tri->vn, -ray->dir) < 0.0f){
        return;
    }*/

    float3 v0 = (float3)(tri->v1.x, tri->v1.y, tri->v1.z);
    float3 v1 = (float3)(tri->v2.x, tri->v2.y, tri->v2.z);
    float3 v2 = (float3)(tri->v3.x, tri->v3.y, tri->v3.z);
    float3 edge1 = v1 - v0, edge2 = v2 - v0;
    float3 h = cross( ray->dir, edge2 );
    float a = dot( edge1, h );
    if (a > -0.00001f && a < 0.00001f) return; // ray parallel to triangle
    float f = 1 / a;
    float3 s = ray->orig - v0;
    float u = f * dot( s, h );
    if (u < 0 || u > 1) return;
    const float3 q = cross( s, edge1 );
    const float v = f * dot( ray->dir, q );
    if (v < 0 || u + v > 1) return;
    const float t = f * dot( edge2, q );
    if (t > 0.0001f && t < ray->hit.t)
        ray->hit.t = t, ray->hit.instPrim = instPrim;
}




float IntersectAABB(struct Ray* ray, __global struct BVHNode* bnode )
{
    __global struct BVHNode* node = &(*bnode);

    float tx1 = (node->min.x - ray->orig.x) * ray->rD.x, tx2 = (node->max.x - ray->orig.x) * ray->rD.x;
    float tmin = min( tx1, tx2 ), tmax = max( tx1, tx2 );
    float ty1 = (node->min.y - ray->orig.y) * ray->rD.y, ty2 = (node->max.y - ray->orig.y) * ray->rD.y;
    tmin = max( tmin, min( ty1, ty2 ) ), tmax = min( tmax, max( ty1, ty2 ) );
    float tz1 = (node->min.z - ray->orig.z) * ray->rD.z, tz2 = (node->max.z - ray->orig.z) * ray->rD.z;
    tmin = max( tmin, min( tz1, tz2 ) ), tmax = min( tmax, max( tz1, tz2 ) );
    if (tmax >= tmin && tmin < ray->hit.t && tmax > 0) return tmin; else return 1e30f;

}

void BVHIntersect(struct Ray* ray, uint instanceIdx,
                  __global struct TriBASE * tri, __global struct BVHNode* bvhNode, __global uint* triIdx, int tri_Cnt)
{
    //printf("idx: %u\n", triIdx[2]);
    //Prepare the ray
    ray->hit.t = FLT_MAX;
    ray->rD = (float3)( native_divide(1 , ray->dir.x),native_divide( 1, ray->dir.y), native_divide(1, ray->dir.z) );
    __global struct BVHNode* node = &bvhNode[0], *stack[16];
    uint stackPtr = 0;
    while (1)
    {
        if (node->triCount > 0) // isLeaf()
        {
            for (uint i = 0; i < node->triCount; i++)
            {
                uint instPrim = (instanceIdx << 20) + triIdx[node->leftFirst + i];
                IntersectTri( ray, &tri[instPrim & 0xfffff], instPrim );
            }
            if (stackPtr == 0) break; else node = stack[--stackPtr];
            continue;
        }

        //FIX: Memory can't keep up with node access -> memory miss
        if(node->leftFirst > tri_Cnt*2){
            printf("ERROR: %d\n", node->leftFirst);
            break;
        }

        __global struct BVHNode* child1 = &bvhNode[node->leftFirst];
        __global struct BVHNode* child2 = &bvhNode[node->leftFirst + 1];
        float dist1 = IntersectAABB( ray, child1 );
        float dist2 = IntersectAABB( ray, child2 );
        if (dist1 > dist2)
        {
            float d = dist1; dist1 = dist2; dist2 = d;
            __global struct BVHNode* c = child1; child1 = child2; child2 = c;
        }
        if (dist1 == 1e30f)
        {
            if (stackPtr == 0) break; else node = stack[--stackPtr];
        }
        else
        {
            node = child1;
            if (dist2 != 1e30f) stack[stackPtr++] = child2;
        }
    }
}


