#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

// Estrutura para armazenar os dados de cada link
struct IFB_LINK {
    std::string url;
};

std::string get_request(const std::string& url) {
    CURL *curl = curl_easy_init();
    std::string result;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void *contents, size_t size, size_t nmemb, std::string *response) {
            ((std::string*) response)->append((char*) contents, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return result;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    
    std::string html_document = get_request("https://www.ifb.edu.br/");
    htmlDocPtr doc = htmlReadMemory(html_document.c_str(), html_document.length(), nullptr, nullptr, HTML_PARSE_NOERROR);

    xmlXPathContextPtr context = xmlXPathNewContext(doc);

    xmlXPathObjectPtr link_html_elements = xmlXPathEvalExpression((xmlChar *) "//div[contains(@class)]//a", context);

    std::vector<IFB_LINK> ifb_links;

    // Verifica se há elementos
    if (link_html_elements && link_html_elements->nodesetval) {
        for (int i = 0; i < link_html_elements->nodesetval->nodeNr; ++i) {
            xmlNodePtr link_html_element = link_html_elements->nodesetval->nodeTab[i];
            
            std::string url = std::string(reinterpret_cast<char *>(xmlGetProp(link_html_element, (xmlChar *) "href")));

            IFB_LINK ifb_link = {url};
            ifb_links.push_back(ifb_link);
        }
    }

    // Libera recursos
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    // Cria e escreve no arquivo CSV
    std::ofstream csv_file("ifb_links.csv");

    if (csv_file.is_open()) {
        csv_file << "url" << std::endl;

        for (const auto& link : ifb_links) {
            csv_file << link.url << std::endl;
        }

        csv_file.close();
    } else {
        std::cerr << "Erro ao abrir o arquivo CSV para escrita." << std::endl;
    }

    curl_global_cleanup();

    return 0;
}
