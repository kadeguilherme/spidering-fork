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

        // Desabilitar a verificação do certificado SSL (use isso com cuidado)
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

    // Regex para encontrar URLs com "ifb" e links que terminam com ".pdf"
    std::regex hrefRegex("href=\"([^\"]*ifb[^\"]*)\"|[^.]*.pdf");

    std::map<std::string, std::string> my_links;

    auto it = std::sregex_iterator(html.begin(), html.end(), hrefRegex);
    auto end = std::sregex_iterator();

    // Percorrer as correspondências e adicionar ao mapa
    for (; it != end; ++it) {
        std::smatch match = *it;
        for (int i = 1; i < match.size(); i++) {
            if (!match[i].str().empty()) {
                my_links.insert(std::make_pair(match[i].str(), match[i].str()));
            }
        }
    }

    // Imprimir os links
    for (const auto& link : my_links) {
        std::cout << "Link encontrado: " << link.first << std::endl;
    }

    return 0;
}
