#ifndef COMM_GATEWAY
#define COMM_GATEWAY

class CommGateway {
public:
    CommGateway();
    void testSMS();
    void testPhoneCall();
private:
    std::string account_sid = "";  
    std::string auth_token = "";    
    std::string from_number = "";       
    std::string to_number = "";  


};

#endif // COMM_GATEWAY