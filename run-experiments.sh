#!/bin/bash

./run-case.sh email-Eu-core-temporal-Dept1.txt m1.txt 0.20 0 0
./run-case.sh email-Eu-core-temporal-Dept1.txt m1.txt 0.20 1 0
./run-case.sh email-Eu-core-temporal-Dept1.txt m1.txt 0.20 1 64
./run-case.sh email-Eu-core-temporal-Dept1.txt m1.txt 0.20 1 512
./run-case.sh email-Eu-core-temporal-Dept1.txt m1.txt 0.20 1 2048

./run-case.sh email-Eu-core-temporal-Dept2.txt m1.txt 0.20 0 0
./run-case.sh email-Eu-core-temporal-Dept2.txt m1.txt 0.20 1 0
./run-case.sh email-Eu-core-temporal-Dept2.txt m1.txt 0.20 1 64
./run-case.sh email-Eu-core-temporal-Dept2.txt m1.txt 0.20 1 512
./run-case.sh email-Eu-core-temporal-Dept2.txt m1.txt 0.20 1 2048
