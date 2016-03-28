/********************************************************************
 * gnc-backend-sql.h: load and save data to SQL                     *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
\********************************************************************/

/**
 * @defgroup SQLBE SQL Backend Core
  @{
*/

/** @addtogroup Columns Columns
    @ingroup SQLBE
*/

/** The SQL backend core is a library which can form the core for a QOF
 *  backend based on an SQL library.
 *
 *  @file gnc-backend-sql.h
 *  @brief load and save data to SQL
 *  @author Copyright (c) 2006-2008 Phil Longstaff <plongstaff@rogers.com>
 */

#ifndef GNC_BACKEND_SQL_H
#define GNC_BACKEND_SQL_H
extern "C"
{
#include "qof.h"
#include "qofbackend-p.h"
#include <gmodule.h>
}

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

struct GncSqlColumnInfo;
struct GncSqlColumnTableEntry;
using EntryVec = std::vector<GncSqlColumnTableEntry>;
using ColVec = std::vector<GncSqlColumnInfo>;
using StrVec = std::vector<std::string>;
using PairVec = std::vector<std::pair<std::string, std::string>>;
typedef struct GncSqlConnection GncSqlConnection;

/**
 * @struct GncSqlBackend
 *
 * Main SQL backend structure.
 */
struct GncSqlBackend
{
    QofBackend be;           /**< QOF backend */
    GncSqlConnection* conn;  /**< SQL connection */
    QofBook* book;           /**< The primary, main open book */
    gboolean loading;        /**< We are performing an initial load */
    gboolean in_query;       /**< We are processing a query */
    gboolean is_pristine_db; /**< Are we saving to a new pristine db? */
    gint obj_total;     /**< Total # of objects (for percentage calculation) */
    gint operations_done;    /**< Number of operations (save/load) done */
    GHashTable* versions;    /**< Version number for each table */
    const gchar* timespec_format;   /**< Format string for SQL for timespec values */
};
typedef struct GncSqlBackend GncSqlBackend;

/**
 * Initialize the SQL backend.
 *
 * @param be SQL backend
 */
void gnc_sql_init (GncSqlBackend* be);

/**
 * Load the contents of an SQL database into a book.
 *
 * @param be SQL backend
 * @param book Book to be loaded
 */
void gnc_sql_load (GncSqlBackend* be,  QofBook* book,
                   QofBackendLoadType loadType);

/**
 * Register a commodity to be committed after loading is complete.
 *
 * Necessary to save corrections made while loading.
 * @param be SQL backend
 * @param comm The commodity item to be committed.
 */
void gnc_sql_push_commodity_for_postload_processing (GncSqlBackend* be,
                                                     gpointer comm);

/**
 * Save the contents of a book to an SQL database.
 *
 * @param be SQL backend
 * @param book Book to be saved
 */
void gnc_sql_sync_all (GncSqlBackend* be,  QofBook* book);

/**
 * An object is about to be edited.
 *
 * @param be SQL backend
 * @param inst Object being edited
 */
void gnc_sql_begin_edit (GncSqlBackend* be, QofInstance* inst);

/**
 * Object editing has been cancelled.
 *
 * @param qbe SQL backend
 * @param inst Object being edited
 */
void gnc_sql_rollback_edit (GncSqlBackend* qbe, QofInstance* inst);

/**
 * Object editting is complete and the object should be saved.
 *
 * @param qbe SQL backend
 * @param inst Object being edited
 */
void gnc_sql_commit_edit (GncSqlBackend* qbe, QofInstance* inst);

/**
 */
typedef struct GncSqlStatement GncSqlStatement;
typedef struct GncSqlResult GncSqlResult;

/**
 *@struct GncSqlStatement
 *
 * Struct which represents an SQL statement.  SQL backends must provide a
 * structure which implements all of the functions.
 */
