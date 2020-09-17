//
// Created by Alan Freitas on 2019-12-09.
//

#include <filesystem>
#include <iostream>
#include <random>
#include <string_view>
#include <fstream>

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

#include <bibliometrics/graph/author_group.h>

namespace bibliometrics {
    std::ostream &operator<<(std::ostream &os, const author_group &d) {
        os << "Author group with " << d.authors_.size() << " authors";
        return os;
    }

    const std::vector<author> &author_group::authors() const {
        return authors_;
    }

    void author_group::authors(const std::vector<author> &authors) {
        authors_ = authors;
    }

    void author_group::add_author(const author &a) {
        authors_.push_back(a);
    }

    void author_group::save(const std::string_view &filename) {
        fs::path f = filename;
        if (fs::is_directory(f)) {
            throw std::runtime_error(f.string() + " is a directory");
        }

        if (!fs::exists(f.parent_path())) {
            std::cout << "Creating parent path " << f.parent_path() << std::endl;
            fs::create_directory(f.parent_path());
            if (!fs::exists(f.parent_path())) {
                throw std::runtime_error("Unable to create parent path " + f.parent_path().string());
            }
        }

        std::ofstream file((std::string(filename)));
        if (!file) {
            throw std::runtime_error(std::string("Cannot open file ") + std::string(filename));
        }
        auto w = csv::make_csv_writer(file);
        w << std::vector<std::string>{"Author ID", "Author", "Paper ID", "Title", "Journal", "Year", "Authors", "Number of Authors","Citations"};
        for (const auto &author : authors_) {
            for (const auto &paper : author) {
                w << std::vector<std::string>{author.id(), author.name(), paper.id(), paper.title(), paper.journal(), std::to_string(paper.year()), paper.authors_string(), std::to_string(paper.authors().size()), std::to_string(paper.citations())};
            }
        }
    }

    size_t author_group::n_papers() {
        size_t s = 0;
        for (const auto &author : authors_) {
            s += author.number_of_papers();
        }
        return s;
    }
}