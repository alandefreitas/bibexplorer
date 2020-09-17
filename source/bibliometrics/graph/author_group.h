//
// Created by Alan Freitas on 2019-12-09.
//

#ifndef BIBLIOMETRICS_AUTHOR_GROUP_H
#define BIBLIOMETRICS_AUTHOR_GROUP_H

#include <vector>
#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <ostream>
#include "author.h"

namespace bibliometrics {
#ifdef CXX_FILESYSTEM_IS_EXPERIMENTAL
    namespace fs = std::experimental::filesystem;
#else
    namespace fs = std::filesystem;
#endif

    class author_group {
    public:
        author_group() = default;

        size_t n_papers();

    public:
        friend std::ostream &operator<<(std::ostream &os, const author_group &department);
        void add_author(const author& a);
        auto begin() {
            return authors_.begin();
        }
        auto end() {
            return authors_.end();
        }
        auto begin() const {
            return authors_.begin();
        }
        auto end() const {
            return authors_.end();
        }

        void save(const std::string_view& filename);

    public /* getter and setter */:
        const std::vector<author> &authors() const;
        void authors(const std::vector<author> &authors);

    private:
        std::vector<author> authors_;
    };
}

#endif //BIBLIOMETRICS_AUTHOR_GROUP_H
