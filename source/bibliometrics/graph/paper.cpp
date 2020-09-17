//
// Created by Alan Freitas on 2019-12-09.
//

#include <fstream>

#include <bibliometrics/graph/paper.h>
#include <bibliometrics/common/algorithm.h>

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace bibliometrics {

    std::ostream &operator<<(std::ostream &os, const paper &paper) {
        os << "{paper: " << paper.title_ << " year: " << paper.year_ << "}";
        return os;
    }

    const std::string &paper::title() const {
        return title_;
    }

    void paper::title(const std::string &title) {
        title_ = title;
    }

    int paper::year() const {
        return year_;
    }

    void paper::year(int year) {
        year_ = year;
    }

    const std::string &paper::journal() const {
        return journal_;
    }

    void paper::journal(const std::string &journal) {
        journal_ = journal;
    }

    const std::vector<std::string> &paper::authors() const {
        return authors_;
    }

    void paper::authors(const std::vector<std::string> &authors) {
        authors_ = authors;
    }

    bool paper::operator<(const paper &rhs) const {
        if (title_ < rhs.title_)
            return true;
        if (rhs.title_ < title_)
            return false;
        if (year_ < rhs.year_)
            return true;
        if (rhs.year_ < year_)
            return false;
        if (journal_ < rhs.journal_)
            return true;
        if (rhs.journal_ < journal_)
            return false;
        if (authors_ < rhs.authors_)
            return true;
        return (rhs.authors_ < authors_);
    }

    bool paper::operator>(const paper &rhs) const {
        return rhs < *this;
    }

    bool paper::operator<=(const paper &rhs) const {
        return !(rhs < *this);
    }

    bool paper::operator>=(const paper &rhs) const {
        return !(*this < rhs);
    }

    const std::string paper::id() const {
        if (!id_.empty()) {
            return id_;
        } else {
            return std::to_string(hash());
        }
    }

    void paper::id(const std::string &id) {
        id_ = id;
    }

    std::string paper::authors_string() const {
        if (authors_.empty()) {
            return "";
        }
        std::string res = authors_[0];
        for (size_t i = 1; i < authors_.size(); ++i) {
            res += "," + authors_[i];
        }
        return res;
    }

    size_t paper::hash() const {
        std::string str = title_ + std::to_string(year_);
        std::hash<std::string> hasher;
        size_t h = hasher(str);
        return h;
    }

    int paper::citations() const {
        return citations_;
    }

    double paper::citations_per_author() const {
        return static_cast<double>(citations_) / authors_.size();
    }

    void paper::citations(int citations) {
        citations_ = citations;
    }

}


