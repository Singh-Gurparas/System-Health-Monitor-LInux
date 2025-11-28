#include <DisplayInformation.h>
#include <SystemInformation.h>
#include "imgui.h"
#include <Helper.h>
#include <Formatting.h>
#include "Graphs.h"
#include <chrono>         // for std::chrono sampling option (optional)
#include <GLFW/glfw3.h>   // if you plan to use glfwGetTime() for timing
#include <cstdlib> // for system()

// Global or static inside render function so data persists across frames
static Graphs graphs(120); // store 120 samples (adjust as desired)

void DisplayInformation::display_main_histogram() {
    // --- SAMPLE CPU TEMP ---
    std::string cpu_temperature_string = SystemInformation::cpu_temperature();
    int cpu_temperature_int = Helper::string_to_int(cpu_temperature_string.substr(0,2));
    float cpuTemp = static_cast<float>(cpu_temperature_int);   // °C

    // --- SAMPLE MEMORY LOAD ---
    std::vector<std::string> memory_information = SystemInformation::memory_information();
    std::string available = memory_information[0];
    std::string used      = memory_information[2];

    std::string a, b;
    for (int i = 0; i < used.length(); i++) {
        if (isdigit(available[i])) a.push_back(available[i]);
        if (isdigit(used[i]))      b.push_back(used[i]);
    }

    float available_f = static_cast<float>(Helper::string_to_int(a));
    float used_f      = static_cast<float>(Helper::string_to_int(b));
    float memPercent  = (used_f / available_f) * 100.0f;     // convert to 0–100

    // --- SAMPLE ONCE PER SECOND ---
    static double lastSample = 0.0;
    double now = glfwGetTime();
    if (lastSample == 0.0) lastSample = now;

    if (now - lastSample >= 1.0) {
        lastSample = now;
        graphs.AddCpuSample(cpuTemp);
        graphs.AddMemorySample(memPercent);
    }

    // --- DRAW GRAPHS ---
    ImGui::Begin("System Monitor");

    ImGui::Text("CPU Temperature History");
    graphs.DrawCpuGraph();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Memory Load History");
    graphs.DrawMemoryGraph();

    ImGui::End();
}


/*void DisplayInformation::display_memory_load_progress_bar() {
    std::vector<std::string> memory_information = SystemInformation::memory_information();

    std::string const available_memory = memory_information[0];
    std::string const consumed_memory = memory_information[2];

    std::string trimmed_available_memory;
    std::string trimmed_consumed_memory;

    for (int i = 0; i < consumed_memory.length(); i++) {
        if (static_cast<int>(available_memory[i]) >= 48 && static_cast<int>(available_memory[i]) <= 57) {
            trimmed_available_memory.push_back(available_memory[i]);
        }
        if (static_cast<int>(consumed_memory[i]) >= 48 && static_cast<int>(consumed_memory[i]) <= 57) {
            trimmed_consumed_memory.push_back(consumed_memory[i]);
        }
    }

    auto const available_memory_float = static_cast<float>(Helper::string_to_int(trimmed_available_memory));
    auto const consumed_memory_float = static_cast<float>(Helper::string_to_int(trimmed_consumed_memory));

    auto const available_minus_consumed_memory = available_memory_float - consumed_memory_float;

    // Percentage is in decimal form ie 5% = 0.05f.
    auto const memory_load_percentage = static_cast<float>(available_minus_consumed_memory / available_memory_float);

    std::string const load_percentage_label = "Memory Load";

    Formatting::centered_imgui_text(load_percentage_label);
    Formatting::vertical_spacing(2);
    ImGui::ProgressBar(memory_load_percentage, ImVec2(600.0f, 0.0f));
}
*/

void DisplayInformation::display_cpu_temperature() {
    // --- Read CPU Temperature as float ---
    std::string tempStr = SystemInformation::cpu_temperature();
    float cpuTemp = std::stof(tempStr);

    // ---- ALERT TRIGGER ----
    static bool emailSent = false;
    if (cpuTemp > 50.0f && !emailSent) {
        system("echo \"System Health Alert!! CPU temperature is over 50 degree celsius\" | msmtp gps06pta@gmail.com");
        std::cout << "[ALERT] CPU temp exceeded 50! Email sent.\n";
        emailSent = true; // Prevent continuous spam
    }

    // ------ Existing ImGui UI (keep same) ------
    if (ImGui::CollapsingHeader("CPU Information")) {
        ImGui::BeginTable("CPU Information", 2);
        ImGui::TableNextColumn();

        ImGui::Text("CPU Temperature");

        ImGui::TableNextColumn();

        ImGui::Text("%s C°", tempStr.c_str());

        ImGui::EndTable();
    }
}


void DisplayInformation::display_gpu_temperature() {
    if (ImGui::CollapsingHeader("GPU Information")) {
        ImGui::BeginTable("GPU Information", 2);
        ImGui::TableNextColumn();

        ImGui::Text("GPU Temperature");

        ImGui::TableNextColumn();

        std::string gpu_temp_placeholder = "<placeholder>";
        ImGui::Text("%s", gpu_temp_placeholder.c_str());

        ImGui::EndTable();
    }
}

void DisplayInformation::display_uptime() {
    if (ImGui::CollapsingHeader("Uptime")) {
        ImGui::BeginTable("Uptime Table", 2);
        ImGui::TableNextColumn();

        ImGui::Text("Uptime");

        ImGui::TableNextColumn();

        SystemInformation::current_uptime_from_proc();
        ImGui::Text("%d hours %d minutes %d seconds", SystemInformation::uptime_hours(), SystemInformation::uptime_minutes(), SystemInformation::uptime_seconds());

        ImGui::EndTable();
    }
}

void DisplayInformation::display_memory_information() {
    std::vector<std::string> memory_information = SystemInformation::memory_information();

    if (ImGui::CollapsingHeader("Memory Information")) {
        ImGui::BeginTable("MemoryInformation", 3);
        ImGui::TableNextColumn();

        for (auto const& line: memory_information) {
            std::string memory_topic;
            std::string memory_data;
            bool string_selection_flag = false;

            auto const memory_info_line_length = line.length();
            for (int i = 0; i < memory_info_line_length-2; i++) {
                if (line[i] == ' ') {
                    string_selection_flag = true;
                    continue;
                }

                string_selection_flag ? memory_data.push_back(line[i]) : memory_topic.push_back(line[i]);
            }

            ImGui::Text("%s", memory_topic.c_str());

            ImGui::TableNextColumn();

            ImGui::Text("%s", memory_data.c_str());

            ImGui::TableNextColumn();

            ImGui::Text("kB");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
        }

        ImGui::EndTable();
    }
}
