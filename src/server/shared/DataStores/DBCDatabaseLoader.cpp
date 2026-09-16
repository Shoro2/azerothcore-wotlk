/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DBCDatabaseLoader.h"
#include "DatabaseEnv.h"
#include "Errors.h"
#include "Log.h"
#include "QueryResult.h"
#include "StringFormat.h"

DBCDatabaseLoader::DBCDatabaseLoader(char const* tableName, char const* dbcFormatString, std::vector<char*>& stringPool)
    : _sqlTableName(tableName),
      _dbcFormat(dbcFormatString),
      _sqlIndexPos(0),
      _recordSize(0),
      _stringPool(stringPool)
{
    // Get sql index position
    int32 indexPos = -1;
    _recordSize = DBCFileLoader::GetFormatRecordSize(_dbcFormat, &indexPos);
    if (indexPos >= 0)
        _sqlIndexPos = indexPos;

    ASSERT(_recordSize);
}

char* DBCDatabaseLoader::Load(uint32& records, char**& indexTable)
{
    std::string query = Acore::StringFormat("SELECT * FROM `{}` ORDER BY `ID` DESC", _sqlTableName);

    // no error if empty set
    QueryResult result = WorldDatabase.Query(query);
    if (!result)
        return nullptr;

    // Check if sql index pos is valid
    if (int32(result->GetFieldCount() - 1) < _sqlIndexPos)
    {
        ASSERT(false, "Invalid index pos for dbc: '{}'", _sqlTableName);
        return nullptr;
    }

    std::unique_ptr<char[]> dataTable = std::make_unique<char[]>(result->GetRowCount() * _recordSize);
    std::unique_ptr<uint32[]> newIndexes = std::make_unique<uint32[]>(result->GetRowCount());
    uint32 newRecords = 0;
    uint32 maxIndexValue = 0;

    // Insert sql data into the data array
    // the index table is resized only after all rows are read, do not access it in this loop
    do
    {
        Field* fields = result->Fetch();
        uint32 indexValue = fields[_sqlIndexPos].Get<uint32>();
        maxIndexValue = std::max(maxIndexValue, indexValue);

        // If exist in DBC file override from DB
        newIndexes[newRecords] = indexValue;
        char* dataValue = &dataTable[newRecords++ * _recordSize];

        uint32 dataOffset = 0;
        uint32 sqlColumnNumber = 0;
        char const* dbcFormat = _dbcFormat;

        for (; (*dbcFormat); ++dbcFormat)
        {
            switch (*dbcFormat)
            {
                case FT_FLOAT:
                    *reinterpret_cast<float*>(&dataValue[dataOffset]) = fields[sqlColumnNumber].Get<float>();
                    dataOffset += sizeof(float);
                    break;
                case FT_IND:
                case FT_INT:
                    *reinterpret_cast<uint32*>(&dataValue[dataOffset]) = fields[sqlColumnNumber].Get<uint32>();
                    dataOffset += sizeof(uint32);
                    break;
                case FT_BYTE:
                    *reinterpret_cast<uint8*>(&dataValue[dataOffset]) = fields[sqlColumnNumber].Get<uint8>();
                    dataOffset += sizeof(uint8);
                    break;
                case FT_STRING:
                    *reinterpret_cast<char**>(&dataValue[dataOffset]) = CloneStringToPool(fields[sqlColumnNumber].Get<std::string>());
                    dataOffset += sizeof(char*);
                    break;
                case FT_SORT:
                case FT_NA:
                case FT_NA_BYTE:
                    break;
                default:
                    ASSERT(false, "Unsupported data type '{}' in table '{}'", *dbcFormat, _sqlTableName);
                    return nullptr;
            }

            ++sqlColumnNumber;
        }

        ASSERT(sqlColumnNumber == result->GetFieldCount(), "SQL format string does not match database for table: '{}'", _sqlTableName);
        ASSERT(dataOffset == _recordSize);
    } while (result->NextRow());

    ASSERT(newRecords == result->GetRowCount());

    // Resize index table
    // rows are ordered by `ID`, which is not the index field of every format (e.g. CurrencyTypes),
    // so the size comes from the highest index value of all rows, not from the first row
    uint32 indexTableSize = std::max(records, maxIndexValue + 1);
    if (indexTableSize > records)
    {
        char** tmpIdxTable = new char* [indexTableSize];
        memset(tmpIdxTable, 0, indexTableSize * sizeof(char*));
        memcpy(tmpIdxTable, indexTable, records * sizeof(char*));
        delete[] indexTable;
        indexTable = tmpIdxTable;
    }

    // insert new records to index table
    for (uint32 i = 0; i < newRecords; ++i)
    {
        // only reachable if the highest index value is the uint32 maximum (size overflow)
        if (newIndexes[i] >= indexTableSize)
        {
            LOG_ERROR("sql.sql", "Table `{}` has a row with index value {} outside of the index table (size {}), skipped.", _sqlTableName, newIndexes[i], indexTableSize);
            continue;
        }

        // cppcheck-suppress autoVariables
        indexTable[newIndexes[i]] = &dataTable[i * _recordSize];
    }

    records = indexTableSize;

    return dataTable.release();
}

char* DBCDatabaseLoader::CloneStringToPool(std::string const& str)
{
    char* buf = new char[str.size() + 1];
    memcpy(buf, str.c_str(), str.size() + 1);
    _stringPool.push_back(buf);
    return buf;
}
