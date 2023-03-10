#pragma once

namespace sqlitepp
{
enum class OpenFlags
{
  READONLY = 0x00000001, /* Ok for sqlite3_open_v2() */
  READWRITE = 0x00000002, /* Ok for sqlite3_open_v2() */
  CREATE = 0x00000004, /* Ok for sqlite3_open_v2() */
  DELETEONCLOSE = 0x00000008, /* VFS only */
  EXCLUSIVE = 0x00000010, /* VFS only */
  AUTOPROXY = 0x00000020, /* VFS only */
  URI = 0x00000040, /* Ok for sqlite3_open_v2() */
  MEMORY = 0x00000080, /* Ok for sqlite3_open_v2() */
  MAIN_DB = 0x00000100, /* VFS only */
  TEMP_DB = 0x00000200, /* VFS only */
  TRANSIENT_DB = 0x00000400, /* VFS only */
  MAIN_JOURNAL = 0x00000800, /* VFS only */
  TEMP_JOURNAL = 0x00001000, /* VFS only */
  SUBJOURNAL = 0x00002000, /* VFS only */
  SUPER_JOURNAL = 0x00004000, /* VFS only */
  NOMUTEX = 0x00008000, /* Ok for sqlite3_open_v2() */
  FULLMUTEX = 0x00010000, /* Ok for sqlite3_open_v2() */
  SHAREDCACHE = 0x00020000, /* Ok for sqlite3_open_v2() */
  PRIVATECACHE = 0x00040000, /* Ok for sqlite3_open_v2() */
  WAL = 0x00080000, /* VFS only */
  NOFOLLOW = 0x01000000, /* Ok for sqlite3_open_v2() */
};

inline OpenFlags operator|(OpenFlags a, OpenFlags b)
{
  return static_cast<OpenFlags>(static_cast<int>(a) | static_cast<int>(b));
}

enum class PrepareFlags
{
  // Can be used instead of zero integer if you don't want any flags
  NONE       =            0x00,
  // Hints to sqlite3 that this statement will be used many times over
  PERSISTENT =            0x01,
  // Noop, no longer required for anything
  NORMALIZE  =            0x02,
  // If set, statement creation will fail with an error if the statement uses virtual tables
  NO_VTAB    =            0x04,
};

inline PrepareFlags operator|(PrepareFlags a, PrepareFlags b)
{
  return static_cast<PrepareFlags>(static_cast<int>(a) | static_cast<int>(b));
}

}