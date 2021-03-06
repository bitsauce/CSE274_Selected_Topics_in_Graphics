#pragma once

#include <optixu/optixu_math_namespace.h> 

using namespace optix;

enum RayTypes
{
	GEOMETRY_HIT_RAY,
	SHADOW_RAY,
	GROUND_TRUTH_RAY,
	NUM_RAYS
};

// Point light
struct BasicLight
{
	float3 pos;
	float3 color;
	int    casts_shadow;
	int    padding;
};

// Parallelogram light
struct ParallelogramLight
{
	float3 corner;
	float3 v1, v2;
	float3 normal;
	float3 emission;
};

//--------------------------------------------------------------
// Per-ray data structs
//--------------------------------------------------------------

struct PerRayData_distances
{
	float3       color;              // Diffuse color
	float2       projected_distance; // Projected distance to light
	float        d1, d2_min, d2_max; // Sampled distances
};

struct PerRayData_geometry_hit
{
	float3 color;
	float  object_id;
	float3 geometry_hit;
	float3 geometry_normal;
	float3 ffnormal;
};

struct PerRayData_ground_truth
{
	float3       color;         // Diffuse color
	unsigned int seed;          // Seed for random sampling
};

struct PerRayData_shadow
{
	bool hit;
	float3 hit_point;
};