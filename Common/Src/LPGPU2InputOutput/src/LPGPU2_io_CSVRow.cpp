// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2InputOutput project is a collection of classes to allow
///         data to be imported/exported to/from CodeXL. The goal is to
///         have an independent library that can be used accross all projects
///         that need access to external data.
///
/// CSVRow interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2InputOutput/Include/LPGPU2_io_CSVRow.h>

namespace lpgpu2 {
namespace io {

/// @brief              Class constructor. Store the reference of vHeader to 
///                     be used for referencing when storing data into the row.
/// @param[in]  vHeader A reference to a list of gtStrings used for storing
///                     data into the row in the correct place.
CSVRow::CSVRow(const gtVector<gtString> &vHeader)
: m_header(vHeader)
{
}

/// @brief               Stores vValue for this row at column vColumn.
/// @param[in]  vColumn  Column to store the value to.
/// @param[in]  vValue   Actual value to store in this row.
/// @return     status   success = The value was added to column for this row,
///                      failure = An error has ocurred and the row remains intact.
status CSVRow::SetRowData(const gtString &vColumn, const gtString &vValue)
{
    try
    {
        m_rowData[vColumn] = vValue;
    }
    catch (...)
    {
        return failure;
    }

    return success;
}

/// @brief              Tries to store all the data from vRowData into this row.
///                     Note that vRowData mustn't contain more fields than columns
///                     associate with this row.
/// @param[in] vRowData List of values to add to this row.
/// @returns   status   success = All the data from vRowData was inserted into this row.
///                     failure = vRowData has more data than this row supports.
status CSVRow::SetRowData(const gtVector<gtString> &vRowData)
{
    status bReturn = failure;

    if (vRowData.size() <= m_header.size())
    {
        // It's fine to have incomplete row data
        for (size_t iColumnIndex = 0; iColumnIndex < vRowData.size(); ++iColumnIndex)
        {
            const auto& columnName = m_header[iColumnIndex];
            const auto& columnValue = vRowData[iColumnIndex];

            SetRowData(columnName, columnValue);
        }

        bReturn = success;
    }

    return bReturn;
}

/// @brief              Queries this row the the value on column vColumn.
/// @param[in]  vColumn The name of the column to retrieve the value from.
/// @param[out] vValue  The value stored in the column.
/// @returnss   status  success = the row has the request data and returned it,
///                     failure = The data was not found. vValue was untouched.
status CSVRow::GetRowData(const gtString &vColumn, gtString &vValue) const
{
    status bReturn = failure;

    const auto it = m_rowData.find(vColumn);

    if (it != m_rowData.cend())
    {
        vValue = it->second;
        bReturn = success;
    }

    return bReturn;
}

} // namespace io
} // namespace lpgpu2