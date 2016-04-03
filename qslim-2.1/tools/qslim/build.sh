#!/bin/bash

sudo make clean
sudo make
sudo service postgresql restart
psql -U mariana -d dp -a -f /home/sato/Desktop/dp/connect_to_db/qslim.sql -h localhost