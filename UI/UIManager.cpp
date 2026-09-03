#include "UIManager.h"
#include "../Map/TextureLoader.hpp"
#include "../Player/PlayerGlobals.hpp"

void UIManager::Init(ID3D11Device* InDevice, SceneManager* InSceneManager)
{
    mSceneManager = InSceneManager;
    mBtnFrameSRV = TextureLoader::CreateTextureFromFile(InDevice, L"Asset/UI/Button_Blank.png");
    mSettingsBtnSRV = TextureLoader::CreateTextureFromFile(InDevice, L"Asset/UI/Button_Settings.png");
    mScoreBannerSRV = TextureLoader::CreateTextureFromFile(InDevice, L"Asset/UI/ScoreFrame.png");
    mBShowOptions = false;
    mBIsExit = false;
}

void UIManager::Shutdown()
{
    if (mBtnFrameSRV)
    {
        mBtnFrameSRV->Release();
        mBtnFrameSRV = nullptr;
    }

    if (mSettingsBtnSRV)
    {
        mSettingsBtnSRV->Release();
        mSettingsBtnSRV = nullptr;
    }

	if (mScoreBannerSRV)
	{
		mScoreBannerSRV->Release();
		mScoreBannerSRV = nullptr;
	}

    mSceneManager = nullptr;
}

void UIManager::DrawBoldText(const char* Text, float PosX, float PosY,
    ImVec4 TextCol, ImVec4 ShadowCol, float Thick)
{
    ImGui::SetCursorPos(ImVec2(PosX - Thick, PosY));
    ImGui::TextColored(ShadowCol, "%s", Text);

    ImGui::SetCursorPos(ImVec2(PosX + Thick, PosY));
    ImGui::TextColored(ShadowCol, "%s", Text);

    ImGui::SetCursorPos(ImVec2(PosX, PosY - Thick));
    ImGui::TextColored(ShadowCol, "%s", Text);

    ImGui::SetCursorPos(ImVec2(PosX, PosY + Thick));
    ImGui::TextColored(ShadowCol, "%s", Text);

    ImGui::SetCursorPos(ImVec2(PosX, PosY));
    ImGui::TextColored(TextCol, "%s", Text);
}

void UIManager::DrawBoldTextScreen(const char* Text, ImVec2 ScreenPos,
    ImU32 TextCol, ImU32 ShadowCol, float Thick)
{
    ImDrawList* DrawList = ImGui::GetWindowDrawList();

    DrawList->AddText(ImVec2(ScreenPos.x - Thick, ScreenPos.y), ShadowCol, Text);
    DrawList->AddText(ImVec2(ScreenPos.x + Thick, ScreenPos.y), ShadowCol, Text);
    DrawList->AddText(ImVec2(ScreenPos.x, ScreenPos.y - Thick), ShadowCol, Text);
    DrawList->AddText(ImVec2(ScreenPos.x, ScreenPos.y + Thick), ShadowCol, Text);
    DrawList->AddText(ImVec2(ScreenPos.x + Thick, ScreenPos.y + Thick), ShadowCol, Text);

    DrawList->AddText(ScreenPos, TextCol, Text);
}

bool UIManager::ImageTextButton(const char* StrId, const char* Text, ID3D11ShaderResourceView* TextureSRV, const ImVec2& Size)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));

    ImVec2 StartPos = ImGui::GetCursorScreenPos();

    bool BClicked = ImGui::ImageButton(
        StrId,
        (ImTextureID)TextureSRV,
        Size,
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 1.0f)
    );

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImVec2 TextSize = ImGui::CalcTextSize(Text);
    ImVec2 TextPos = ImVec2(
        StartPos.x + (Size.x - TextSize.x) * 0.5f,
        StartPos.y + (Size.y - TextSize.y) * 0.5f
    );

    ImU32 DarkBrownColor = IM_COL32(74, 35, 6, 255);

    DrawBoldTextScreen(Text, TextPos, DarkBrownColor, DarkBrownColor, 0.5f);

    return BClicked;
}

