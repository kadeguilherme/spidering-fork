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
    std::map<std::string, std::vector<std::string>> my_link;

    auto it = std::sregex_iterator(html.begin(), html.end(), hrefRegex);
    auto end = std::sregex_iterator();

    // Percorrer as correspondências e imprimir os valores de href
    for (; it != end; ++it) {
         std::smatch match = *it;
        //std::cout << it->str() << std::endl;
        //std::cout << "Href encontrado: " << match[1].str() << std::endl;
        //my_link.insert(std::make_pair( match[1].str() ,match[1].str()));
        my_link[match[1].str()] = {};
    }

    // Adicionar valores às chaves (uma lista de valores)
    for (auto& pair : my_link) {
        std::cout << "Digite os valores para " << pair.first << " (digite 'fim' para terminar): ";
        auto it = std::sregex_iterator(html.begin(), html.end(), link_regex);
        auto end = std::sregex_iterator();
        for (; it != end; ++it) {
         std::smatch match = *it;
        //std::cout << it->str() << std::endl;
        //std::cout << "Href encontrado: " << match[1].str() << std::endl;
        //my_link.insert(std::make_pair( match[1].str() ,match[1].str()));
        std::cout << "Chave: " << pair.first << ", Valores: " << match << std::endl;
        }
        std::string value;
        while (std::cin >> value && value != "fim") {
            pair.second.push_back(value);
        }
        std::cin.clear();  // Limpar o estado do fluxo para que a entrada do usuário funcione corretamente
    }

    // Iterar sobre o mapa e imprimir cada par chave-valor
    for (const auto& pair : my_link) {
        std::cout << "Chave: " << pair.first << ", Valores: ";
        for (const auto& value : pair.second) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}