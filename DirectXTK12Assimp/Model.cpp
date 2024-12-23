#include "pch.h"




#define NOMINMAX
#include <iostream>
#include <cassert>
#include <d3dcompiler.h>
#include <algorithm>
#include <functional>
#include <ResourceUploadBatch.h>
#pragma comment(lib, "d3dcompiler.lib")


//TODO:�R���X�g���N�^�Ń��f�����[�h�E�o�b�t�@�쐬�E�V�F�[�_�[�쐬���s��



education::Model::Model(DX::DeviceResources* deviceresources, const char* path,int height,int width)
{
    if (!LoadModel(path))
    {
        std::abort();
    }

    /*
    �ǉ���
    */
    if (FAILED(CreateBuffer(deviceresources,width,height)))
    {
        std::abort();
    }
    if (FAILED((deviceresources)))
    {
        std::abort();
    }
	m_pipelineState = CreateGraphicsPipelineState(deviceresources->GetD3DDevice(), m_rootSignature, L"VertexShader.hlsl", L"PixelShader.hlsl");
	if (m_pipelineState == nullptr)
	{
		std::abort();
	}
}

bool education::Model::LoadModel(const char* path)
{


    m_scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals);
    if (!m_scene || !m_scene->mRootNode)
    {
        auto error = m_importer.GetErrorString();

        return false;
    }
    vertices = GenerateVertices();
    // �X�P�[���l��ݒ�
    float scaleFactor = 10.0f;


    for (int i = 0; i < vertices.size(); i++)
    {
        //��Z�@vertices[i].position = vertices[i].position,100.0f;
    }

    // ���_����0�łȂ����Ƃ��m�F
    assert(vertices.size() > 0 && "���_�̐����Ɏ��s");

    // �C���f�b�N�X����3�̔{���ł���A���C���f�b�N�X�����_���𒴂��Ȃ����Ƃ��m�F
    assert(indices.size() % 3 == 0 && "�C���f�b�N�X����3�̔{���łȂ�");
    assert(indices.size() <= vertices.size() * 3 && "�C���f�b�N�X�����_���𒴂��Ă��܂�");

    return true;
}

std::vector<DirectX::VertexPositionNormalColorTexture> education::Model::GenerateVertices()
{
    assert(m_scene);



    std::vector< DirectX::VertexPositionNormalColorTexture> outvertices;
    outvertices.clear();

    for (unsigned int i = 0; i < m_scene->mNumMeshes; i++)
    {
        aiMesh* mesh = m_scene->mMeshes[i];



        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            DirectX::VertexPositionNormalColorTexture vertex = {};
            aiVector3D pos = mesh->mVertices[j];


            vertex.position = { pos.x , pos.y , pos.z };
            vertex.normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

            if (mesh->mTextureCoords[0])
            {
                vertex.textureCoordinate.x = mesh->mTextureCoords[0][j].x;
                vertex.textureCoordinate.y = mesh->mTextureCoords[0][j].y;
            }
            else
            {
                vertex.textureCoordinate.x = 0.0f;
                vertex.textureCoordinate.y = 0.0f;
            }

            outvertices.push_back(vertex);
        }

        // �C���f�b�N�X�̐ݒ�
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }
    }



    return outvertices;
}

