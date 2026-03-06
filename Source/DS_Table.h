/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*  DS_Table.h */


#pragma once
#ifdef _MSC_VER
#pragma warning( push )
#endif

#include "DS_List.h"
#include "DS_BPlusTree.h"
#include "RakMemoryOverride.h"
#include "Export.h"
#include "RakString.h"

#define _TABLE_BPLUS_TREE_ORDER 16
#define _TABLE_MAX_COLUMN_NAME_LENGTH 64

/*
 * DataStructures 命名空间的添加仅是为了避免常见数据结构名称导致的编译器错误
 * 由于这些数据结构是独立的，如果需要，你可以在 RakNet 之外将它们用于自己的项目。
 */
namespace DataStructures
{

	/*
	 * Holds a set of columns, a set of rows, and rows times columns cells.
	 * The table data structure is useful if you want to store a set of structures and perform queries on those structures.<BR>
	 * This is a relatively simple and fast implementation of the types of tables commonly used in databases.<BR>
	 * See TableSerializer to serialize data members of the table.<BR>
	 * See LightweightDatabaseClient and LightweightDatabaseServer to transmit the table over the network.
	 */
	class RAK_DLL_EXPORT Table
	{
	public:
			
		enum ColumnType
		{
			/* Cell::i used */
			NUMERIC,

			/* Cell::c used to hold a 以空字符结尾的字符串. */
			STRING,

			/* Cell::c holds data.  Cell::i holds data length of c in bytes. */
			BINARY,

			/* Cell::c holds data.  Not deallocated. Set manually by assigning ptr. */
			POINTER,
		};
		
		
		/* Holds the actual data in the table */
		/* 注意：如果修改了此结构体，swig 文件中的对应结构体也需要同步修改 */
		struct RAK_DLL_EXPORT Cell
		{
			Cell();
			~Cell() noexcept;
			Cell(double numericValue, char *charValue, void *ptr, ColumnType type);
			void SetByType(double numericValue, char *charValue, void *ptr, ColumnType type);
			void Clear();
			
			/* 数值 */
			void Set(int input);
			void Set(unsigned int input);
			void Set(double input);

			/* 字符串 */
			void Set(const char *input);

			/* 二进制 */
			void Set(const char *input, int inputLength);

			/* Pointer */
			void SetPtr(void* p);

			/* 数值 */
			void Get(int *output);
			void Get(double *output);

			/* 字符串 */
			void Get(char *output);

			/* 二进制 */
			void Get(char *output, int *outputLength);

			RakNet::RakString ToString(ColumnType columnType);

			/* 赋值运算符 and copy 构造函数 */
			Cell& operator = ( const Cell& input );
			Cell( const Cell & input);

			ColumnType EstimateColumnType(void) const;

			bool isEmpty;
			double i;
			char *c;
			void *ptr;
		};

		/*
		 * Stores the name and type of the column
		 * 内部使用
		 */
		/* 注意：如果修改了此结构体，swig 文件中的对应结构体也需要同步修改 */
		struct RAK_DLL_EXPORT ColumnDescriptor
		{
			ColumnDescriptor();
			~ColumnDescriptor() noexcept;
			ColumnDescriptor(const char cn[_TABLE_MAX_COLUMN_NAME_LENGTH],ColumnType ct);

			char columnName[_TABLE_MAX_COLUMN_NAME_LENGTH];
			ColumnType columnType;
		};

		/* Stores the list of cells for this row, and a special flag used for 内部使用 sorting */
		/* 注意：如果修改了此结构体，swig 文件中的对应结构体也需要同步修改 */
		struct RAK_DLL_EXPORT Row
		{
			/* list of cells */
			DataStructures::List<Cell*> cells;

			/* 数值 */
			void UpdateCell(unsigned columnIndex, double value);

			/* 字符串 */
			void UpdateCell(unsigned columnIndex, const char *str);

			/* 二进制 */
			void UpdateCell(unsigned columnIndex, int byteLength, const char *data);
		};
		
		/* Operations to perform for cell comparison */
		enum FilterQueryType
		{
			QF_EQUAL,
			QF_NOT_EQUAL,
			QF_GREATER_THAN,
			QF_GREATER_THAN_EQ,
			QF_LESS_THAN,
			QF_LESS_THAN_EQ,
			QF_IS_EMPTY,
			QF_NOT_EMPTY,
		};

		/* Compare the cell value for a row at columnName to the cellValue using operation. */
		/* 注意：如果修改了此结构体，swig 文件中的对应结构体也需要同步修改 */
		struct RAK_DLL_EXPORT FilterQuery
		{
			FilterQuery();
			~FilterQuery() noexcept;
			FilterQuery(unsigned column, Cell *cell, FilterQueryType op);

			/* If columnName is specified, columnIndex will be looked up using it. */
			char columnName[_TABLE_MAX_COLUMN_NAME_LENGTH];
			unsigned columnIndex;
			Cell *cellValue;
			FilterQueryType operation;
		};

