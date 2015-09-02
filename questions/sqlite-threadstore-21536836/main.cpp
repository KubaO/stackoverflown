#include <QCoreApplication>
#include <sqlite3.h>
#include <QThreadStorage>
#include <QMap>

class SQLiteStmt {
    sqlite3_stmt* m_stmt;
    int m_rc;
    friend class SQLiteDB;
private:
    explicit SQLiteStmt(sqlite3_stmt* stmt, int rc) : m_stmt(stmt), m_rc(rc) {}
public:
    ~SQLiteStmt() { sqlite3_finalize(m_stmt); m_stmt = 0; }
    int status() const { return m_rc; }
    operator sqlite3_stmt*() const { return m_stmt; }
};

class SQLiteDB {
  sqlite3* m_db;
  Q_DISABLE_COPY(SQLiteDB)
public:
  SQLiteDB() : m_db(0) {}
  int open(const QString & filename) {
    close();
    return sqlite3_open16(filename.utf16(), &m_db);
  }
  int close() {
    sqlite3* const db = m_db;
    m_db = 0;
    if (db) return sqlite3_close(db);
    return SQLITE_OK;
  }
  SQLiteStmt prepare(const QString & sql) {
      sqlite3_stmt* stmt;
      int rc = sqlite3_prepare16_v2(m_db, sql.utf16(), -1, &stmt, 0);
      return SQLiteStmt(stmt, rc);
  }
  ~SQLiteDB() { close(); }
  operator sqlite3*() const { return m_db; }
};

void run()
{
    QThreadStorage<SQLiteDB> dbData;
    SQLiteDB & db(dbData.localData());
    db.open(":memory:");
    int rc = sqlite3_busy_timeout(db, 100);
    SQLiteStmt stmt = db.prepare("SELECT * FROM myTable");
    if (stmt.status() == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        // ...
    }
}


int main(int, char **)
{
    return 0;
}
