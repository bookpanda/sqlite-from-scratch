# Building SQLite from scratch
- [SQLite's file format](https://www.sqlite.org/fileformat.html)
- how indexed data is [stored in B-trees](https://jvns.ca/blog/2014/10/02/how-does-sqlite-work-part-2-btrees/)
and more.

```bash
./your_program.sh ./sample.db .dbinfo

# test with real sqlite
hexdump -C sample.db
sqlite3 sample.db "SELECT * FROM sqlite_schema;"

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

## Varints encoding
1. break binary number into 7-bit chunks
2. add highest bit (MSB) to each chunk to show:
- 1xxx xxxx = more chunks follow
- 0xxx xxxx = this is the last chunk

> SQLite's varint can store up to 9 bytes (8*7 bits + 8 bits from 9th chunk = 64-bit unsigned integer).

#### decoding varints: 0x1b
- 0x1b = 0001 1011
- break into 7-bit chunks: "0"001 1011
- value = 16 + 8 + 2 + 1 = 27
#### decoding varints: 0x81 0x47
- "1"000 0001  "0"100 0111
- value = 128 + 64 + 4 + 2 + 1 = 199 (skip the MSB when counting power of 2)

## Structure
A b-tree page is divided into regions in the following order:
1. The 100-byte database file header (found on page 1 only)
2. The 8 (leaf page) or 12 (interior page) byte b-tree page header
3. The cell pointer array
4. Unallocated space
5. The cell content area
6. The reserved region

### Cell 
1. The size of the record, in bytes (varint)
2. The rowid (varint)
3. The record (record format)

### Record
1. Header:
   - Size of the header, including this value (varint)
   - Serial type code for each column in the record, in order (varint)
2. Body:
   - The value of each column in the record, in order (format varies based on serial type code)

### Cell from page 1 of sample.db
```bash
00000ec0           78 03 07 17 1b  1b 01 81 47 74 61 62 6c  |   x.......Gtabl|
00000ed0  65 6f 72 61 6e 67 65 73  6f 72 61 6e 67 65 73 04  |eorangesoranges.|
00000ee0  43 52 45 41 54 45 20 54  41 42 4c 45 20 6f 72 61  |CREATE TABLE ora|
00000ef0  6e 67 65 73 0a 28 0a 09  69 64 20 69 6e 74 65 67  |nges.(..id integ|
00000f00  65 72 20 70 72 69 6d 61  72 79 20 6b 65 79 20 61  |er primary key a|
00000f10  75 74 6f 69 6e 63 72 65  6d 65 6e 74 2c 0a 09 6e  |utoincrement,..n|
00000f20  61 6d 65 20 74 65 78 74  2c 0a 09 64 65 73 63 72  |ame text,..descr|
00000f30  69 70 74 69 6f 6e 20 74  65 78 74 0a 29           |iption text.)   |

// Size of the record (varint): 120 = 7 + 5 + 7 + 7 + 1 + 93
78

// The rowid (safe to ignore)
03

// Record header
07     // Size of record header (varint): 7

17     // Serial type for sqlite_schema.type (varint):     23
       // Size of sqlite_schema.type =                     (23-13)/2 = 5

1b     // Serial type for sqlite_schema.name (varint):     27
       // Size of sqlite_schema.name =                     (27-13)/2 = 7

1b     // Serial type for sqlite_schema.tbl_name (varint): 27
       // Size of sqlite_schema.tbl_name =                 (27-13)/2 = 7

01     // Serial type for sqlite_schema.rootpage (varint): 1
       // 8-bit twos-complement integer

81 47  // Serial type for sqlite_schema.sql (varint):      199
       // Size of sqlite_schema.sql =                      (199-13)/2 = 93

// Record body
74 61 62 6c 65        // Value of sqlite_schema.type:     "table"
6f 72 61 6e 67 65 73  // Value of sqlite_schema.name:     "oranges"
6f 72 61 6e 67 65 73  // Value of sqlite_schema.tbl_name: "oranges"  <---
...
```

### First page
```bash
00000000  53 51 4c 69 74 65 20 66  6f 72 6d 61 74 20 33 00  |SQLite format 3.|
00000010  10 00 01 01 00 40 20 20  00 00 00 05 00 00 00 04  |.....@  ........|
00000020  00 00 00 00 00 00 00 00  00 00 00 02 00 00 00 04  |................|
00000030  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 00  |................|
00000040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 05  |................|
00000060  00 2e 4b 90|0d 00 00|00  03|0e c3 00|0f 8f|0f 3d| |..K............=|
00000070 |0e c3|00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000ec0  00 00 00|78|03|07 17 1b  1b 01 81 47|74 61 62 6c  |...x.......Gtabl|
00000ed0  65|6f 72 61 6e 67 65 73 |6f 72 61 6e 67 65 73|04| |eorangesoranges.|
00000ee0  43 52 45 41 54 45 20 54  41 42 4c 45 20 6f 72 61  |CREATE TABLE ora|
00000ef0  6e 67 65 73 0a 28 0a 09  69 64 20 69 6e 74 65 67  |nges.(..id integ|
00000f00  65 72 20 70 72 69 6d 61  72 79 20 6b 65 79 20 61  |er primary key a|
00000f10  75 74 6f 69 6e 63 72 65  6d 65 6e 74 2c 0a 09 6e  |utoincrement,..n|
00000f20  61 6d 65 20 74 65 78 74  2c 0a 09 64 65 73 63 72  |ame text,..descr|
00000f30  69 70 74 69 6f 6e 20 74  65 78 74 0a 29 50 02 06  |iption text.)P..|
00000f40  17 2b 2b 01 59 74 61 62  6c 65 73 71 6c 69 74 65  |.++.Ytablesqlite|
00000f50  5f 73 65 71 75 65 6e 63  65 73 71 6c 69 74 65 5f  |_sequencesqlite_|
00000f60  73 65 71 75 65 6e 63 65  03 43 52 45 41 54 45 20  |sequence.CREATE |
00000f70  54 41 42 4c 45 20 73 71  6c 69 74 65 5f 73 65 71  |TABLE sqlite_seq|
00000f80  75 65 6e 63 65 28 6e 61  6d 65 2c 73 65 71 29 6f  |uence(name,seq)o|

```
- page header: 0d 00 00 00  03 0e c3 00
- cell pointer array: 0f 8f, 0f 3d, 0e c3 (not sorted, as it is sorted by rowid)