struct GncSqlStatement
{
    void (*dispose) (GncSqlStatement*);
    gchar* (*toSql) (GncSqlStatement*);
    void (*addWhereCond) (GncSqlStatement*, QofIdTypeConst, gpointer,
                          const PairVec&);
};
#define gnc_sql_statement_dispose(STMT) \
        (STMT)->dispose(STMT)
#define gnc_sql_statement_to_sql(STMT) \
        (STMT)->toSql(STMT)
#define gnc_sql_statement_add_where_cond(STMT,TYPENAME,OBJ,COL_VAL_PAIR) \
        (STMT)->addWhereCond(STMT, TYPENAME, OBJ, COL_VAL_PAIR)

/**
 * @struct GncSqlConnection
 *
 * Struct which represents the connection to an SQL database.  SQL backends
 * must provide a structure which implements all of the functions.
 */
struct GncSqlConnection
{
    void (*dispose) (GncSqlConnection*);
    GncSqlResult* (*executeSelectStatement) (GncSqlConnection*, GncSqlStatement*); /**< Returns NULL if error */
    gint (*executeNonSelectStatement) (GncSqlConnection*, GncSqlStatement*); /**< Returns -1 if error */
    GncSqlStatement* (*createStatementFromSql) (GncSqlConnection*, const gchar*);
    gboolean (*doesTableExist) (GncSqlConnection*, const gchar*);  /**< Returns true if successful */
    gboolean (*beginTransaction) (GncSqlConnection*); /**< Returns TRUE if successful, FALSE if error */
    gboolean (*rollbackTransaction) (GncSqlConnection*); /**< Returns TRUE if successful, FALSE if error */
    gboolean (*commitTransaction) (GncSqlConnection*); /**< Returns TRUE if successful, FALSE if error */
    gboolean (*createTable) (GncSqlConnection*, const gchar*, const ColVec&); /**< Returns TRUE if successful, FALSE if error */
    gboolean (*createIndex) (GncSqlConnection*, const gchar*, const gchar*, const EntryVec&); /**< Returns TRUE if successful, FALSE if error */
    gboolean (*addColumnsToTable) (GncSqlConnection*, const gchar* table, const ColVec&); /**< Returns TRUE if successful, FALSE if error */
    gchar* (*quoteString) (const GncSqlConnection*, const char*);
};
#define gnc_sql_connection_dispose(CONN) (CONN)->dispose(CONN)
#define gnc_sql_connection_execute_select_statement(CONN,STMT) \
        (CONN)->executeSelectStatement(CONN,STMT)
#define gnc_sql_connection_execute_nonselect_statement(CONN,STMT) \
        (CONN)->executeNonSelectStatement(CONN,STMT)
#define gnc_sql_connection_create_statement_from_sql(CONN,SQL) \
        (CONN)->createStatementFromSql(CONN,SQL)
#define gnc_sql_connection_does_table_exist(CONN,NAME) \
        (CONN)->doesTableExist(CONN,NAME)
#define gnc_sql_connection_begin_transaction(CONN) \
        (CONN)->beginTransaction(CONN)
#define gnc_sql_connection_rollback_transaction(CONN) \
        (CONN)->rollbackTransaction(CONN)
#define gnc_sql_connection_commit_transaction(CONN) \
        (CONN)->commitTransaction(CONN)
#define gnc_sql_connection_create_table(CONN,NAME,COLLIST) \
        (CONN)->createTable(CONN,NAME,COLLIST)
#define gnc_sql_connection_create_index(CONN,INDEXNAME,TABLENAME,COLTABLE) \
        (CONN)->createIndex(CONN,INDEXNAME,TABLENAME,COLTABLE)
#define gnc_sql_connection_add_columns_to_table(CONN,TABLENAME,COLLIST) \
        (CONN)->addColumnsToTable(CONN,TABLENAME,COLLIST)
#define gnc_sql_connection_quote_string(CONN,STR) \
        (CONN)->quoteString(CONN,STR)

/**
 * Struct used to represent a row in the result of an SQL SELECT statement.
 * SQL backends must provide a structure which implements all of the functions.
 */

