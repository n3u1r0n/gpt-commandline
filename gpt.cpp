#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class GPT {
private:
    const std::string default_url = "https://api.openai.com/v1/chat/completions";
    const std::string default_model = "gpt-3.5-turbo";
    const float default_temperature = 0.9;
    std::vector<json> messages;
    std::string api_key;
    std::string url;
    std::string model;
    float temperature;

public:
    GPT(const std::string& apikey_filename = ".apikey", const std::string& url = "", const std::string& model = "", float temperature = 0.0) :
        api_key(""), url(default_url), model(default_model), temperature(default_temperature)
    {
        std::ifstream apikey_file(apikey_filename);
        if (apikey_file.is_open())
        {
            getline(apikey_file, api_key);
            apikey_file.close();
        }
        else
        {
            std::cerr << "No API key found. Please create a file named \".apikey\" and put your API key in it." << std::endl;
            exit(1);
        }
        if (!url.empty())
            this->url = url;
        if (!model.empty())
            this->model = model;
        if (temperature != 0.0)
            this->temperature = temperature;
    }

    std::string post(const std::string& message)
    {
        messages.push_back({ {"content", message}, {"role", "user"} });
        std::string response_string;
        CURL* curl = curl_easy_init();
        if (curl) {
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            std::string authorization_header = "Authorization: Bearer " + api_key;
            headers = curl_slist_append(headers, authorization_header.c_str());

            json data = { {"model", model}, {"messages", messages}, {"temperature", temperature} };
            std::string data_string = data.dump();

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data_string.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &GPT::write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                response_string = "Error: " + std::string(curl_easy_strerror(res));
            curl_easy_cleanup(curl);
        } else {
            response_string = "Error: curl_easy_init() failed";
        }
        messages.back()["role"] = "assistant";
        json response = json::parse(response_string);
        std::cout << "Response JSON: " <<  response.dump(2) << std::endl;
        return response["choices"][0]["text"];
    }

    static size_t write_callback(void* ptr, size_t size, size_t nmemb, std::string* data)
    {
        size_t realsize = size * nmemb;
        data->append((char*)ptr, realsize);
        return realsize;
    }
};

int main(int argc, char* argv[])
{
    GPT gpt;
    if (argc > 1)
    {
        std::string message;
        for (int i = 1; i < argc; i++)
            message += argv[i] + std::string(" ");
        std::cout << ">>> " << message;
        std::string reply = gpt.post(message);
        std::cout << "<<< " << reply << std::endl;
    }

    while (true)
    {
        std::string message;
        std::cout << ">>> ";
        std::getline(std::cin, message);
      if (message == "exit")
            break;
        std::string reply = gpt.post(message);
        std::cout << "<<< " << reply << std::endl;
    }

    return 0;
}
