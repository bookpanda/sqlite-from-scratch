# Building SQLite from scratch
- [SQLite's file format](https://www.sqlite.org/fileformat.html)
- how indexed data is [stored in B-trees](https://jvns.ca/blog/2014/10/02/how-does-sqlite-work-part-2-btrees/)
and more.

```bash
./your_program.sh ./sample.db .dbinfo

# test with real sqlite
sqlite3 sample.db "select * from apples"
sqlite3 sample.db "select * from oranges"
sqlite3 sample.db "select id, name from apples"
```

There are two other databases that you can use:

1. `superheroes.db`:
   - This is a small version of the test database used in the table-scan stage.
   - It contains one table: `superheroes`.
   - It is ~1MB in size.
1. `companies.db`:
   - This is a small version of the test database used in the index-scan stage.
   - It contains one table: `companies`, and one index: `idx_companies_country`
   - It is ~7MB in size.

These aren't included in the repository because they're large in size. You can
download them by running this script:

```sh
./download_sample_databases.sh
```

## Endians
e.g. we have number 0x1234
### Big-endian
- byte[0] = 0x12
- byte[1] = 0x34
- SQLite uses big-endian for all integers in the file format.
### Little-endian
- byte[0] = 0x34
- byte[1] = 0x12
