//
// Created by Alan Freitas on 2019-12-07.
//

#ifndef BIBLIOMETRICS_AUTHOR_H
#define BIBLIOMETRICS_AUTHOR_H

#include <string>
#include <vector>
#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <ostream>

#include <bibliometrics/graph/paper.h>

namespace bibliometrics {
#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
    namespace fs = std::experimental::filesystem;
#else
    namespace fs = std::filesystem;
#endif

    class author {
    public:
        explicit author() = default;

    public /* non-modifying */:
        int number_of_papers() const;
        int number_of_papers(int year_begin, int year_end) const;
        int citations() const;
        int citations(int year_begin, int year_end) const;
        std::vector<int> citations_list() const;
        std::vector<int> citations_list(int year_begin, int year_end) const;
        double citations_per_author(int year_begin, int year_end) const;
        double citations_per_author() const;
        double impact_factor() const;
        double impact_factor(int year_begin, int year_end) const;
        int h_index() const;
        int h_index(int year_begin, int year_end) const;
        int h_core() const;
        int h_core(int year_begin, int year_end) const;
        int i10_index() const;
        int i10_index(int year_begin, int year_end) const;

        auto begin() {
            return papers_.begin();
        }

        auto end() {
            return papers_.end();
        }

        auto begin() const {
            return papers_.begin();
        }

        auto end() const {
            return papers_.end();
        }

    public /* modifying */:
        void add_paper(const paper& p);

    public /* getters and setters */:
        const std::string id() const;
        void id(const std::string &id);

        const std::string &name() const;
        void name(const std::string &name);

        const std::vector<paper> &papers() const;
        void papers(const std::vector<paper> &papers);

    public /* auxiliary */:
        bool operator<(const author &rhs) const;
        bool operator>(const author &rhs) const;
        bool operator<=(const author &rhs) const;
        bool operator>=(const author &rhs) const;

        friend std::ostream &operator<<(std::ostream &os, const author &author);

    private:
        std::string id_;
        std::string name_;
        std::vector<paper> papers_;
    };
}

#endif //BIBLIOMETRICS_AUTHOR_H
