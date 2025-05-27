runserver:
    g++ copaint_server.cpp -o server.o -lsfml-graphics
    ./server.o
runclient:
    g++ -std=c++17 copaint_client.cpp -Wall -o client.o -lsfml-graphics -lsfml-window -lsfml-system
    ./client.o