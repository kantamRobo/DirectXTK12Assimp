#include "pch.h"




#define NOMINMAX
#include <BufferHelpers.h>
#include <iostream>
#include <cassert>
#include <d3dcompiler.h>
#include <algorithm>
#include <functional>
#include "Model.h"
#include <ResourceUploadBatch.h>
#pragma comment(lib, "d3dcompiler.lib")


//TODO:コンストラクタでモデルロード・バッファ作成・シェーダー作成を行う



education::Model::Model(DX::DeviceResources* deviceresources, const char* path)
{
    if (!LoadModel(path))
    {
        std::abort();
    }

    /*
    追加分
    */
    if (FAILED(CreateBuffer(deviceresources)))
    {
        std::abort();
    }
    if (FAILED(CreateShaders(deviceresources)))
    {
        std::abort();
    }
    if (FAILED(craetepipelineState(deviceresources)))
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
    // スケール値を設定
    float scaleFactor = 10.0f;


    for (int i = 0; i < vertices.size(); i++)
    {
        //乗算　vertices[i].position = vertices[i].position,100.0f;
    }

    // 頂点数が0でないことを確認
    assert(vertices.size() > 0 && "頂点の生成に失敗");

    // インデックス数が3の倍数であり、かつインデックスが頂点数を超えないことを確認
    assert(indices.size() % 3 == 0 && "インデックス数が3の倍数でない");
    assert(indices.size() <= vertices.size() * 3 && "インデックスが頂点数を超えています");

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

        // インデックスの設定
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

HRESULT education::Model::CreateBuffer(DX::DeviceResources* deviceResources)
{
	DirectX::ResourceUploadBatch resourceUpload(deviceResources->GetD3DDevice());

    resourceUpload.Begin();
    // 頂点バッファの作成
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
    // インデックスバッファの作成
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
    return S_OK;
}
HRESULT education::Model::CreateShaders(const DX::DeviceResources* deviceResources)
{
    //パイプラインステートの作成
    auto device = deviceResources->GetD3DDevice();

 

    // 頂点シェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
    auto hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

   
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }


    // ピクセルシェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }
    // 入力レイアウトの作成
    layout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

   
    if (FAILED(hr))
    {
        return hr;
    }




    return hr;
}


HRESULT education::Model::craetepipelineState(const DX::DeviceResources* deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

    // 注意
    rasterizerDesc.FrontCounterClockwise = true;
  
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.AntialiasedLineEnable = false;

    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { m_layout.data(), static_cast<UINT>(m_layout.size() )};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));


    if (FAILED(hr))
    {
        return hr;
    }

    return hr;
}





void education::Model::Draw(const DX::DeviceResources* DR) {
    if (vertices.empty() || indices.empty()) {
        return;
    }
    auto device = DR->GetD3DDevice();
    UINT size = sizeof(DirectX::VertexPositionNormalColorTexture);
    auto offset = 0u;
    auto context = DR->GetD3DDeviceContext();
    context->IASetInputLayout(m_modelInputLayout.Get());
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &size, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
}