HRESULT education::Model::CreateBuffer(DX::DeviceResources* deviceResources,int height,int width)
{
	DirectX::ResourceUploadBatch resourceUpload(deviceResources->GetD3DDevice());

    resourceUpload.Begin();
    // ���_�o�b�t�@�̍쐬
    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(
            deviceResources->GetD3DDevice(),
			resourceUpload,
            vertices.data(),
            static_cast<int>(vertices.size()),
            sizeof(DirectX::VertexPositionNormalColorTexture),
            D3D12_RESOURCE_STATE_COMMON,
            m_vertexBuffer.GetAddressOf()
        )
    );
    // �C���f�b�N�X�o�b�t�@�̍쐬
    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(
            deviceResources->GetD3DDevice(),
            resourceUpload,
            indices.data(),
            static_cast<int>(indices.size()),
            sizeof(unsigned short),
            D3D12_RESOURCE_STATE_COMMON,
            m_indexBuffer.GetAddressOf()
        )
    );

	
    //(DirectXTK12Assimp�Œǉ�)
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(DirectX::VertexPositionNormalColorTexture);
	m_vertexBufferView.SizeInBytes = sizeof(DirectX::VertexPositionNormalColorTexture) * vertices.size();

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_indexBufferView.SizeInBytes = sizeof(unsigned short) * indices.size();

    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);
    DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, up);

    float    fov = DirectX::XMConvertToRadians(45.0f);
    float    aspect = height / width;
    float    nearZ = 0.1f;
    float    farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    SceneCB cb;
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));

  
    //�萔�o�b�t�@�̍쐬(DIrectXTK12Assimp�Œǉ�)
    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(
            deviceResources->GetD3DDevice(),
            resourceUpload,
            &cb,
            sizeof(cb),
            sizeof(unsigned short),
            D3D12_RESOURCE_STATE_COMMON,
			m_ConstantBuffer.GetAddressOf()
        )
    );

    //�萔�o�b�t�@���}�b�v(DIrectXTK12Assimp�Œǉ�)
	CD3DX12_RANGE readRange(0, 0);
	void* m_pCbvDataBegin;
	DX::ThrowIfFailed(
		m_ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin))
	);
	//�萔�o�b�t�@�Ƀf�[�^���R�s�[
	memcpy(m_pCbvDataBegin, &cb, sizeof(cb));
	//�萔�o�b�t�@���A���}�b�v
	m_ConstantBuffer->Unmap(0, nullptr);
	// ���\�[�X�̃A�b�v���[�h���I��
	auto uploadResourcesFinished = resourceUpload.End(deviceResources->GetCommandQueue());
    return S_OK;
}


using Microsoft::WRL::ComPtr;
//(DIrectXTK12Assimp�Œǉ�)
// �O���t�B�b�N�p�C�v���C���X�e�[�g���쐬����֐�
ComPtr<ID3D12PipelineState> education::Model::CreateGraphicsPipelineState(
    ComPtr<ID3D12Device> device,
    ComPtr<ID3D12RootSignature> rootSignature,
    const std::wstring& vertexShaderPath,
    const std::wstring& pixelShaderPath)
{
    // �V�F�[�_�[���R���p�C��
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(
        vertexShaderPath.c_str(),
        nullptr,
        nullptr,
        "main", // �G���g���[�|�C���g
        "vs_5_0", // �V�F�[�_�[���f��
        0,
        0,
        &vertexShader,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile vertex shader");
    }

    hr = D3DCompileFromFile(
        pixelShaderPath.c_str(),
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &pixelShader,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile pixel shader");
    }

    // ���̓��C�A�E�g���`
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // ���X�^���C�U�[�X�e�[�g
    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // �u�����h�X�e�[�g
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // �[�x/�X�e���V���X�e�[�g
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    // �O���t�B�b�N�p�C�v���C���X�e�[�g�̐ݒ�
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    // �p�C�v���C���X�e�[�g�I�u�W�F�N�g���쐬
    ComPtr<ID3D12PipelineState> pipelineState;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create pipeline state");
    }

    return pipelineState;
}
//(DIrectXTK12Assimp�Œǉ�)
void education::Model::CreateDescriptors(DX::DeviceResources* DR)
{

	m_resourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(
		DR->GetD3DDevice(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		Descriptors::Count
	);
}





//(DIrectXTK12Assimp�Œǉ�)
void education::Model::Draw(const DX::DeviceResources* DR) {


    DirectX::ResourceUploadBatch resourceUpload(DR->GetD3DDevice());
    
    resourceUpload.Begin();
    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("Vertices or indices buffer is empty.\n");
        return;
    }

    auto commandList = DR->GetCommandList();
	auto renderTarget = DR->GetRenderTarget();
    if (!commandList) {
        OutputDebugStringA("Command list is null.\n");
        return;
    }

    // ���̓A�Z���u���[�ݒ�
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   

    // ���[�g�V�O�l�`���ݒ�
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    
    commandList->SetGraphicsRootDescriptorTable(0, m_resourceDescriptors->GetGpuHandle(Count));

    // �p�C�v���C���X�e�[�g�ݒ�
    commandList->SetPipelineState(m_pipelineState.Get());

    // �`��R�[��
    commandList->DrawIndexedInstanced(
        static_cast<UINT>(indices.size()), // �C���f�b�N�X��
        1,                                 // �C���X�^���X��
        0,                                 // �J�n�C���f�b�N�X
        0,                                 // ���_�I�t�Z�b�g
        0                                  // �C���X�^���X�I�t�Z�b�g
    );
    auto uploadResourcesFinished = resourceUpload.End(
        DR->GetCommandQueue());

    uploadResourcesFinished.wait();
}
