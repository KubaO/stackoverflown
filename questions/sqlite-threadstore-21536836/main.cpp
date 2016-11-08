// https://github.com/KubaO/stackoverflown/tree/master/questions/sqlite-threadstore-21536836
#include <QtCore>
#include <sqlite3.h>

class SQLiteStmt {
   Q_DISABLE_COPY(SQLiteStmt)
   sqlite3_stmt* m_stmt;
   int m_rc;
   friend class SQLiteDB;
private:
   explicit SQLiteStmt(sqlite3_stmt* stmt, int rc) : m_stmt(stmt), m_rc(rc) {}
public:
   SQLiteStmt(SQLiteStmt && o) : m_stmt(o.m_stmt), m_rc(o.m_rc) {
      o.m_stmt = nullptr;
   }
   ~SQLiteStmt() { sqlite3_finalize(m_stmt); m_stmt = nullptr; }
   int status() const { return m_rc; }
   operator sqlite3_stmt*() const { return m_stmt; }
};

class SQLiteDB {
   Q_DISABLE_COPY(SQLiteDB)
   sqlite3* m_db = nullptr;
public:
   SQLiteDB() = default;
   int open(const QString & filename) {
      close();
      return sqlite3_open16(filename.utf16(), &m_db);
   }
   int close() {
      sqlite3* const db = m_db;
      m_db = nullptr;
      if (db) return sqlite3_close(db);
      return SQLITE_OK;
   }
   int exec(const QString & sql) {
      auto stmt = prepare(sql);
      int rc = stmt.status();
      if (rc == SQLITE_OK) do {
         rc = sqlite3_step(stmt);
      } while (rc == SQLITE_ROW);
      return rc;
   }
   SQLiteStmt prepare(const QString & sql) {
      sqlite3_stmt* stmt;
      auto rc = sqlite3_prepare16_v2(m_db, sql.utf16(), -1, &stmt, nullptr);
      return SQLiteStmt{stmt, rc};
   }
   ~SQLiteDB() { close(); }
   operator sqlite3*() const { return m_db; }
};

int main() {
   QThreadStorage<SQLiteDB> dbData;
   SQLiteDB & db(dbData.localData());
   if (db.open(":memory:") != SQLITE_OK)
      qDebug() << "open failed";
   if (db.exec("CREATE TABLE myTable(value NUMERIC)") != SQLITE_DONE)
      qDebug() << "create failed";
   if (db.exec("INSERT INTO myTable VALUES (10), (20), (30)") != SQLITE_DONE)
      qDebug() << "insert failed";
   auto stmt = db.prepare("SELECT * FROM myTable");
   if (stmt.status() == SQLITE_OK) while (true) {
      int rc = sqlite3_step(stmt);
      if (rc != SQLITE_ROW) break;
      qDebug() << sqlite3_column_int(stmt, 0);
   }
}
