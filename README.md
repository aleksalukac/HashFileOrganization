
# File organization in DBMS - Hash file organization in C++

DBMS for a cinema database using:
1. Entity designed for a movie (ID - primary key, movie name, duration, date, movie theater code etc.)
2. Inserting a record in the destined bucket (key transformation used to choose the bucket)
3. Reading, editing, logical deleting records in the file
4. Static file with a linked lists for each bucket for overflow buckets
5. Reading and inserting records from a csv file