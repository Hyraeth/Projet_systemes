FROM alpine:latest
RUN apk update
RUN apk add --no-cache bash
RUN apk add libc-dev
RUN apk add gcc
RUN apk add make
RUN apk add clang
RUN mkdir /home/projet
RUN mkdir /home/projet/commandes
RUN mkdir /home/projet/headers
RUN mkdir /home/projet/test
ENV PATH="/home/projet:${PATH}"
COPY tsh.c /home/projet
COPY Makefile /home/projet
COPY commandes /home/projet/commandes
COPY headers /home/projet/headers
COPY test /home/projet/test
WORKDIR /home/projet
RUN make
WORKDIR /
RUN /bin/bash


