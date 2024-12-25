/*
 * IniManager - INI configuration file management for C++
 * ===============================
 * Licensed under the MIT license
 * Copyright(c) 2024 Jacob Nilsson
 */

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>

#define INIMANAGER_HPP

/**
 * @brief A class to manage INI files
 */
class IniManager {
public:
    using Key = std::string;
    using Value = std::string;
    using HeaderValue = std::unordered_map<Key, Value>;
    using HeaderKey = std::string;
    using Config = std::unordered_map<HeaderKey, HeaderValue>;
private:
    Config parsed_map{};

    void parse(const std::string& data) {
        parsed_map.clear();

        if (data.empty()) {
            throw std::invalid_argument("data is empty");
        }

        std::stringstream ss{data};
        std::string it{};
        std::string current_header{};
        while (std::getline(ss, it)) {
            it.erase(std::remove_if(it.begin(), it.end(), ::isspace), it.end());
            if (it.empty() || it.at(0) == ';' || it.at(0) == '#') {
                continue;
            }

            // check if it's a header
            if (it.at(0) == '[' && it.at(it.size() - 1) == ']') {
                current_header = it.substr(1, it.size() - 2);
                continue;
            }

            if (current_header.empty()) {
                continue;
            }

            // find = and split
            auto pos = it.find('=');
            if (pos + 1 == std::string::npos) {
                continue;
            }

            std::string key = it.substr(0, pos);
            std::string value = it.substr(pos + 1);

            // remove any possible comments & quotes
            pos = value.find(';');
            if (pos != std::string::npos) {
                if (pos - 1 != std::string::npos && value.at(pos - 1) != '\\') {
                    value = value.substr(0, pos);
                }
            }

            pos = value.find('#');
            if (pos != std::string::npos) {
                if (pos - 1 != std::string::npos && value.at(pos - 1) != '\\') {
                    value = value.substr(0, pos);
                }
            }
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            parsed_map[current_header][key] = value;
        }
    }
public:
    IniManager() = default;
    ~IniManager() = default;
    explicit IniManager(const std::string& data, bool is_file = false) {
        load(data, is_file);
    }

    void load(const std::string& data, bool is_file = false) {
        this->parsed_map.clear();

        std::string buffer{};

        if (is_file) {
            std::ifstream file(data);
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    buffer += line + "\n";
                }
                file.close();
            }
        }

        parse(is_file ? buffer : data);
    }

    [[nodiscard]] Value& get(const std::string& header, const std::string& key) {
        if (header.empty()) {
            throw std::invalid_argument("header is empty; call get_data instead");
        }

        if (key.empty()) {
            throw std::invalid_argument("key is empty; call get_header instead");
        }

        if (parsed_map.find(header) == parsed_map.end()) {
            throw std::invalid_argument("header not found");
        }

        return parsed_map[header][key];
    }

    [[nodiscard]] Config get_data() const {
        return parsed_map;
    }

    [[nodiscard]] HeaderValue& get_header(const std::string& header) {
        if (header.empty()) {
            throw std::invalid_argument("header is empty");
        }

        if (parsed_map.find(header) == parsed_map.end()) {
            parsed_map[header] = {};
        }

        return parsed_map[header];
    }

    [[nodiscard]] std::string to_string() {
        std::string ret{};
        for (const auto& [header, values] : parsed_map) {
            if (values.empty() || header.empty()) {
                continue;
            }

            ret += "[" + header + "]\n";

            for (const auto& [key, value] : values) {
                ret += key;
                ret += "=" + value + "\n";
            }

            ret += "\n";
        }
        return ret;
    }

    void save(const std::string& file) {
        std::ofstream out{file};
        if (out.is_open()) {
            out << to_string();
            out.close();
        } else {
            throw std::runtime_error("could not open file for writing");
        }
    }

    void set(const std::string& header, const std::string& key, const std::string& value) {
        if (header.empty()) {
            throw std::invalid_argument("header is empty");
        }
        if (key.empty()) {
            throw std::invalid_argument("key is empty");
        }
        if (value.empty()) {
            parsed_map[header].erase(key);
            return;
        }
        parsed_map[header][key] = value;
    }

    [[nodiscard]] HeaderValue& operator[](const std::string& header) {
        return get_header(header);
    }
};
