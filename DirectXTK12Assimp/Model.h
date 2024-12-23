#pragma once

#include "pch.h"
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DirectXMath.h>
namespace education {
	class Model
	{
	public:

		~Model() {};
		Model(DX::DeviceResources* deviceresources, const char* path);
		bool LoadModel(const char* path);
		std::vector<DirectX::VertexPositionNormalColorTexture> GenerateVertices();
		HRESULT CreateBuffer(DX::DeviceResources* deviceResources);
		HRESULT CreateShaders(const DX::DeviceResources* deviceResources);
		void CreateBuffers(const DX::DeviceResources* deviceResources);
		HRESULT craetepipelineState(const DX::DeviceResources* deviceResources);
		void Draw(const DX::DeviceResources* DR);


		const aiScene* m_scene;
		Assimp::Importer m_importer;

		//�o�b�t�@
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_layout;
		std::vector<DirectX::VertexPositionNormalColorTexture> vertices;
		std::vector<unsigned short> indices;

		DirectX::XMMATRIX modelmat;
		//�V�F�[�_�[�̍쐬
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;//�V�K�ǉ�
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;//�V�K�ǉ�
		
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;//�V�K�ǉ�
	};

}

// Z�o�b�t�@��L����
// Z�o�b�t�@�ւ̏������݂�L����
// �o�b�N�J�����O�𖳌���