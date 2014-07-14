#pragma once

#include <fstream>

#include <fc/interprocess/process.hpp>

class ManagedStream
{
public:
  ManagedStream(fc::process& process, const fc::path& logfile);
  ~ManagedStream();

  void log_stdout_to_file();
  void log_stderr_to_file();

  void stop_log_stdout();

  void log_to_file(const char* buf, size_t len);

private:
  fc::process*      _process;
  fc::future<void>  _stdout_reader_done;

  std::string       _out_logfile;
  std::string       _err_logfile;
};

