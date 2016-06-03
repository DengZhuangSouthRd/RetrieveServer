#include "pgdb.h"

PGDB::PGDB(string con_info) {
    m_con_str = con_info;
    m_isWorking = false;
    p_pg_con = NULL;
    init();
}

PGDB::~PGDB() {
    if(p_pg_con != NULL && p_pg_con->is_open()) {
        p_pg_con->disconnect();
        delete p_pg_con;
        p_pg_con = NULL;
    }
}

void PGDB::init() {
    try {
        p_pg_con = new connection(m_con_str);
        if(p_pg_con != NULL && p_pg_con->is_open()) {
            cout << "Opened Successful: " << p_pg_con->dbname() << endl;
            m_isWorking = true;
        } else {
            cerr << "Can't Open PostgreSQL !" << endl;
            if(p_pg_con != NULL)
                delete p_pg_con;
            p_pg_con = NULL;
            m_isWorking = false;
        }
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }
}

void PGDB::reconnect() {
    if(p_pg_con == NULL) {
        init();
    } else {
        delete p_pg_con;
        p_pg_con = NULL;
        init();
    }
}

bool PGDB::is_Working() {
    return m_isWorking;
}

bool PGDB::pg_exec_sql(string sql) {
    try {
        work W(*p_pg_con);
        W.exec(sql);
        W.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}

bool PGDB::pg_fetch_sql(string sql, result& fetch_res) {
    try {
        nontransaction N(*p_pg_con);
        result R(N.exec(sql));
        fetch_res = R;
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}
