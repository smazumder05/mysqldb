# mysqldb

![](https://www.sqlite.org/images/arch2.gif)

My very own implementation of sqlite3

1. What format is data saved in? (in memory and on disk)
2. When does it move from memory to disk?
3. Why can there only be one primary key per table?
4. How does rolling back a transaction work?
5. How are indexes formatted?
6. When and how does a full table scan happen?
7. What format is a prepared statement saved in?

### Components of SQLite3 that will be implemented

1. SQL CLI
2. Command Processor or Interpreter
3. Virtual Machine - that executes the prepared SQL statements
4. Backend storage
  - In memory
  - Disk using b-trees
