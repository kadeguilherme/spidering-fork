#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include <thread>
#include <mutex>

std::mutex coutMutex;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string GetHtmlFromUrl(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Erro ao inicializar libcurl." << std::endl;
        return "";
    }

    std::string result;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    // Desativar verificação de certificado SSL (use com cautela)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Erro ao realizar a solicitação: " << curl_easy_strerror(res) << std::endl;
    }

    // Limpar e liberar recursos
    curl_easy_cleanup(curl);

    return result;
}

struct ThreadData {
    std::string html;
    std::regex regex;
    std::vector<std::string> result;
};

void thread_extrair(ThreadData* thread) {
    auto it = std::sregex_iterator(thread->html.begin(), thread->html.end(), thread->regex);
    auto end = std::sregex_iterator();

    for (; it != end; ++it) {
        std::smatch match = *it;
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            thread->result.push_back(match.str(1));  // Usar match.str(1) para obter o grupo de captura (URL)
        }
    }
}

std::string TratarLink(const std::string& link) {
    std::string linkTratado = link;

    size_t pos = linkTratado.find("href=\"");
    if (pos != std::string::npos) {
        linkTratado.erase(0, pos + 6); // Remover até o final de "href=\""
    }

    if (!linkTratado.empty() && linkTratado.back() == '"') {
        linkTratado.pop_back();
    }

    // Verificar se o link já contém o domínio
    if (linkTratado.find("https://www.ifb.edu.br") == std::string::npos) {
        // Adicionar o domínio no início
        linkTratado = "https://www.ifb.edu.br" + linkTratado;
    }

    return linkTratado;
}

void BuscarPDFs(const std::vector<std::string>& links) {
    std::regex pdfRegex("href=\"([^\"]+\\.pdf)");
    std::vector<std::thread> threads;

    for (const auto& pdfLink : links) {
        threads.emplace_back([pdfLink, &pdfRegex]() {
            std::string url = TratarLink(pdfLink);
            std::string html = GetHtmlFromUrl(url);

            ThreadData pdf{html, pdfRegex, {}};
            thread_extrair(&pdf);

            for (const auto& pdfItem : pdf.result) {
                std::cout << pdfItem << std::endl;
            }
        });
    }

    // Aguardar a conclusão de todas as threads
    for (auto& thread : threads) {
        thread.join();
    }
}

int main() {
    std::string url = "https://www.ifb.edu.br/";
    std::string html = GetHtmlFromUrl(url);
    std::regex hrefRegex("href=\"([^\"]+)\"");

    ThreadData link{html, hrefRegex, {}};
    thread_extrair(&link);

    for (const auto& linkItem : link.result) {
        BuscarPDFs({linkItem});
    }
    return 0;
}
