/* Copyright (c) 2018-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Common.hlsl"

float3 Refract(float3 wi, float3 n, float eta)
{
	// Compute $\cos \theta_\roman{t}$ using Snell's law
	float cosThetaI = dot(n, wi);
	float sin2ThetaI = max(0, float(1 - cosThetaI * cosThetaI));
	float sin2ThetaT = eta * eta * sin2ThetaI;

	// Handle total internal reflection for transmission
	if (sin2ThetaT >= 1) return false;
	float cosThetaT = sqrt(1 - sin2ThetaT);
	return eta * -wi + (eta * cosThetaI - cosThetaT) * float3(n);
}

// ---[ Ray Generation Shader ]---

[shader("raygeneration")]
void RayGen()
{
	const float3 directionalLightDir = normalize(-float3(0, 0, 1));
	const float3 ambientLight = float3(0.5f, 0.5f, 0.5f);

	uint2 LaunchIndex = DispatchRaysIndex().xy;
	uint2 LaunchDimensions = DispatchRaysDimensions().xy;

	float2 d = (((LaunchIndex.xy + 0.5f) / resolution.xy) * 2.f - 1.f);
	float aspectRatio = (resolution.x / resolution.y);

	// Setup the ray
	RayDesc ray;
	ray.Origin = viewOriginAndTanHalfFovY.xyz;
	ray.Direction = normalize((d.x * view[0].xyz * viewOriginAndTanHalfFovY.w * aspectRatio) - (d.y * view[1].xyz * viewOriginAndTanHalfFovY.w) + view[2].xyz);
	ray.TMin = 0.1f;
	ray.TMax = 1000.f;

	// Trace the ray
	HitInfo payload;
	payload.ShadedColorAndHitT = float4(0.f, 0.f, 0.f, 0.f);

	TraceRay(
		SceneBVH,
		RAY_FLAG_NONE,
		0xFF,
		0,
		0,
		0,
		ray,
		payload);

	RTOutput[LaunchIndex.xy] = float4(payload.ShadedColorAndHitT.rgb, 1.f);
	float3 nDotDir = max(dot(payload.normal, directionalLightDir), 0.0);
	if (payload.ShadedColorAndHitT.a < 0 || payload.instanceID <= 0)
	{
		if (payload.ShadedColorAndHitT.a >= 0)
		{
			RTOutput[LaunchIndex.xy].rgb *= nDotDir * 1.5f + ambientLight;
		}
		return;
	}

	float3 prevDir = ray.Direction;

	ray.Origin = ray.Origin + prevDir * payload.ShadedColorAndHitT.a;
	ray.Direction = reflect(prevDir, payload.normal);
	ray.TMin = 0.1f;
	ray.TMax = 1000.f;

	uint hitInstanceID = payload.instanceID;

	TraceRay(
		SceneBVH,
		RAY_FLAG_NONE,
		0x01,
		0,
		0,
		0,
		ray,
		payload);
	if (payload.ShadedColorAndHitT.a >= 0)
	{
		RTOutput[LaunchIndex.xy] += float4(payload.ShadedColorAndHitT.rgb, 1.f);

		RTOutput[LaunchIndex.xy].a = 1.0f;
	}

	ray.Origin = ray.Origin + prevDir * payload.ShadedColorAndHitT.a;
	ray.Direction = refract(prevDir, payload.normal, 1.31f);
	ray.TMin = 0.1f;
	ray.TMax = 1000.f;

	hitInstanceID = payload.instanceID;

	TraceRay(
		SceneBVH,
		RAY_FLAG_NONE,
		0x01,
		0,
		0,
		0,
		ray,
		payload);


	if (payload.instanceID != hitInstanceID && payload.ShadedColorAndHitT.a >= 0)
	{
		RTOutput[LaunchIndex.xy] += float4(payload.ShadedColorAndHitT.rgb, 1.f);

		RTOutput[LaunchIndex.xy].a = 1.0f;
	}
	RTOutput[LaunchIndex.xy].rgb *= nDotDir * 1.5f + ambientLight;
}
