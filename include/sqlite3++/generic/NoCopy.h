#pragma once
namespace sqlitepp
{

class NoCopy
{
public:
  NoCopy() {}
  NoCopy(const NoCopy& other) = delete;
  NoCopy& operator=(const NoCopy& other) = delete;
};

}