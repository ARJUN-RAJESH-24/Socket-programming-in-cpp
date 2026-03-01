FROM alpine:latest

RUN apk add --no-cache g++

WORKDIR /app

COPY Task2_Concurrent/Task_2server.cpp server.cpp
COPY Task2_Concurrent/Task_2client.cpp client.cpp

RUN g++ server.cpp -o server && g++ client.cpp -o client

EXPOSE 8080

CMD ["./server"]
