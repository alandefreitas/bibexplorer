//
// Created by Alan Freitas on 2019-12-09.
//

#ifndef BIBLIOMETRICS_ALGORITHM_H
#define BIBLIOMETRICS_ALGORITHM_H

#include <string>
#include <vector>
#include <numeric>
#include <cmath>

#include <tidy.h>
#include <tidybuffio.h>

#include <pugixml.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

namespace bibliometrics {
    std::string tidy_html(const std::string &html);

    std::string curl_request(const std::string &url, const std::vector<std::string>& extra_headers = {}, long timeout = 10);

    void stream_node(const pugi::xml_node& node);

    void replace_new_line(std::string& text, char new_char = ' ');

    std::string slugify(const std::string& name);

    std::string encode_html(const std::string& text);

    std::string url_encode(const std::string &value);

/// Computes the Levenshtein distance between two strings.
    int levenshtein(const std::string& a, const std::string& b, bool ignore_case);

    std::string to_string(bool v);

    std::string fileread(const std::string& filename);
    void filesave(const std::string& contents, const std::string& filename);

    std::string lowercase(const std::string& str);

    std::vector<std::string> explode(const std::string& str, char delim);



    template <class T>
    std::vector<T> flatten(const std::vector<std::vector<T>>& x) {
        std::vector<T> x_line;
        x_line.reserve(x.size() * x[0].size());
        for (size_t i = 0; i < x.size(); ++i) {
            x_line.insert(x_line.end(), x[i].begin(), x[i].end());
        }
        return x_line;
    }

    template <class T>
    T mean(const std::vector<T>& x) {
        T m = 0;
        for (size_t i = 0; i < x.size(); ++i) {
            m += x[i];
        }
        return m / x.size();
    }

    /// \see https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Covariance
    template <class T>
    T covariance(const std::vector<T>& x, const std::vector<T>& y) {
        size_t n = std::min(x.size(), y.size());
        T sum12 = 0;
        T sum1 = std::accumulate(x.begin(), x.end(), T(0));
        T sum2 = std::accumulate(y.begin(), y.end(), T(0));
        for (size_t i = 0; i < n; ++i) {
            sum12 += x[i] * y[i];
        }
        return (sum12 - sum1 * sum2 /n)/n;
    }

    template <class T, bool CORRECTED = true>
    T stddev(const std::vector<T>& x) {
        T mean_x = mean(x);
        T m = 0;
        for (size_t i = 0; i < x.size(); ++i) {
            m += pow(x[i] - mean_x, 2);
        }
        if (CORRECTED) {
            m /= x.size() - 1;
        } else {
            m /= x.size();
        }
        return sqrt(m);
    }

    template <class T>
    T pearson(const std::vector<T>& x, const std::vector<T>& y) {
        const T cov = covariance(x, y);
        const T stdx = stddev<T,false>(x);
        const T stdy = stddev<T,false>(y);
        if (stdx != 0 && stdy != 0) {
            return cov / (stdx * stdy);
        } else {
            // This is an heuristic to replace NaN
            return cov > static_cast<T>(0) ? static_cast<T>(1) : static_cast<T>(0);
        }
    }

    template <class T, class T2 = size_t, bool BREAK_TIES = true>
    std::vector<T2> ranks(const std::vector<T>& x) {
        // idxs
        std::vector<size_t> idxs(x.size());
        std::iota(idxs.begin(),idxs.end(),0);
        std::sort(idxs.begin(), idxs.end(), [&](size_t a, size_t b) {
            return x[a] < x[b];
        });
        // ranks
        std::vector<T2> r(x.size());
        for (size_t i = 0; i < idxs.size(); ++i) {
            r[idxs[i]] = static_cast<T2>(i + 1);
        }
        if constexpr (BREAK_TIES) {
            // take two positions from idxs and break the tie with an average
            auto break_tie = [&](size_t tie_begin, size_t tie_end) {
                size_t tie_size = tie_end-tie_begin;
                if (tie_size > 1) {
                    size_t idx_first_tie_element = idxs[tie_begin];
                    size_t idx_last_tie_element = idxs[tie_end-1];
                    T2 new_value = (r[idx_first_tie_element] + r[idx_last_tie_element]) / 2;
                    for (size_t j = tie_begin; j < tie_end; ++j) {
                        size_t idx_current_tie_element = idxs[j];
                        r[idx_current_tie_element] = new_value;
                    }
                }
            };

            size_t tie_begin = 0;
            for (size_t i = 1; i < x.size(); ++i) {
                size_t original_element_idx = idxs[i];
                size_t idx_first_tie_element = idxs[tie_begin];
                if (x[original_element_idx] != x[idx_first_tie_element]) {
                    break_tie(tie_begin, i);
                    tie_begin = i;
                }
            }
            break_tie(tie_begin, x.size());
        }
        return r;
    }

    template <class T>
    T spearman(const std::vector<T>& x, const std::vector<T>& y) {
        std::vector<T> rx = ranks<T,T,true>(x);
        std::vector<T> ry = ranks<T,T,true>(y);
        return pearson(rx,ry);
    }

}


#endif //BIBLIOMETRICS_ALGORITHM_H