class GncSqlRow
{
public:
    virtual ~GncSqlRow() = default;
    virtual int64_t get_int_at_col (const char* col) = 0;
    virtual float get_float_at_col (const char* col) = 0;
    virtual double get_double_at_col (const char* col) = 0;
    virtual std::string get_string_at_col (const char* col) = 0;
    virtual time64 get_time64_at_col (const char* col) = 0;
};

/**
 * @struct GncSqlResult
 *
 * Struct used to represent the result of an SQL SELECT statement.  SQL
 * backends must provide a structure which implements all of the functions.
 */
struct GncSqlResult
{
    guint (*getNumRows) (GncSqlResult*);
    GncSqlRow* (*getFirstRow) (GncSqlResult*);
    GncSqlRow* (*getNextRow) (GncSqlResult*);
    void (*dispose) (GncSqlResult*);
};
#define gnc_sql_result_get_num_rows(RESULT) \
        (RESULT)->getNumRows(RESULT)
#define gnc_sql_result_get_first_row(RESULT) \
        (RESULT)->getFirstRow(RESULT)
#define gnc_sql_result_get_next_row(RESULT) \
        (RESULT)->getNextRow(RESULT)
#define gnc_sql_result_dispose(RESULT) \
        (RESULT)->dispose(RESULT)

/**
 * @struct GncSqlObjectBackend
 *
 * Struct used to handle a specific engine object type for an SQL backend.
 * This handler should be registered with gnc_sql_register_backend().
 *
 * commit()         - commit an object to the db
 * initial_load()   - load stuff when new db opened
 * create_tables()  - create any db tables
 * compile_query()  - compile a backend object query
 * run_query()      - run a compiled query
 * free_query()     - free a compiled query
 * write()          - write all objects
 */
typedef struct
{
    int         version;                /**< Backend version number */
    const std::string   type_name;      /**< Engine object type name */
    /** Commit an instance of this object to the database
     * @return TRUE if successful, FALSE if error
     */
    gboolean (*commit) (GncSqlBackend* be, QofInstance* inst);
    /** Load all objects of this type from the database */
    void (*initial_load) (GncSqlBackend* be);
    /** Create database tables for this object */
    void (*create_tables) (GncSqlBackend* be);
    /** Compile a query on these objects */
    gpointer (*compile_query) (GncSqlBackend* be, QofQuery* pQuery);
    /** Run a query on these objects */
    void (*run_query) (GncSqlBackend* be, gpointer pQuery);
    /** Free a query on these objects */
    void (*free_query) (GncSqlBackend* be, gpointer pQuery);
    /** Write all objects of this type to the database
     * @return TRUE if successful, FALSE if error
     */
    gboolean (*write) (GncSqlBackend* be);
} GncSqlObjectBackend;
#define GNC_SQL_BACKEND             "gnc:sql:1"
#define GNC_SQL_BACKEND_VERSION 1
using GncSqlObjectBackendPtr = GncSqlObjectBackend*;
using OBEEntry = std::tuple<std::string, GncSqlObjectBackendPtr>;
using OBEVec = std::vector<OBEEntry>;
void gnc_sql_register_backend(OBEEntry&&);
void gnc_sql_register_backend(GncSqlObjectBackendPtr);
const OBEVec& gnc_sql_get_backend_registry();

/**
 * Basic column type
 */
typedef enum
{
    BCT_STRING,
    BCT_INT,
    BCT_INT64,
    BCT_DATE,
    BCT_DOUBLE,
    BCT_DATETIME
} GncSqlBasicColumnType;


// Type for conversion of db row to object.
#define CT_STRING "ct_string"
#define CT_GUID "ct_guid"
#define CT_INT "ct_int"
#define CT_INT64 "ct_int64"
#define CT_TIMESPEC "ct_timespec"
#define CT_GDATE "ct_gdate"
#define CT_NUMERIC "ct_numeric"
#define CT_DOUBLE "ct_double"
#define CT_BOOLEAN "ct_boolean"
#define CT_ACCOUNTREF "ct_accountref"
#define CT_BUDGETREF "ct_budgetref"
#define CT_COMMODITYREF "ct_commodityref"
#define CT_LOTREF "ct_lotref"
#define CT_TXREF "ct_txref"

