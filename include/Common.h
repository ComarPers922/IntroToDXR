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

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN			// Exclude rarely-used items from Windows headers.
#endif

#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <dxc/dxcapi.h>
#include <dxc/dxcapi.use.h>

#include <string>
#include <vector>

//--------------------------------------------------------------------------------------
// Macro Defines
//--------------------------------------------------------------------------------------

#define NAME_D3D_RESOURCES 1
#define SAFE_RELEASE( x ) { if ( x ) { x->Release(); x = NULL; } }
#define SAFE_DELETE( x ) { if( x ) delete x; x = NULL; }
#define SAFE_DELETE_ARRAY( x ) { if( x ) delete[] x; x = NULL; }
#define ALIGN(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

enum KeyBoardStatus
{
	NONE = 0, FORWARD = 1, BACK = 2, UP = 4, DOWN = 8, LEFT = 16, RIGHT = 32
};

class MouseStatus
{
public:
	const int SCREEN_WIDTH = GetSystemMetrics(SM_CXFULLSCREEN);
	const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYFULLSCREEN);

	bool isRightClicked = false;
	void SetCurPos(const int& x, const int& y)
	{
		lastX = curX;
		lastY = curY;

		curX = x;
		curY = y;
	}

	void SetRBtnPos(const int& x, const int& y)
	{
		rBtnPosX = x;
		rBtnPosY = y;
	}

	DirectX::FXMVECTOR GetDelta()
	{
		auto deltaX = (curX - rBtnPosX) / SCREEN_WIDTH;
		if (abs(deltaX) < 0.01f)
		{
			deltaX = 0;
		}

		auto deltaY = (curY - rBtnPosY) / SCREEN_HEIGHT;
		if (abs(deltaY) < 0.01f)
		{
			deltaY = 0;
		}

		return DirectX::FXMVECTOR{ deltaX, deltaY };
	}
	float rBtnPosX = 0, rBtnPosY = 0;

private:
	float curX = 0, curY = 0, lastX = 0, lastY = 0;
};

class InstanceIDPool
{
private:
	UINT curID = 0;
public:
	UINT GetNewID()
	{
		return curID++;
	}
};

template<typename T>
class Singleton
{
public:
	static T instance;
};

template<typename T>
T Singleton<T>::instance;