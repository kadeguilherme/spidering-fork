Comando para pesquisar a li
'''RUN
apt search libcurl
'''

Comando para instalar
'''RUN
apt install libcurl4-openssl-dev
'''

Compilar
g++ -o scraper.out scraper.cpp -I/usr/include/libxml2 -lxml2 -lcurl
