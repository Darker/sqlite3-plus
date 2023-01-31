#pragma once
#include "flags.h"
#include "generic/NoCopy.h"
#include "generic/StrUnowned.h"
#include "exceptions/SQLiteError.h"

#include <string>
#include <memory>
#include "generic/std_format_polyfill.h"
#include <functional>
#include <cstdint>


namespace sqlitepp
{

class Database;

class RawStatement : public NoCopy
{
  friend class Database;
  friend class BindHelper;
public:
  class BindHelper : public NoCopy
  {
    friend class RawStatement;
  public:
    void Bind(int intval);
    void Bind(std::int64_t intval);
    void Bind(double doubleVal);
    void Bind(const char* strval);
    void Bind(const char* strval, std::size_t strLen);
    void Bind(const void* blobData, std::size_t dataLen);

    ~BindHelper();
  protected:
    BindHelper(RawStatement& stmt) : _stmt(stmt) {}
    void Reset() { index = 0; }
  private:
    RawStatement& _stmt;
    int index = 0;

    // Checks if the number of bound params does not exceed number of params in the query
    void bindSanityCheck() const;
  };

  class ReadHelper : public NoCopy
  {
    friend class RawStatement;
  public:
    //void Read(int intval);
    //void Read(__int64 intval);
    //void Read(double doubleVal);
    //void Read(const char* strval);
    //void Read(const char* strval, std::size_t strLen);
    //void Read(const void* blobData, std::size_t dataLen);
    int ReadInt();
    std::int64_t ReadLongInt();
    double ReadDouble();
    StrUnowned ReadString();
    BytesUnowned ReadBlob();

  protected:
    ReadHelper(RawStatement& stmt) : _stmt(stmt) {}

  private:
    RawStatement& _stmt;
    int index = 0;

    // Checks if the number of read params does not exceed the number of result columns
    void readSanityCheck() const;
  };

  // Callback to handle each returned row, rows will be supplied as long as they are available and callback returns true
  using RowCallback = std::function<bool(ReadHelper&)>;

  RawStatement(const std::string& query, PrepareFlags flags = PrepareFlags::NONE);
  ~RawStatement();

  void Execute(const RowCallback& rowHandler);

  BindHelper& getBinder() { return _binder; }
  void Init();
  void SetDb(Database* db);
protected:
  struct Private;
  std::unique_ptr<Private> _private;

  
  
private:
  bool _isValid = true;
  bool _initCalled = false;
  int _bindCount = 0;
  Database* _db;
  std::string _query;
  PrepareFlags _flags;
  BindHelper _binder;
  // set to true when executing, for debugging purposes
  bool _executing = false;
};

inline void RawStatement::BindHelper::bindSanityCheck() const
{
  if (index > _stmt._bindCount)
  {
    throw SQLiteError(std::format("Attempted to bind {} parameters out of max {}", (index + 1), (_stmt._bindCount + 1)));
  }
}

}
