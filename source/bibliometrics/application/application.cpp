//
// Created by Alan Freitas on 17/08/20.
//

// C++
#include <vector>
#include <iomanip>
#include <string>
#include <algorithm>

#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

// ImGui
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

// ImPlot
#include "implot.h"

// Spdlog
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/fmt/bundled/ostream.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// XML
#include <pugixml.hpp>

// Json
#include <nlohmann/json.hpp>

// CSV
#include <cmath>
#ifndef SYSTEM_HAS_ISNAN_OUTSIDE_STD
inline bool isnan(long double v) noexcept { return std::isnan(v); }
#endif
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4458)
#pragma warning(disable : 4244)
#pragma warning(disable : 4456)
#pragma warning(disable : 4702)
#pragma warning(disable : 4996)
#pragma warning(disable : 4127)
#endif
#include <csv.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Internal
#include "application.h"
#include "bibliometrics/common/algorithm.h"
#include "bibliometrics/graph/author.h"

namespace bibliometrics {
#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
    namespace fs = std::experimental::filesystem;
#else
    namespace fs = std::filesystem;
#endif

    application::application(std::vector<std::string> input_files, std::string output, bool gui,
                                                   bool verbose_tracing) : input_files_(input_files), output_stem_(output), verbose_(verbose_tracing), gui_(gui) {
        setup_logger();
        find_valid_input_files();
        setup_output_dir();
        init_opengl_and_imgui();
    }

