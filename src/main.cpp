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

/*
 * TODO List:
 * 0. Find a way to load a secondary texture, so there will be two textures in the shader program. ��
 * 1. Use ASSIMP to load a model. I can start with a simple model (like heart.obj) first, then construct a complex sence.
 * 2. Do some simple GI in the simple. (Reflection)
 * 3. Add lightings.
 */

#include <memory>

#include "Window.h"
#include "Graphics.h"
#include "Utils.h"
#include <shlobj.h>
#include <strsafe.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

/**
 * Your ray tracing application!
 */
class DXRApplication
{
public:
	
	void Init(ConfigInfo &config) 
	{		
		// Create a new window
		HRESULT hr = Window::Create(config.width, config.height, config.instance, window, L"Introduction to DirectX Raytracing (DXR)");
		Utils::Validate(hr, L"Error: failed to create window!");

		d3d.width = config.width;
		d3d.height = config.height;
		d3d.vsync = config.vsync;

		// Load a model
		// Utils::LoadModel(config.model, model, material);
		bearResources.model = std::shared_ptr<Model>(Model::ImportMeshFromFileOfIndex(config.model, 1, bearResources.material));
		crystalResources.model = std::shared_ptr<Model>(Model::ImportMeshFromFileOfIndex(config.model, 0, crystalResources.material));
		
		// Initialize the shader compiler
		D3DShaders::Init_Shader_Compiler(shaderCompiler);

		// Initialize D3D12
		D3D12::Create_Device(d3d);
		D3D12::Create_Command_Queue(d3d);
		D3D12::Create_Command_Allocator(d3d);
		D3D12::Create_Fence(d3d);		
		D3D12::Create_SwapChain(d3d, window);
		D3D12::Create_CommandList(d3d);
		D3D12::Reset_CommandList(d3d);

		// Create common bearResources
		D3DResources::Create_Descriptor_Heaps(d3d, bearResources);
		D3DResources::Create_BackBuffer_RTV(d3d, bearResources);

		D3DResources::Create_Vertex_Buffer(d3d, bearResources, *bearResources.model);
		D3DResources::Create_Index_Buffer(d3d, bearResources, *bearResources.model);

		D3DResources::Create_Vertex_Buffer(d3d, crystalResources, *crystalResources.model);
		D3DResources::Create_Index_Buffer(d3d, crystalResources, *crystalResources.model);

		D3DResources::Create_Texture(d3d, bearResources, bearResources.material);
		D3DResources::Create_Texture(d3d, auxResources, CYAN_TEX_PATH);
		D3DResources::Create_Texture(d3d, crystalResources, crystalResources.material);

		D3DResources::Create_View_CB(d3d, bearResources);
		D3DResources::Create_Material_CB(d3d, bearResources, crystalResources);
		
		// Create DXR specific bearResources
		DXR::Create_Bottom_Level_AS(d3d, mainDXR, bearResources, *bearResources.model);
		DXR::Create_Bottom_Level_AS(d3d, auxDXR, crystalResources, *crystalResources.model);

		DXR::Create_Bottom_Level_AS_Instance(d3d, mainDXR, bearResources);
		DXR::Create_Bottom_Level_AS_Instance(d3d, auxDXR, crystalResources);
		DXR::Create_Top_Level_AS(d3d, mainDXR, auxDXR, bearResources);


		DXR::Create_DXR_Output(d3d, bearResources);
		DXR::Create_Descriptor_Heaps(d3d, mainDXR, bearResources, auxResources, crystalResources);
		DXR::Create_RayGen_Program(d3d, mainDXR, shaderCompiler);
		DXR::Create_Miss_Program(d3d, mainDXR, shaderCompiler);
		DXR::Create_Closest_Hit_Program(d3d, mainDXR, shaderCompiler);
		DXR::Create_Pipeline_State_Object(d3d, mainDXR);
		DXR::Create_Shader_Table(d3d, mainDXR, bearResources);

		d3d.cmdList->Close();
		ID3D12CommandList* pGraphicsList = { d3d.cmdList };
		d3d.cmdQueue->ExecuteCommandLists(1, &pGraphicsList);

		D3D12::WaitForGPU(d3d);
		D3D12::Reset_CommandList(d3d);
	}
	
	void Update() 
	{
		D3DResources::Update_View_CB(d3d, bearResources);
	}

	void Render() 
	{		
		DXR::Build_Command_List(d3d, mainDXR, bearResources);
		D3D12::Present(d3d);
		D3D12::MoveToNextFrame(d3d);
		D3D12::Reset_CommandList(d3d);
	}

	void Cleanup() 
	{
		D3D12::WaitForGPU(d3d);
		CloseHandle(d3d.fenceEvent);

		DXR::Destroy(mainDXR);
		D3DResources::Destroy(bearResources);
		D3DResources::Destroy(crystalResources);
		D3DShaders::Destroy(shaderCompiler);
		D3D12::Destroy(d3d);

		DestroyWindow(window);
	}
	
private:
	HWND window;
	// Model model;
	// Material material;

	DXRGlobal mainDXR = {};
	DXRGlobal auxDXR = {};
	D3D12Global d3d = {};
	D3D12Resources bearResources = {};
	D3D12Resources crystalResources = {};
	D3D12ShaderCompilerInfo shaderCompiler;
	D3D12Resources auxResources = {};
};

static std::wstring GetLatestWinPixGpuCapturerPath()
{
	LPWSTR programFilesPath = nullptr;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

	std::wstring pixSearchPath = programFilesPath + std::wstring(L"\\Microsoft PIX\\*");

	WIN32_FIND_DATA findData;
	bool foundPixInstallation = false;
	wchar_t newestVersionFound[MAX_PATH];

	HANDLE hFind = FindFirstFile(pixSearchPath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
				(findData.cFileName[0] != '.'))
			{
				if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
				{
					foundPixInstallation = true;
					StringCchCopy(newestVersionFound, _countof(newestVersionFound), findData.cFileName);
				}
			}
		} while (FindNextFile(hFind, &findData) != 0);
	}

	FindClose(hFind);

	if (!foundPixInstallation)
	{
		// TODO: Error, no PIX installation found
	}

	wchar_t output[MAX_PATH];
	StringCchCopy(output, pixSearchPath.length(), pixSearchPath.data());
	StringCchCat(output, MAX_PATH, &newestVersionFound[0]);
	StringCchCat(output, MAX_PATH, L"\\WinPixGpuCapturer.dll");

	return &output[0];
}


/**
 * Program entry point.
 */
int WINAPI wmain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) 
{
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
	}
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HRESULT hr = EXIT_SUCCESS;
	{
		MSG msg = { 0 };

		// Get the application configuration
		ConfigInfo config;
		hr = Utils::ParseCommandLine(lpCmdLine, config);
		if (hr != EXIT_SUCCESS) return hr;

		// Initialize
		DXRApplication app;
		app.Init(config);

		// Main loop
		while (WM_QUIT != msg.message) 
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			app.Update();
			app.Render();
		}

		app.Cleanup();
	}

#if defined _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif

	return hr;
}