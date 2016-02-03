#!/bin/bash

sudo make clean
sudo make merge2filesPG
sudo service postgresql restart
psql -U mariana -d dp -a -f merge2files.sql -h localhost