void UIManager::Render(int InCurrentSceneIndex)
{
    switch (InCurrentSceneIndex)
    {
    case 0:
        RenderTitleUI();
        RenderSettingsUI(InCurrentSceneIndex);
        break;

    case 1:
        RenderScore();
        RenderSettingsUI(InCurrentSceneIndex);
        break;

    case 2:
        RenderEndingUI();
        break;
    }
}

void UIManager::RenderTitleUI()
{
    if (mBShowOptions)
    {
        return;
    }

    ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
    float ButtonW = 240.0f;
    float ButtonH = 80.0f;

    float PosX = (DisplaySize.x - ButtonW) * 0.5f;
    float PosY = DisplaySize.y * 0.70f;

    ImGui::SetNextWindowPos(ImVec2(PosX, PosY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ButtonW + 20.0f, ButtonH + 20.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("TitleUI", nullptr, Flags);
    ImGui::SetWindowFontScale(1.3f);

    if (ImageTextButton("##PlayButton", u8"게임 시작", mBtnFrameSRV, ImVec2(ButtonW, ButtonH)))
    {
        if (mSceneManager)
        {
            mSceneManager->NextScene();
        }
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
    ImGui::PopStyleVar();
}

void UIManager::RenderSettingsUI(int InCurrentSceneIndex)
{
    ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
    float IconSize = 46.0f;

    ImGui::SetNextWindowPos(ImVec2(DisplaySize.x - IconSize - 20.0f, 20.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(IconSize + 10.0f, IconSize + 10.0f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags IconFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("SettingsIconButton", nullptr, IconFlags);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.4f));

    if (ImGui::ImageButton("##SettingsBtn", (ImTextureID)mSettingsBtnSRV, ImVec2(IconSize, IconSize)))
    {
        mBShowOptions = !mBShowOptions;
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    ImGui::End();

    if (mBShowOptions)
    {
        float ModalW = 420.0f;
        float ModalH = 320.0f;

        ImGui::SetNextWindowPos(ImVec2((DisplaySize.x - ModalW) * 0.5f, (DisplaySize.y - ModalH) * 0.5f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ModalW, ModalH), ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.13f, 0.22f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.10f, 0.20f, 0.35f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.14f, 0.30f, 0.52f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.65f, 0.95f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.20f, 0.45f, 0.70f, 0.60f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.96f, 1.00f, 1.00f));

        bool BIsOpen = true;
        ImGuiWindowFlags ModalFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
        if (ImGui::Begin(u8"게임 설정 & 조작법", &BIsOpen, ModalFlags))
        {
            if (!BIsOpen)
            {
                mBShowOptions = false;
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.38f, 0.82f, 1.00f, 1.00f), u8"[ 조작 방법 ]");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BulletText(u8"마우스 좌클릭 드래그: 발사 방향/파워 조절");
            ImGui::Spacing();
            ImGui::BulletText(u8"마우스 클릭 해제: 튕겨 도약");
            ImGui::Spacing();
            ImGui::BulletText(u8"벽과 발판을 딛고 최상층을 향해 올라가세요!");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            if (InCurrentSceneIndex == 1)
            {
                if (ImageTextButton("##BtnResume", u8"계속하기", mBtnFrameSRV, ImVec2(185.0f, 48.0f)))
                {
                    mBShowOptions = false;
                }

                ImGui::SameLine();

                if (ImageTextButton("##BtnRestart", u8"타이틀로", mBtnFrameSRV, ImVec2(185.0f, 48.0f)))
                {
                    mBShowOptions = false;
                    if (mSceneManager)
                    {
                        mSceneManager->NextScene();
                        mSceneManager->NextScene();
                    }
                }
            }
            else
            {
                if (ImageTextButton("##BtnClose", u8"닫기", mBtnFrameSRV, ImVec2(380.0f, 48.0f)))
                {
                    mBShowOptions = false;
                }
            }
        }
        ImGui::End();

        ImGui::PopStyleColor(6);
        ImGui::PopStyleVar(3);
    }
}

void UIManager::RenderEndingUI()
{
    ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;

    {
        float ScoreBoxW = 450.0f;
        float ScoreBoxH = 150.0f;

        ImGui::SetNextWindowPos(ImVec2((DisplaySize.x - ScoreBoxW) * 0.5f, DisplaySize.y * 0.40f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ScoreBoxW, ScoreBoxH), ImGuiCond_Always);

        ImGuiWindowFlags ScoreFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar;

        ImGui::Begin("EndingScoreDisplay", nullptr, ScoreFlags);

        ImGui::SetWindowFontScale(6.0f);

        char ScoreBuf[64];
        sprintf_s(ScoreBuf, "%.1f m", PlayerGlobals::HIGH_SCORE);

        float TextWidth = ImGui::CalcTextSize(ScoreBuf).x;
        float BasePosX = (ScoreBoxW - TextWidth) * 0.5f;
        float BasePosY = 10.0f;

        ImVec4 TextCol = ImVec4(74.0f / 255.0f, 35.0f / 255.0f, 6.0f / 255.0f, 1.0f);

        DrawBoldText(ScoreBuf, BasePosX, 0.0f, TextCol, TextCol, 1.5f);

        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }

    {
        float ModalW = 340.0f;
        float ModalH = 160.0f;
        ImGui::SetNextWindowPos(ImVec2((DisplaySize.x - ModalW) * 0.5f, DisplaySize.y * 0.65f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ModalW, ModalH), ImGuiCond_Always);

        ImGuiWindowFlags Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

        ImGui::Begin("EndingUI", nullptr, Flags);

        if (ImageTextButton("##BtnEndingRestart", u8"게임 재시작", mBtnFrameSRV, ImVec2(320.0f, 45.0f)))
        {
            if (mSceneManager)
            {
                mSceneManager->NextScene();
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if(ImageTextButton("##BtnEndingExit", u8"게임 종료", mBtnFrameSRV, ImVec2(320.0f, 45.0f)))
        {
            mBIsExit = true;
        }

        ImGui::End();
    }
}

void UIManager::RenderScore()
{
    ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;

    float BannerW = 160.0f;
    float BannerH = 90.0f;

    float PosX = (DisplaySize.x - BannerW) * 0.5f;
    float PosY = 15.0f;

    ImGui::SetNextWindowPos(ImVec2(PosX, PosY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(BannerW, BannerH), ImGuiCond_Always);

    ImGuiWindowFlags Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("InGameHeightHUD", nullptr, Flags);

    if (mScoreBannerSRV)
    {
        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        ImGui::Image((ImTextureID)mScoreBannerSRV, ImVec2(BannerW, BannerH));
    }

    char CurNumBuf[32];
    char MaxNumBuf[32];
    sprintf_s(CurNumBuf, "%.1f m", PlayerGlobals::PLAYERLOCATION.y);
    sprintf_s(MaxNumBuf, "%.1f m", PlayerGlobals::HIGH_SCORE);

    const char* CurLabel = (const char*)u8"현재 높이 : ";
    const char* MaxLabel = (const char*)u8"최고 높이 : ";

    char FullCurBuf[64];
    char FullMaxBuf[64];
    sprintf_s(FullCurBuf, "%s%s", CurLabel, CurNumBuf);
    sprintf_s(FullMaxBuf, "%s%s", MaxLabel, MaxNumBuf);

    ImVec4 BrownTextCol = ImVec4(74.0f / 255.0f, 35.0f / 255.0f, 6.0f / 255.0f, 1.0f);

    float CurTextW = ImGui::CalcTextSize(FullCurBuf).x;
    DrawBoldText(FullCurBuf, (BannerW - CurTextW) * 0.5f, 18.0f, BrownTextCol, BrownTextCol, 0.5f);

    float HighText = ImGui::CalcTextSize(FullMaxBuf).x;
    DrawBoldText(FullMaxBuf, (BannerW - HighText) * 0.5f, 48.0f, BrownTextCol, BrownTextCol, 0.5f);

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
    ImGui::PopStyleVar();
}
