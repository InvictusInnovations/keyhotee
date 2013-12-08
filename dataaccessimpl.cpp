#include "dataaccessimpl.h"

TConnectionStatusDS::TConnectionStatusDS()
  {
  App = bts::application::instance();
  }

TConnectionStatusDS::~TConnectionStatusDS()
  {
  /// Nothing to do here atm
  }

unsigned int TConnectionStatusDS::GetConnectionCount() const
  {
  return static_cast<unsigned int>(App->get_network()->get_connections().size());
  }

