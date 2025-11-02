#include <iostream>
#include <fstream> // For file logging
#include <curl/curl.h>
#include <algorithm>
#include "communication_gateway.h"


CommGateway::CommGateway() {
    
}

void CommGateway::init() {
    std::ifstream secretFile("secrets.txt");
    if (!secretFile.is_open()) {
        std::cerr << "Failed to open secrets.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(secretFile, line)) {
        // Remove not characters
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the position of '=' and then skip the lines without it
        std::size_t equalsPos = line.find('=');
        if (equalsPos == std::string::npos)
            continue; 

        std::string key = line.substr(0, equalsPos);
        std::string value = line.substr(equalsPos + 1);

        // Remove not character again and semicolon at the end
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t;") + 1);
        
        if (!value.empty() && value[0] == '"')
            value.erase(0, 1);
        if (!value.empty() && value[value.length() - 1] == '"')
            value.pop_back();

        // Find the correct member variable
        if (key == "account_sid") {
            account_sid = value;
        } else if (key == "auth_token") {
            auth_token = value;
        } else if (key == "from_number") {
            from_number = value;
        } else if (key == "to_number") {
            to_number = value;
        }
    }

    secretFile.close();
}

// Callback function to handle response (we'll just discard it for simplicity)
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string data(static_cast<char*>(contents), total_size);

    // Log to console
    std::cout << "WriteCallback: Received " << total_size << " bytes\n";
    // Optionally print response data (be cautious with large responses)
    std::cout << "Response data: " << data.substr(0, 100) << "..." << std::endl; // Limit output for brevity

    // Log to file
    std::ofstream log_file("sms_log.txt", std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "WriteCallback: Failed to open log file 'sms_log.txt'" << std::endl;
        return 0; // Indicate error to curl
    }

    log_file << "WriteCallback: Received " << total_size << " bytes\n";
    log_file << "Response data: " << data << "\n";
    log_file.close();

    // Check for specific error indicators in the response (e.g., Twilio error)
    if (data.find("error") != std::string::npos || data.find("Error") != std::string::npos) {
        std::cerr << "WriteCallback: Potential error in response: " << data.substr(0, 100) << "..." << std::endl;
        // Optionally, return 0 to signal an error to curl, but for now, we continue
    }

    return total_size; // Return the number of bytes processed
}

void CommGateway::testSMS() {
    std::cout << "Testing SMS message: Sending test message...\n";
    // Placeholder for SMS test logic
    std::string message = "Cat detected, open the door";

    
    CURL* curl = curl_easy_init();
    if (curl) {
        // Set URL
        std::string url = "https://api.twilio.com/2010-04-01/Accounts/" + account_sid + "/Messages.json";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Basic Auth: SID:Token (base64 encoded internally by curl)
        std::string credentials = account_sid + ":" + auth_token;
        curl_easy_setopt(curl, CURLOPT_USERNAME, account_sid.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, auth_token.c_str());

        // POST form data
        // Problem this way: creating a temporary string that gets destroyed at the end of the statement, 
        // but libcurl doesn't immediately copy the POST data - it only stores the pointer and uses it later when you call curl_easy_perform().
        // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ("To=" + to_number + "&From=" + from_number + "&Body=" + message).c_str());  // result error: A 'To' phone number is required
        std::string post_data = "To=" + to_number + "&From=" + from_number + "&Body=" + message;
        std::cout << "POST data: " << post_data << std::endl;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

        // Headers for form POST
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Follow redirects and ignore SSL peer verification (for testing; use CA bundle in production)
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        // Response handler
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        // Perform request
        CURLcode res = curl_easy_perform(curl);

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return;
        } else {
            std::cout << "SMS sent successfully!" << std::endl;
        }
    } else {
        std::cerr << "curl_easy_init() failed" << std::endl;
        return;
    }


}

void CommGateway::testPhoneCall() {
    std::cout << "Testing phone call: Initiating test call...\n";
    // Placeholder for phone call test logic
}