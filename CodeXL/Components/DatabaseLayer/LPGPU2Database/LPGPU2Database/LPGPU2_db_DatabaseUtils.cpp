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

// Local:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseUtils.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// SQLite3
#include <sqlite3.h>

namespace lpgpu2 {
namespace db {

/// @brief                      Retrieves a formatted error message from an sqlite3 connection
/// @param[in]  pConnection     The sqlite3 connection
/// @returns    gtString        A string with the the SQLite error message
static gtString GetSQLiteErrorMessage(sqlite3* pConnection)
{
    if (pConnection != nullptr)
    {
        int errCode = sqlite3_errcode(pConnection);
        if (errCode != SQLITE_OK)
        {
            gtString sqliteErrStr;
            sqliteErrStr << sqlite3_errstr(errCode) << L" : " << sqlite3_errmsg(pConnection);

            return sqliteErrStr;
        }
    }

    return gtString{};
}

/// @brief              Class constructor. Just checks if statement is valid and stores it
/// @param[in] stmt     SQLite3 statement where the operations are going to be performed
SQLiteStatementGuard::SQLiteStatementGuard(sqlite3_stmt *stmt):
    m_stmt(stmt)
{
    GT_ASSERT_EX(stmt != nullptr, L"Invalid Statement");

#if defined ENABLE_QUERY_PROFILING
    m_queryStartTime = std::chrono::system_clock::now();
#endif
}

/// @brief  Class destructor. Resets the statement upon destruction.
///         Uses the RAII pattern
SQLiteStatementGuard::~SQLiteStatementGuard()
{
#if defined ENABLE_QUERY_PROFILING
    if (IsValid())
    {        
        const auto *sql = sqlite3_sql(m_stmt);
        const auto now = std::chrono::system_clock::now();
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_queryStartTime);

        if (elapsedMs > std::chrono::milliseconds{ 15 })
        {
            gtString log;
            log << L"(" << static_cast<unsigned long>(elapsedMs.count()) << L" ms) Query: " << sql;
            log.replace(L"\n", L" ");

            OS_OUTPUT_DEBUG_LOG(log.asCharArray(), OS_DEBUG_LOG_INFO);
        }        
    }
#endif
    