		/* Increasing or decreasing sort order */
		enum SortQueryType
		{
			QS_INCREASING_ORDER,
			QS_DECREASING_ORDER,
		};
		
		/* Sort on increasing or decreasing order for a particular column */
		/* 注意：如果修改了此结构体，swig 文件中的对应结构体也需要同步修改 */
		struct RAK_DLL_EXPORT SortQuery
		{
			/* The index of the table column we are sorting on */
			unsigned columnIndex;

			/* 参见 SortQueryType */
			SortQueryType operation;
		};

		/* 构造函数 */
		Table();

		/* 析构函数 */
		~Table() noexcept;

		/*
		 * Adds a column to the table
		 * 参数[输入] columnName The name of the column
		 * 参数[输入] columnType What type of data this column will hold
		 * 返回值: The index of the new column
		 */
		unsigned AddColumn(const char columnName[_TABLE_MAX_COLUMN_NAME_LENGTH], ColumnType columnType);

		/*
		 * 移除 column by index
		 * 参数[输入] columnIndex The index of the column to remove
		 */
		void RemoveColumn(unsigned columnIndex);

		/*
		 * 获取 index of a column by name
		 * Column indices are stored in the order they are added.
		 * 参数[输入] columnName The name of the column
		 * 返回值: The index of the column, or (unsigned)-1 if no such column
		 */
		unsigned ColumnIndex(char columnName[_TABLE_MAX_COLUMN_NAME_LENGTH]) const;
		unsigned ColumnIndex(const char *columnName) const;

		/*
		 * Gives the string name of the column at a certain index
		 * 参数[输入] index The index of the column
		 * 返回值: The name of the column, or 0 if an invalid index
		 */
		char* ColumnName(unsigned index) const;

		/*
		 * 返回 type of a column, referenced by index
		 * 参数[输入] index The index of the column
		 * 返回值: The type of the column
		 */
		ColumnType GetColumnType(unsigned index) const;

		/*
		 * 返回 number of columns
		 * 返回值: The number of columns in the table
		 */
		unsigned GetColumnCount(void) const;

		/*
		 * 返回 number of rows
		 * 返回值: The number of rows in the table
		 */
		unsigned GetRowCount(void) const;

		/*
		 * Adds a row to the table
		 * New rows are added with empty values for all cells.  However, if you specify initialCelLValues you can specify initial values
		 * It's up to you to ensure that the values in the specific cells match the type of data used by that row
		 * rowId can be considered the primary key for the row.  It is much faster to lookup a row by its rowId than by searching keys.
		 * rowId must be unique
		 * Rows are stored in sorted order in the table, using rowId as the sort key
		 * 参数[输入] rowId The UNIQUE primary key for the row.  This can never be changed.
		 * 参数[输入] initialCellValues Initial values to give the row (可选)
		 * 返回值: The newly added row
		 */
		Table::Row* AddRow(unsigned rowId);
		Table::Row* AddRow(unsigned rowId, DataStructures::List<Cell> &initialCellValues);
		Table::Row* AddRow(unsigned rowId, DataStructures::List<Cell*> &initialCellValues, bool copyCells=false);

		/*
		 * Removes a row specified by rowId.
		 * 参数[输入] rowId The ID of the row
		 * 返回值: true if the row was deleted. False if not.
		 */
		bool RemoveRow(unsigned rowId);

		/*
		 * Removes all the rows with IDs that the specified table also has.
		 * 参数[输入] tableContainingRowIDs The IDs of the rows
		 */
		void RemoveRows(Table *tableContainingRowIDs);

		/*
		 * Updates a particular cell in the table.
		 * 注意: If you are going to update many cells of a particular row, it is more efficient to call GetRow and perform the operations on the row directly.
		 * 注意: Row pointers do not change, so you can also write directly to the rows for more efficiency.
		 * 参数[输入] rowId The ID of the row
		 * 参数[输入] columnIndex The column of the cell
		 * 参数[输入] value The data to set
		 */
		bool UpdateCell(unsigned rowId, unsigned columnIndex, int value);
		bool UpdateCell(unsigned rowId, unsigned columnIndex, char *str);
		bool UpdateCell(unsigned rowId, unsigned columnIndex, int byteLength, char *data);
		bool UpdateCellByIndex(unsigned rowIndex, unsigned columnIndex, int value);
		bool UpdateCellByIndex(unsigned rowIndex, unsigned columnIndex, char *str);
		bool UpdateCellByIndex(unsigned rowIndex, unsigned columnIndex, int byteLength, char *data);

		/*
		 * Note this is much less efficient to call than GetRow, then working with the cells directly.
		 * Numeric, string, binary
		 */
		void GetCellValueByIndex(unsigned rowIndex, unsigned columnIndex, int *output);
		void GetCellValueByIndex(unsigned rowIndex, unsigned columnIndex, char *output);
		void GetCellValueByIndex(unsigned rowIndex, unsigned columnIndex, char *output, int *outputLength);

