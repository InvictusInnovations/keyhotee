#pragma once

#include <fc/interprocess/process.hpp>

using namespace fc;

class managed_stream
{
public:
  managed_stream(fc::process& process);
  ~managed_stream();

  void log_stdout_stderr_to_file(const fc::path& logfile);

private:
  fc::process*    _process;
};

