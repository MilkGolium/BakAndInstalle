#ifndef INIT_H
#define INIT_H

// initDatabase: open / create the database and execute the given SQL.
// Returns 1 on success, 0 on failure.
int initDatabase(const char* dbPath, const char* sql);

#endif // INIT_H
