// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2InputOutput project is a collection of classes to allow
///         data to be imported/exported to/from CodeXL. The goal is to
///         have an independent library that can be used across all projects
///         that need access to external data.
///
/// CSVFileWriter implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2InputOutput/Include/LPGPU2_io_CSVFileWriter.h>

// STL:
#include <fstream>

namespace lpgpu2 {
namespace io {

/// @brief Class constructor. No work is done here.
/// @param[in]  vFilePath   The filepath to write this CSV on.
/// @param[in]  vbOverride  true = will override an existing file
///                         with the same name,
///                         false = returns false when trying to write the data
///                         in a file that already exists.
CSVFileWriter::CSVFileWriter(const osFilePath &vFilePath, bool vbOverride)
    : m_filePath(vFilePath)
    , m_bOverrideExistingFile(vbOverride)
{
}

/// @brief Class destructor. Flushes the data and closes the file.
CSVFileWriter::~CSVFileWriter()
{
    Close();
}

/// @brief      Flushes the data to the disk and resets the state of the file.
/// @returns    true = All the content stored was written to the disk,
///             false = An error has occurred on the write process.
status CSVFileWriter::Close()
{
    auto bReturn = failure;

    if (!m_bIsFileClosed)
    {
        bReturn = Flush();
        m_bWrittenHeaderAndColumnRow = false;        

        m_bIsFileClosed = true;
    }
    else
    {
        bReturn = success;
    }

    return bReturn;
}

/// @brief      Flushes the data to the disk and clear all the stored rows but does
///             not reset the state of file. This allows to flushes the data when 
///             necessary while keeping the file open to new insertions.
/// @returns    status success = All the content stored was written to the disk,
///                    failure = An error has occurred on the write process.
bool CSVFileWriter::Flush()
{
    status bReturn = failure;

    if (!m_bOverrideExistingFile && m_filePath.exists())
    {
        bReturn = failure;
        SetErrorMessage(L"Trying override an existing file. m_bOverrideExistingFile is false and the file already exists.");
    }
    else
    {   
        std::wofstream csvFileStream;
        
        // Need to check to open the file with the correct flags.
        // In the case where someone call flush twice, need to open the file with app.
        if (m_bOverrideExistingFile && m_filePath.exists() && !m_bWrittenHeaderAndColumnRow)
        {
            csvFileStream.open(m_filePath.asString().asASCIICharArray(), std::fstream::out);
        }
        else
        {
            csvFileStream.open(m_filePath.asString().asASCIICharArray(), std::fstream::app);
        }

        if (csvFileStream.fail())
        {
            bReturn = failure;
            SetErrorMessage(L"Cannot open file for writing.");
        }
        else
        {
            if (!m_bWrittenHeaderAndColumnRow)
            {
                WriteHeaderAndColumnRow(csvFileStream);
                m_bWrittenHeaderAndColumnRow = true;
            }

            WriteRows(csvFileStream);

            m_rows.clear();

            bReturn = success;
        }
    }

    return bReturn;
}

/// @brief                   Add a new column to the CSV file. This column
///                          can be used later on CSVRow::SetRowData() method.
/// @param[in]  vColumnName  Name of the column to add to the CSV file.
/// @return     status       success = The column was added,
///                          failure = The column already exists.
status CSVFileWriter::AddColumn(const gtString &vColumnName)
{
    status bReturn = failure;

    try
    {
        if (m_columnSet.find(vColumnName) == m_columnSet.end())
        {
            m_columns.push_back(vColumnName);
            m_columnSet.insert(vColumnName);
        }
    }
    catch (...)
    {
        bReturn = failure;
        SetErrorMessage(L"Error adding column to CSV");
    }

    return bReturn;    
}

/// @brief                    Add a list of columns to the CSV file. These columns
///                           can be used later on CSVRow::SetRowData() method.
/// @param[in]  vColumnNames  List of columns to add to the CSV file.
/// @return     status        success = All the columns were added,
///                           failure = An error has occurred when adding some of
///                                     the columns.
status CSVFileWriter::AddColumns(const gtVector<gtString> &vColumnNames)
{
    status bReturn = success;

    for (const auto& columnName : vColumnNames)
    {
        bReturn &= AddColumn(columnName);
    }

    return bReturn;
}

/// @brief                  Adds a header to the CSV file. The headers will be
///                         written as comments at the beginning of the CSV file.
/// @param[in] vHeaderName  Header to be added to the CSV file.
/// @return    status       success = The header was added to the CSV file,
///                         failure = An error has occurred when adding the header.
status CSVFileWriter::AddHeader(const gtString &vHeaderName)
{
    status bReturn = failure;

    try
    {
        m_headers.push_back(vHeaderName);
        bReturn = success;
    }
    catch (...)
    {
        bReturn = failure;
        SetErrorMessage(L"Error adding header to CSV");
    }

    return bReturn;
    
}

/// @brief                           Adds a new row to the CSV and returns
///                                  a managed pointer that can be used to 
///                                  add the data to the row.
/// @returns std::shared_ptr<CSVRow> A CSVRow pointer that the user can use
///                                  to add data to the CSV file.
/// @warning                         Will return nullptr in case of an 
///                                  allocation failure.
std::shared_ptr<CSVRow> CSVFileWriter::AddRow()
{
    try
    {
        auto csvRow = std::make_shared<CSVRow>(m_columns);
        m_rows.push_back(csvRow);
        return csvRow;
    }
    catch (...)
    {
        SetErrorMessage(L"Error creating another row to the CSV.");
    }

    return nullptr;
}

/// @brief                      Write the header, as a comment, and the column names
///                             at the top of the CSV file.
/// @param[in] vCsvFileStream   File stream where the headers and columns are going
///                             to be written.
/// @warning                    The file stream should be opened before calling this
///                             method or nothing will happen.
/// @return   status            Always returns success.
status CSVFileWriter::WriteHeaderAndColumnRow(std::wofstream &vCsvFileStream) const
{
    if (vCsvFileStream.is_open())
    {
        // Print headers
        for (const auto& header : m_headers)
        {
            vCsvFileStream << m_cCommentChar << header.asCharArray() << std::endl;
        }

        // Print Columns
        for (size_t iColumnIndex = 0; iColumnIndex < m_columns.size(); ++iColumnIndex)
        {
            vCsvFileStream << m_columns[iColumnIndex].asCharArray();

            if (iColumnIndex < m_columns.size() - 1)
            {
                vCsvFileStream << m_cSeparator << L" ";
            }
        }

        vCsvFileStream << std::endl;
    }   

    return success;
}

/// @brief                      Write the rows into the CSV file.
/// @param[in] vCsvFileStream   File stream where the rows are going to be written.
/// @warning                    The file stream should be opened before calling this
///                             method or nothing will happen.
/// @return   status            Always returns success.
status CSVFileWriter::WriteRows(std::wofstream &vCsvFileStream) const
{
    if (vCsvFileStream.is_open())
    {
        if (!m_rows.empty())
        {
            // Print rows
            for (const std::shared_ptr<CSVRow>& row : m_rows)
            {
                for (size_t iColumnIndex = 0; iColumnIndex < m_columns.size(); ++iColumnIndex)
                {
                    const gtString& column = m_columns[iColumnIndex];
                    gtString value;

                    if (row->GetRowData(column, value))
                    {
                        vCsvFileStream << value.asCharArray();

                        if (iColumnIndex < m_columns.size() - 1)
                        {
                            vCsvFileStream << m_cSeparator << L" ";
                        }
                    }
                }

                vCsvFileStream << std::endl;
            }
        }        
    }    

    return success;
}

} // namespace io
} // namespace lpgpu2
