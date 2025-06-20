#include "pch.h"




#define NOMINMAX
#include <iostream>
#include <cassert>
#include <d3dcompiler.h>
#include <algorithm>
#include <functional>
#include <ResourceUploadBatch.h>
#pragma comment(lib, "d3dcompiler.lib")


//TODO:コンストラクタでモデルロード・バッファ作成・シェーダー作成を行う



education::Model::Model(DirectX::GraphicsMemory* graphicsmemory,DX::DeviceResources* deviceresources, const char* path,int height,int width)
{
    if (!LoadModel(path))
    {
        std::abort();
    }

    /*
    追加分
    */
    if (FAILED(CreateBuffer(graphicsmemory,deviceresources,width,height)))
    {
        std::abort();
    }
    CreateDescriptors(deviceresources);
    
	// ルートシグネチャの作成
    
	m_pipelineState = CreateGraphicsPipelineState(deviceresources,  L"VertexShader.hlsl", L"PixelShader.hlsl");
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

HRESULT education::Model::CreateBuffer(DirectX::GraphicsMemory* graphicsmemory,DX::DeviceResources* deviceResources,int height,int width)
{
	DirectX::ResourceUploadBatch resourceUpload(deviceResources->GetD3DDevice());

    resourceUpload.Begin();
    // 頂点バッファの作成
    /*
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
    */

    /*
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
    */
    auto vertbytesize = sizeof(DirectX::VertexPositionNormalColorTexture) * vertices.size();
    auto indexbytesize = sizeof(unsigned short) * indices.size();
    m_vertexBuffer = graphicsmemory->Allocate(vertbytesize);
    m_indexBuffer = graphicsmemory->Allocate(indexbytesize);
    memcpy(m_vertexBuffer.Memory(), vertices.data(), vertbytesize);
    memcpy(m_indexBuffer.Memory(), indices.data(), indexbytesize);

    //(DirectXTK12Assimpで追加)
	m_vertexBufferView.BufferLocation = m_vertexBuffer.GpuAddress();
	m_vertexBufferView.StrideInBytes =    sizeof(DirectX::VertexPositionNormalColorTexture);
    m_vertexBufferView.SizeInBytes = vertbytesize;

    m_indexBufferView.BufferLocation = m_indexBuffer.GpuAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = indexbytesize;
    
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);
    DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, up);

    constexpr float fov = DirectX::XMConvertToRadians(45.0f);
    float    aspect = (height / width);
    float    nearZ = 0.1f;
    float    farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    SceneCB cb;
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));

  
    //定数バッファの作成(DIrectXTK12Assimpで追加)
  
    //https://github.com/microsoft/DirectXTK12/wiki/GraphicsMemory
  

SceneCBResource = graphicsmemory->AllocateConstant(cb);
   
	// リソースのアップロードを終了
	auto uploadResourcesFinished = resourceUpload.End(deviceResources->GetCommandQueue());
    return S_OK;
}


using Microsoft::WRL::ComPtr;
//(DIrectXTK12Assimpで追加)
// グラフィックパイプラインステートを作成する関数
Microsoft::WRL::ComPtr<ID3D12PipelineState> education::Model::CreateGraphicsPipelineState(
   DX::DeviceResources* deviceresources,
    
    const std::wstring& vertexShaderPath,
    const std::wstring& pixelShaderPath)
{
    // シェーダーをコンパイル
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;
    DirectX::RenderTargetState rtState(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
    HRESULT hr = D3DCompileFromFile(
        vertexShaderPath.c_str(),
        nullptr,
        nullptr,
        "main", // エントリーポイント
        "vs_5_0", // シェーダーモデル
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

    // 入力レイアウトを定義
    m_layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    // Create root parameters and initialize first (constants)
    CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[RootParameterIndex::ConstantBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    // Root parameter descriptor
    CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};

    // use all parameters
    rsigDesc.Init(static_cast<UINT>(std::size(rootParameters)), rootParameters, 0, nullptr, rootSignatureFlags);

	DX::ThrowIfFailed(DirectX::CreateRootSignature(deviceresources->GetD3DDevice(), &rsigDesc, m_rootSignature.ReleaseAndGetAddressOf()));
    
    //https://github.com/microsoft/DirectXTK12/wiki/PSOs,-Shaders,-and-Signatures
    // 
    // 
    D3D12_INPUT_LAYOUT_DESC inputlayaout = { m_layout.data(), m_layout.size()};
    DirectX::EffectPipelineStateDescription pd(
        &inputlayaout,
        DirectX::CommonStates::Opaque,
        DirectX::CommonStates::DepthDefault,
        DirectX::CommonStates::CullCounterClockwise,
        rtState);
    D3D12_SHADER_BYTECODE vertexshaderBCode = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()};

    
    D3D12_SHADER_BYTECODE pixelShaderBCode = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()};
    // パイプラインステートオブジェクトを作成
    ComPtr<ID3D12PipelineState> pipelineState;
	pd.CreatePipelineState(
		deviceresources->GetD3DDevice(),
        m_rootSignature.Get(),
        vertexshaderBCode,
		
        pixelShaderBCode,
	
		pipelineState.GetAddressOf()
	);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create pipeline state");
    }

    return pipelineState;
}
//(DIrectXTK12Assimpで追加)
void education::Model::CreateDescriptors(DX::DeviceResources* DR)
{

	m_resourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(
		DR->GetD3DDevice(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		Descriptors::Count
	);
}





//(DIrectXTK12Assimpで追加)
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

    // 入力アセンブラー設定
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   

    // ルートシグネチャ設定
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    //2024/12/30/9:42
	commandList->SetGraphicsRootConstantBufferView(0, SceneCBResource.GpuAddress());

    // パイプラインステート設定
    commandList->SetPipelineState(m_pipelineState.Get());

    // 描画コール
    commandList->DrawIndexedInstanced(
        static_cast<UINT>(indices.size()), // インデックス数
        1,                                 // インスタンス数
        0,                                 // 開始インデックス
        0,                                 // 頂点オフセット
        0                                  // インスタンスオフセット
    );
    auto uploadResourcesFinished = resourceUpload.End(
        DR->GetCommandQueue());

    uploadResourcesFinished.wait();
}
