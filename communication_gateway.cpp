#include <iostream>
#include <fstream> // For file logging
#include <curl/curl.h>
#include <algorithm>
#include "communication_gateway.h"

using namespace std::chrono;

CommGateway::CommGateway() {
    steady_clock::time_point last_alert_time = steady_clock::time_point::min();
}

void CommGateway::init() {
    std::ifstream secretFile("config.txt");
    if (!secretFile.is_open()) {
        std::cerr << "Failed to open config.txt" << std::endl;
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

size_t TwillioResultCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string data(static_cast<char*>(contents), total_size);

    // Log to console
    std::cout << "TwillioResultCallback: Received " << total_size << " bytes\n";
    std::cout << "Response data: " << data.substr(0, 100) << "..." << std::endl;

    // Log to file
    std::ofstream log_file("sms_log.txt", std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "TwillioResultCallback: Failed to open log file 'sms_log.txt'" << std::endl;
        return 0;
    }

    log_file << "TwillioResultCallback: Received " << total_size << " bytes\n";
    log_file << "Response data: " << data << "\n";
    log_file.close();

    // Check for specific error indicators
    if (data.find("error") != std::string::npos || data.find("Error") != std::string::npos) {
        std::cerr << "TwillioResultCallback: Potential error in response: " << data.substr(0, 100) << "..." << std::endl;
    }

    return total_size; // Return the number of bytes processed
}

void CommGateway::SendSMS(std::string message) {
    std::cout << "Sending SMS message\n";
    // std::string message = "Cat detected, open the door";

    
    CURL* curl = curl_easy_init();
    if (curl) {
        // Set URL
        std::string url = "https://api.twilio.com/2010-04-01/Accounts/" + account_sid + "/Messages.json";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Basic Auth: SID:Token
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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TwillioResultCallback);

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

void CommGateway::SendPhoneCall(std::string message) {
    std::cout << "Making phone call: \n";
    if (message.empty()) {
        std::cerr << "Cannot make call with empty message\n";
        return;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl_easy_init() failed\n";
        return;
    }

    //  URL for creating a new Call (not Messages)
    std::string url = "https://api.twilio.com/2010-04-01/Accounts/" + account_sid + "/Calls.json";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Basic Auth (same as SMS)
    curl_easy_setopt(curl, CURLOPT_USERNAME, account_sid.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, auth_token.c_str());

    // Minimal TwiML that just speaks the message
    // std::string twiml = "<Response><Say>" + message + "</Say><Pause length=\"2\"/><Say>" + message + "</Say></Response>";
    std::string twiml = "<Response><Say>" + message + "</Say><Pause></Pause><Say>" + message + "</Say></Response>";

    // Escape & " ' < > characters that might appear in the message
    size_t pos = 0;
    while ((pos = twiml.find("&", pos)) != std::string::npos) {
        twiml.replace(pos, 1, "&amp;");
        pos += 5;
    }
    pos = 0;
    while ((pos = twiml.find("\"", pos)) != std::string::npos) {
        twiml.replace(pos, 1, "&quot;");
        pos += 6;
    }

    std::string post_data = "To=" + to_number + "&From=" + from_number + "&Twiml="  + twiml;

    std::cout << "POST data: " << post_data << std::endl;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

    // Headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);   // ? only for testing!

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TwillioResultCallback);

    // Execute
    CURLcode res = curl_easy_perform(curl);

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return;
    }

    std::cout << "Call initiated successfully!\n";
}

void CommGateway::SendAlert() {
    std::cout << "Sending alert: \n";

    steady_clock::time_point now = steady_clock::now();
    seconds durationSinceLastAlert = duration_cast<seconds>(now - last_alert_time);
    if (durationSinceLastAlert < seconds(CommGateway::ALERT_SUPPRESSION_SECONDS)) { // e.g. 10 minutes
        std::cout << "Alert already sent recently. Suppressing duplicate alert.\n";
        return;
    } else {
        last_alert_time = now;
        if (isSMSorVoice) {
            SendSMS("Cat or dog detected. Open the door.");
        } else {
            SendPhoneCall("Cat or dog detected. Open the door.");
        }
    }
}