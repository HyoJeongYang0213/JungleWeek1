#pragma once

#include <d3d11.h>
#include "../Scene/SceneManager.h"
#include "../ImGui/imgui.h"

class UIManager
{
public:
    static UIManager& Get()
    {
        static UIManager Instance;
        return Instance;
    }

    void Init(ID3D11Device* InDevice, SceneManager* InSceneManager);
    void Shutdown();

    void Render(int InCurrentSceneIndex);

    bool IsOptionsOpen() const { return mBShowOptions; }
    void SetOptionsOpen(bool InShow) { mBShowOptions = InShow; }
    bool ShouldExit() const { return mBIsExit; }

private:
    UIManager() = default;
    ~UIManager() = default;
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    void DrawBoldText(const char* Text, float PosX, float PosY,
        ImVec4 TextCol, ImVec4 ShadowCol, float Thick = 1.2f);
    void DrawBoldTextScreen(const char* Text, ImVec2 ScreenPos,
        ImU32 TextCol, ImU32 ShadowCol, float Thick = 1.2f);

    bool ImageTextButton(const char* StrId, const char* Text, ID3D11ShaderResourceView* TextureSRV, const ImVec2& Size);
    void RenderTitleUI();
    void RenderSettingsUI(int InCurrentSceneIndex);
    void RenderEndingUI();
    void RenderScore();
private:
    SceneManager* mSceneManager = nullptr;
    ID3D11ShaderResourceView* mBtnFrameSRV = nullptr;
    ID3D11ShaderResourceView* mSettingsBtnSRV = nullptr;
    ID3D11ShaderResourceView* mScoreBannerSRV = nullptr;

    bool mBShowOptions = false;
    bool mBIsExit = false;
};
