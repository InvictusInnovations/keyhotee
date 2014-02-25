#include "dataaccessimpl.h"

TConnectionStatusDS::TConnectionStatusDS()
  {
  App = bts::application::instance();
  }

TConnectionStatusDS::~TConnectionStatusDS()
  {
  /// Nothing to do here atm
  }

unsigned int TConnectionStatusDS::GetPeerConnectionCount() const
  {
  return static_cast<unsigned int>(App->get_network()->get_connections().size());
  }

bool TConnectionStatusDS::IsMailConnected() const
  {
  return App->is_mail_connected();
  }

