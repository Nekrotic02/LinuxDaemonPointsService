#include "daemon.hpp"
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

using namespace daemonpp;
using namespace std::chrono_literals;

std::string readFileContents(const std::string& filename)
{
    std::ifstream file(filename);
    std::string contents;

    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            contents += line + "\n";
        }
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
    if (!contents.empty() && contents.back() == '\n')
    {
        contents.pop_back();
    }
    return contents;
}

class point_daemon : public daemon
{
private:
    CURL* curl;

public:
    point_daemon() : curl(nullptr) {}

    void on_start(const dconfig& cfg) override
    {
        while (true)
        {
            std::string Team1LinuxAPI = "439e7545dd9d777fcd2f1ede63bbffcf";
            std::string MachineID = "Team1Linux";
            CURLcode res;

            // Initial winsock
            curl_global_init(CURL_GLOBAL_ALL);

            // get curl handle
            curl = curl_easy_init();

            if (curl)
            {
                // Filename Declaration
                std::string filename = "/home/Team1LinuxFlag.txt";

                // Retrieve Contents of file
                std::string fileContents = readFileContents(filename);

                // set target URL
                curl_easy_setopt(curl, CURLOPT_URL, "http://172.16.1.24:5000/api/post_data");

                std::string jsonData = "{\"MachineID\": \"" + MachineID + "\", \"TeamID\": \"" + fileContents + "\"}";

                // Specify the POST data
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

                // Set Custom Header for API
                struct curl_slist* headers = nullptr;
                headers = curl_slist_append(headers, ("X-API-Key: " + Team1LinuxAPI).c_str());
                headers = curl_slist_append(headers, "Content-Type: application/json");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                // Perform the request
                res = curl_easy_perform(curl);

                if (res != CURLE_OK)
                {
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                }

                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
            }

            curl_global_cleanup();

            // Sleep for 5000 milliseconds (5 seconds)
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }

        dlog::info("point_daemon::on_start(): point_daemon version: " + cfg.get("version") + " started successfully!");
    }

    void on_update() override
    {
        dlog::info("point_daemon::on_update()");
    }

    void on_stop() override
    {
        if (curl)
        {
            curl_easy_cleanup(curl);
            curl_global_cleanup();
        }
        dlog::info("point_daemon::on_stop()");
    }

    void on_reload(const dconfig& cfg) override
    {
        dlog::info("point_daemon::on_reload(): new daemon version from updated config: " + cfg.get("version"));
    }
};

int main(int argc, const char* argv[])
{
    point_daemon dmn;                                // create a daemon instance
    dmn.set_name("point_daemon");                    // set daemon name to identify logs in syslog
    dmn.set_update_duration(3s);                     // set duration to sleep before triggering the on_update callback 3 seconds
    dmn.set_cwd("/");                                // set daemon's current working directory to root /
    dmn.run(argc, argv);                             // run your daemon
    return EXIT_SUCCESS;
}