		/*
		 * 获取 a row.  More efficient to do this and access Row::cells than to repeatedly call GetCell
		 * You can also update cells in rows from this function.
		 * 参数[输入] rowId The ID of the row
		 * 返回值: The desired row, or 0 if no such row.
		 */
		Row* GetRowByID(unsigned rowId) const;

		/*
		 * 获取 a row at a specific index
		 * rowIndex should be less than GetRowCount()
		 * 参数[输入] rowIndex The index of the row
		 * 参数[输出] key The ID of the row returned
		 * 返回值: The desired row, or 0 if no such row.
		 */
		Row* GetRowByIndex(unsigned rowIndex, unsigned *key) const;

		/*
		 * Queries the table, optionally returning only a subset of columns and rows.
		 * 参数[输入] columnSubset An array of column indices.  Only columns in this array are returned.  Pass 0 for all columns
		 * 参数[输入] numColumnSubset The number of elements in columnSubset
		 * 参数[输入] inclusionFilters An array of FilterQuery.  All filters must pass for the row to be returned.
		 * 参数[输入] numInclusionFilters The number of elements in inclusionFilters
		 * 参数[输入] rowIds An arrow of row IDs.  Only these rows with these IDs are returned.  Pass 0 for all rows.
		 * 参数[输入] numRowIDs The number of elements in rowIds
		 * 参数[输出] result The result of the query.  If no rows are returned, the table will only have columns.
		 */
		void QueryTable(unsigned *columnIndicesSubset, unsigned numColumnSubset, FilterQuery *inclusionFilters, unsigned numInclusionFilters, unsigned *rowIds, unsigned numRowIDs, Table *result);

		/*
		 * Sorts the table by rows
		 * You can sort the table in ascending or descending order on one or more columns
		 * Columns have precedence in the order they appear in the sortQueries array
		 * If a row cell on column n has the same value as a a different row on column n, then the row will be compared on column n+1
		 * 参数[输入] sortQueries A list of SortQuery structures, defining the sorts to perform on the table
		 * 参数[输入] numColumnSubset The number of elements in numSortQueries
		 * 参数[输出] out The address of an array of Rows, which will receive the sorted output.  The array must be long enough to contain all returned rows, up to GetRowCount()
		 */
		void SortTable(Table::SortQuery *sortQueries, unsigned numSortQueries, Table::Row** out);

		/* Frees all memory in the table. */
		void Clear();

		/*
		 * Prints out the names of all the columns.
		 * 参数[输出] out A pointer to an array of bytes which will hold the output.
		 * 参数[输入] outLength The size of the out array
		 * 参数[输入] columnDelineator What character to print to delineate columns
		 */
		void PrintColumnHeaders(char *out, int outLength, char columnDelineator) const;

		/*
		 * Writes a text representation of the row to out.
		 * 参数[输出] out A pointer to an array of bytes which will hold the output.
		 * 参数[输入] outLength The size of the out array
		 * 参数[输入] columnDelineator What character to print to delineate columns
		 * 参数[输入] printDelineatorForBinary Binary output is not printed.  True to still print the delineator.
		 * 参数[输入] inputRow The row to print
		 */
		void PrintRow(char *out, int outLength, char columnDelineator, bool printDelineatorForBinary, Table::Row* inputRow) const;

		/* Direct access to make things easier. */
		const DataStructures::List<ColumnDescriptor>& GetColumns(void) const;

		/* Direct access to make things easier. */
		const DataStructures::BPlusTree<unsigned, Row*, _TABLE_BPLUS_TREE_ORDER>& GetRows(void) const;

		/* 获取 head of a linked list containing all the row data */
		DataStructures::Page<unsigned, Row*, _TABLE_BPLUS_TREE_ORDER> * GetListHead();

		/*
		 * 获取 first free row id
		 * This could be made more efficient.
		 */
		unsigned GetAvailableRowId(void) const;

		Table& operator = ( const Table& input );

	protected:
		Table::Row* AddRowColumns(unsigned rowId, Row *row, DataStructures::List<unsigned> columnIndices);

		void DeleteRow(Row *row);

		void QueryRow(DataStructures::List<unsigned> &inclusionFilterColumnIndices, DataStructures::List<unsigned> &columnIndicesToReturn, unsigned key, Table::Row* row, FilterQuery *inclusionFilters, Table *result);

		/* 16 is arbitrary and is the order of the BPlus tree.  Higher orders are better for searching while lower orders are better for */
		/* Insertions and deletions. */
		DataStructures::BPlusTree<unsigned, Row*, _TABLE_BPLUS_TREE_ORDER> rows;

		/* Columns in the table. */
		DataStructures::List<ColumnDescriptor> columns;
	};
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
