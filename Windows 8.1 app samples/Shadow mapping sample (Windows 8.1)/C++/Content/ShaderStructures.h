#pragma once

namespace ShadowMapSample
{
    // Constant buffer used to send view and projection matrices and camera (eye)
    // position to the vertex shader.
    struct ViewProjectionConstantBuffer
    {
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
        DirectX::XMFLOAT4   pos;
    };

    // Constant buffer used to send model matrix to the vertex shader.
    struct ModelConstantBuffer
    {
        DirectX::XMFLOAT4X4 model;
    };

    // Used to send per-vertex data to the vertex shader.
    struct VertexPositionTexNormColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 tex;
        DirectX::XMFLOAT3 norm;
        DirectX::XMFLOAT3 color;
    };
}