enum ColumnFlags : int
{
    COL_NO_FLAG = 0,
        COL_PKEY = 0x01,        /**< The column is a primary key */
        COL_NNUL = 0x02,    /**< The column may not contain a NULL value */
        COL_UNIQUE = 0x04,  /**< The column must contain unique values */
        COL_AUTOINC = 0x08 /**< The column is an auto-incrementing int */
        };

/**
 * Contains all of the information required to copy information between an
 * object and the database for a specific object property.
 *
 * If an entry contains a gobj_param_name value, this string is used as the
 * property name for a call to g_object_get() or g_object_set().  If the
 * gobj_param_name value is NULL but qof_param_name is not NULL, this value
 * is used as the parameter name for a call to
 * qof_class_get_parameter_getter().  If both of these values are NULL, getter
 * and setter are the addresses of routines to return or set the parameter
 * value, respectively.
 *
 * The database description for an object consists of an array of
 * GncSqlColumnTableEntry objects, with a final member having col_name == NULL.
 */
struct GncSqlColumnTableEntry
{
    GncSqlColumnTableEntry (const char* name, const char*type, unsigned int s,
                            ColumnFlags f, const char* gobj_name = nullptr,
                            const char* qof_name = nullptr,
                            QofAccessFunc get = nullptr,
                            QofSetterFunc set = nullptr) :
        col_name{name}, col_type{type}, size{s}, flags{f},
        gobj_param_name{gobj_name}, qof_param_name{qof_name}, getter{get},
        setter{set} {}
    GncSqlColumnTableEntry (const char* name, const char*type, unsigned int s,
                            int f, const char* gobj_name = nullptr,
                            const char* qof_name = nullptr,
                            QofAccessFunc get = nullptr,
                            QofSetterFunc set = nullptr) :
        col_name{name}, col_type{type}, size{s},
        flags{static_cast<ColumnFlags>(f)},
        gobj_param_name{gobj_name}, qof_param_name{qof_name}, getter{get},
        setter{set} {}
    const char* col_name;        /**< Column name */
    const char* col_type;        /**< Column type */
    unsigned int size;         /**< Column size in bytes, for string columns */
    ColumnFlags flags;           /**< Column flags */
    const char* gobj_param_name; /**< If non-null, g_object param name */
    const char* qof_param_name;  /**< If non-null, qof parameter name */
    QofAccessFunc getter;        /**< General access function */
    QofSetterFunc setter;        /**< General setter function */
};

inline bool operator==(const GncSqlColumnTableEntry& l,
                       const GncSqlColumnTableEntry& r)
{
    return strcmp(l.col_name, r.col_name) == 0 &&
        strcmp(l.col_type, r.col_type) == 0;
}

inline bool operator!=(const GncSqlColumnTableEntry& l,
                       const GncSqlColumnTableEntry& r)
{
    return !(l == r);
}

/**
 *  information required to create a column in a table.
 */
struct GncSqlColumnInfo
{
    GncSqlColumnInfo (std::string&& name, GncSqlBasicColumnType type,
                      unsigned int size = 0, bool unicode = false,
                      bool autoinc = false, bool primary = false,
                      bool not_null = false) :
        m_name{name}, m_type{type}, m_size{size}, m_unicode{unicode},
        m_autoinc{autoinc}, m_primary_key{primary}, m_not_null{not_null}
        {}
    GncSqlColumnInfo(const GncSqlColumnTableEntry& e, GncSqlBasicColumnType t,
                     unsigned int size = 0, bool unicode = true) :
        m_name{e.col_name}, m_type{t}, m_size{size}, m_unicode{unicode},
        m_autoinc(e.flags & COL_AUTOINC),
        m_primary_key(e.flags & COL_PKEY),
        m_not_null(e.flags & COL_NNUL) {}
    std::string m_name; /**< Column name */
    GncSqlBasicColumnType m_type; /**< Column basic type */
    unsigned int m_size; /**< Column size (string types) */
    bool m_unicode; /**< Column is unicode (string types) */
    bool m_autoinc; /**< Column is autoinc (int type) */
    bool m_primary_key; /**< Column is the primary key */
    bool m_not_null; /**< Column forbids NULL values */
};

