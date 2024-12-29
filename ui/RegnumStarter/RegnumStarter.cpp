#include "RegnumStarter.h"
#include <fstream>
#include <sstream>


static int selectedAccount = -1;
extern std::string setting_regnumInstallPath;

void UpdateConfigValue(const std::string& key, const std::string& value) {
    std::string configPath = setting_regnumInstallPath + "\\game.cfg";
    std::ifstream configFileRead(configPath);
    std::string configContent;
    bool lineExists = false;

    if (configFileRead.is_open()) {
        std::string line;
        while (std::getline(configFileRead, line)) {
            if (line.find(key) != std::string::npos) {
                line = key + " = " + value;
                lineExists = true;
            }
            configContent += line + "\n";
        }
        configFileRead.close();
    }

    if (!lineExists) {
        configContent += key + " = " + value + "\n";
    }

    std::ofstream configFileWrite(configPath);
    if (configFileWrite.is_open()) {
        configFileWrite << configContent;
        configFileWrite.close();
    } else {
        LogDebug("Failed to open game.cfg for writing");
    }
}

void runRoClientGame(const std::string& regnumLoginUser, const std::string& regnumLoginPassword) {
    // Update cl_update_all_resources in game.cfg
    UpdateConfigValue("cl_update_all_resources", "1");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::string regnumPath = setting_regnumInstallPath;
    
    std::string executablePath = regnumPath + "\\LiveServer\\ROClientGame.exe";
    std::string command = "powershell.exe -Command \"cd '" + regnumPath + "\\LiveServer'; .\\ROClientGame.exe '" + regnumLoginUser + "' '" + regnumLoginPassword + "'\"";
    
    if (!CreateProcess(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, regnumPath.c_str(), &si, &pi)) {
        LogDebug("Failed to start the Regnum Online client");
    } else {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void CheckAndUpdateConfig() {
    std::string configPath = setting_regnumInstallPath + "\\game.cfg";
    std::ifstream configFileRead(configPath);
    std::unordered_map<std::string, std::string> configMap;

    if (configFileRead.is_open()) {
        std::string line;
        while (std::getline(configFileRead, line)) {
            std::istringstream lineStream(line);
            std::string key;
            if (std::getline(lineStream, key, '=')) {
                std::string value;
                if (std::getline(lineStream, value)) {
                    configMap[key] = value;
                }
            }
        }
        configFileRead.close();
    }

    bool updated = false;

    auto updateIfDifferent = [&](const std::string& key, const std::string& value) {
        if (configMap[key] != value) {
            UpdateConfigValue(key, value);
            updated = true;
        }
    };

    updateIfDifferent("snd_sound_volume", std::to_string(soundVolume));
    updateIfDifferent("snd_music_volume", std::to_string(enableMusic ? 1 : 0));
    updateIfDifferent("enable_sound_effects", std::to_string(enableSoundEffects ? 1 : 0));
    updateIfDifferent("cl_show_loading_screen", std::to_string(showLoadingScreen ? 1 : 0));
    updateIfDifferent("show_intro", std::to_string(showIntro ? 1 : 0));
    updateIfDifferent("cl_cpu_idle_time", "0");
    updateIfDifferent("cl_update_all_resources", "0");
    updateIfDifferent("vg_fullscreen_borderless", "1");
    updateIfDifferent("vg_fullscreen_mode", "0");

    if (updated) {
        LogDebug("Configuration file updated with saved settings.");
    }
}

void ShowRegnumSettings(bool& show_RegnumSettings) {

    static bool configChecked = false;
    if (!configChecked) {
        CheckAndUpdateConfig();
        configChecked = true;
    }

    if (!show_RegnumSettings) return;

    static ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_SelectDirectory);
    static bool showFileDialog = false;

    ImGui::SeparatorText("Regnum Installation Path");

    ImGui::Text("Path to the Regnum Online installation directory");

    if (ImGui::Button("Change")) {
        fileDialog.Open();
        showFileDialog = true;
    }

    ImGui::SameLine();
    ImGui::Text("%s", setting_regnumInstallPath.c_str());

    if (showFileDialog) {
        fileDialog.Display();
        if (fileDialog.HasSelected()) {
            setting_regnumInstallPath = fileDialog.GetSelected().string();
            fileDialog.ClearSelected();
            showFileDialog = false;
        }
    }

    static bool filesChecked = false; // Static flag to ensure the code runs only once

    if (!filesChecked) {
        // Check if splash_nge.ogg and splash_nge.png exist
        std::string livePath = setting_regnumInstallPath + "\\LiveServer\\";
        std::vector<std::string> filesToCheck = {
            "splash_nge.png",
            "splash_nge.ogg"
        };
        bool filesExist = true;
        for (const auto& file : filesToCheck) {
            std::string filePath = livePath + file;
            std::ifstream infile(filePath);
            if (!infile.good()) {
                filesExist = false;
                break;
            }
        }

        // If files exist and the checkbox is saved as false, delete the files
        if (filesExist && !showIntro) {
            for (const auto& file : filesToCheck) {
                std::string filePath = livePath + file;
                if (remove(filePath.c_str()) != 0) {
                    LogDebug("Failed to delete file: " + filePath);
                } else {
                    LogDebug("Deleted file: " + filePath);
                }
            }
        }

        // If files do not exist and the checkbox is saved as true, download the files
        if (!filesExist && showIntro) {
            std::vector<std::pair<std::string, std::string>> filesToDownload = {
                {"https://patch.regnumstarter.cor-forum.de/assets/splash_nge.png", livePath + "splash_nge.png"},
                {"https://patch.regnumstarter.cor-forum.de/assets/splash_nge.ogg", livePath + "splash_nge.ogg"}
            };
            for (const auto& file : filesToDownload) {
                HRESULT hr = URLDownloadToFile(NULL, file.first.c_str(), file.second.c_str(), 0, NULL);
                if (SUCCEEDED(hr)) {
                    LogDebug("Downloaded file: " + file.second);
                } else {
                    LogDebug("Failed to download file: " + file.second);
                }
            }
        }

        filesChecked = true; // Set the flag to true after the operation is performed
    }

    ImGui::SeparatorText("Settings");

    if (ImGui::TreeNode("Settings")) {
        ImGui::SeparatorText("Sound Settings");
        ImGui::Text("Sound Volume");
        ImGui::SliderFloat("##Sound Volume", &soundVolume, 0.0f, 100.0f, "%.0f%%");
        ImGui::Checkbox("Music", &enableMusic);
        ImGui::SameLine();
        ImGui::Checkbox("Sound Effects", &enableSoundEffects);

        ImGui::SeparatorText("Advanced");
        ImGui::Checkbox("Show Loading Screen", &showLoadingScreen);
        ImGui::Checkbox("Show Intro", &showIntro);

        if (ImGui::Button("Save Settings")) {
            UpdateConfigValue("snd_sound_volume", std::to_string(soundVolume));
            UpdateConfigValue("snd_music_volume", std::to_string(enableMusic ? 1 : 0));
            UpdateConfigValue("enable_sound_effects", std::to_string(enableSoundEffects ? 1 : 0));
            UpdateConfigValue("cl_show_loading_screen", std::to_string(showLoadingScreen ? 1 : 0));
            UpdateConfigValue("show_intro", std::to_string(showIntro ? 1 : 0));
            SaveSettings();

            std::string livePath = setting_regnumInstallPath + "\\LiveServer\\";
            std::vector<std::string> filesToDelete = {
                "splash_nge.png",
                "splash_nge.ogg"
            };

            if (showIntro) {
                // Check if files exist and download if they don't
                for (const auto& file : filesToDelete) {
                    std::string filePath = livePath + file;
                    std::ifstream infile(filePath);
                    if (!infile.good()) {
                        LogDebug("File does not exist: " + filePath + ". Downloading...");
                        std::string url = "https://patch.regnumstarter.cor-forum.de/assets/" + file; // Replace with actual URL

                        // Download file using URLDownloadToFile
                        HRESULT hr = URLDownloadToFile(NULL, url.c_str(), filePath.c_str(), 0, NULL);
                        if (SUCCEEDED(hr)) {
                            LogDebug("Downloaded file: " + filePath);
                        } else {
                            LogDebug("Failed to download file: " + filePath);
                        }
                    } else {
                        LogDebug("File already exists: " + filePath);
                    }
                }
            } else {
                // Delete files
                for (const auto& file : filesToDelete) {
                    std::string filePath = livePath + file;
                    if (remove(filePath.c_str()) != 0) {
                        LogDebug("Failed to delete file make sure to select your Game Path: " + filePath);
                        // MessageBox(NULL, "Failed to delete file make sure to select your Game Path", "RegnumStarter", MB_OK | MB_TOPMOST);
                    } else {
                        LogDebug("Deleted file: " + filePath);
                    }
                }
            }
        }
        ImGui::TreePop();
    }
}

void ShowRegnumAccounts(bool& show_RegnumAccounts) {
    if (!show_RegnumAccounts) return;

    ImGui::SeparatorText("Regnum Accounts");

    if (ImGui::Button("Add Account")) {
        ImGui::OpenPopup("Add Account");
    }

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

    ServerOption serverOptions[] = { {"ra", "Ra"} };
    ReferrerOption referrerOptions[] = { {"nge", "NGE / NGD"}, {"gmg", "Gamigo (Deutsch)"}, {"boa", "Boacompra"} };
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

        std::string deleteButtonLabel = "Delete##" + std::to_string(account.id);
        if (ImGui::Button(deleteButtonLabel.c_str())) {
            DeleteRegnumAccount(account.id);
        }

        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    if (ImGui::BeginPopup("Add Account")) {
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
            if (strlen(regnumUsername) > 0 && strlen(regnumPassword) > 0) {
                bool accountExists = false;
                for (const auto& account : regnumAccounts) {
                    if (strcmp(account.username.c_str(), regnumUsername) == 0) {
                        accountExists = true;
                        break;
                    }
                }

                if (!accountExists) {
                    SaveRegnumAccount(
                        regnumUsername, 
                        regnumPassword, 
                        serverOptions[currentServer].id, 
                        referrerOptions[currentReferrer].id, 
                        regnumId[0] == '\0' ? 0 : atoi(regnumId)
                    );
                    ImGui::CloseCurrentPopup();
                } else {
                    MessageBox(NULL, "Account already exists.", "RegnumStarter", MB_OK | MB_TOPMOST);
                    LogDebug("Account already exists.");
                }
            } else {
                MessageBox(NULL, "Username or password cannot be empty.", "RegnumStarter", MB_OK | MB_TOPMOST);
                LogDebug("Username or password cannot be empty.");
            }
        }
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}