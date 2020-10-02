// C++
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>

// External
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>

// Internal
#include <bibliometrics/common/system.h>
#include <bibliometrics/common/algorithm.h>
#include <bibliometrics/application/application.h>

int main(int argc, char *argv[]) {
    using namespace bibliometrics;

    ///////////////////////////////////////////////////////
    /// Load configuration file                         ///
    ///////////////////////////////////////////////////////
    // Load default settings from configuration file
    // The command line options, if existent, will override these defaults
    nlohmann::json j;
    {
        std::ifstream config_in("config.json");
        if (config_in) {
            config_in >> j;
        }
    }
    if (!j.is_object()) {
        j = nlohmann::json::object({});
    }

    // Default settings
    auto json_or_default = [](const nlohmann::json &j, const std::string& name, const auto& default_value) {
        auto it = j.find(name);
        if (it != j.end()) {
            return it.value().get<std::decay_t<decltype(default_value)>>();
        }
        return default_value;
    };
    std::vector<std::string> input = json_or_default(j,"input",std::vector<std::string>{"./input"});
    std::string output = json_or_default(j,"output",std::string("./output"));
    bool gui = json_or_default(j, "gui", true);
    bool show_version = json_or_default(j, "show_version", false);
    bool show_help = json_or_default(j, "show_help", false);
    bool verbose = json_or_default(j, "verbose", is_debug());
    std::string microsoft_key = json_or_default(j, "microsoft_key", std::string(""));
    std::string elsevier_key = json_or_default(j, "elsevier_key", std::string(""));

    ///////////////////////////////////////////////////////
    /// Declare command line options                    ///
    ///////////////////////////////////////////////////////
    cxxopts::Options options("bibexplorer", "Read production data and create a graph of paper and citations");

    options.add_options("Required arguments")
            ("i,input", "Input files or directories (xml, csv, or xsl)", cxxopts::value(input))
            ("o,output", "Output file or directory", cxxopts::value(output));

    options.add_options("Optional arguments")
            ("g,gui", "Graphical Interface", cxxopts::value(gui)->default_value(to_string(gui)))
            ("d,debug", "Verbose debug", cxxopts::value(verbose)->default_value(to_string(verbose)))
            ("v,version", "Print version", cxxopts::value(show_version)->default_value(to_string(show_version)))
            ("h,help", "Print usage", cxxopts::value(show_help)->default_value(to_string(show_help)));

    options.add_options("API keys")
            ("m,microsoft", "Key for Microsoft Academic API",cxxopts::value(microsoft_key))
            ("e,elsevier", "Key for Elsevier Developers API",cxxopts::value(elsevier_key))
            ;

    ///////////////////////////////////////////////////////
    /// Parse command line options                      ///
    ///////////////////////////////////////////////////////
    // Parse the command line options
    auto result = options.parse(argc, argv);

    ///////////////////////////////////////////////////////
    /// Save configuration back to config.json          ///
    ///////////////////////////////////////////////////////
    // Save configuration back into config.json
    // config.json learns about new parameters from the command line
    // The old parameters are not replaced
    auto save_if_not_there = [](nlohmann::json &j, const std::string& name, const auto& v) {
        if (j.find(name) == j.end()) {
            j[name] = v;
        }
    };
    save_if_not_there(j,"input",input);
    save_if_not_there(j,"output",output);
    save_if_not_there(j,"gui",gui);
    save_if_not_there(j,"verbose",verbose);
    if (!microsoft_key.empty()) {
        save_if_not_there(j,"microsoft_key",microsoft_key);
    }
    if (!elsevier_key.empty()) {
        save_if_not_there(j,"elsevier_key",elsevier_key);
    }
    {
        std::ofstream config_out("./config.json");
        if (!config_out) {
            std::cout << "Cannot open configuration file" << std::endl;
        }
        config_out << std::setw(4) << j << std::endl;
    }

    ///////////////////////////////////////////////////////
    /// Run according to the options                    ///
    ///////////////////////////////////////////////////////
    // Show help
    if (show_help || input.empty()) {
        options.custom_help("-i input_directory -o output_directory");
        std::cout << options.help({"Required arguments","Optional arguments","API keys"}) << std::endl;
        std::cout << "The input directory might contain xml, csv, and xsl files" << std::endl;
        std::cout << "The output directory will contain graphs with production and citation data" << std::endl;
        std::cout << "Include as many API keys as possible to fetch paper information" << std::endl;
        return 0;
    }

    // Show version
    if (show_version) {
        std::cout << "bibexplorer: version 0.0.1" << std::endl;
        return 0;
    }

    // Create application and run
    application app(input, output, gui, verbose);
    if (!microsoft_key.empty()) {
        app.microsoft_key(microsoft_key);
    }
    if (!elsevier_key.empty()) {
        app.elsevier_key(elsevier_key);
    }
    app.run();

    return 0;
}
