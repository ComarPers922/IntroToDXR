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

// ---[ Structures ]---

struct HitInfo
{
	float4 ShadedColorAndHitT;
	float3 normal;
	uint instanceID;
};

struct Attributes 
{
	float2 uv;
};

// ---[ Constant Buffers ]---

cbuffer ViewCB : register(b0)
{
	float4x4 view;
	float4 viewOriginAndTanHalfFovY;
	float2 resolution;
};

cbuffer MaterialCB : register(b1)
{
	float4 textureResolution;
	float4 crystalTextureResolution;
};

// ---[ Resources ]---

RWTexture2D<float4> RTOutput				: register(u0);
RaytracingAccelerationStructure SceneBVH	: register(t0);

ByteAddressBuffer indices					: register(t1);
ByteAddressBuffer vertices					: register(t2);
ByteAddressBuffer crystalIndices			: register(t3);
ByteAddressBuffer crystalVertices			: register(t4);
Texture2D<float4> albedo					: register(t5);
Texture2D<float4> auxAlbedo					: register(t6);
Texture2D<float4> crystalAlbedo				: register(t7);

// ---[ Helper Functions ]---

struct VertexAttributes
{
	float3 position;
	float2 uv;
	float3 normal;
};

uint3 GetIndices(uint triangleIndex, uint instanceID)
{
	uint baseIndex = (triangleIndex * 3);
	int address = (baseIndex * 4);
	uint3 result = indices.Load3(address);
	if (instanceID == 1)
	{
		result = crystalIndices.Load3(address);
	}
	return result;
}

VertexAttributes GetVertexAttributes(uint triangleIndex, float3 barycentrics, uint instanceID)
{
	uint3 indices = GetIndices(triangleIndex, instanceID);
	VertexAttributes v;
	v.position = float3(0, 0, 0);
	v.uv = float2(0, 0);
	v.normal = float3(0, 0, 0);

	if (instanceID == 1)
	{
		for (uint i = 0; i < 3; i++)
		{
			int address = (indices[i] * 8) * 4;
			v.position += asfloat(crystalVertices.Load3(address)) * barycentrics[i];
			address += (3 * 4);
			v.uv += asfloat(crystalVertices.Load2(address)) * barycentrics[i];
			address += (2 * 4);
			v.normal += asfloat(crystalVertices.Load3(address)) * barycentrics[i];
		}
	}
	else
	{
		for (uint i = 0; i < 3; i++)
		{
			int address = (indices[i] * 8) * 4;
			v.position += asfloat(vertices.Load3(address)) * barycentrics[i];
			address += (3 * 4);
			v.uv += asfloat(vertices.Load2(address)) * barycentrics[i];
			address += (2 * 4);
			v.normal += asfloat(vertices.Load3(address)) * barycentrics[i];
		}
	}

	return v;
}