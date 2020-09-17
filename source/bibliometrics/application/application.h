//
// Created by Alan Freitas on 17/08/20.
//

#ifndef BIBLIOMETRICS_application_H
#define BIBLIOMETRICS_application_H

// C++
#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <future>

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// SpdLog
#include <spdlog/spdlog.h>

// Internal
#include <bibliometrics/graph/author_group.h>

namespace bibliometrics {
#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
    namespace fs = std::experimental::filesystem;
#else
    namespace fs = std::filesystem;
#endif

    class application {
      public:
        /// Create a parser application
        application(std::vector<std::string> input_files, std::string output,
                    bool gui, bool verbose_tracing);

        ~application();

        /// Parse papers and download information
        void run();

      public /* getters and setters */:
        const std::string &microsoft_key() const;
        void microsoft_key(const std::string &microsoft_key);
        const std::string &elsevier_key() const;
        void elsevier_key(const std::string &elsevier_key);

      private /* initialization */:
        static bool is_valid_extension(const std::string_view &ext);
        void find_valid_input_files();
        void setup_output_dir();
        void setup_logger();
        void init_opengl_and_imgui();

      private /* parsing */:
        /// Parse the information from the individual CVs
        void run_parser();

        /// Parse information from a lattes CV
        author parse_lattes(const fs::path &input);

        /// Parse information from a CSV file
        author parse_csv(const fs::path &input);

        /// Parse information from a JSON file
        author parse_json(const fs::path &input);

      private /* fetch data */:
        /// Go through the APIs looking for paper information
        void run_scraper();

        /// Get data from the microsoft API
        void fetch_microsoft(const fs::path &cache_dir, const author &author,
                             paper &paper);

      private /* save results */:
        void save_author_group();

      private /* user interface */:
        void render_gui();

      private /* helpers */:
        /// Check if a future object is ready
        template <class FUTURE_TYPE> bool is_ready(const FUTURE_TYPE &f) {
            return f.wait_for(std::chrono::seconds(0)) ==
                   std::future_status::ready;
        }

        /// Make a curl request and keep rendering `render_gui` while
        /// we don't have a response
        std::string
        curl_request_while_render(const std::string &url,
                                  const std::vector<std::string> &extra_headers,
                                  long timeout);

      private /* members */:
        std::vector<std::string> input_files_;
        fs::path output_stem_;
        bool verbose_{false};
        std::shared_ptr<spdlog::logger> log;
        static constexpr std::string_view default_output_stem{"bibliography"};
        std::string microsoft_key_;
        std::string elsevier_key_;
        author_group internal_group_;

      private /* interface */:
        bool gui_{true};
        GLFWwindow *window_;
        int display_w_{0};
        int display_h_{0};
        bool persistent_window_ = true;
        const std::string *current_input_file_{nullptr};
        size_t input_files_parsed_{0};
        const author *current_author_{nullptr};
        const paper *current_paper_{nullptr};
        size_t papers_fetched_{0};
        size_t total_papers_to_fetch_{0};
        size_t authors_fetched_{0};
        size_t total_authors_to_fetch_{0};
        std::vector<float> citation_histogram_ = {0.0f};
        int indicator_time_window_size_{5};
    };
} // namespace bibliometrics

#endif // BIBLIOMETRICS_application_H
