#pragma once

#include <d3d11.h>
#include <wincodec.h>
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace TextureLoader
{
    // WIC를 이용한 텍스처 생성 함수
    inline ID3D11ShaderResourceView* CreateTextureFromFile(ID3D11Device* device, const wchar_t* filename)
    {
        if (!device || !filename) return nullptr;

        IWICImagingFactory* pFactory = nullptr;
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pFactory)
        );
        if (FAILED(hr)) return nullptr;

        // 이미지 디코더 생성
        IWICBitmapDecoder* pDecoder = nullptr;
        hr = pFactory->CreateDecoderFromFilename(
            filename,
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            &pDecoder
        );
        if (FAILED(hr))
        {
            pFactory->Release();
            return nullptr;
        }

        // 첫 번째 프레임 추출
        IWICBitmapFrameDecode* pFrame = nullptr;
        hr = pDecoder->GetFrame(0, &pFrame);
        if (FAILED(hr))
        {
            pDecoder->Release();
            pFactory->Release();
            return nullptr;
        }

        // 32비트 RGBA 포맷 변환기 생성
        IWICFormatConverter* pConverter = nullptr;
        hr = pFactory->CreateFormatConverter(&pConverter);
        if (FAILED(hr))
        {
            pFrame->Release();
            pDecoder->Release();
            pFactory->Release();
            return nullptr;
        }

        hr = pConverter->Initialize(
            pFrame,
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0f,
            WICBitmapPaletteTypeCustom
        );
        if (FAILED(hr))
        {
            pConverter->Release();
            pFrame->Release();
            pDecoder->Release();
            pFactory->Release();
            return nullptr;
        }

        UINT width = 0, height = 0;
        pConverter->GetSize(&width, &height);

        // 픽셀 바이트 데이터 복사
        UINT rowPitch = width * 4;
        UINT imageSize = rowPitch * height;
        std::vector<BYTE> pixelData(imageSize);
        pConverter->CopyPixels(nullptr, rowPitch, imageSize, pixelData.data());

        // COM 인터페이스 해제
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();

        //  2D 텍스처 생성
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA subData = {};
        subData.pSysMem = pixelData.data();
        subData.SysMemPitch = rowPitch;

        ID3D11Texture2D* texture = nullptr;
        hr = device->CreateTexture2D(&desc, &subData, &texture);
        if (FAILED(hr) || !texture) return nullptr;

        // 셰이더 리소스 뷰 생성
        ID3D11ShaderResourceView* srv = nullptr;
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(texture, &srvDesc, &srv);
        texture->Release();

        return srv;
    }

    inline ID3D11ShaderResourceView* CreateTextureFromFile(ID3D11Device* device, const std::string& filename)
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, nullptr, 0);
        if (len <= 0) return nullptr;

        std::wstring wFilename(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, &wFilename[0], len);

        return CreateTextureFromFile(device, wFilename.c_str());
    }

    // 텍스처 샘플러 상태 생성 함수
    inline ID3D11SamplerState* CreateSamplerState(ID3D11Device* device)
    {
        if (!device) return nullptr;

        D3D11_SAMPLER_DESC sampDesc = {};
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        ID3D11SamplerState* sampler = nullptr;
        device->CreateSamplerState(&sampDesc, &sampler);
        return sampler;
    }
}
