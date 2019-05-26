#! /usr/bin/env bash

# driver program to reset all db schema as well as loading the data


rm -fr ./tmp/*

#
# step 1. reset db
#
sudo -u postgres psql -d postgres<<EOF
--
-- Create Database
--
drop database if exists orders;
drop role if exists $USER;
create database orders;
CREATE ROLE $USER WITH SUPERUSER LOGIN PASSWORD 'password';
EOF


#
# step 2. import pre order load schema
#         this step does not add any index/constraints/pkey to
#         orders table as constraint check/index building as data
#         are imported are not efficient for dataset with billioins
#         of records
#
psql -d orders -f ./schema/pre_load.sql


#
# step 3. import reference data
# Notes: postgresql COPY command is used here for importing
#        reference tables. Its a good idea to use it for orders
#        as well, but the assignment requires a python script
#        for that
#
psql -d orders<<EOF
--
-- Import Reference Data
--
\copy order_status from './data/order_status_types.csv' csv header;
\copy payment_status from './data/payment_status_types.csv' csv header;
\copy shipping_status from './data/shipping_status_types.csv' csv header;
EOF

#
# step 4. bulk import order data
#


# first splitting the orders.csv file into multiple csvs ignoring the header
# for large csv files, its not necessary to do the split
# so this setup so its really just to demonstrate the idea
pushd ./tmp
tail -n +2 ../data/orders.csv | split -dl 100 -a 3 - orders.csv.
popd

# generate a csv file list to work with
# then submit it to load_orders.py which can parralize the loading
# we are using 3 threads to load the data, which is consist with
# the number of files. again it might be overkill for the input
# of this specific task. The purpose is to demonstrate the idea
# exactly how many parralel processing can be done would depend
# on some db profiling
pushd ./python
readlink -f ../tmp/orders.csv* > ./files.txt
time python3 ./load_orders.py --num-workers 3 ./files.txt
popd

# Notes: there will be a pythong/timings.csv file generated
#        that has running stats for each splitted file
#        just a bonus feature



#
# step 5. import post order load schema
#         which builds the index/constraints for the db
#         with existing data
#
psql -d orders -f ./schema/post_load.sql


#
# step 6. gen csv report for the report which
#         shows aggregate order total by date
#         for orders that are completed, paid
#         and do not reuired shipping
#
#
pushd ./python
python3 ./gen_reports.py
popd
