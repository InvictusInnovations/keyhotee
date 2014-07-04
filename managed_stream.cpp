#include "managed_stream.hpp"

#include <fstream>

#include <fc/thread/thread.hpp>


managed_stream::managed_stream(fc::process& process)
{
  _process = &process;
}


managed_stream::~managed_stream()
{}

void managed_stream::log_stdout_stderr_to_file(const fc::path& logfile)
{
  std::string string = logfile.string();
  auto out_logfile = string + ".bitshare_client.out";
  auto err_logfile = string + ".bitshare_client.err";

  std::shared_ptr<std::ofstream> stdoutfile = std::make_shared<std::ofstream>(out_logfile);
  std::shared_ptr<std::ofstream> stderrfile = std::make_shared<std::ofstream>(err_logfile);
  fc::buffered_istream_ptr out_stream = _process->out_stream();
  fc::buffered_istream_ptr err_stream = _process->err_stream();
  
  auto stdout_reader_done = fc::async([out_stream, stdoutfile]()
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

