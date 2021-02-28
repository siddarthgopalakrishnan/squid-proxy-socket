#!/bin/bash
clear

sleep 1
gcc http_proxy_download.c -o http_proxy_download.out

sleep 1
./http_proxy_download.out $1 182.75.45.22 13128 csf303 csf303 index.html logo.gif