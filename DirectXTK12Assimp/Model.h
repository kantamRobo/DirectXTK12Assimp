#pragma once

#include "pch.h"
#include <vector>

#include <DeviceResources.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DirectXMath.h>
#include <DescriptorHeap.h>
enum Descriptors
{
	WindowsLogo,
	CourierFont,
	ControllerFont,
	GamerPic,
	Count
};

// Create root signature.
enum RootParameterIndex
{
	ConstantBuffer,
	TextureSRV,
	TextureSampler,
	RootParameterCount
};
struct SceneCB {
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};
namespace education {
	class Model
	{
	public:

		~Model() {};
		
		Model(DirectX::GraphicsMemory* graphicsmemory,DX::DeviceResources* deviceresources, const char* path, int height, int width);
		bool LoadModel(const char* path);
		std::vector<DirectX::VertexPositionNormalColorTexture> GenerateVertices();
		HRESULT CreateBuffer(DirectX::GraphicsMemory* graphicsmemory,DX::DeviceResources* deviceResources, int height, int width);
		
		
		std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
		
		Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(DX::DeviceResources* deviceresources, const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath);
		void CreateDescriptors(DX::DeviceResources* DR);
		void Draw(const DX::DeviceResources* DR);
		
		
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	
		const aiScene* m_scene;
		Assimp::Importer m_importer;

		DirectX::GraphicsResource m_vertexBuffer;//新規追加
		DirectX::GraphicsResource m_indexBuffer;//新規追加
		//DirectX::GraphicsResource m_ConstantBuffer;//新規追加
		//バッファ
		//Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		//Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_layout;
		std::vector<DirectX::VertexPositionNormalColorTexture> vertices;
		std::vector<unsigned short> indices;

		DirectX::XMMATRIX modelmat;
		//シェーダーの作成
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;//新規追加
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;//新規追加
		DirectX::GraphicsResource SceneCBResource;//新規追加
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;//新規追加
	};

}

// Zバッファを有効化
// Zバッファへの書き込みを有効化
// バックカリングを無効化