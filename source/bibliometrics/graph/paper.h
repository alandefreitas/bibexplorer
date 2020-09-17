//
// Created by Alan Freitas on 2019-12-07.
//

#ifndef BIBLIOMETRICS_PAPER_H
#define BIBLIOMETRICS_PAPER_H

#include <cereal/archives/json.hpp>

#include <string>
#include <vector>
#include <ostream>

namespace bibliometrics {
    class paper {
    public:
        template <class T>
        void emplace_author(T author_data) {
            authors_.emplace_back(author_data);
        }

        std::string authors_string() const;

        size_t hash() const;

        bool operator<(const paper &rhs) const;

        bool operator>(const paper &rhs) const;

        bool operator<=(const paper &rhs) const;

        bool operator>=(const paper &rhs) const;
    public /* getters and setters */:
        friend std::ostream &operator<<(std::ostream &os, const paper &paper);

        const std::string &title() const;
        void title(const std::string &title);

        int year() const;
        void year(int year);

        const std::string &journal() const;
        void journal(const std::string &journal);

        const std::vector<std::string> &authors() const;
        void authors(const std::vector<std::string> &authors);

        const std::string id() const;
        void id(const std::string &id);

        int citations() const;
        void citations(int citations);

        double citations_per_author() const;

    private:
        std::string id_;
        std::string title_;
        int year_;
        std::string journal_;
        std::vector<std::string> authors_;
        int citations_{0};
    };
}

#endif //BIBLIOMETRICS_PAPER_H
