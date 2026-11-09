# build stage: compile the server
FROM debian:stable-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends g++ \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY solver.h solver.cpp server.cpp httplib.h json.hpp ./
RUN g++ -std=c++17 -O2 -pthread -static-libstdc++ -static-libgcc -o server server.cpp solver.cpp

# run stage: only the compiled server and the web folder
FROM debian:stable-slim
RUN useradd --system app
WORKDIR /app
COPY --from=build /app/server ./server
COPY web ./web
USER app
CMD ["./server"]