#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <string>
#include <algorithm>
#include <type_traits>

using namespace std;

string trim(const string& str) {
    auto start = str.find_first_not_of(" \t");
    auto end = str.find_last_not_of(" \t");

    if (start == string::npos || end == string::npos)
        return "";

    return str.substr(start, end - start + 1);
}

class IniParserError : public runtime_error {
public:
    IniParserError(const string& message) : runtime_error(message) {}
};

class IniParser {
public:
    IniParser(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            throw IniParserError("Unable to open file: " + filename);
        }
        parse_file(file);
    }

    template<typename T>
    T get_value(const string& section, const string& key) const {
        auto section_it = data.find(section);
        if (section_it == data.end()) {
            throw IniParserError("Section not found: " + section);
        }

        auto key_it = section_it->second.find(key);
        if (key_it == section_it->second.end()) {
            throw IniParserError("Key not found: " + key);
        }

        return convert_value<T>(key_it->second);
    }

private:
    map<string, map<string, string>> data;

    void parse_file(ifstream& file) {
        string line;
        string current_section;

        while (getline(file, line)) {
            line = trim(line);

            if (line.empty() || line.front() == ';') {
                continue;
            }

            if (line.front() == '[' && line.back() == ']') {
                current_section = trim(line.substr(1, line.length() - 2));
            }
            else {
                auto pos = line.find('=');
                if (pos == string::npos) {
                    throw IniParserError("Syntax error: " + line);
                }

                string key = trim(line.substr(0, pos));
                string value = trim(line.substr(pos + 1));

                if (current_section.empty()) {
                    throw IniParserError("Key-value pair found outside of any section");
                }

                data[current_section][key] = value;
            }
        }
    }

    template<typename T>
    T convert_value(const string& value) const {
        if constexpr (is_same_v<T, int>) {
            size_t processed;
            int result = stoi(value, &processed);
            if (processed != value.length()) {
                throw IniParserError("Invalid integer format: " + value);
            }
            return result;
        }
        else if constexpr (is_same_v<T, double>) {
            // Заменяем запятую на точку для поддержки европейского формата
            string normalized_value = value;
            replace(normalized_value.begin(), normalized_value.end(), ',', '.');

            size_t processed;
            double result = stod(normalized_value, &processed);
            if (processed != normalized_value.length()) {
                throw IniParserError("Invalid double format: " + value);
            }
            return result;
        }
        else if constexpr (is_same_v<T, string>) {
            return value;
        }
        else {
            static_assert(sizeof(T) == 0, "Type not supported");
        }
    }
};

int main() {
    try {
        IniParser parser("example.ini");

        int intValue = parser.get_value<int>("Section1", "var1");
        cout << "Section1, var1: " << intValue << endl;

        string stringValue = parser.get_value<string>("Section1", "var2");
        cout << "Section1, var2: " << stringValue << endl;

        double doubleValue = parser.get_value<double>("Section1", "var3");
        cout << "Section1, var3: " << doubleValue << endl;
    }
    catch (const IniParserError& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}