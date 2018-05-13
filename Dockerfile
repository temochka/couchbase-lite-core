FROM ubuntu:18.04

RUN apt-get update && apt-get install -y build-essential cmake clang libc++abi-dev libc++-dev libicu-dev git

RUN mkdir /source

WORKDIR /source