inline bool operator==(const GncSqlColumnInfo& l,
                       const GncSqlColumnInfo& r)
{
    return l.m_name == r.m_name && l.m_type == r.m_type;
}

inline bool operator!=(const GncSqlColumnInfo& l,
                       const GncSqlColumnInfo& r)
{
    return !(l == r);
}

typedef enum
{
    OP_DB_INSERT,
    OP_DB_UPDATE,
    OP_DB_DELETE
} E_DB_OPERATION;

typedef void (*GNC_SQL_LOAD_FN) (const GncSqlBackend* be,
                                 GncSqlRow* row, QofSetterFunc setter,
                                 gpointer pObject,
                                 const GncSqlColumnTableEntry& table_row);
typedef void (*GNC_SQL_ADD_COL_INFO_TO_LIST_FN) (const GncSqlBackend* be,
                                                 const GncSqlColumnTableEntry& table_row,
                                                 ColVec& vec);
typedef void (*GNC_SQL_ADD_VALUE_TO_VEC_FN) (const GncSqlBackend* be,
                                             QofIdTypeConst obj_name,
                                             const gpointer pObject,
                                             const GncSqlColumnTableEntry& table_row,
                                             PairVec& vec);

/**
 * @struct GncSqlColumnTypeHandler
 *
 * The GncSqlColumnTypeHandler struct contains pointers to routines to handle
 * different options for a specific column type.
 *
 * A column type maps a property value to one or more columns in the database.
 */
typedef struct
{
    /**
     * Routine to load a value into an object from the database row.
     */
    GNC_SQL_LOAD_FN                 load_fn;

    /**
     * Routine to add a GncSqlColumnInfo structure for the column type to a
     * GList.
     */
    GNC_SQL_ADD_COL_INFO_TO_LIST_FN add_col_info_to_list_fn;

    /**
     * Add a pair of the table column heading and object's value's string
     * representation to a PairVec; used for constructing WHERE clauses and
     * UPDATE statements.
     */
    GNC_SQL_ADD_VALUE_TO_VEC_FN add_value_to_vec_fn;
} GncSqlColumnTypeHandler;

/**
 * Returns the QOF access function for a column.
 *
 * @param obj_name QOF object type name
 * @param table_row DB table column
 * @return Access function
 */
QofAccessFunc gnc_sql_get_getter (QofIdTypeConst obj_name,
                                  const GncSqlColumnTableEntry& table_row);

/**
 * Performs an operation on the database.
 *
 * @param be SQL backend struct
 * @param op Operation type
 * @param table_name SQL table name
 * @param obj_name QOF object type name
 * @param pObject Gnucash object
 * @param table DB table description
 * @return TRUE if successful, FALSE if not
 */
gboolean gnc_sql_do_db_operation (GncSqlBackend* be,
                                  E_DB_OPERATION op,
                                  const gchar* table_name,
                                  QofIdTypeConst obj_name,
                                  gpointer pObject,
                                  const EntryVec& table);

/**
 * Executes an SQL SELECT statement and returns the result rows.  If an error
 * occurs, an entry is added to the log, an error status is returned to qof and
 * NULL is returned.
 *
 * @param be SQL backend struct
 * @param statement Statement
 * @return Results, or NULL if an error has occured
 */
GncSqlResult* gnc_sql_execute_select_statement (GncSqlBackend* be,
                                                GncSqlStatement* statement);

