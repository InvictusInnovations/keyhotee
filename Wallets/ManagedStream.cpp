#include "ManagedStream.hpp"

#include <fc/thread/thread.hpp>


ManagedStream::ManagedStream(fc::process& process, const fc::path& logfile)
{
  _process = &process;

  std::string string = logfile.string();
  _out_logfile = string + ".bitshare_client.out";
  _err_logfile = string + ".bitshare_client.err";
}

ManagedStream::~ManagedStream()
{}

void ManagedStream::log_stdout_to_file()
{
  std::shared_ptr<std::ofstream> stdoutfile =
    std::make_shared<std::ofstream>(_out_logfile, std::ofstream::out | std::ofstream::app);
  fc::buffered_istream_ptr out_stream = _process->out_stream();

  _stdout_reader_done = fc::async([out_stream, stdoutfile]()
  {
    char buf[1024];
    for(;;)
    {
      size_t bytes_read = out_stream->readsome(buf, sizeof(buf));
      if(!bytes_read)
        break;
      stdoutfile->write(buf, bytes_read);
      stdoutfile->flush();
    }
  });
}

void ManagedStream::log_stderr_to_file()
{
  std::shared_ptr<std::ofstream> stderrfile = std::make_shared<std::ofstream>(_err_logfile);
  fc::buffered_istream_ptr err_stream = _process->err_stream();

  auto stderr_reader_done = fc::async([err_stream, stderrfile]()
  {
    char buf[1024];
    for(;;)
    {
      size_t bytes_read = err_stream->readsome(buf, sizeof(buf));
      if(!bytes_read)
        break;
      stderrfile->write(buf, bytes_read);
      stderrfile->flush();
    }
  });
}

void ManagedStream::stop_log_stdout()
{
  _stdout_reader_done.cancel();
}

void ManagedStream::log_to_file(const char* buf, size_t len)
{
  std::ofstream  stdoutfile;
  stdoutfile.open(_out_logfile, std::ofstream::out | std::ofstream::app);
  stdoutfile.write(buf, len);
  stdoutfile.flush();
  stdoutfile.close();
}