    if (SQLITE_OK != sqlite3_reset(m_stmt))
    {
        AppendSQLiteErrorMessage();
        GT_ASSERT_EX(false, GetErrorMessage().asCharArray());
    }
}

/// @brief          Returns if the statement is valid and if 
///                 all the binds performed are ok.
/// @returns bool   true if the statement is valid and if 
///                 there is no error on bindings so far.
bool SQLiteStatementGuard::IsValid() const
{
    return m_stmt != nullptr && m_bBindsAreOk;
}

/// @brief          While this returns true there is another row
///                 to fetch from the current statement execution
/// @returns bool   true if there is another row to be fetched, false
///                 if there are not more results to fetch.
bool SQLiteStatementGuard::Step() const
{
    bool bReturn = false;

    if (IsValid())
    {
        const int rc = sqlite3_step(m_stmt);
        bReturn = (rc == SQLITE_ROW);
    }

    return bReturn;
}

/// @brief          Executes the statement and checks if SQLite returns
///                 SQLITE_DONE (no more rows). This is meant to be used
///                 by insert statements and other single shot statements
/// @returns status success if SQLite returns SQLITE_DONE, failure otherwise
status SQLiteStatementGuard::Exec()
{
    status bReturn = false;

    if (IsValid())
    {
        const int rc = sqlite3_step(m_stmt);
        bReturn = (rc == SQLITE_DONE);

        if (!bReturn)
        {
            AppendSQLiteErrorMessage();
        }
    }

    return bReturn;
}

/// @brief          Finalizes the current statement and checks for errors
/// @return status  success if sqlite3_finalize returns SQLITE_OK, false otherwise.
status SQLiteStatementGuard::Finalize()
{
    status bReturn = (SQLITE_OK == sqlite3_finalize(m_stmt));

    if (!bReturn)
    {
        AppendSQLiteErrorMessage();
    }

    m_stmt = nullptr;
    return bReturn;
}

/// @brief          Convenience function to call Exec() and Finalize() on a statement.
/// @return status  success if both sqlite3_finalize and sqlite3_prepare_v2 returns SQLITE_OK, false otherwise.
status SQLiteStatementGuard::ExecAndFinalize()
{
    return Exec() && Finalize();
}

/// @brief               Returns the formatted error message generated
///                      by this statement operations.
/// @returns    gtString The current formatted error message.
const gtString& SQLiteStatementGuard::GetErrorMessage() const
{
    return m_ErrorMsg;
}

/// @brief                   Gets an int value at columnIndex.
/// @param[in]  vColumnIndex Index of the column where to get the value.
/// @param[out] vValue       The value of the column requested as an int.
void SQLiteStatementGuard::GetValue(int vColumnIndex, int &vValue) const
{
    vValue = sqlite3_column_int(m_stmt, vColumnIndex);
}

/// @brief                  Gets a gtUInt32 value at vColumnIndex.
/// @param[in] vColumnIndex Index of the column where to get the value.
/// @param[out] vValue      The value of the column requested as gtUInt32.
void SQLiteStatementGuard::GetValue(int vColumnIndex, gtUInt32 &vValue) const
{
    vValue = sqlite3_column_int(m_stmt, vColumnIndex);
}

/// @brief                  Gets a gtUInt64 value at vColumnIndex.
/// @param[in] vColumnIndex Index of the column where to get the value.
/// @param[out] vValue      The value of the column requested as gtUInt64.
void SQLiteStatementGuard::GetValue(int vColumnIndex, gtUInt64& vValue) const
{
    vValue = sqlite3_column_int64(m_stmt, vColumnIndex);
}

/// @brief                   Gets a gtFloat32 value at columnIndex
/// @param[in]  vColumnIndex Index of the column where to get the value
/// @param[out] vValue       The value of the column requested as gtFloat32.
void SQLiteStatementGuard::GetValue(int vColumnIndex, gtFloat32 &vValue) const
{
    vValue = static_cast<gtFloat32>(sqlite3_column_double(m_stmt, vColumnIndex));
}

/// @brief                  Gets a double value at columnIndex
/// @param[in]  columnIndex Index of the column where to get the value
/// @param[out] vValue      The value of the column requested as a double.
void SQLiteStatementGuard::GetValue(int vColumnIndex, double &vValue) const
{
    vValue = sqlite3_column_double(m_stmt, vColumnIndex);
}

/// @brief                   Gets a blob as a gtVector<gtUByte> at columnIndex
/// @param[in]  vColumnIndex Index of the column where to get the value
/// @param[out] vValue       The value of the column requested as a gtVector<gtUBype>.
void SQLiteStatementGuard::GetValue(int vColumnIndex, gtVector<gtUByte> &vValue) const
{
    const void* pRawBlob = sqlite3_column_blob(m_stmt, vColumnIndex);
    const int blobSizeBytes = sqlite3_column_bytes(m_stmt, vColumnIndex);
    if (pRawBlob != nullptr)
    {
        const gtUByte* pBlob = reinterpret_cast<const gtUByte*>(pRawBlob);        
        vValue.assign(pBlob, pBlob + blobSizeBytes);        
    }
}

/// @brief                   Gets a text as a gtString at columnIndex
/// @param[in]  vColumnIndex Index of the column where to get the value
/// @param[out] vValue       The value of the column requested as a gtString.
void SQLiteStatementGuard::GetValue(int vColumnIndex, gtString &vValue) const
{
    const unsigned char* pRawText = sqlite3_column_text(m_stmt, vColumnIndex);
    if (pRawText != nullptr)
    {
        vValue.fromUtf8String(reinterpret_cast<const char*>(pRawText));
    }
}

/// @brief                   Binds a gtString into a named field
/// @param[in]  vColumnName  Name of the field to bind the value
/// @param[in]  vValue       Value to bind into the field
/// @returns    status       success if the bind was successful, failure otherwise
status SQLiteStatementGuard::BindValue(const char *vColumnName, const gtString &vValue) const
{
    status bReturn = failure;

    const int columnIndex = sqlite3_bind_parameter_index(m_stmt, vColumnName);
    GT_IF_WITH_ASSERT_EX(columnIndex > 0, L"No matching parameter in query")
    {
        std::string valueStr{ vValue.asASCIICharArray() };
        int rc = sqlite3_bind_text(m_stmt, columnIndex, valueStr.data(), valueStr.size(), SQLITE_TRANSIENT);
        bReturn = (rc == SQLITE_OK) ? success : failure;
    }

    return bReturn;
}

/// @brief                  Binds a int into a named field
/// @param[in]  vColumnName Name of the field to bind the value
/// @param[in]  vValue      Value to bind into the field
/// @returns    status      success if the bind was successful, failure otherwise
status SQLiteStatementGuard::BindValue(const char *vColumnName, int vValue) const
{
    status bReturn = failure;

    const int columnIndex = sqlite3_bind_parameter_index(m_stmt, vColumnName);
    GT_IF_WITH_ASSERT_EX(columnIndex > 0, L"No matching parameter in query")
    {
        const int rc = sqlite3_bind_int(m_stmt, columnIndex, vValue);
        bReturn = (rc == SQLITE_OK) ? success : failure;
    }

    return bReturn;
}

/// @brief                  Binds a gtUInt32 into a named field
/// @param[in]  vColumnName Name of the field to bind the value
/// @param[in]  vValue      Value to bind into the field
/// @returns    status      success if the bind was successful, failure otherwise
status SQLiteStatementGuard::BindValue(const char *vColumnName, gtUInt32 vValue) const
{
    return BindValue(vColumnName, static_cast<gtUInt64>(vValue));
}

/// @brief                   Binds a gtUInt64 into a named field
/// @param[in]  vColumnName  Name of the field to bind the value
/// @param[in]  vValue       Value to bind into the field
/// @returns    status       success if the bind was successful, failure otherwise
status SQLiteStatementGuard::BindValue(const char *vColumnName, gtUInt64 vValue) const
{
    status bReturn = failure;

    int columnIndex = sqlite3_bind_parameter_index(m_stmt, vColumnName);
    GT_IF_WITH_ASSERT_EX(columnIndex > 0, L"No matching parameter in query")
    {
        int rc = sqlite3_bind_int64(m_stmt, columnIndex, vValue);
        bReturn = (rc == SQLITE_OK) ? success : failure;
    }

    return bReturn;
}

/// @brief                  Binds a double into a named field
/// @param[in]  vColumnName Name of the field to bind the value
/// @param[in]  vValue      Value to bind into the field
/// @returns    status      success if the bind was successful, failure otherwise
status SQLiteStatementGuard::BindValue(const char *vColumnName, double vValue) const
{
    status bReturn = failure;

    int columnIndex = sqlite3_bind_parameter_index(m_stmt, vColumnName);
    GT_IF_WITH_ASSERT_EX(columnIndex > 0, L"No matching parameter in query")
    {
        int rc = sqlite3_bind_double(m_stmt, columnIndex, vValue);
        bReturn = (rc == SQLITE_OK) ? success : failure;
    }

    return bReturn;
}

/// @brief                 Binds a float into a named field
/// @param[in]  vColumnName Name of the field to bind the value
/// @param[in]  vValue      Value to bind into the field
/// @returns    status       success if the bind was successful, failure otherwise
status SQLiteStatementGuard::BindValue(const char *vColumnName, float vValue) const
{
    return BindValue(vColumnName, static_cast<double>(vValue));
}

/// @brief                 Binds a gtVector<gtUByte> (blob) into a named field
/// @param[in]  vColumnName Name of the field to bind the value
/// @param[in]  vValue      Value to bind into the field
/// @returns    status       success if the bind was successful, failure otherwise
bool SQLiteStatementGuard::BindValue(const char *vColumnName, const gtVector<gtUByte> &vValue) const
{
    status bReturn = failure;

    int columnIndex = sqlite3_bind_parameter_index(m_stmt, vColumnName);
    GT_IF_WITH_ASSERT_EX(columnIndex > 0, L"No matching parameter in query")
    {
        int rc = sqlite3_bind_blob(m_stmt, columnIndex, vValue.data(), vValue.size() * sizeof(gtUByte), SQLITE_TRANSIENT);
        bReturn = (rc == SQLITE_OK) ? success : failure;
    }

    return bReturn;
}

/// @brief  Appends a SQLite error message into 
///         the current SQLiteStatementGuard's state
void SQLiteStatementGuard::AppendSQLiteErrorMessage()
{
    sqlite3* pConnection = sqlite3_db_handle(m_stmt);
    if (pConnection != nullptr)
    {
        m_ErrorMsg << GetSQLiteErrorMessage(pConnection) << L"\n";
    }
    else
    {
        m_ErrorMsg << L"Invalid database connection \n";
    }
}

/// @brief                  Class Constructor. Checks if the connection is valid
///                         and stores the pointer to the SQLite connection
/// @param[in]  pConnection SQLite database connection
SQLiteDatabaseGuard::SQLiteDatabaseGuard(sqlite3 *pConnection):
    m_pConnection(pConnection)
{    
}

/// @brief                    Prepares and returns a SQLite statement. Stores
///                           an error message in case something goes wrong.
/// @param[in]  pSql          SQL statement to be prepared with sqlite3_prepare_v2
/// @returns    sqlite3_stmt* Pointer to a prepared sqlite3 statement
sqlite3_stmt* SQLiteDatabaseGuard::Prepare(const std::string &vSqlStr)
{
    sqlite3_stmt *pStmt = nullptr;
    if (PrepareInternal(vSqlStr, &pStmt) == success)
    {
        return pStmt;
    }
    else
    {
        AppendSQLiteErrorMessage();

        if (sqlite3_finalize(pStmt) != SQLITE_OK)
        {
            AppendSQLiteErrorMessage();
        }
    }

    return nullptr;
}

status SQLiteDatabaseGuard::Finalize(sqlite3_stmt *vpStmt)
{        
    status bReturn = failure;

    if (vpStmt != nullptr && sqlite3_db_handle(vpStmt) == m_pConnection)
    {
        if (sqlite3_finalize(vpStmt) != SQLITE_OK)
        {
            AppendSQLiteErrorMessage();
        }
        else
        {
            bReturn = success;
        }
    }

    return bReturn;
}

/// @brief                      Get the list of all the tables in this database.
///                             This will actually query SQLite sqlite_master
///                             hidden table.
/// @returns gtVector<gtString> A list of all database table names.
/// @warning                    returns an empty list in case something goes wrong.
status SQLiteDatabaseGuard::GetAllTableNames(gtVector<gtString> &vTableNames) const
{
    sqlite3_stmt *pGetTablesStmt = nullptr;
    status bReturn = PrepareInternal("SELECT name from sqlite_master WHERE type='table'", &pGetTablesStmt);

    if (bReturn == success)
    {
        SQLiteStatementGuard stmtGuard{ pGetTablesStmt };

        if (stmtGuard.IsValid())
        {
            while (stmtGuard.Step())
            {
                vTableNames.emplace_back(stmtGuard.Get<gtString>(0));

                // If at least one table is found consider a success
                bReturn = success;
            }
        }        
    }

    return bReturn;
}

/// @brief                          Get a list of table column names. This will
///                                 use PRAGMA table_info() SQLite command to query
///                                 information about the table.
/// @param[in]  vTableName          Name of the table to query for column names.
/// @returns    gtVector<gtString>  A list of table's column names.
/// @see                            <a href="https://sqlite.org/pragma.html#pragma_table_info">SQLite PRAGMA table_info()</a>
status SQLiteDatabaseGuard::GetTableColumnNames(const gtString &vTableName, gtVector<gtString> &vTableColumns) const
{    
    gtString pragmaTableInfoQuery;
    pragmaTableInfoQuery << L"PRAGMA table_info(" << vTableName << L")";

    sqlite3_stmt *pGetTableInfoStmt = nullptr;
    status bReturn = PrepareInternal(pragmaTableInfoQuery.asASCIICharArray(), &pGetTableInfoStmt);

    if (bReturn == success)
    {
        SQLiteStatementGuard stmtGuard{ pGetTableInfoStmt };

        if (stmtGuard.IsValid())
        {
            while (stmtGuard.Step())
            {
                // Since there is no way to SELECT the PRAGMA info table, we have to get the
                // value for the column name from the column number 1
                vTableColumns.emplace_back(stmtGuard.Get<gtString>(1));

                // If at least one column is found consider a success
                bReturn = success;
            }
        }
    }    

    return bReturn;
}

/// @brief                 Get all the data from the table vTableName as gtStrings.
/// @param[in] vTableName  Name of the table to query the data for.
/// @returns   gtVector<gtVector<gtString>> A list where each line has the contents of a
///                                         row. The order of the columns will be the same
///                                         as the one returned by 
///                                         SQLiteDatabaseGuard::GetTableColumnNames().
/// @see SQLiteDatabaseGuard::GetTableColumnNames().
status SQLiteDatabaseGuard::GetTableData(const gtString &vTableName, gtVector<gtVector<gtString>> &vTableData) const
{
    // Starts by getting all column names for the requested table
    gtVector<gtString> tableColumnNames;
    status bReturn = GetTableColumnNames(vTableName, tableColumnNames);

    if (bReturn == success)
    {
        // Then builds the SELECT query to ensure the order from the above call
        gtString querySql = L"SELECT ";

        for (size_t iColumnNameIndex = 0; iColumnNameIndex < tableColumnNames.size(); ++iColumnNameIndex)
        {
            querySql << tableColumnNames[iColumnNameIndex];

            if (iColumnNameIndex < tableColumnNames.size() - 1)
            {
                querySql << ",";
            }
        }

        querySql << L" FROM " << vTableName;

        // Finally, the statement is prepared and we get the data
        sqlite3_stmt *pGetTableDataStmt = nullptr;
        bReturn = PrepareInternal(querySql.asASCIICharArray(), &pGetTableDataStmt);

        SQLiteStatementGuard stmtGuard{ pGetTableDataStmt };

        if (stmtGuard.IsValid())
        {            
            while (stmtGuard.Step())
            {
                gtVector<gtString> rowData;                
                for (size_t i = 0; i < tableColumnNames.size(); ++i)
                {
                    rowData.emplace_back(stmtGuard.Get<gtString>(i));
                }

                vTableData.push_back(rowData);

                // If at least one row is obtained consider a success
                bReturn = success;
            }            
        }
    }

    return bReturn;
}

/// @brief  Checks if the current SQLite connection is inside a
///         transaction.
/// @returns bool   true = The connection is inside a transaction,
///                 false = The connection is not inside a transaction.
bool SQLiteDatabaseGuard::IsTransactionActive() const
{
    auto bReturn = false;

    if (IsValid())
    {
        bReturn = sqlite3_get_autocommit(m_pConnection) == 0;
    }

    return bReturn;
}

/// @brief         Checks if the connection is valid or not
/// @returns  bool true if the connection is valid (not nullptr), false otherwise
bool SQLiteDatabaseGuard::IsValid() const
{
    return m_pConnection != nullptr;
}

/// @brief         Convenience function to check if there are 
///                any errors stored in this SQLiteDatabaseGuard
/// @returns  bool true if there are errors stored, false otherwise
bool SQLiteDatabaseGuard::HasErrors() const
{
    return !m_ErrorMsg.isEmpty();
}

/// @brief            Gets the current error message
/// @returns gtString The current error message, if any
const gtString &SQLiteDatabaseGuard::GetErrorMessage() const
{
    return m_ErrorMsg;
}

/// @brief  Append the latest SQLite error to the current state of this database guard
void SQLiteDatabaseGuard::AppendSQLiteErrorMessage()
{
    m_ErrorMsg << GetSQLiteErrorMessage(m_pConnection) << L"\n";
}

status SQLiteDatabaseGuard::PrepareInternal(const std::string &vSqlStr, sqlite3_stmt **vpStmt) const
{
    status bReturn = false;

    if (IsValid() && !vSqlStr.empty())
    {        
        int rc = sqlite3_prepare_v2(m_pConnection, vSqlStr.data(), -1, vpStmt, nullptr);
        GT_IF_WITH_ASSERT_EX(rc == SQLITE_OK, GetSQLiteErrorMessage(m_pConnection).asCharArray())
        {
            bReturn = success;
        }
    }

    return bReturn;
}

} // namespace db
} // namespace lpgpu2