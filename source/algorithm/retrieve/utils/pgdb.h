#ifndef PGDB_H
#define PGDB_H

#include <iostream>
#include <string.h>
#include <map>
#include <string>
#include <fstream>
#include <vector>

#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

class PGDB {
public:
    PGDB(string con_info);
    ~PGDB();

public:
    void init();
    void reconnect();
    bool pg_exec_sql(string sql);
    bool pg_fetch_sql(string sql, result& fetch_res);
    bool is_Working();

private:
    string m_con_str;
    connection *p_pg_con;
    bool m_isWorking;
};

#endif // PGDB_H
