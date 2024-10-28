#include "RegnumStarter.h"

void runRoClientGame(const std::string& regnumLoginUser, const std::string& regnumLoginPassword) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::string regnumPath = setting_regnumInstallPath;
    
    std::string executablePath = regnumPath + "\\LiveServer\\ROClientGame.exe";
    std::string command = "\"" + executablePath + "\" \"" + regnumLoginUser + "\" \"" + regnumLoginPassword + "\"";

    LogDebug("Starting Regnum Online client with command: " + command);
    
    if (!CreateProcess(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, regnumPath.c_str(), &si, &pi)) {
        Log("Failed to start the Regnum Online client");
    } else {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void ShowRegnumStarter(bool& show_RegnumStarter) {
    if (!show_RegnumStarter) return;

    ImGui::Text("These settings are not working yet. Please use the Regnum Online client for now.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    static ImGui::FileBrowser fileDialog;
    static bool showFileDialog = false;

    if (ImGui::Button("Select Regnum Online Installation Path")) {
        showFileDialog = true;
    }

    if (showFileDialog) {
        fileDialog.Display();
    }

    if (fileDialog.HasSelected()) {
        setting_regnumInstallPath = fileDialog.GetSelected().string();
        fileDialog.ClearSelected();
        showFileDialog = false;
    }

    ImGui::Text("Selected Path: %s", setting_regnumInstallPath.c_str());

    ImGui::Columns(4, "RegnumAccounts");
    ImGui::Separator();
    ImGui::Text("Username");
    ImGui::NextColumn();
    ImGui::Text("Server");
    ImGui::NextColumn();
    ImGui::Text("Referrer");
    ImGui::NextColumn();
    ImGui::Text("Actions");
    ImGui::NextColumn();
    ImGui::Separator();

    static char regnumId[128] = "";
    static char regnumUsername[128] = "";
    static char regnumPassword[128] = "";
    static char regnumServer[128] = "";
    static char regnumReferrer[128] = "";

    ServerOption serverOptions[] = { {"val", "Valhalla"}, {"ra", "Ra"} };
    ReferrerOption referrerOptions[] = { {"nge", "NGE"}, {"gmg", "Gamigo"}, {"boa", "Boacompra"} };
    static int currentServer = 0;
    static int currentReferrer = 0;

    for (const auto& account : regnumAccounts) {
        ImGui::Text("%s", account.username.c_str());
        ImGui::NextColumn();

        const char* serverName = account.server.c_str();
        for (const auto& serverOption : serverOptions) {
            if (strcmp(serverOption.id, account.server.c_str()) == 0) {
                serverName = serverOption.name;
                break;
            }
        }
        ImGui::Text("%s", serverName);
        ImGui::NextColumn();

        const char* referrerName = account.referrer.c_str();
        for (const auto& referrerOption : referrerOptions) {
            if (strcmp(referrerOption.id, account.referrer.c_str()) == 0) {
                referrerName = referrerOption.name;
                break;
            }
        }
        ImGui::Text("%s", referrerName);
        ImGui::NextColumn();

        std::string editButtonLabel = "Edit##" + std::to_string(account.id);
        if (ImGui::Button(editButtonLabel.c_str())) {
            snprintf(regnumId, IM_ARRAYSIZE(regnumId), "%d", account.id);
            snprintf(regnumUsername, IM_ARRAYSIZE(regnumUsername), "%s", account.username.c_str());
            snprintf(regnumPassword, IM_ARRAYSIZE(regnumPassword), "%s", account.password.c_str());
            snprintf(regnumServer, IM_ARRAYSIZE(regnumServer), "%s", account.server.c_str());
            snprintf(regnumReferrer, IM_ARRAYSIZE(regnumReferrer), "%s", account.referrer.c_str());

            for (int i = 0; i < IM_ARRAYSIZE(serverOptions); ++i) {
                if (strcmp(serverOptions[i].id, account.server.c_str()) == 0) {
                    currentServer = i;
                    break;
                }
            }
            for (int i = 0; i < IM_ARRAYSIZE(referrerOptions); ++i) {
                if (strcmp(referrerOptions[i].id, account.referrer.c_str()) == 0) {
                    currentReferrer = i;
                    break;
                }
            }
        }

        ImGui::SameLine();
        std::string deleteButtonLabel = "Delete##" + std::to_string(account.id);
        if (ImGui::Button(deleteButtonLabel.c_str())) {
            DeleteRegnumAccount(account.id);
        }

        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::Separator();

    ImGui::InputText("Username", regnumUsername, IM_ARRAYSIZE(regnumUsername));
    ImGui::InputText("Password", regnumPassword, IM_ARRAYSIZE(regnumPassword), ImGuiInputTextFlags_Password);
    ImGui::Combo("Server", &currentServer, [](void* data, int idx, const char** out_text) {
        *out_text = ((ServerOption*)data)[idx].name;
        return true;
    }, serverOptions, IM_ARRAYSIZE(serverOptions));
    ImGui::Combo("Referrer", &currentReferrer, [](void* data, int idx, const char** out_text) {
        *out_text = ((ReferrerOption*)data)[idx].name;
        return true;
    }, referrerOptions, IM_ARRAYSIZE(referrerOptions));

    if (ImGui::Button("Save Account")) {
        SaveRegnumAccount(
            regnumUsername, 
            regnumPassword, 
            serverOptions[currentServer].id, 
            referrerOptions[currentReferrer].id, 
            regnumId[0] == '\0' ? 0 : atoi(regnumId)
        );
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    static int selectedAccount = -1;
    if (ImGui::BeginCombo("##Select Account", selectedAccount == -1 ? "Select an account" : regnumAccounts[selectedAccount].username.c_str())) {
        for (int i = 0; i < regnumAccounts.size(); i++) {
            bool isSelected = (selectedAccount == i);
            if (ImGui::Selectable(regnumAccounts[i].username.c_str(), isSelected)) {
                selectedAccount = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("Play")) {
        if (selectedAccount != -1) {
            const auto& account = regnumAccounts[selectedAccount];
            runRoClientGame(account.username, account.password);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    static float soundVolume = 0.5f;
    ImGui::SliderFloat("Sound Volume", &soundVolume, 0.0f, 1.0f);

    static bool enableMusic = true;
    ImGui::Checkbox("Enable Music", &enableMusic);

    static bool enableSoundEffects = true;
    ImGui::Checkbox("Enable Sound Effects", &enableSoundEffects);

    if (ImGui::Button("Save Settings")) {
        SaveSettings();
    }
}