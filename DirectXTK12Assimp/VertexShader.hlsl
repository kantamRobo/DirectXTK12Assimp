struct VS_INPUT
{
    float4 pos : POSITION;   // 頂点位置
    float3 normal : NORMAL;  // 法線
    float2 tex : TEXCOORD0;  // テクスチャ座標
};
 
cbuffer ConstantBuffer
{
    float4x4 World;         // ワールド変換行列
    float4x4 View;          // ビュー変換行列
    float4x4 Projection;    // 透視射影変換行列
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION; // 出力のスクリーン座標
    float3 normal : NORMAL;   // 出力の法線
    float2 tex : TEXCOORD0;   // 出力のテクスチャ座標
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // ワールド変換、ビュー変換、プロジェクション変換を適用
    float4 worldPosition = mul(input.pos, World);
    float4 viewPosition = mul(worldPosition, View);
    output.pos = mul(viewPosition, Projection);

    // 法線とテクスチャ座標はそのまま出力
    output.normal = mul(float4(input.normal, 0.0f), World).xyz; // 法線もワールド座標系に変換
    output.tex = input.tex;

    return output;
}
