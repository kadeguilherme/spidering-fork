#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <map>
#include <regex>
#include <vector>
#include <thread>
#include <mutex>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string GetHtmlFromUrl(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string result;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "Erro ao realizar a solicitação: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return result;
}

void ProcessUrls(const std::vector<std::string>& urls, std::map<std::string, std::vector<std::string>>& linkMap, std::mutex& mtx) {
    for (const auto& url : urls) {
        std::string html = GetHtmlFromUrl(url);

        std::regex hrefRegex("href=\"(https[^\"]*ifg\\.edu\\.br[^\"]*)\"");
        std::regex link_regex("https://.*\\.pdf");

        std::smatch match;
        std::vector<std::string> links;

        auto it = std::sregex_iterator(html.begin(), html.end(), hrefRegex);
        auto end = std::sregex_iterator();

        for (; it != end; ++it) {
            std::smatch match = *it;
            links.push_back(match[1].str());
        }

        std::lock_guard<std::mutex> lock(mtx);
        linkMap[url] = links;
    }
}

int main() {
    std::string base_url = "https://www.ifg.edu.br/";
    std::vector<std::string> seed_urls = {base_url};

    std::map<std::string, std::vector<std::string>> my_link;
    std::mutex my_mutex;

    std::vector<std::thread> threads;
    for (int i = 0; i < seed_urls.size(); ++i) {
        threads.emplace_back(ProcessUrls, std::ref(seed_urls), std::ref(my_link), std::ref(my_mutex));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Abrir um arquivo para escrever os links de PDF
    std::ofstream outputFile("pdf_links.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para escrita." << std::endl;
        return 1;
    }

    // Iterar sobre o mapa e escrever cada link no arquivo
    for (const auto& pair : my_link) {
        outputFile << "URL: " << pair.first << "\nLinks encontrados: ";
        for (const auto& link : pair.second) {
            outputFile << link << " ";
        }
        outputFile << "\n\n";
    }

    // Fechar o arquivo
    outputFile.close();

    std::cout << "Links salvos em pdf_links.txt" << std::endl;

    return 0;
}