    void application::init_opengl_and_imgui() {
        auto glfw_error_callback = [](int error, const char* description) {
          fprintf(stderr, "Glfw Error %d: %s\n", error, description);
        };

        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            return;
        }
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        window_ = glfwCreateWindow(1280, 720, "BibExplorer", nullptr, nullptr);
        if (window_ == nullptr) {
            return;
        }
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);
        bool err = gladLoadGL() == 0;
        if (err) {
            log->error("Failed to initialize OpenGL loader!");
            return;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        // Look for all fonts in the system
        std::vector<std::string> font_directories = {
            // Windows
            "%WINDIR%/fonts",
            // Mac OS
            "/System/Library/Fonts",
            "/Library/Fonts",
            "~/Library/Fonts",
            "/Network/Library/Fonts",
            // Linux
            "/usr/share/fonts",
            "/usr/local/share/fonts",
            "~/.fonts"
        };
        std::vector<fs::path> font_files;
        for (const auto &font_directory : font_directories) {
            if (fs::is_directory(font_directory)) {
                log->trace("{} is a directory", font_directory);
                for (const auto &font_file : fs::directory_iterator{font_directory}) {
                    if (fs::is_regular_file(font_file)) {
                        const auto& p = font_file.path();
                        if (p.extension() == ".ttf") {
                            log->trace("Emplacing font {}", p.string());
                            font_files.emplace_back(p);
                        }
                    }
                }
            }
        }

        bool default_font_is_set = false;
        auto try_to_set_font = [&](const std::string& font_name, float size) {
          if (!default_font_is_set) {
                for (const auto &font_file : font_files) {
                    if (font_file.filename().string().find(font_name) !=
                        std::string::npos) {
                        log->info("Setting font to {}", font_file);
                        io.Fonts->AddFontFromFileTTF(font_file.string().c_str(),size);
                        default_font_is_set = true;
                        break;
                    }
                }
            }
        };

        try_to_set_font("Helvetica",13);
        try_to_set_font("Roboto",13);
        try_to_set_font("Arial",15);
        try_to_set_font("Sans",15);
        try_to_set_font("Times",15);
        if (!default_font_is_set) {
            io.Fonts->AddFontDefault();
        }

        ImGui::StyleColorsClassic();
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void application::setup_logger() {
        log = spdlog::stdout_color_mt("console", spdlog::color_mode::always);
        log->set_pattern("[%d/%b/%Y %H:%M:%S] [%^%l%$] %^%v%$");
        log->info("Starting parser application");
        if (verbose_) {
            log->set_level(spdlog::level::trace);
        }
    }

    void application::find_valid_input_files() {
        log->debug("Parsing input files");
        std::vector<std::string> input_files;

        // Get files from directories
        for (const auto &input_file : input_files_) {
            if (fs::is_directory(input_file)) {
                log->trace("{} is a directory", input_file);
                for (const auto &p : fs::directory_iterator{input_file}) {
                    if (!fs::is_directory(p)) {
                        log->trace("Emplacing {} in input files", p.path());
                        input_files.emplace_back(p.path().string());
                    }
                }
            }
        }
        input_files_.clear();

        // Filter valid file formats
        for (const auto &input_file : input_files) {
            fs::path p = input_file;
            if (is_valid_extension(p.extension().string())) {
                input_files_.emplace_back(input_file);
            } else {
                log->trace("{} has an invalid extension.", p.filename());
            }
        }

        log->debug("Final input files: {}", input_files_);
    }

    bool application::is_valid_extension(const std::string_view &ext) {
        return ext == ".csv" || ext == ".json" || ext == ".xml";
    }

    void application::setup_output_dir() {
        log->debug("Parsing output {}", output_stem_);
        if (fs::exists(output_stem_)) {
            if (fs::is_directory(output_stem_)) {
                log->trace("Output is an existing directory");
                output_stem_ /= default_output_stem;
                log->trace("Output stem: {}", output_stem_);
            } else {
                log->trace("Output is an existing file");
                output_stem_ = output_stem_.parent_path() / output_stem_.stem();
                log->trace("Output stem: {}", output_stem_);
            }
        } else {
            if (!output_stem_.has_extension()) {
                log->trace("Output is a directory {}. Add default stem.", output_stem_);
                output_stem_ /= default_output_stem;
                log->trace("Output stem: {}", output_stem_);
            } else {
                log->trace("Output is a file. Removing extension and use stem.");
                output_stem_ = output_stem_.parent_path() / output_stem_.stem();
                log->trace("Output stem: {}", output_stem_);
            }
        }
        log->debug("Final output stem: {}", output_stem_);
    }

    void application::run() {
        run_parser();
        run_scraper();
    }

    void application::save_author_group() {
        fs::path author_group_file = output_stem_;
        author_group_file += ".csv";
        log->info("Saving author group with {} authors as {}", internal_group_.authors().size(), author_group_file);
        internal_group_.save(author_group_file.string());
    }

    void application::run_parser() {
        log->info("Parsing input files");
        for (const auto &input_file : input_files_) {
            current_input_file_ = &input_file;
            render_gui();
            log->trace("Parsing {}", input_file);
            fs::path input = input_file;
            if (input.extension() == ".xml") {
                author a = parse_lattes(input);
                log->info("New author: {} (id: {})", a.name(), a.id());
                internal_group_.add_author(a);
            } else if (input.extension() == ".csv") {
                author a = parse_csv(input);
                log->info("New author: {} (id: {})", a.name(), a.id());
                internal_group_.add_author(a);
            } else if (input.extension() == ".json") {
                author a = parse_json(input);
                log->info("New author: {} (id: {})", a.name(), a.id());
                internal_group_.add_author(a);
            }
            ++input_files_parsed_;
        }
        // save
        save_author_group();
    }

    void application::run_scraper() {
        log->info("Downloading references");
        // Make sure cache directory is there
        fs::path cache_dir = "cache";
        fs::path ms_cache_dir = cache_dir / "microsoft";
        if (!fs::exists(cache_dir)) {
            fs::create_directory(cache_dir);
        }

        // List keys and ensure sub-cache directories are there
        if (!microsoft_key_.empty()) {
            log->info("Microsoft key: {}", microsoft_key_);
            if (!fs::exists(ms_cache_dir)) {
                fs::create_directory(ms_cache_dir);
            }
        }

        if (!elsevier_key_.empty()) {
            log->info("Elsevier key: {}", elsevier_key_);
            if (!fs::exists(ms_cache_dir)) {
                fs::create_directory(ms_cache_dir);
            }
        }

        // Iterate papers and fetch data based on many APIs
        total_papers_to_fetch_ = internal_group_.n_papers();
        papers_fetched_ = 0;
        total_authors_to_fetch_ = internal_group_.authors().size();
        authors_fetched_ = 0;
        for (auto &author : internal_group_) {
            current_author_ = &author;
            for (auto &paper : author) {
                current_paper_ = &paper;
                fetch_microsoft(ms_cache_dir,author,paper);

                // More APIs:
                // - https://github.com/CrossRef/rest-api-doc
                // - Scopus: through elsevier
                // - https://dev.elsevier.com/documentation/PlumXMetricsAPI.wadl
                // - https://ceu.libguides.com/apis
                // - https://libraries.mit.edu/scholarly/publishing/apis-for-scholarly-resources/
                // - https://guides.lib.berkeley.edu/information-studies/apis
                // - https://api.semanticscholar.org
                // - https://serpapi.com/google-scholar-api
                // - https://dev.springernature.com
                // - https://www.lib.ncsu.edu/text-and-data-mining/scholarly-apis-datasets
                // - https://guides.lib.purdue.edu/c.php?g=412592
                // - https://www.nature.com/articles/d41586-018-04190-5
                // - https://dev.mendeley.com

                // Unfortunately, google scholar does not offer any API for querying papers.
                // This would be very useful but it's not worth the workarounds to make
                // that happen for now.
                // However, we have so many APIs that your paper is very likely to be found
                // in one of them.
                if (citation_histogram_.size() < static_cast<size_t>(paper.citations())) {
                    citation_histogram_.resize(static_cast<size_t>(paper.citations() + 1));
                }
                ++citation_histogram_[paper.citations()];
                ++papers_fetched_;
                render_gui();
            }
            ++authors_fetched_;
        }
        save_author_group();
    }

    void application::render_gui() {
        constexpr auto HelpMarker = [](const char* desc) {
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(desc);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        };

        constexpr auto HelpOnWidget = [](const char* desc) {
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(desc);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        };

        if (!glfwWindowShouldClose(window_)) {
            static ImVec4 clear_color = ImVec4(0.0f, 0.4470f, 0.7410f, 0.f);
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

            static bool show_processing_window{true};
            static bool show_indicators_window{true};
            static bool show_configuration_window{true};
#ifdef BUILD_DEMO_IMGUI
            static bool show_demo_window{false};
            static bool show_plot_demo_window{false};
#endif
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("View")) {
                    ImGui::MenuItem("Processing", nullptr, &show_processing_window);
                    ImGui::MenuItem("Indicators", nullptr, &show_indicators_window);
                    ImGui::Separator();
                    ImGui::MenuItem("Configuration", nullptr, &show_configuration_window);
#ifdef BUILD_DEMO_IMGUI
                    ImGui::Separator();
                    ImGui::MenuItem("Demo Widgets", nullptr, &show_demo_window);
                    ImGui::MenuItem("Demo Plots", nullptr, &show_plot_demo_window);
#endif
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            time_t theTime = time(NULL);
            struct tm *aTime = localtime(&theTime);
            int year = aTime->tm_year + 1900;
            int min_year = year;
            for (const auto &author : internal_group_) {
                for (const auto &paper : author) {
                    if (paper.year() > 1000 && paper.year() < min_year) {
                        min_year = paper.year();
                    }
                }
            }

            constexpr size_t n_indicators = 7;
            const char* indicators[] = { "Citations/Author", "Citations", "H-Index", "H-Core", "i10-Index", "Impact Factor", "Papers" };
            constexpr size_t n_indicators_with_avg = n_indicators + 1;
            const char* indicators_with_avg[] = { "Citations/Author", "Citations", "H-Index", "H-Core", "i10-Index", "Impact Factor", "Papers" , "Average" };

            static int sort_authors_by_current = 0;
            int max_time_window = year - min_year + 1;
            if (show_configuration_window) {
                ImGui::Begin("Configuration", &show_configuration_window);
                ImGui::Text("Logging");
                ImGui::Checkbox("Verbose", &verbose_);
                if (verbose_) {
                    log->set_level(spdlog::level::trace);
                } else {
                    log->set_level(spdlog::level::info);
                }
                ImGui::Separator();
                ImGui::Text("GUI");
                ImGui::Checkbox("Persistent Window", &persistent_window_);
                ImGui::SameLine();
                HelpMarker("Close the GUI when done processing");
                ImGui::ColorEdit3("Background", (float*)&clear_color);
                if (ImGui::TreeNode("Style")) {
                    ImGui::ShowStyleEditor();
                    ImGui::TreePop();
                    ImGui::Separator();
                }
#ifdef BUILD_DEMO_IMGUI
                ImGui::Checkbox("Show Demo Window", &show_demo_window);
                ImGui::SameLine();
                HelpMarker("The demo window is just a handy reference for developers.");
                ImGui::Checkbox("Show Plot Demo Window", &show_plot_demo_window);
                ImGui::SameLine();
                HelpMarker("The demo window is just a handy reference for developers.");
#endif
                ImGui::Separator();
                if (ImGui::TreeNode("Input Files","Input Files (%zu)", input_files_.size())) {
                    for (const auto &input_file : input_files_) {
                        ImGui::BulletText("%s", fs::path(input_file).filename().string().c_str());
                    }
                    ImGui::TreePop();
                }
                ImGui::Separator();
                ImGui::Text("Output Files");
                std::string output_bib_ = output_stem_.string() + ".csv";
                ImGui::Text("Bibliography: %s", output_bib_.c_str());
                ImGui::Separator();

                auto abbreviate = [](const std::string& str) {
                    size_t n_show = std::min(static_cast<size_t>(str.size()/2), static_cast<size_t>(8UL));
                    return str.substr(0,n_show) + (str.size() > n_show ? std::string(str.size() - n_show, '*') : std::string{});
                };
                ImGui::Text("API Keys");
                ImGui::Columns(2, "API Keys");
                ImGui::Separator();
                ImGui::Text("API"); ImGui::NextColumn();
                ImGui::Text("Key"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Text("Microsoft Academic");
                ImGui::SameLine();
                HelpMarker("Register: https://www.microsoft.com/en-us/research/project/academic-knowledge/\n"
                                 "Docs: https://docs.microsoft.com/en-us/academic-services/project-academic-knowledge/introduction");
                ImGui::NextColumn();
                ImGui::Text("%s", abbreviate(microsoft_key_).c_str()); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Text("Elsevier Developers");
                ImGui::SameLine();
                HelpMarker("Register: http://dev.elsevier.com/\nDocs: https://dev.elsevier.com/api_docs.html");
                ImGui::NextColumn();
                ImGui::Text("%s", abbreviate(elsevier_key_).c_str());  ImGui::NextColumn();
                ImGui::Columns(1);
                ImGui::Separator();

                ImGui::Text("Indicators");
                ImGui::DragInt("Time Window", &indicator_time_window_size_, 1, 0, max_time_window, "%d years");
                ImGui::SameLine();
                HelpMarker(fmt::format("Size of time window in years.\nIndicators will consider productions in the last {} years", indicator_time_window_size_).c_str());
                ImGui::Text("Time Window: %d-%d", year - indicator_time_window_size_ + 1, year);
                ImGui::Combo("Author order", &sort_authors_by_current, indicators, n_indicators);
                ImGui::End();
            }

#ifdef BUILD_DEMO_IMGUI
            if (show_demo_window) {
                ImGui::ShowDemoWindow(&show_demo_window);
            }

            if (show_plot_demo_window) {
                ImPlot::ShowDemoWindow(&show_plot_demo_window);
            }
#endif

            if (show_processing_window) {
                ImGui::Begin("Processing", &show_processing_window);
                ImGui::Text("Parsing");
                if (current_input_file_) {
                    ImGui::Text("Current Input: %s", current_input_file_->c_str());
                } else {
                    ImGui::Text("Current Input: (none)");
                }
                ImGui::Text("Input Files Parsed");
                ImGui::ProgressBar(float(input_files_parsed_)/float(input_files_.size()), ImVec2(0.0f,0.0f));
                ImGui::Separator();
                ImGui::Text("Fetching");
                ImGui::Text("Author");
                if (current_author_) {
                    ImGui::Text("ID: %s", current_author_->id().c_str());
                    ImGui::Text("Name: %s", current_author_->name().c_str());
                    ImGui::Text("Papers: %u", current_author_->number_of_papers());
                    ImGui::ProgressBar(float(authors_fetched_)/float(total_authors_to_fetch_), ImVec2(0.0f,0.0f));
                }
                ImGui::Separator();
                ImGui::Text("Paper");
                if (current_paper_) {
                    ImGui::Text("ID: %s", current_paper_->id().c_str());
                    ImGui::Text("Title: %s", current_paper_->title().c_str());
                    ImGui::Text("Journal: %s", current_paper_->journal().c_str());
                    ImGui::Text("Year: %d", current_paper_->year());
                    ImGui::Text("Citations: %d", current_paper_->citations());
                    ImGui::ProgressBar(float(papers_fetched_)/float(total_papers_to_fetch_), ImVec2(0.0f,0.0f));
                }
                ImGui::Separator();
                ImGui::Text("Citations");
                float scale_max = citation_histogram_.size() == 1 ? citation_histogram_[0] : citation_histogram_[1];
                char overlay[32];
                sprintf(overlay, "Citations");
                ImGui::PlotHistogram("##values", citation_histogram_.data(), static_cast<int>(citation_histogram_.size()), 0, overlay, 0.0f, scale_max, ImVec2(0.f, 40.0f));
                for (const int &cit_i : {0, 1, 2, 3, 4, 5, 10, 20, 50, 100}) {
                    if (citation_histogram_.size() > static_cast<size_t>(cit_i)) {
                        size_t sum = static_cast<size_t>(std::accumulate(citation_histogram_.begin() + cit_i, citation_histogram_.end(), 0.f));
                        ImGui::Text("%zu papers with %u or more citations", sum, cit_i);
                    } else {
                        break;
                    }
                }
                ImGui::End();
            }

            auto get_indicator = [](const auto &author, int indicator_idx, int year_begin, int year_end) {
                switch (indicator_idx) {
                    case 0:
                        return static_cast<float>(author.citations_per_author(year_begin,year_end));
                    case 1:
                        return static_cast<float>(author.citations(year_begin,year_end));
                    case 2:
                        return static_cast<float>(author.h_index(year_begin,year_end));
                    case 3:
                        return static_cast<float>(author.h_core(year_begin,year_end));
                    case 4:
                        return static_cast<float>(author.i10_index(year_begin,year_end));
                    case 5:
                        return static_cast<float>(author.impact_factor(year_begin,year_end));
                    case 6:
                        return static_cast<float>(author.number_of_papers(year_begin,year_end));
                    default:
                        return static_cast<float>(author.citations_per_author(year_begin,year_end));
                }
            };

            auto sort_author_ptrs = [&](std::vector<const author*>& author_ptrs, int indicator_current) {
                std::sort(author_ptrs.begin(),author_ptrs.end(), [&](const author* &a, const author*& b) {
                    return get_indicator(*a,indicator_current,year-indicator_time_window_size_+1,year) > get_indicator(*b,indicator_current,year-indicator_time_window_size_+1,year);
                });
            };


            const size_t n_authors = internal_group_.authors().size();

            if (show_indicators_window) {
                ImGui::Begin("Indicators", &show_indicators_window);

                static int indicator_current = 0;
                ImGui::Combo("Indicator", &indicator_current, indicators, IM_ARRAYSIZE(indicators));
                ImGui::SameLine();
                HelpMarker(fmt::format("See the \"summary\" tab for a brief explanation of indicators", indicator_time_window_size_).c_str());

                ImGui::DragInt("Time Window", &indicator_time_window_size_, 1, 1, max_time_window, "%d years");
                ImGui::SameLine();
                HelpMarker(fmt::format("Size of time window in years.\nIndicators will consider productions in the last {} years", indicator_time_window_size_).c_str());

                static ImPlotColormap map = ImPlotColormap_Viridis;
                if (ImGui::Button("Change Colormap", ImVec2(225, 0)))
                    map = (map + 1) % ImPlotColormap_COUNT;
                ImPlot::SetColormap(map);
                ImGui::SameLine();
                static const char *cmap_names[] = {"Default", "Dark", "Pastel", "Paired", "Viridis",
                                                   "Plasma", "Hot", "Cool", "Pink", "Jet"};
                ImGui::LabelText("##Colormap Index", "%s", cmap_names[map]);

                std::vector<const author*> author_ptrs_for_plots;
                for (const auto &author : internal_group_) {
                    author_ptrs_for_plots.emplace_back(&author);
                }
                sort_author_ptrs(author_ptrs_for_plots,indicator_current);

                ImGui::Separator();

                if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
                    if (ImGui::BeginTabItem("Timeline")) {
                        // Add save button: https://github.com/aiekick/ImGuiFileDialog / https://github.com/AirGuanZ/imgui-filebrowser
                        static std::vector<std::vector<float>> xs, ys;
                        xs.clear();
                        ys.clear();
                        double y_min = 0;
                        double y_max = 0;
                        for (const auto &author_ptr : author_ptrs_for_plots) {
                            const auto &author = *author_ptr;
                            std::vector<float> x, y;
                            for (int i = min_year; i <= year; ++i) {
                                x.emplace_back(static_cast<float>(i));
                                y.emplace_back(get_indicator(author,indicator_current,i - indicator_time_window_size_ + 1,i));
                                if (y.back() < y_min) {
                                    y_min = y.back();
                                }
                                if (y.back() > y_max) {
                                    y_max = y.back();
                                }
                            }
                            xs.emplace_back(x);
                            ys.emplace_back(y);
                        }

                        if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
                            if (ImGui::BeginTabItem("Line Plot")) {
                                static float line_weight_ = 2;
                                ImGui::DragFloat("Line Weight", &line_weight_, 1.f, 1.f, 100.f, "%.f units");
                                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, line_weight_);
                                ImPlot::SetNextPlotLimits(min_year, year, y_min, y_max, ImGuiCond_Always);
                                if (ImPlot::BeginPlot(indicators[indicator_current], "Year",
                                                      indicators[indicator_current], ImVec2(-1, -1))) {
                                    size_t i = 0;
                                    for (const auto &author_ptr : author_ptrs_for_plots) {
                                        const auto &author = *author_ptr;
                                        std::string name_and_ind;
                                        if (!ys[i].empty()) {
                                            name_and_ind = fmt::format("{}: {}", author.name(), ys[i].back());
                                        } else {
                                            name_and_ind = fmt::format("{}: {}", author.name(), 0.f);
                                        }
                                        ImPlot::PlotLine(name_and_ind.c_str(), xs[i].data(), ys[i].data(), static_cast<int>(xs[i].size()));
                                        ++i;
                                    }
                                    ImPlot::EndPlot();
                                }
                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem("Table")) {
                                const size_t n_cols = ys[0].size();
                                static std::vector<float> indicator_table(n_authors * n_cols);
                                indicator_table.resize(n_authors * n_cols);
                                static float table_min;
                                table_min = ys[0][0];
                                static float table_max;
                                table_max = ys[0][0];
                                for (size_t i = 0; i < n_authors; ++i) {
                                    for (size_t j = 0; j < n_cols; ++j) {
                                        indicator_table[i * n_cols + j] = ys[i][j];
                                        if (ys[i][j] > table_max) {
                                            table_max = ys[i][j];
                                        }
                                        if (ys[i][j] < table_min) {
                                            table_min = ys[i][j];
                                        }
                                    }
                                }
                                ImGui::SetNextItemWidth(225);
                                static ImPlotAxisFlags axes_flags =
                                        ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax | ImPlotAxisFlags_TickLabels;
                                std::vector<const char *> ylabels;
                                for (const auto &author_ptr : author_ptrs_for_plots) {
                                    const auto &author = *author_ptr;
                                    ylabels.emplace_back(author.name().c_str());
                                }
                                ImPlot::SetNextPlotTicksY(1. - 1.0 / n_authors / 2., 0. + 1.0 / n_authors / 2., static_cast<int>(n_authors), ylabels.data());
                                std::vector<std::string> years_labels;
                                for (const auto &label_year : xs[0]) {
                                    years_labels.emplace_back(fmt::format("{:0>2d}", static_cast<int>(label_year) % 100));
                                }
                                std::vector<const char *> xlabels;
                                for (const auto &year_label : years_labels) {
                                    xlabels.emplace_back(year_label.c_str());
                                }
                                ImPlot::SetNextPlotTicksX(0. + 1.0 / xs[0].size() / 2., 1. - 1.0 / xs[0].size() / 2., static_cast<int>(xs[0].size()), xlabels.data());
                                if (ImPlot::BeginPlot("##HeatmapTimeline", "Year", "Author", ImVec2(-1, -1), 0, axes_flags, axes_flags)) {
                                    ImPlot::PlotHeatmap("heat", indicator_table.data(), static_cast<int>(n_authors), static_cast<int>(n_cols), table_min, table_max, "%.f");
                                    ImPlot::EndPlot();
                                }
                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem("Stability")) {
                                ImGui::Text("%s", fmt::format("How much can indicators from the past predict {} in the next {} years?",indicators[indicator_current], indicator_time_window_size_).c_str());
                                // prediction window needs room for data that comes in the next years
                                int max_prediction_time_window = max_time_window - indicator_time_window_size_;
                                static std::vector<std::vector<float>> table(n_indicators, std::vector<float>(max_prediction_time_window, 0.f));

                                static size_t papers_fetched_when_last_preprocessed_this{0};
                                static auto last_calculation = std::chrono::high_resolution_clock::now() - std::chrono::seconds(5);
                                static int last_indicator_queried = 10000;
                                static int last_time_window_queried = 10000;

                                bool data_has_changed = papers_fetched_when_last_preprocessed_this < papers_fetched_;

                                auto current_time = std::chrono::high_resolution_clock::now();
                                bool it_has_been_a_while = (current_time - last_calculation) > std::chrono::seconds(5);

                                bool indicator_changed = last_indicator_queried != indicator_current;
                                bool time_window_changed = last_time_window_queried != indicator_time_window_size_;

                                if ((data_has_changed && it_has_been_a_while) || (indicator_changed || time_window_changed)) {
                                    // future work, calculate this on another thread
                                    papers_fetched_when_last_preprocessed_this = papers_fetched_;
                                    last_calculation = current_time;
                                    last_indicator_queried = indicator_current;
                                    last_time_window_queried = indicator_time_window_size_;

                                    table = std::vector<std::vector<float>>(n_indicators, std::vector<float>(max_prediction_time_window));
                                    for (int i = 0; i < static_cast<int>(n_indicators); ++i) {
                                        for (int j = 0; j < max_prediction_time_window; ++j) {
                                            std::vector<float> correlations;
                                            // for each possible window of that size
                                            for (int k = min_year; k < min_year + max_prediction_time_window; ++k) {
                                                // calculate indicator i for all authors for all time windows of size j
                                                std::vector<float> x;
                                                // indicator indicator_current for all authors for all time windows of size j
                                                std::vector<float> y;
                                                for (const auto &author : internal_group_) {
                                                    x.emplace_back(get_indicator(author, i, k, k + j));
                                                    y.emplace_back(get_indicator(author, indicator_current, k + j + 1, k + j + indicator_time_window_size_));
                                                }
                                                // we don't want to correlate periods when nothing happened
                                                auto is_zero = [](auto x) { return x == 0; };
                                                bool periods_have_data = !std::all_of(x.begin(), x.end(), is_zero) && !std::all_of(y.begin(), y.end(), is_zero);
                                                if (periods_have_data) {
                                                    // calculate correlation between these indicators
                                                    correlations.emplace_back(spearman(x, y));
                                                }
                                            }
                                            // table <- avg correlation
                                            bool there_are_some_valid_correlations = !correlations.empty();
                                            table[i][j] = there_are_some_valid_correlations ? static_cast<float>(mean(correlations)) * 100.f : 0.f;
                                        }
                                    }
                                }
                                ImPlot::SetNextPlotTicksY(1 - 1.0 / n_indicators / 2, 0 + 1.0 / n_indicators / 2, n_indicators, indicators);
                                std::vector<std::string> years_labels;
                                for (int j = 1; j <= max_prediction_time_window; ++j) {
                                    years_labels.emplace_back(fmt::format("{:0>2d}", static_cast<int>(j) % 100));
                                }
                                std::vector<const char *> xlabels;
                                xlabels.reserve(years_labels.size());
                                for (const auto &year_label : years_labels) {
                                    xlabels.emplace_back(year_label.c_str());
                                }
                                if (xlabels.size() > 1) {
                                    ImPlot::SetNextPlotTicksX(0. + 1.0 / years_labels.size() / 2., 1. - 1.0 / years_labels.size() / 2., static_cast<int>(years_labels.size()), xlabels.data());
                                }
                                static ImPlotAxisFlags axes_flags =
                                        ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax | ImPlotAxisFlags_TickLabels;
                                if (ImPlot::BeginPlot("##HeatmapStability", "Number of Previous Years Used for Prediction", "Indicator used for Prediction", ImVec2(-1, -1), 0, axes_flags, axes_flags)) {
                                    auto table_f = flatten(table);
                                    ImPlot::PlotHeatmap("heat", table_f.data(), n_indicators, max_prediction_time_window, 0, 100, "%.f%%");
                                    ImPlot::EndPlot();
                                }
                                ImGui::EndTabItem();
                            }

                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Compare")) {
                        if (internal_group_.authors().size() > 1) {
                            // row -> indicator
                            // col -> author
                            static std::vector<float> indicator_map(n_authors * n_indicators);
                            indicator_map.resize(n_authors * n_indicators);
                            auto it = author_ptrs_for_plots.begin();
                            for (int i = 0; i < static_cast<int>(internal_group_.authors().size()); ++i) {
                                const auto &author = **it;
                                for (int j = 0; j < static_cast<int>(n_indicators); ++j) {
                                    indicator_map[i * n_indicators + j] = get_indicator(author, j,year - indicator_time_window_size_ + 1,year);
                                }
                                ++it;
                            }
                            static float scale_min = 0;
                            static float scale_max = 100.f;
                            // normalize heatmap
                            for (size_t j = 0; j < n_indicators; ++j) {
                                float col_min = indicator_map[0 * n_indicators + j];
                                float col_max = indicator_map[0 * n_indicators + j];
                                for (size_t i = 0; i < n_authors; ++i) {
                                    if (indicator_map[i * n_indicators + j] > col_max) {
                                        col_max = indicator_map[i * n_indicators + j];
                                    }
                                    if (indicator_map[i * n_indicators + j] < col_min) {
                                        col_min = indicator_map[i * n_indicators + j];
                                    }
                                }
                                float col_range = col_max - col_min;
                                for (size_t i = 0; i < n_authors; ++i) {
                                    if (indicator_map[i * n_indicators + j] != 0.) {
                                        indicator_map[i * n_indicators + j] -= col_min;
                                        if (col_range != 0.) {
                                            indicator_map[i * n_indicators + j] /= col_range;
                                        }
                                        indicator_map[i * n_indicators + j] *= scale_max;
                                    }
                                }
                            }
                            ImGui::SetNextItemWidth(225);
                            static ImPlotAxisFlags axes_flags =
                                    ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax | ImPlotAxisFlags_TickLabels;
                            std::vector<const char *> ylabels;
                            for (const auto &author_ptr : author_ptrs_for_plots) {
                                const auto &author = *author_ptr;
                                ylabels.emplace_back(author.name().c_str());
                            }
                            ImPlot::SetNextPlotTicksX(0 + 1.0 / n_indicators / 2, 1 - 1.0 / n_indicators / 2,
                                                      n_indicators, indicators);
                            ImPlot::SetNextPlotTicksY(1 - 1.0 / n_authors / 2, 0 + 1.0 / n_authors / 2, static_cast<int>(n_authors),
                                                      ylabels.data());
                            if (ImPlot::BeginPlot("##Heatmap1", NULL, NULL, ImVec2(-1, -1), 0, axes_flags, axes_flags)) {
                                ImPlot::PlotHeatmap("heat", indicator_map.data(), static_cast<int>(n_authors), static_cast<int>(n_indicators), scale_min, scale_max, "%.1f%%");
                                ImPlot::EndPlot();
                            }
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Correlations")) {
                        constexpr size_t n_coefficients = 2;
                        const char* coefficients[] = { "Pearson - Parametric", "Spearman - Non-Parametric" };
                        static int coefficient = 0;
                        auto get_coefficient = [](const std::vector<float>& x, const std::vector<float>& y, int coefficient_index) {
                            if (coefficient_index == 0) {
                                return pearson<float>(x, y);
                            } else {
                                return spearman<float>(x, y);
                            }
                        };
                        ImGui::Combo("Coefficient", &coefficient, coefficients, n_coefficients);
                        if (ImGui::TreeNode("How do I interpret these correlations?")) {
                            ImGui::Text("Correlations help us identify how much value an indicator provides when compared to other indicators.");
                            ImGui::Text("Note that, because normalizations ultimately rely on the individual papers, one indicator is always a necessary condition for the next.\nThus, correlations tend to almost never be too low.");
                            ImGui::Text("In other words: ");
                            ImGui::BulletText("High correlations are always expected");
                            ImGui::BulletText("Median correlations indicate a problem");
                            ImGui::BulletText("Low correlations probably indicate a huge problem");
                            ImGui::Text("Also, note that correlations consider all authors. Thus:");
                            ImGui::BulletText("High correlations do not mean there is no problem with any individual author.");
                            ImGui::BulletText("High correlations do not mean more accurate indicators are not worth considering.");
                            ImGui::BulletText("On the other hand, low correlations do indicate a generalized problem in this group of authors.");
                            ImGui::TreePop();
                        }
                        std::vector<std::vector<float>> indicator_table(n_indicators);
                        int year_begin = year - indicator_time_window_size_ + 1;
                        for (int i = 0; i < static_cast<int>(n_indicators); ++i) {
                            for (const auto &author : internal_group_) {
                                indicator_table[i].emplace_back(get_indicator(author, i, year_begin, year));
                            }
                        }
                        // correlations have an extra row for averages
                        std::vector<std::vector<float>> correlations(n_indicators_with_avg, std::vector<float>(n_indicators));
                        for (size_t i = 0; i < n_indicators; ++i) {
                            for (size_t j = 0; j < n_indicators; ++j) {
                                correlations[i][j] = get_coefficient(indicator_table[i],indicator_table[j],coefficient) * 100;
                            }
                        }
                        for (size_t j = 0; j < n_indicators; ++j) {
                            for (size_t i = 0; i < n_indicators; ++i) {
                                correlations.back()[j] += correlations[i][j];
                            }
                            correlations.back()[j] /= n_indicators;
                        }
                        auto flat_correlations = flatten(correlations);
                        float correlation_min = *std::min_element(flat_correlations.begin(),flat_correlations.end());
                        float correlation_max = *std::max_element(flat_correlations.begin(),flat_correlations.end());
                        ImPlot::SetNextPlotTicksX(0 + 1.0 / n_indicators / 2, 1 - 1.0 / n_indicators / 2,
                                                  n_indicators, indicators);
                        ImPlot::SetNextPlotTicksY(1 - 1.0 / n_indicators_with_avg / 2, 0 + 1.0 / n_indicators_with_avg / 2, n_indicators_with_avg, indicators_with_avg);
                        static ImPlotAxisFlags axes_flags =
                                ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax | ImPlotAxisFlags_TickLabels;
                        if (ImPlot::BeginPlot("##Heatmap2", NULL, NULL, ImVec2(-1, -1), 0, axes_flags, axes_flags)) {
                            ImPlot::PlotHeatmap("Indicator Correlations", flat_correlations.data(), n_indicators_with_avg, n_indicators, correlation_min, correlation_max, "%.0f%%");
                            ImPlot::EndPlot();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Citations")) {
                        static bool horz = false;
                        static bool aggregate = true;
                        ImGui::Checkbox("All Authors",&aggregate);
                        ImGui::SameLine();
                        ImGui::Checkbox("Invert Axes",&horz);
                        double y_max = 0.;
                        double x_max = 0.;
                        int year_begin = year - indicator_time_window_size_ + 1;
                        for (const auto &author : internal_group_) {
                            auto l = author.citations_list(year_begin, year);
                            l.erase(std::remove(l.begin(), l.end(), 0),l.end());
                            x_max = std::max(x_max, double(l.size()));
                            auto it = std::max_element(l.begin(), l.end());
                            if (it != l.end()) {
                                if (aggregate) {
                                    y_max += double(*it);
                                } else {
                                    y_max = std::max(y_max, double(*it));
                                }
                            }
                        }
                        if (horz) {
                            ImPlot::SetNextPlotLimits(0., y_max, 0. + 0.5, x_max + 0.5, ImGuiCond_Always);
                            if (int(x_max) > 1) {
                                ImPlot::SetNextPlotTicksY(double(1.),double(x_max),int(x_max));
                            }
                        } else {
                            ImPlot::SetNextPlotLimits(0. + 0.5, x_max + 0.5, 0., y_max, ImGuiCond_Always);
                            if (int(x_max) > 1) {
                                ImPlot::SetNextPlotTicksX(double(1.),double(x_max),int(x_max));
                            }
                        }
                        if (ImPlot::BeginPlot("Citation Histogram", horz ? "Citations":  "Number of Papers", horz ? "Number of Papers" : "Citations", ImVec2(-1,-1), 0)) {
                           auto it = author_ptrs_for_plots.begin();
                            std::vector<double> agg_citations;
                            std::vector<double> agg_paper_rank;
                            for (size_t i = 0; i < n_authors; ++i) {
                                const auto &author = **it;
                                std::vector<double> citations;
                                std::vector<double> paper_rank;
                                size_t rank = 1;
                                for (const auto &paper : author) {
                                    if (paper.year() >= year_begin && paper.year() <= year) {
                                        citations.emplace_back(paper.citations());
                                        paper_rank.emplace_back(static_cast<double>(rank));
                                        ++rank;
                                    }
                                }
                                std::sort(citations.begin(), citations.end(), std::greater<>());
                                if (!aggregate) {
                                    if (horz) {
                                        ImPlot::PlotBarsH(author.name().c_str(), citations.data(), paper_rank.data(), static_cast<int>(citations.size()), 1.f);
                                    } else {
                                        ImPlot::PlotBars(author.name().c_str(), paper_rank.data(), citations.data(), static_cast<int>(citations.size()), 1.f);
                                    }
                                } else {
                                    agg_citations.resize(std::max(agg_citations.size(), citations.size()));
                                    for (size_t j = 0; j < citations.size(); ++j) {
                                        agg_citations[j] += citations[j];
                                    }
                                    agg_paper_rank.resize(std::max(agg_paper_rank.size(), paper_rank.size()));
                                }
                                ++it;
                            }
                            if (aggregate) {
                                std::iota(agg_paper_rank.begin(), agg_paper_rank.end(), 1.f);
                                size_t h_core = 0;
                                for (size_t i = 0; i < agg_citations.size(); ++i) {
                                    if (agg_citations[i] > i) {
                                        ++h_core;
                                    } else {
                                        break;
                                    }
                                }
                                if (horz) {
                                    ImPlot::PlotBarsH("All authors", agg_citations.data(), agg_paper_rank.data(),
                                                      static_cast<int>(agg_citations.size()), 1.f);
                                    ImVec2 rmin = ImPlot::PlotToPixels(ImPlotPoint(0.0f, 0.5f));
                                    ImVec2 rmax = ImPlot::PlotToPixels(ImPlotPoint(float(h_core), float(h_core)+0.5));
                                    ImGui::GetWindowDrawList()->AddRect(rmin, rmax, IM_COL32(128,0,255,255), 0.f, ImDrawCornerFlags_All, 5.f);
                                    ImPlot::PlotText(fmt::format("H-Core:\n{} papers", h_core).c_str(), float(h_core)/2 - 0.5f, float(h_core)/2 + .5f);
                                } else {
                                    ImPlot::PlotBars("All authors", agg_paper_rank.data(), agg_citations.data(),
                                                     static_cast<int>(agg_citations.size()), 1.f);
                                    ImVec2 rmin = ImPlot::PlotToPixels(ImPlotPoint(0.5f, 0.0f));
                                    ImVec2 rmax = ImPlot::PlotToPixels(ImPlotPoint(float(h_core)+0.5, float(h_core)));
                                    ImGui::GetWindowDrawList()->AddRect(rmin, rmax, IM_COL32(128,0,255,255), 0.f, ImDrawCornerFlags_All, 5.f);
                                    ImPlot::PlotText(fmt::format("H-Core: {} papers", h_core).c_str(), float(h_core)/2, y_max > h_core * 30 ? float(h_core)-float(h_core)/4 : float(h_core)/2);
                                }
                            }
                            ImPlot::EndPlot();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Authors")) {
                        std::vector<const author *> author_ptrs;
                        for (const auto &author : internal_group_) {
                            author_ptrs.emplace_back(&author);
                        }
                        sort_author_ptrs(author_ptrs, sort_authors_by_current);
                        for (const auto &author_ptr : author_ptrs) {
                            const auto &author = *author_ptr;
                            if (ImGui::TreeNode(author.name().c_str())) {
                                ImGui::Text("ID: %s", author.id().c_str());
                                ImGui::Text("Name: %s", author.name().c_str());
                                if (ImGui::TreeNode("Indicators")) {
                                    ImGui::Columns(3, "Indicator Summary");
                                    ImGui::Separator();
                                    ImGui::Separator();
                                    int begin_year = year - indicator_time_window_size_ + 1;
                                    ImGui::Text("Time Window");
                                    ImGui::NextColumn();
                                    ImGui::Text("%d-%d", begin_year, year);
                                    ImGui::NextColumn();
                                    ImGui::Text("%d-%d", min_year, year);
                                    ImGui::NextColumn();
                                    ImGui::Separator();
                                    ImGui::Separator();
                                    ImGui::Text("Citations/Author");
                                    ImGui::SameLine();
                                    HelpMarker("Citations/Author help us mitigate \"honorary authors\"");
                                    ImGui::NextColumn();
                                    ImGui::Text("%.2f", author.citations_per_author(begin_year, year));
                                    ImGui::NextColumn();
                                    ImGui::Text("%.2f", author.citations_per_author());
                                    ImGui::NextColumn();
                                    ImGui::Separator();
                                    ImGui::Separator();
                                    ImGui::Text("Citations");
                                    ImGui::SameLine();
                                    HelpMarker(
                                            "Total citations can be inflated with \"honorary authors\". It is useful to indicate capacity to collaborate with other researcher to achieve high-impact publications.");
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.citations(begin_year, year));
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.citations());
                                    ImGui::NextColumn();
                                    ImGui::Separator();

                                    ImGui::Text("H-Index");
                                    ImGui::SameLine();
                                    HelpMarker(
                                            "The H-Index is a low-resolution indicator that does not prevent \"honorary authors\" and can be inflated with medianly cited publications.");
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.h_index(begin_year, year));
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.h_index());
                                    ImGui::NextColumn();
                                    ImGui::Separator();

                                    ImGui::Text("H-Core");
                                    ImGui::SameLine();
                                    HelpMarker("The H-Core has better resolution than the H-Index.");
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.h_core(begin_year, year));
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.h_core());
                                    ImGui::NextColumn();
                                    ImGui::Separator();

                                    ImGui::Text("i10-Index");
                                    ImGui::SameLine();
                                    HelpMarker("The i10-Index is the number of papers with at least 10 citations.");
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.i10_index(begin_year, year));
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.i10_index());
                                    ImGui::NextColumn();
                                    ImGui::Separator();
                                    ImGui::Separator();

                                    ImGui::Text("Impact Factor");
                                    ImGui::SameLine();
                                    HelpMarker(
                                            "Impact Factor is not a productivity indicator. It is only useful to understand on what sort of paper this researcher tends to focus. The impact factor can be inflated by restricting the number of publications.");
                                    ImGui::NextColumn();
                                    ImGui::Text("%.2f", author.impact_factor(begin_year, year));
                                    ImGui::NextColumn();
                                    ImGui::Text("%.2f", author.impact_factor());
                                    ImGui::NextColumn();
                                    ImGui::Separator();
                                    ImGui::Separator();

                                    ImGui::Text("Number of Papers");
                                    ImGui::SameLine();
                                    HelpMarker(
                                            "Number of Papers is neither a quality nor a productivity indicator! It's ONLY useful to indicate whether a researcher is active.");
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.number_of_papers(begin_year, year));
                                    ImGui::NextColumn();
                                    ImGui::Text("%u", author.number_of_papers());
                                    ImGui::NextColumn();
                                    ImGui::Columns(1);
                                    ImGui::Separator();

                                    ImGui::TreePop();
                                }
                                if (ImGui::TreeNode("Papers", "Papers (%u)", author.number_of_papers())) {
                                    auto papers = author.papers();
                                    std::sort(papers.begin(), papers.end(), [](const auto &a, const auto &b) {
                                        return a.citations() > b.citations();
                                    });
                                    for (const auto &paper : papers) {
                                        if (ImGui::TreeNode(paper.title().c_str())) {
                                            ImGui::Text("ID: %s", paper.id().c_str());
                                            ImGui::Text("%zu authors: %s", paper.authors().size(),
                                                        paper.authors_string().c_str());
                                            ImGui::Text("Title: %s", paper.title().c_str());
                                            ImGui::Text("Journal: %s", paper.journal().c_str());
                                            ImGui::Text("Year: %d", paper.year());
                                            ImGui::Text("Citations: %d", paper.citations());
                                            ImGui::TreePop();
                                        }
                                    }
                                    ImGui::TreePop();
                                }
                                ImGui::TreePop();
                            }
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Summary")) {
                        const char *citation_weight_help = "Weighting production by citations is intended to value high impact publications.\nHowever, the number of citations is only a proxy for impact.\nOne has to judge whether these citations reflect the missions of the institution.\nIf not normalized, the number of citations can also be influenced by honorary authorships and self-citations\n";
                        const char *constant_weight_help = "A constant weight values quantity over quality\nThe only point of these indicators is to identify if the researcher is active";
                        const char *hcore_help = "The H-Core is the H papers with at least H citations.\nThis is intended to disregard papers with imprecise data and avoid the superstar effect. Both constraints are debatable.\nThe appropriate proportion between papers and citations is also an arbitrary factor that gives the indicator low resolution and can value median publications.\nIf not normalized, the H-Core might be very sensitive to self-citations\nYou can graphically see the total H-Core in the \"Citations\" tab";
                        const char *i10_constraint_help = "This constraint disregards papers with less than 10 citations.\nThis is intended to ignore papers with imprecise data without penalizing superstars.";
                        const char *number_of_paper_norm_help = "Normalizing for the number of papers is not appropriate to measure productivity.\nThis normalization intended to provide an overview of what kind of paper the author tends to publish.";

                        ImGui::Columns(4);
                        ImGui::Separator();
                        ImGui::Text("Indicator"); ImGui::NextColumn();
                        ImGui::Text("Paper Weight"); ImGui::SameLine();
                        HelpOnWidget("The paper weight is the main resource to choose what kind of phenomenon we want to measure"); ImGui::NextColumn();
                        ImGui::Text("Constraint"); ImGui::SameLine();
                        HelpOnWidget(fmt::format("All indicators are constrained to an adjustable time window of {} years",indicator_time_window_size_).c_str()); ImGui::NextColumn();
                        ImGui::Text("Normalization"); ImGui::SameLine();
                        HelpOnWidget(fmt::format("Normalization might \n1) avoid tricks to inflate indicators (such as Honorary Authorship) or \n2) change the meaning of indicators (like from total to average indicators)",indicator_time_window_size_).c_str()); ImGui::NextColumn();
                        ImGui::Separator();
                        ImGui::Separator();

                        ImGui::Text("Citations/Author"); ImGui::SameLine();
                        HelpOnWidget("Also called the \"Total Impact Factor\""); ImGui::NextColumn();
                        ImGui::Text("Citations"); ImGui::SameLine();
                        HelpOnWidget(citation_weight_help); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Text("Number of Authors"); ImGui::SameLine();
                        HelpOnWidget("This normalization mitigates \"Honorary Authors\""); ImGui::NextColumn();
                        ImGui::Separator();

                        ImGui::Text("Citations"); ImGui::SameLine();
                        HelpOnWidget("Total number of citations an author has"); ImGui::NextColumn();
                        ImGui::Text("Citations"); ImGui::SameLine();
                        HelpOnWidget(citation_weight_help); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Separator();

                        ImGui::Text("H-Index"); ImGui::SameLine();
                        HelpOnWidget("Number of H papers of at least H citations"); ImGui::NextColumn();
                        ImGui::Text("Constant"); ImGui::SameLine();
                        HelpOnWidget(constant_weight_help); ImGui::NextColumn();
                        ImGui::Text("H-Core"); ImGui::SameLine();
                        HelpOnWidget(hcore_help); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Separator();

                        ImGui::Text("H-Core"); ImGui::SameLine();
                        HelpOnWidget("Number of citations of the H most cited papers with at least H citations"); ImGui::NextColumn();
                        ImGui::Text("Citations"); ImGui::SameLine();
                        HelpOnWidget(citation_weight_help); ImGui::NextColumn();
                        ImGui::Text("H-Core"); ImGui::SameLine();
                        HelpOnWidget(hcore_help); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Separator();

                        ImGui::Text("i10-Core"); ImGui::SameLine();
                        HelpOnWidget("Number of papers with at least 10 citations"); ImGui::NextColumn();
                        ImGui::Text("Constant"); ImGui::SameLine();
                        HelpOnWidget(constant_weight_help); ImGui::NextColumn();
                        ImGui::Text("10 or more citations"); ImGui::SameLine();
                        HelpOnWidget(i10_constraint_help);
                        ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Separator();

                        ImGui::Text("Impact Factor"); ImGui::SameLine();
                        HelpOnWidget("Average number of citations in a paper"); ImGui::NextColumn();
                        ImGui::Text("Citations"); ImGui::SameLine();
                        HelpOnWidget(citation_weight_help); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Text("Number of Papers"); ImGui::SameLine();
                        HelpOnWidget(number_of_paper_norm_help); ImGui::NextColumn();
                        ImGui::Separator();

                        ImGui::Text("Number of Papers"); ImGui::SameLine();
                        HelpOnWidget("Total number of papers. A necessary condition for all other indicators."); ImGui::NextColumn();
                        ImGui::Text("Constant"); ImGui::SameLine();
                        HelpOnWidget(constant_weight_help); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Text("-"); ImGui::NextColumn();
                        ImGui::Separator();
                        ImGui::Columns(1);

                        ImGui::Text("Hover over the cells for a brief description");

                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }
                ImGui::End();
            }

            ImGui::Render();
            glfwGetFramebufferSize(window_, &display_w_, &display_h_);
            glViewport(0, 0, display_w_, display_h_);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window_);
        }
    }

    std::string application::curl_request_while_render(const std::string &url, const std::vector<std::string>& extra_headers, long timeout = 10) {
        auto download_task = std::async([&]() {
            return curl_request(url, extra_headers, timeout);
        });
        if (gui_ && window_) {
            while (!is_ready(download_task) && !glfwWindowShouldClose(window_)) {
                render_gui();
            }
        }
        return download_task.get();
    }

    void application::fetch_microsoft(const fs::path& ms_cache_dir, const author& author, paper& paper) {
        // Url search format
        // https://docs.microsoft.com/en-us/academic-services/project-academic-knowledge/reference-paper-entity-attributes
        constexpr int entity_type = 0; // entity = paper
        std::string url = fmt::format("https://api.labs.cognitive.microsoft.com/academic/v1.0/evaluate?expr=And(Composite(AA.AuN=='{}'),Y=[{},{}],Ti='{}',Ty='{}')&model=latest&count=10&offset=0&attributes=Id,Ty,Ti,Y,CC,CitCon,ECC,AA.AfId,AA.AfN,AA.AuId,AA.AuN,AA.DAuN,AA.DAfN,AA.S,AW,BT,C.CId,C.CN,D,DN,DOI,F.DFN,F.FId,F.FN,FamId,FP,I,J.JId,J.JN,LP,PB,Pt,RId,S,Ti,V,VFN,VSN,W,Y",curlpp::escape(lowercase(author.name())), paper.year() - 1, paper.year() + 1,curlpp::escape(lowercase(paper.title())),entity_type);

        // Cache path
        fs::path ms_paper_path = ms_cache_dir;
        ms_paper_path /= std::to_string(std::hash<std::string>()(url));
        ms_paper_path += ".json";

        std::string contents;
        bool get_contents_from_cache = fs::exists(ms_paper_path);
        if (get_contents_from_cache) {
            log->debug("Looking for paper from cache: {}", ms_paper_path.string());
            contents = fileread(ms_paper_path.string());
        } else if (!microsoft_key_.empty()) {
            log->debug("Getting data from url: {}", url);
            try {
                contents = curl_request_while_render(url, {fmt::format("Ocp-Apim-Subscription-Key: {}",microsoft_key_)});
            } catch (const std::exception& e) {
                log->warn(e.what());
                return;
            }
            filesave(contents, ms_paper_path.string());
        }
        std::string contents_tmp = contents;
        replace_new_line(contents_tmp);
        log->trace("Contents: {}", contents_tmp);

        nlohmann::json r = nlohmann::json::parse(contents);
        log->trace("Json from microsoft: {}", r.dump());
        auto it = r.find("entities");
        if (it != r.end()) {
            nlohmann::json entities = it.value();
            if (entities.empty()) {
                log->debug("No entities found for {}", std::quoted(paper.title()));
            } else {
                log->info("{} entities found for {}", entities.size(), std::quoted(paper.title()));
                auto it2 = entities[0].find("CC");
                if (it2 != entities[0].end()) {
                    if (it2.value().is_number()) {
                        const auto citations = it2.value().get<int>();
                        log->info("This paper has {} citations", citations);
                        paper.citations(std::max(citations, paper.citations()));
                    }
                } else {
                    log->info("Cannot find the field \"CC\" for this paper");
                }
            }
        }
    }

    author application::parse_lattes(const fs::path &filepath) {
        log->debug("Parsing lattes file {}", filepath);
        author res;

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(filepath.string().data());
        if (!result) {
            log->critical("Invalid XML file {}", filepath.filename());
            return res;
        }
        log->debug("Reading XML file {}", filepath);

        // save id
        pugi::xpath_node curriculo_data_xpath = doc.select_node("/CURRICULO-VITAE");
        pugi::xml_node curriculo_data = curriculo_data_xpath.node();
        res.id(curriculo_data.attribute("NUMERO-IDENTIFICADOR").as_string());

        // save name
        pugi::xpath_node author_main_data_xpath = doc.select_node("/CURRICULO-VITAE/DADOS-GERAIS");
        pugi::xml_node author_main_data = author_main_data_xpath.node();
        res.name(author_main_data.attribute("NOME-COMPLETO").as_string());

        // save papers
        pugi::xpath_node_set journal_papers_xpath = doc.select_nodes(
                "/CURRICULO-VITAE/PRODUCAO-BIBLIOGRAFICA/ARTIGOS-PUBLICADOS/ARTIGO-PUBLICADO");
        for (const auto &journal_paper_xpath : journal_papers_xpath) {
            pugi::xml_node journal_paper = journal_paper_xpath.node();
            pugi::xpath_node paper_data_xpath = journal_paper.select_node("DADOS-BASICOS-DO-ARTIGO");
            pugi::xml_node paper_data = paper_data_xpath.node();

            paper p;
            p.title(paper_data.attribute("TITULO-DO-ARTIGO").as_string());
            p.year(paper_data.attribute("ANO-DO-ARTIGO").as_int());

            pugi::xpath_node paper_details_xpath = journal_paper.select_node("DETALHAMENTO-DO-ARTIGO");
            pugi::xml_node paper_details = paper_details_xpath.node();
            p.journal(paper_details.attribute("TITULO-DO-PERIODICO-OU-REVISTA").as_string());

            pugi::xpath_node_set authors_xpath = journal_paper.select_nodes("AUTORES");
            for (const auto &author_xpath : authors_xpath) {
                pugi::xml_node author = author_xpath.node();
                p.emplace_author(author.attribute("NOME-COMPLETO-DO-AUTOR").as_string());
            }

            res.add_paper(p);
        }

        pugi::xpath_node_set book_papers_xpath = doc.select_nodes(
                "/CURRICULO-VITAE/PRODUCAO-BIBLIOGRAFICA/LIVROS-E-CAPITULOS/CAPITULOS-DE-LIVROS-PUBLICADOS/CAPITULO-DE-LIVRO-PUBLICADO");
        for (const auto &book_paper_xpath : book_papers_xpath) {
            pugi::xml_node book_paper = book_paper_xpath.node();
            pugi::xpath_node paper_data_xpath = book_paper.select_node("DADOS-BASICOS-DO-CAPITULO");
            pugi::xml_node paper_data = paper_data_xpath.node();
            paper p;
            p.title(paper_data.attribute("TITULO-DO-CAPITULO-DO-LIVRO").as_string());
            p.year(paper_data.attribute("ANO").as_int());

            pugi::xpath_node paper_details_xpath = book_paper.select_node("DETALHAMENTO-DO-CAPITULO");
            pugi::xml_node paper_details = paper_details_xpath.node();
            p.journal(paper_details.attribute("TITULO-DO-LIVRO").as_string());

            pugi::xpath_node_set authors_xpath = book_paper.select_nodes("AUTORES");
            for (const auto &author_xpath : authors_xpath) {
                pugi::xml_node author = author_xpath.node();
                p.emplace_author(author.attribute("NOME-COMPLETO-DO-AUTOR").as_string());
            }

            res.add_paper(p);
        }

        pugi::xpath_node_set conference_papers_xpath = doc.select_nodes(
                "/CURRICULO-VITAE/PRODUCAO-BIBLIOGRAFICA/TRABALHOS-EM-EVENTOS/TRABALHO-EM-EVENTOS");
        for (const auto &conference_paper_xpath : conference_papers_xpath) {
            pugi::xml_node conference_paper = conference_paper_xpath.node();
            pugi::xpath_node paper_data_xpath = conference_paper.select_node("DADOS-BASICOS-DO-TRABALHO");
            pugi::xml_node paper_data = paper_data_xpath.node();
            paper p;
            p.title(paper_data.attribute("TITULO-DO-TRABALHO").as_string());
            p.year(paper_data.attribute("ANO-DO-TRABALHO").as_int());

            pugi::xpath_node paper_details_xpath = conference_paper.select_node("DETALHAMENTO-DO-TRABALHO");
            pugi::xml_node paper_details = paper_details_xpath.node();
            p.journal(paper_details.attribute("NOME-DO-EVENTO").as_string());

            pugi::xpath_node_set authors_xpath = conference_paper.select_nodes("AUTORES");
            for (const auto &author_xpath : authors_xpath) {
                pugi::xml_node author = author_xpath.node();
                p.emplace_author(author.attribute("NOME-COMPLETO-DO-AUTOR").as_string());
            }

            res.add_paper(p);
        }
        return res;
    }

    author application::parse_csv(const fs::path &filepath) {
        log->debug("Parsing CSV file {}", filepath);
        author res;

        std::string id;
        std::string name;
        csv::CSVReader reader(filepath.string());
        for (csv::CSVRow& row: reader) {
            // get author information and make sure it's consistent
            std::string id_curr_row = row["Author ID"].get<std::string>();
            std::string name_curr_row = row["Author"].get<std::string>();
            if (!id.empty() && id != id_curr_row) {
                log->error("File {1} includes two different author IDs. This is not supported. IDs {2} and {3}. Skipping {3}.", filepath.filename(), id, id_curr_row);
                continue;
            } else if (id != id_curr_row) {
                id = id_curr_row;
                res.id(id);
            }
            if (!name.empty() && name != name_curr_row) {
                log->error("File {1} includes two different author names. This is not supported. Names {2} and {3}. Skipping {3}.", filepath.filename(), name, name_curr_row);
                continue;
            } else if (name != name_curr_row) {
                name = name_curr_row;
                res.name(name);
            }
            // get paper information
            paper p;
            p.id(row["Paper ID"].get<std::string>());
            p.title(row["Title"].get<std::string>());
            p.journal(row["Journal"].get<std::string>());
            p.year(row["Year"].get<int>());
            p.authors(explode(row["Authors"].get<std::string>(), ','));
            p.citations(row["Citations"].get<int>());
            res.add_paper(p);
        }

        return res;
    }

    author application::parse_json(const fs::path &filepath) {
        log->debug("Parsing JSON file {}", filepath);
        author res;

        std::ifstream i(filepath);
        nlohmann::json reader;
        i >> reader;

        auto& j_id = reader["Author ID"];
        if (j_id.is_string()) {
            res.id(j_id.get<std::string>());
        } else if (j_id.is_number()) {
            res.id(std::to_string(j_id.get<int>()));
        }

        res.name(reader["Author"].get<std::string>());

        for (const auto& json_paper: reader["Papers"]) {
            // get paper information
            paper p;
            auto& pj_id = reader["Paper ID"];
            if (pj_id.is_string()) {
                res.id(pj_id.get<std::string>());
            } else if (pj_id.is_number()) {
                res.id(std::to_string(pj_id.get<int>()));
            }
            p.title(json_paper["Title"].get<std::string>());
            p.journal(json_paper["Journal"].get<std::string>());
            p.year(json_paper["Year"].get<int>());
            p.authors(json_paper["Authors"].get<std::vector<std::string>>());
            p.citations(json_paper["Citations"].get<int>());
            res.add_paper(p);
        }

        return res;
    }

    const std::string &application::microsoft_key() const {
        return microsoft_key_;
    }

    void application::microsoft_key(const std::string &microsoft_key) {
        microsoft_key_ = microsoft_key;
    }

    const std::string &application::elsevier_key() const {
        return elsevier_key_;
    }

    void application::elsevier_key(const std::string &elsevier_key) {
        elsevier_key_ = elsevier_key;
    }

    application::~application() {
        while (!glfwWindowShouldClose(window_) && persistent_window_) {
            render_gui();
        }
        if (window_ != nullptr) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            glfwDestroyWindow(window_);
            glfwTerminate();
        }
    }

}