/**
 * Executes an SQL SELECT statement from an SQL char string and returns the
 * result rows.  If an error occurs, an entry is added to the log, an error
 * status is returned to qof and NULL is returned.
 *
 * @param be SQL backend struct
 * @param sql SQL SELECT string
 * @return Results, or NULL if an error has occured
 */
GncSqlResult* gnc_sql_execute_select_sql (GncSqlBackend* be, const gchar* sql);

/**
 * Executes an SQL non-SELECT statement from an SQL char string.
 *
 * @param be SQL backend struct
 * @param sql SQL non-SELECT string
 * @returns Number of rows affected, or -1 if an error has occured
 */
gint gnc_sql_execute_nonselect_sql (GncSqlBackend* be, const gchar* sql);

/**
 * Creates a statement from an SQL char string.
 *
 * @param be SQL backend struct
 * @param sql SQL char string
 * @return Statement
 */
GncSqlStatement* gnc_sql_create_statement_from_sql (GncSqlBackend* be,
                                                    const gchar* sql);

/**
 * Loads a Gnucash object from the database.
 *
 * @param be SQL backend struct
 * @param row DB result row
 * @param obj_name QOF object type name
 * @param pObject Object to be loaded
 * @param table DB table description
 */
void gnc_sql_load_object (const GncSqlBackend* be, GncSqlRow* row,
                          QofIdTypeConst obj_name, gpointer pObject,
                          const EntryVec& table);

/**
 * Checks whether an object is in the database or not.
 *
 * @param be SQL backend struct
 * @param table_name DB table name
 * @param obj_name QOF object type name
 * @param pObject Object to be checked
 * @param table DB table description
 * @return TRUE if the object is in the database, FALSE otherwise
 */
gboolean gnc_sql_object_is_it_in_db (GncSqlBackend* be,
                                     const gchar* table_name,
                                     QofIdTypeConst obj_name,
                                     const gpointer pObject,
                                     const EntryVec& table );

/**
 * Returns the version number for a DB table.
 *
 * @param be SQL backend struct
 * @param table_name Table name
 * @return Version number, or 0 if the table does not exist
 */
gint gnc_sql_get_table_version (const GncSqlBackend* be,
                                const gchar* table_name);

gboolean gnc_sql_set_table_version (GncSqlBackend* be,
                                    const gchar* table_name,
                                    gint version);

/**
 * Creates a table in the database
 *
 * @param be SQL backend struct
 * @param table_name Table name
 * @param table_version Table version
 * @param col_table DB table description
 * @return TRUE if successful, FALSE if unsuccessful
 */
gboolean gnc_sql_create_table (GncSqlBackend* be,
                               const gchar* table_name,
                               gint table_version,
                               const EntryVec& col_table);

/**
 * Creates a temporary table in the database.  A temporary table does not
 * have a version number added to the versions table.
 *
 * @param be SQL backend struct
 * @param table_name Table name
 * @param col_table DB table description
 * @return TRUE if successful, FALSE if unsuccessful
 */
gboolean gnc_sql_create_temp_table (const GncSqlBackend* be,
                                    const gchar* table_name,
                                    const EntryVec& col_table);

/**
 * Creates an index in the database
 *
 * @param be SQL backend struct
 * @param index_name Index name
 * @param table_name Table name
 * @param col_table Columns that the index should index
 * @return TRUE if successful, FALSE if unsuccessful
 */
gboolean gnc_sql_create_index (const GncSqlBackend* be, const char* index_name,
                               const char* table_name, const EntryVec& col_table);

/**
 * Loads the object guid from a database row.  The table must have a column
 * named "guid" with type CT_GUID.
 *
 * @param be SQL backend struct
 * @param row Database row
 * @return GncGUID
 */

const GncGUID* gnc_sql_load_guid (const GncSqlBackend* be, GncSqlRow* row);

/**
 * Loads the transaction guid from a database row.  The table must have a column
 * named "tx_guid" with type CT_GUID.
 *
 * @param be SQL backend struct
 * @param row Database row
 * @return GncGUID
 */

