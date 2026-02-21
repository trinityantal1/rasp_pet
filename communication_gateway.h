#ifndef COMM_GATEWAY
#define COMM_GATEWAY
#include <chrono>

class CommGateway {
public:
    CommGateway();
    void init();
    void SendSMS(std::string message);
    void SendPhoneCall(std::string message);
    void SendAlert();

private:
    std::string account_sid = "";
    std::string auth_token = "";
    std::string from_number = "";
    std::string to_number = "";
    bool isSMSorVoice = true; // true for SMS, false for voice call;
    std::chrono::steady_clock::time_point last_alert_time;

    static constexpr int ALERT_SUPPRESSION_SECONDS = 600;  // 10 minutes
};

#endif // COMM_GATEWAY