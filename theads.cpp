#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include <thread>
#include <mutex> //é utilizado para garantir que apenas uma thread por vez tenha acesso a uma seção crítica do código
#include <fstream>

std::mutex coutMutex;

//Essa função é utilizada como callback pelo libcurl para processar os dados recebidos durante o download.
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}
//Usa a biblioteca libcurl para realizar uma requisição HTTP à URL fornecida e retorna o HTML da página.
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
//Uma estrutura para armazenar dados que serão compartilhados entre threads.
struct ThreadData {
    std::string html;
    std::regex regex;
    std::vector<std::string> result;
};
//Uma função que recebe um nome de arquivo e um vetor de strings e salva as strings no arquivo.
void salvar_arquivos(const std::string& filename, const std::vector<std::string>& resultado) {
    std::ofstream outputFile(filename, std::ios::app);  // Abre em modo de apêndice
    if (outputFile.is_open()) {
        for (const auto& item : resultado) {
            outputFile << item << std::endl;
        }
        outputFile.close();
    } else {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
    }
}
//Uma função que usa expressões regulares para extrair dados de uma string HTML e armazená-los na estrutura ThreadData.
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
//Remove partes desnecessárias de um link e adiciona o domínio se necessário.
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
//Uma função que inicia threads para buscar links para arquivos PDF em paralelo.
void BuscarPDFs(const std::vector<std::string>& links) {
    std::string link_tradado_pdf;
    std::regex pdfRegex("href=\"([^\"]+\\.pdf)");
    std::vector<std::thread> threads;

    for (const auto& pdfLink : links) {
        threads.emplace_back([pdfLink, &pdfRegex]() {
            std::string url = TratarLink(pdfLink);
            std::string html = GetHtmlFromUrl(url);

            ThreadData pdf{html, pdfRegex, {}};
            thread_extrair(&pdf);

            for (const auto& pdf : pdf.result) {
                std::cout << pdf << std::endl;
                std::string link_tratado_pdf = TratarLink(pdf);
                salvar_arquivos("pdf_links_do_principal.txt", {link_tratado_pdf});
            }
        });
    }

    // Aguardar a conclusão de todas as threads
    for (auto& thread : threads) {
        thread.join();
    }
}
//função principal
int main() {
    std::string url = "https://www.ifb.edu.br/";
    std::string html = GetHtmlFromUrl(url);
    std::regex hrefRegex("href=\"([^\"]+)\"");
    std::regex pdf_linkRegex("href=\"([^\"]+\\.pdf)");

    ThreadData link{html, hrefRegex, {}};
    thread_extrair(&link);

    ThreadData link_pdf{html, pdf_linkRegex, {}};
    thread_extrair(&link_pdf);

    for (const auto& link : link.result) {
        std::cout << link << std::endl;
        salvar_arquivos("links_principal.txt", {link});
    }

    for (const auto& pdf : link_pdf.result) {
        std::cout << pdf << std::endl;
        salvar_arquivos("pdf_principal.txt", {pdf});
    }

    for (const auto& link : link.result) {
        BuscarPDFs({link});
    }

    return 0;
}