const GncGUID* gnc_sql_load_tx_guid (const GncSqlBackend* be, GncSqlRow* row);

/**
 * Creates a basic SELECT statement for a table.
 *
 * @param be SQL backend struct
 * @param table_name Table name
 * @return Statement
 */
GncSqlStatement* gnc_sql_create_select_statement (GncSqlBackend* be,
                                                  const gchar* table_name);

/**
 * Registers a column handler for a new column type.
 *
 * @param colType Column type
 * @param handler Column handler
 */
void gnc_sql_register_col_type_handler (const gchar* colType,
                                        const GncSqlColumnTypeHandler* handler);

/**
 * Adds a GValue for an object reference GncGUID to the end of a GSList.
 *
 * @param be SQL backend struct
 * @param obj_name QOF object type name
 * @param pObject Object
 * @param table_row DB table column description
 * @param pList List
 */
void gnc_sql_add_objectref_guid_to_vec (const GncSqlBackend* be,
                                        QofIdTypeConst obj_name,
                                        const gpointer pObject,
                                        const GncSqlColumnTableEntry& table_row,
                                        PairVec& vec);

/**
 * Adds a column info structure for an object reference GncGUID to the end of a
 * GList.
 *
 * @param be SQL backend struct
 * @param table_row DB table column description
 * @param pList List
 */
void gnc_sql_add_objectref_guid_col_info_to_list (const GncSqlBackend* be,
                                                  const GncSqlColumnTableEntry& table_row,
                                                  ColVec& vec);

/**
 * Appends the ascii strings for a list of GUIDs to the end of an SQL string.
 *
 * @param str SQL string
 * @param list List of GUIDs
 * @param maxCount Max # of GUIDs to append
 * @return Number of GUIDs appended
 */
guint gnc_sql_append_guid_list_to_sql (GString* str, GList* list,
                                       guint maxCount);

/**
 * Initializes DB table version information.
 *
 * @param be SQL backend struct
 */
void gnc_sql_init_version_info (GncSqlBackend* be);

/**
 * Finalizes DB table version information.
 *
 * @param be SQL backend struct
 */
void gnc_sql_finalize_version_info (GncSqlBackend* be);

/**
 * Commits a "standard" item to the database.  In most cases, a commit of one
 * object vs another differs only in the table name and column table.
 *
 * @param be SQL backend
 * @param inst Instance
 * @param tableName SQL table name
 * @param obj_name QOF object type name
 * @param col_table Column table
 * @return TRUE if successful, FALSE if not
 */
gboolean gnc_sql_commit_standard_item (GncSqlBackend* be, QofInstance* inst,
                                       const gchar* tableName,
                                       QofIdTypeConst obj_name,
                                       const EntryVec& col_table);

/**
 * Gets an integer value (of any size) from a GValue.
 *
 * @param value Source value
 * @return Integer value
 */
gint64 gnc_sql_get_integer_value (const GValue* value);

/**
 * Converts a Timespec value to a string value for the database.
 *
 * @param be SQL backend
 * @param ts Timespec to be converted
 * @return String representation of the Timespec
 */
gchar* gnc_sql_convert_timespec_to_string (const GncSqlBackend* be,
                                           Timespec ts);

/**
 * Upgrades a table to a new structure.  The upgrade is done by creating a new
 * table with the new structure, SELECTing the old data into the new table,
 * deleting the old table, then renaming the new table.  Therefore, this will
 * only work if the new table structure is similar enough to the old table that
 * the SELECT will work.
 *
 * @param be SQL backend
 * @param table_name SQL table name
 * @param col_table Column table
 */
void gnc_sql_upgrade_table (GncSqlBackend* be, const gchar* table_name,
                            const EntryVec& col_table);

/**
 * Adds one or more columns to an existing table.
 *
 * @param be SQL backend
 * @param table_name SQL table name
 * @param new_col_table Column table for new columns
 * @return TRUE if successful, FALSE if unsuccessful
 */
gboolean gnc_sql_add_columns_to_table (GncSqlBackend* be, const char* table_name,
                                       const EntryVec& new_col_table);

