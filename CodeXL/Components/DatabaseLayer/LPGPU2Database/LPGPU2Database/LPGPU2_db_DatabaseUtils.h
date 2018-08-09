// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customisation of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// SQLite3 database utilities
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_DATABASE_UTILS_H_INCLUDE
#define LPGPU2_DATABASE_UTILS_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_FnStatus.h>

// STL:
#include <chrono>

// Uncomment this to dump SQLite query times to CodeXL's log
// #define ENABLE_QUERY_PROFILING

// Declarations:
struct sqlite3;
struct sqlite3_stmt;

namespace lpgpu2 {
namespace db {

using namespace fnstatus;

/// @brief  This is a utility class to deal with sqlite_stmt opaque objects.
/// 
/// SQLite C API is too verbose. This class has utilities to deal with
/// SQL statements in terms of getting and binding values. The statement
/// is reset when this object is destroyed.
///
/// In case of fail, the user can get the error message by calling
/// GetErrorMessage()
/// 
/// Example usage:
/// 
/// SQLiteStatementGuard stmtGuard{ mySqlStmt };
///
/// // binding a value to a prepared statement
/// stmtGuard.Bind("@name", 10);
/// stmtGuard.Bind("@id", 42.0f);
///
/// // getting a value
/// int myId = stmtGuard.Get<int>(0)
/// gtString myString = stmtGuard.Get<gtString>(1)

/// @warning    This object is meant to be used in local scopes.
/// @date       30/08/2017
/// @author     Thales Sabino
// clang-format off
class SQLiteStatementGuard
{
// Methods
public:
    explicit SQLiteStatementGuard(sqlite3_stmt* stmt);
    ~SQLiteStatementGuard();

    // Disable copy of this object
    SQLiteStatementGuard(const SQLiteStatementGuard&) = delete;
    SQLiteStatementGuard(SQLiteStatementGuard&&) = delete;

    // Common operations
    bool IsValid() const;
    bool Step() const;
    status Exec();
    status Finalize();
    status ExecAndFinalize();

    // Retrieves the error message
    const gtString& GetErrorMessage() const;

// Methods:
public:

    // Inline template methods
    template<typename ColumnType>
    ColumnType Get(int columnIndex) const
    {
        ColumnType value{};

        if (IsValid() && columnIndex >= 0)
        {            
            GetValue(columnIndex, value);                    
        }

        return value;
    }

    template<typename ValueType>
    status Bind(const char* columnName, ValueType value)
    {
        if (IsValid())
        {
            m_bBindsAreOk &= BindValue(columnName, value);

            if (m_bBindsAreOk)
            {
                AppendSQLiteErrorMessage();
            }
        }

        return m_bBindsAreOk;
    }    

// Methods:
private:

    // Get value overloads
    void GetValue(int vColumnIndex, int &vValue) const;    
    void GetValue(int vColumnIndex, gtUInt32 &vValue) const;
    void GetValue(int vColumnIndex, gtUInt64 &vValue) const;
    void GetValue(int vColumnIndex, gtFloat32 &vValue) const;
    void GetValue(int vColumnIndex, double &vValue) const;
    void GetValue(int vColumnIndex, gtVector<gtUByte> &vValue) const;
    void GetValue(int vColumnIndex, gtString &vValue) const;

    // bind values overloads. More types can be added if necessary
    status BindValue(const char *vColumnName, const gtString &vValue) const;
    status BindValue(const char *vColumnName, int vValue) const;    
    status BindValue(const char *vColumnName, gtUInt32 vValue) const;
    status BindValue(const char *vColumnName, gtUInt64 vValue) const;
    status BindValue(const char *vColumnName, double vValue) const;
    status BindValue(const char *vColumnName, float vValue) const;
    status BindValue(const char *vColumnName, const gtVector<gtUByte> &vValue) const;    

    // Error handling
    void AppendSQLiteErrorMessage();

// Attributes:
private:
    sqlite3_stmt* m_stmt = nullptr;
    status m_bBindsAreOk = success;
    gtString m_ErrorMsg;

#if defined ENABLE_QUERY_PROFILING
    std::chrono::time_point<std::chrono::system_clock> m_queryStartTime;
#endif
};


/// @brief  This is a utility class to deal with sqlite3 opaque objects.
/// 
/// SQLite C API is too verbose. This class has utilities to deal with
/// SQL database connections in terms preparing statements.
///
/// In case of fail, the user can get the error message by calling
/// GetErrorMessage()
/// 
/// @warning    This object is meant to be used in local scopes.
/// @date       30/08/2017
/// @author     Thales Sabino
class SQLiteDatabaseGuard
{
// Methods:
public:
    explicit SQLiteDatabaseGuard(sqlite3* pConnection);

    // Disable copy of this object
    SQLiteDatabaseGuard(const SQLiteDatabaseGuard&) = delete;
    SQLiteDatabaseGuard(SQLiteDatabaseGuard&&) = delete;

    // Prepare a sqlite3_stmt
    sqlite3_stmt* Prepare(const std::string &vpSql);
    status Finalize(sqlite3_stmt *vpStmt);

    // Database generic query functions
    status GetAllTableNames(gtVector<gtString> &vTableNames) const;
    status GetTableColumnNames(const gtString &vTableName, gtVector<gtString> &vTableColumns) const;
    status GetTableData(const gtString &vTableName, gtVector<gtVector<gtString>> &vTableData) const;

    // Error handling
    bool IsTransactionActive() const;
    bool IsValid() const;    
    bool HasErrors() const;
    const gtString& GetErrorMessage() const;

// Methods:
private:
    void AppendSQLiteErrorMessage();
    status PrepareInternal(const std::string &sqlStr, sqlite3_stmt **vpStmt) const;

// Attributes:
private:
    sqlite3 *m_pConnection = nullptr;
    gtString m_ErrorMsg;
};
// clang-format on

} // namespace db
} // namespace lpgpu2

#endif // LPGPU2_DB_DATABASE_UTILS_H_INCLUDE