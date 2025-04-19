#include <iostream>
#include <curl/curl.h>
#include <string>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string generate_payload_url(const std::string& input_url) {
    std::string payload = "' OR '1'='1";
    if (input_url.find('?') != std::string::npos) {
        return input_url + "&id=" + payload;
    } else if (input_url.back() == '/') {
        return input_url + "?id=" + payload;
    } else {
        return input_url + "/?id=" + payload;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Foydalanish: " << argv[0] << " <URL yoki IP-manzil>" << std::endl;
        return 1;
    }

    std::string input_url = argv[1];

    if (input_url.find("http://") == std::string::npos && input_url.find("https://") == std::string::npos) {
        std::cerr << "[x] URL noto‘g‘ri formatda. Iltimos, URLni http:// yoki https:// bilan boshlang!" << std::endl;
        return 1;
    }

    std::string full_url = generate_payload_url(input_url);

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "[x] So‘rov yuborishda xatolik: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return 1;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        std::cout << "[+] HTTP javob kodi: " << response_code << std::endl;

        if (response_code == 406 || response_code == 403) {
            std::cout << "[!] Ehtimoliy WAF aniqlandi (shubhali so‘rov bloklandi)" << std::endl;
        } else if (readBuffer.find("Access Denied") != std::string::npos || 
                   readBuffer.find("WAF") != std::string::npos) {
            std::cout << "[!] Javob matnida WAF mavjudligi belgilari topildi!" << std::endl;
        } else {
            std::cout << "[+] Hech qanday WAF mavjudligi aniqlanmadi" << std::endl;
        }

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "[x] CURL ishga tushmadi" << std::endl;
        return 1;
    }

    return 0;
}
