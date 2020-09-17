//
// Created by Alan Freitas on 2019-12-09.
//

#include <sstream>
#include <fstream>
#include <thread>
#include <algorithm>
#include <functional>

#include <bibliometrics/common/algorithm.h>

namespace bibliometrics {

    std::string tidy_html(const std::string &html) {
        // http://tidy.sourceforge.net/libintro.html#example
        const char *input = html.data();
        // const char* input = "<title>Foo</title><p>Foo!";
        TidyBuffer output{};
        output.allocator = nullptr;
        output.bp = nullptr;
        output.size = 0;
        output.allocated = 0;
        output.next = 0;
        TidyBuffer errbuf;
        errbuf.allocator = nullptr;
        int rc = -1;
        Bool ok;

        // Initialize "document"
        TidyDoc tdoc = tidyCreate();
        // printf("Tidying:\t%s\n", input);

        // Convert to XHTML
        ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
        if (ok)
            // Capture diagnostics
            rc = tidySetErrorBuffer(tdoc, &errbuf);
        if (rc >= 0)
            // Parse the input
            rc = tidyParseString(tdoc, input);
        if (rc >= 0)
            // Tidy it up!
            rc = tidyCleanAndRepair(tdoc);
        if (rc >= 0)
            // Kvetch
            rc = tidyRunDiagnostics(tdoc);
        // If error, force output.
        if (rc > 1)
            rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
        if (rc >= 0)
            // Pretty Print
            rc = tidySaveBuffer(tdoc, &output);

        std::string result;
        if (rc >= 0) {
            if (rc > 0) {
                // printf("\nDiagnostics:\n\n%s", errbuf.bp);
            }
            result = std::string((char *) output.bp);
            // printf("\nAnd here is the result:\n\n%s", output.bp);
        } else {
            printf("A severe error (%d) occurred.\n", rc);
        }

        tidyBufFree(&output);
        tidyBufFree(&errbuf);
        tidyRelease(tdoc);
        return result;
    }

    std::string curl_request(const std::string &url, const std::vector<std::string>& extra_headers, long timeout) {
        // Create client
        constexpr auto HEADER_ACCEPT = "Accept:text/html,application/xhtml+xml,application/xml";

        // https://developers.whatismybrowser.com/useragents/explore/operating_system_name/linux/
        constexpr auto HEADER_USER_AGENT = "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.70 Safari/537.17";

        curlpp::Easy request;

        // Specify the URL
        request.setOpt(curlpp::options::Url(url));
        request.setOpt(curlpp::options::Timeout(timeout));
        request.setOpt(curlpp::options::SslEngineDefault());

        // Specify some headers
        std::list<std::string> headers;
        headers.emplace_back(HEADER_ACCEPT);
        headers.emplace_back(HEADER_USER_AGENT);
        headers.insert(headers.end(), extra_headers.begin(), extra_headers.end());
        request.setOpt(new curlpp::options::HttpHeader(headers));
        request.setOpt(new curlpp::options::FollowLocation(true));

        // Configure curlpp to use stream
        std::ostringstream responseStream;
        curlpp::options::WriteStream streamWriter(&responseStream);
        request.setOpt(streamWriter);

        // Collect response
        request.perform();
        std::string content = responseStream.str();

        // Clean HTML
        return content;
    }

    void stream_node(const pugi::xml_node &node) {
        // constexpr char* node_types[] = { "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration" };

        std::cout << /* node_types[node.type()] */ "name='" << node.name() << "', value='" << node.value() << "'\n";
        std::cout << "node.text(): " << node.text() << std::endl;
        for (const auto &att : node.attributes()) {
            std::cout << att.name() << ": " << att.value() << std::endl;
        }
        for (const pugi::xpath_node &child : node.children()) {
            stream_node(child.node());
        }
    }

    void replace_new_line(std::string &text, char new_char) {
        std::replace(text.begin(), text.end(), '\n', new_char);
        std::replace(text.begin(), text.end(), '\r', new_char);
    }

    std::string slugify(const std::string &name) {
        std::string slug = name;
        std::transform(slug.begin(), slug.end(), slug.begin(), [](char c) { return static_cast<char>(::tolower(static_cast<int>(c))); });
        std::replace(slug.begin(), slug.end(), ' ', '+');
        return slug;
    }

    int levenshtein(const std::string &a, const std::string &b, bool ignore_case) {
        // Allocate distance matrix
        std::vector<std::vector<int>> d(a.size() + 1, std::vector<int>(b.size() + 1, 0));

        // Get character comparer
        std::function<bool(char,char)> is_equal = [&ignore_case](char a, char b) {
            return ignore_case ? tolower(static_cast<int>(a)) == tolower(static_cast<int>(b)) : a == b;
        };

        // Compute distance
        for (size_t i = 0; i <= a.size(); i++) {
            d[i][0] = static_cast<int>(i);
        }

        for (size_t j = 0; j <= b.size(); j++) {
            d[0][j] = static_cast<int>(j);
        }

        for (size_t i = 1; i <= a.size(); i++) {
            for (size_t j = 1; j <= b.size(); j++) {
                if (is_equal(a[i - 1], b[j - 1])) {
                    // No change required
                    d[i][j] = d[i - 1][j - 1];
                } else {
                    d[i][j] = std::min(d[i - 1][j] + 1,    // Deletion
                                       std::min(d[i][j - 1] + 1,    // Insertion
                                                d[i - 1][j - 1] + 1));       // Substitution
                }
            }
        }

        // Return final value
        return d[a.size()][b.size()];
    }

    std::string encode_html(const std::string &data) {
        std::string buffer;
        buffer.reserve(data.size());
        for (size_t pos = 0; pos != data.size(); ++pos) {
            switch (data[pos]) {
                // https://dev.w3.org/html5/html-author/charref
                case '&':
                    buffer.append("&amp;");
                    break;
                    // case '\"': buffer.append("&quot;");      break;
                case '\"':
                    buffer.append("%22");
                    break;
                case '\'':
                    buffer.append("&apos;");
                    break;
                case '(':
                    buffer.append("&lpar;");
                    break;
                case ')':
                    buffer.append("&rpar;");
                    break;
                case '<':
                    buffer.append("&lt;");
                    break;
                case '>':
                    buffer.append("&gt;");
                    break;
                case ',':
                    buffer.append("&comma;");
                    break;
                default:
                    buffer.append(&data[pos], 1);
                    break;
            }
        }
        return buffer;
    }

    std::string to_string(bool v) {
        return v ? std::string("true") : std::string("false");
    }

    std::string fileread(const std::string& filename) {
        std::ifstream t(filename);
        if (!t) {
            throw std::runtime_error("Cannot open the file " + filename);
        }
        std::string str((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
        return str;
    }

    void filesave(const std::string& contents, const std::string& filename) {
        std::ofstream t(filename);
        if (!t) {
            throw std::runtime_error("Cannot open the file " + filename);
        }
        t << contents;
    }

    std::string lowercase(const std::string& str) {
        std::string str_copy = str;
        std::transform(str_copy.begin(), str_copy.end(), str_copy.begin(),
                       [](char c){ return static_cast<char>(std::tolower(static_cast<int>(c))); });
        return str_copy;
    }

    std::vector<std::string> explode(std::string const & s, char delim) {
        // https://stackoverflow.com/questions/12966957/is-there-an-equivalent-in-c-of-phps-explode-function
        std::vector<std::string> result;
        std::istringstream iss(s);

        for (std::string token; std::getline(iss, token, delim); )
        {
            result.push_back(std::move(token));
        }

        return result;
    }

}