/**
 * Specifies the load order for a set of objects.  When loading from a database,
 * the objects will be loaded in this order, so that when later objects have
 * references to objects, those objects will already have been loaded.
 *
 * @param load_order NULL-terminated array of object type ID strings
 */
void gnc_sql_set_load_order(StrVec&& load_order);

void _retrieve_guid_ (gpointer pObject,  gpointer pValue);

gpointer gnc_sql_compile_query (QofBackend* pBEnd, QofQuery* pQuery);
void gnc_sql_free_query (QofBackend* pBEnd, gpointer pQuery);
void gnc_sql_run_query (QofBackend* pBEnd, gpointer pQuery);

typedef struct
{
    GncSqlBackend* be;
    gboolean is_ok;
} write_objects_t;

template <typename T> T
get_row_value_from_object(QofIdTypeConst obj_name, const gpointer pObject,
                          const GncSqlColumnTableEntry& table_row)
{
    return get_row_value_from_object<T>(obj_name, pObject, table_row,
                                        std::is_pointer<T>());
}

template <typename T> T
get_row_value_from_object(QofIdTypeConst obj_name, const gpointer pObject,
                          const GncSqlColumnTableEntry& table_row,
                          std::true_type)
{
    g_return_val_if_fail(obj_name != nullptr && pObject != nullptr, nullptr);
    T result = nullptr;
    if (table_row.gobj_param_name != nullptr)
        g_object_get(pObject, table_row.gobj_param_name, &result, NULL );
    else
    {
        QofAccessFunc getter = gnc_sql_get_getter(obj_name, table_row);
        if (getter != nullptr)
            result = reinterpret_cast<T>((getter)(pObject, nullptr));
    }
    return result;
}

template <typename T> T
get_row_value_from_object(QofIdTypeConst obj_name, const gpointer pObject,
                          const GncSqlColumnTableEntry& table_row,
                          std::false_type)
{
    g_return_val_if_fail(obj_name != nullptr && pObject != nullptr,
                         static_cast<T>(0));
    T result = static_cast<T>(0);
    if (table_row.gobj_param_name != nullptr)
        g_object_get(pObject, table_row.gobj_param_name, &result, NULL );
    else
    {
        QofAccessFunc getter = gnc_sql_get_getter(obj_name, table_row);
        if (getter != nullptr)
            result = reinterpret_cast<T>((getter)(pObject, nullptr));
    }
    return result;
}

template <typename T> void
add_value_to_vec(const GncSqlBackend* be, QofIdTypeConst obj_name,
                 const gpointer pObject,
                 const GncSqlColumnTableEntry& table_row,
                 PairVec& vec)
{
    add_value_to_vec<T>(be, obj_name, pObject, table_row, vec,
                        std::is_pointer<T>());
}

template <typename T> void
add_value_to_vec(const GncSqlBackend* be, QofIdTypeConst obj_name,
                 const gpointer pObject,
                 const GncSqlColumnTableEntry& table_row,
                 PairVec& vec, std::true_type)
{
    T s = get_row_value_from_object<T>(obj_name, pObject, table_row);

    if (s != nullptr)
    {
        std::ostringstream stream;
        stream << *s;
        vec.emplace_back(std::make_pair(std::string{table_row.col_name},
                                        stream.str()));
        return;
    }
}

template <typename T> void
add_value_to_vec(const GncSqlBackend* be, QofIdTypeConst obj_name,
                 const gpointer pObject,
                 const GncSqlColumnTableEntry& table_row,
                 PairVec& vec, std::false_type)
{
    T s = get_row_value_from_object<T>(obj_name, pObject, table_row);

    std::ostringstream stream;
    stream << s;
    vec.emplace_back(std::make_pair(std::string{table_row.col_name},
                                    stream.str()));
    return;
}


#endif /* GNC_BACKEND_SQL_H */

/**
  @}  end of the SQL Backend Core doxygen group
*/
