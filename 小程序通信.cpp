#include <filesystem>
#include <httplib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

std::string getUpdateData(const std::string& filepath) {
    std::ifstream file(filepath); // Open text file
    std::string data;

    if (file.is_open()) {
        std::string line;

        // Read the text file line by line
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;

            // Parse key-value pairs
            if (std::getline(iss, key, ':') && std::getline(iss, value)) {
                // Remove spaces from both ends of key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                // Build data string
                data += "\"" + key + "\": \"" + value + "\", ";
            }
        }

        file.close(); // Close text file

        // Delete the last comma and space
        if (!data.empty()) {
            data.erase(data.size() - 2);
        }
    }
    else {
        std::cout << "Failed to open the file: " << filepath << std::endl;
    }

    // Wrap data in JSON format
    data = "{" + data + "}";

    return data;
}

std::string getDataFromLine(const std::string& line) {
    std::istringstream iss(line);
    std::string key, value;

    // Parse key-value pairs
    if (std::getline(iss, key, ':') && std::getline(iss, value)) {
        // Remove spaces from both ends of key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        // Build data string
        return "\"" + key + "\": \"" + value + "\"";
    }

    return "";
}

std::string getHistoryData() {
    std::string data = "[";

    // Iterate over all files in the "data" directory
    for (const auto& file : std::filesystem::directory_iterator("data")) {
        std::ifstream infile(file.path().string());
        std::string line;
        std::string tempData = "";
        while (std::getline(infile, line)) {
            std::string formattedLine = getDataFromLine(line);
            if (!formattedLine.empty()) {
                if (!tempData.empty()) {
                    tempData += ", ";
                }
                tempData += formattedLine;
            }
            // If the line is empty, it means we have finished a set of data.
            else if (!tempData.empty()) {
                data += "{" + tempData + "}, ";
                tempData = "";
            }
        }
        // Handle the last set of data in the file
        if (!tempData.empty()) {
            data += "{" + tempData + "}, ";
        }
    }

    // Delete the last comma and space
    if (data.size() > 2) {
        data.erase(data.size() - 2);
    }

    // Wrap data in JSON format
    data += "]";

    std::cout << "History data prepared for sending: " << data << std::endl;

    return data;
}





int main(void)
{
    httplib::Server svr;

    svr.Get("/getData", [](const httplib::Request&, httplib::Response& res) {
        std::cout << "Received request for data. Processing..." << std::endl;
    std::string data = getUpdateData("update.txt");
    res.set_content(data, "application/json");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    std::cout << "Data sent back to client." << std::endl;
        });

    svr.Get("/getHistory", [](const httplib::Request&, httplib::Response& res) {
        std::cout << "Received request for history data. Processing..." << std::endl;
    std::string data = getHistoryData();
    res.set_content(data, "application/json");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    std::cout << "History data sent back to client." << std::endl;
        });

    std::cout << "Server starting... Listening on 0.0.0.0:6826" << std::endl;
    svr.listen("0.0.0.0", 6826);

    // Keep the server running.
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
