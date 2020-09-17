//
// Created by Alan Freitas on 2019-12-09.
//

#include <iostream>
#include <random>

#include "author.h"

namespace bibliometrics {

    const std::string &author::name() const {
        return name_;
    }

    std::ostream &operator<<(std::ostream &os, const author &author) {
        os << "{id: " << author.id_ << ", ";
        os << "name: " << author.name_ << ", ";
        os << "papers: " << author.papers_.size() << "}";
        return os;
    }

    int author::number_of_papers() const {
        return static_cast<int>(papers_.size());
    }

    int author::number_of_papers(int year_begin, int year_end) const {
        int s = 0;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                ++s;
            }
        }
        return s;
    }

    int author::citations() const {
        int s = 0;
        for (const auto &paper : papers_) {
            s += paper.citations();
        }
        return s;
    }

    int author::citations(int year_begin, int year_end) const {
        int s = 0;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                s += paper.citations();
            }
        }
        return s;
    }

    std::vector<int> author::citations_list() const {
        std::vector<int> l;
        for (const auto &paper : papers_) {
            l.emplace_back(paper.citations());
        }
        return l;
    }

    std::vector<int> author::citations_list(int year_begin, int year_end) const {
        std::vector<int> l;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                l.emplace_back(paper.citations());
            }
        }
        return l;
    }

    int author::i10_index() const {
        int s = 0;
        for (const auto &paper : papers_) {
            if (paper.citations() >= 10) {
                ++s;
            }
        }
        return s;
    }

    int author::i10_index(int year_begin, int year_end) const {
        int s = 0;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                if (paper.citations() >= 10) {
                    ++s;
                }
            }
        }
        return s;
    }

    int author::h_index() const {
        std::vector<int> c;
        for (const auto &paper : papers_) {
            c.emplace_back(paper.citations());
        }
        std::sort(c.begin(),c.end(),std::greater<>());
        for (size_t i = 0; i < c.size(); ++i) {
            if (c[i] < static_cast<int>(i + 1)) {
                return static_cast<int>(i);
            }
        }
        return 0;
    }

    int author::h_index(int year_begin, int year_end) const {
        std::vector<int> c;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                c.emplace_back(paper.citations());
            }
        }
        std::sort(c.begin(),c.end(),std::greater<>());
        for (size_t i = 0; i < c.size(); ++i) {
            if (c[i] < static_cast<int>(i + 1)) {
                return static_cast<int>(i);
            }
        }
        return 0;
    }

    int author::h_core() const {
        std::vector<int> c;
        for (const auto &paper : papers_) {
            c.emplace_back(paper.citations());
        }
        std::sort(c.begin(),c.end(),std::greater<>());
        int s = 0;
        for (size_t i = 0; i < c.size(); ++i) {
            if (c[i] < static_cast<int>(i + 1)) {
                return s;
            } else {
                s += c[i];
            }
        }
        return 0;
    }

    int author::h_core(int year_begin, int year_end) const {
        std::vector<int> c;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                c.emplace_back(paper.citations());
            }
        }
        std::sort(c.begin(),c.end(),std::greater<>());
        int s = 0;
        for (size_t i = 0; i < c.size(); ++i) {
            if (c[i] < static_cast<int>(i + 1)) {
                return s;
            } else {
                s += c[i];
            }
        }
        return 0;
    }

    double author::citations_per_author() const {
        double s = 0;
        for (const auto &paper : papers_) {
            s += paper.citations_per_author();
        }
        return s;
    }

    double author::citations_per_author(int year_begin, int year_end) const {
        double s = 0;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                s += paper.citations_per_author();
            }
        }
        return s;
    }

    double author::impact_factor() const {
        double s = 0;
        for (const auto &paper : papers_) {
            s += paper.citations();
        }
        return s / papers_.size();
    }

    double author::impact_factor(int year_begin, int year_end) const {
        double s = 0;
        int n = 0;
        for (const auto &paper : papers_) {
            if (paper.year() >= year_begin && paper.year() <= year_end) {
                s += paper.citations();
                ++n;
            }
        }
        if (n != 0) {
            return s / n;
        } else {
            return 0.;
        }
    }

    const std::string author::id() const {
        if (!id_.empty()) {
            return id_;
        } else {
            std::hash<std::string> hasher;
            size_t h = hasher(name_);
            return std::to_string(h);
        }
    }

    void author::id(const std::string &id) {
        id_ = id;
    }

    void author::name(const std::string &name) {
        name_ = name;
    }

    const std::vector<paper> &author::papers() const {
        return papers_;
    }

    void author::papers(const std::vector<paper> &papers) {
        papers_ = papers;
    }

    bool author::operator<(const author &rhs) const {
        return name_ < rhs.name_;
    }

    bool author::operator>(const author &rhs) const {
        return rhs < *this;
    }

    bool author::operator<=(const author &rhs) const {
        return !(rhs < *this);
    }

    bool author::operator>=(const author &rhs) const {
        return !(*this < rhs);
    }

    void author::add_paper(const paper &p) {
        papers_.emplace_back(p);
    }

}