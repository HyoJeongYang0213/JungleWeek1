#pragma once

#include <d3d11.h>
#include "GameState.hpp"
#include "../Scene/SceneManager.h"
#include "../ImGui/imgui.h"
#include "../Renderer/WindowGlobals.hpp"

namespace GameUI
{
    inline bool ImageTextButton(const char* StrId, const char* Text, ID3D11ShaderResourceView* TextureSRV, const ImVec2& Size)
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

        ImDrawList* DrawList = ImGui::GetWindowDrawList();
        ImU32 DarkBrownColor = IM_COL32(74, 35, 6, 255);
        DrawList->AddText(TextPos, DarkBrownColor, Text);

        return BClicked;
    }

    inline void RenderTitleUI(GameContext& Context, SceneManager& InSceneManager, ID3D11ShaderResourceView* BtnFrameSRV)
    {
        if (Context.mBShowOptions)
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

        ImGuiWindowFlags Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoScrollbar;

        ImGui::Begin("TitleUI", nullptr, Flags);

        ImGui::SetWindowFontScale(1.3f);

        bool BClicked = ImageTextButton(
            "##PlayButton",
            u8"게임 시작",
            BtnFrameSRV,
            ImVec2(ButtonW, ButtonH)
        );

        ImGui::SetWindowFontScale(1.0f);

        if (BClicked)
        {
            Context.mCurrentState = EGameState::Playing;
            InSceneManager.NextScene();
        }

        ImGui::End();
    }

    inline void RenderSettingsUI(GameContext& Context, SceneManager& InSceneManager,
        ID3D11ShaderResourceView* SettingsBtnSRV, ID3D11ShaderResourceView* BtnFrameSRV)
    {
        ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
        float IconSize = 46.0f;

        ImGui::SetNextWindowPos(ImVec2(DisplaySize.x - IconSize - 20.0f, 20.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(IconSize + 10.0f, IconSize + 10.0f), ImGuiCond_Always);

        ImGuiWindowFlags IconFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoScrollbar;

        ImGui::Begin("SettingsIconButton", nullptr, IconFlags);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.4f));

        bool BClicked = ImGui::ImageButton(
            "##SettingsBtn",
            (ImTextureID)SettingsBtnSRV,
            ImVec2(IconSize, IconSize),
            ImVec2(0.0f, 0.0f),
            ImVec2(1.0f, 1.0f)
        );

        ImGui::PopStyleColor(3);

        if (BClicked)
        {
            Context.mBShowOptions = !Context.mBShowOptions;
        }

        ImGui::End();

        if (Context.mBShowOptions)
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

            ImGuiWindowFlags ModalFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
            ImGui::Begin(u8"게임 설정 & 조작법", &Context.mBShowOptions, ModalFlags);

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

            if (Context.mCurrentState == EGameState::Playing)
            {
                if (ImageTextButton("##BtnResume", u8"계속하기", BtnFrameSRV, ImVec2(185.0f, 48.0f)))
                {
                    Context.mBShowOptions = false;
                }

                ImGui::SameLine();

                if (ImageTextButton("##BtnRestart", u8"타이틀로", BtnFrameSRV, ImVec2(185.0f, 48.0f)))
                {
                    Context.mBShowOptions = false;
                    Context.mCurrentState = EGameState::Title;
                    InSceneManager.NextScene();
                    InSceneManager.NextScene();
                }
            }
            else
            {
                if (ImageTextButton("##BtnClose", u8"닫기", BtnFrameSRV, ImVec2(380.0f, 48.0f)))
                {
                    Context.mBShowOptions = false;
                }
            }

            ImGui::End();

            ImGui::PopStyleColor(6);
            ImGui::PopStyleVar(3);
        }
    }

    inline void RenderEndingUI(GameContext& Context, SceneManager& InSceneManager, bool& BIsExit, ID3D11ShaderResourceView* BtnFrameSRV)
    {
        ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
        float ModalW = 340.0f;
        float ModalH = 220.0f;
        ImGui::SetNextWindowPos(ImVec2((DisplaySize.x - ModalW) * 0.5f, (DisplaySize.y - ModalH) * 0.5f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ModalW, ModalH), ImGuiCond_Always);

        ImGuiWindowFlags Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
        ImGui::Begin("EndingUI", nullptr, Flags);

        ImGui::Spacing();
        ImGui::SetWindowFontScale(1.4f);
        float TextWidth = ImGui::CalcTextSize("GAME OVER").x;
        ImGui::SetCursorPosX((ModalW - TextWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.95f, 0.25f, 0.25f, 1.0f), "GAME OVER");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImageTextButton("##BtnEndingRestart", u8"게임 재시작", BtnFrameSRV, ImVec2(320.0f, 45.0f)))
        {
            Context.mCurrentState = EGameState::Title;
            InSceneManager.NextScene();
        }

        ImGui::Spacing();

        if (ImageTextButton("##BtnEndingExit", u8"게임 종료", BtnFrameSRV, ImVec2(320.0f, 45.0f)))
        {
            BIsExit = true;
        }

        ImGui::End();
    }
}