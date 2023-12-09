#include <iostream>
#include <string>
#include <curl/curl.h>
#include <map>
#include <regex>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append((char*)contents, total_size);
    return total_size;
}

std::string GetHtmlFromUrl(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string result;

    // Inicializar o libcurl
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

        // Limpar e liberar recursos
        curl_easy_cleanup(curl);
    }

    // Finalizar o libcurl
    curl_global_cleanup();

    return result;
}

int main() {
    std::string url = "https://www.ifb.edu.br/";
    std::string html = GetHtmlFromUrl(url);
    std::regex hrefRegex("href=\"(https[^\"]*ifb\\.edu\\.br[^\"]*)\"");
    std::regex link_regex("https://.*\\.pdf");

    std::smatch match;
    std::vector<std::string> my_link;

    auto it = std::sregex_iterator(html.begin(), html.end(), hrefRegex);
    auto end = std::sregex_iterator();

  // Adicionar os links
    for (; it != end; ++it) {
         std::smatch match = *it;
        my_link.push_back(match.str());
    }

  // Adicionar os PDFS 
    for (auto& pair : my_link) {
        auto pdf = std::sregex_iterator(html.begin(), html.end(), link_regex);
        auto pdfsend = std::sregex_iterator();
        for (; pdf != pdfsend; ++pdf) {
                std::smatch matchpdf = *pdf;
            my_link.push_back(matchpdf.str());
            }
    }
// Iterar sobre o mapa e imprimir cada par chave-valor
    for (const auto& pair : my_link) {
        std::cout << pair << std::endl;
    }

    return 0;
}