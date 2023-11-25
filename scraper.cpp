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
    std::regex hrefRegex("href=\"([^\"]*ifb[^\"]*)\"");
    std::map<std::string,std::string> my_link;

    auto it = std::sregex_iterator(html.begin(), html.end(), hrefRegex);
    auto end = std::sregex_iterator();

    // Percorrer as correspondências e imprimir os valores de href
    for (; it != end; ++it) {
        std::smatch match = *it;
        //std::cout << "Href encontrado: " << match[1].str() << std::endl;
        my_link.insert(std::make_pair( match[1].str() ,match[1].str())
    }


    return 0;
}