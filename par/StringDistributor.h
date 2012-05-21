#ifndef __PAR__STRINGDISTRIBUTOR_H__
#define __PAR__STRINGDISTRIBUTOR_H__

#include <list>
#include <map>
#include "ex/BaseException.h"
#include "par/Distributor.h"
#include "sys/ints.h"

namespace par {

using namespace ex;
using namespace ptr;
using namespace std;

class StringDistributor : public Distributor {
private:
  struct header {
    size_t nbytes;
    int sender;
    uint32_t id;
    uint32_t order;
  };
  typedef multimap<header,
                   DPtr<uint8_t>*,
                   bool(*)(const header &h1, const header&)>
          RecvMap;
  typedef list<DPtr<uint8_t>*> SendList;
  RecvMap recv_packets;
  SendList send_packets;
  Distributor *dist;
  const size_t packet_size;
  const int my_rank;
  int pending_send_rank;
  uint32_t id_counter;
  bool no_more_sends;
  static bool headerlt(const header &h1, const header &h2);
public:
  StringDistributor(const int my_rank, const size_t packet_size,
                    Distributor *dist) throw(BaseException<void*>,
                                             BaseException<size_t>);
  virtual ~StringDistributor() throw(DistException);

  virtual void init() throw(DistException, BadAllocException);
  virtual bool send(const int rank, DPtr<uint8_t> *msg) throw(DistException);
  virtual DPtr<uint8_t> *receive() throw(DistException);
  virtual void noMoreSends() throw(DistException);
  virtual bool done() throw(DistException);
};

}

#endif /* __PAR__STRINGDISTRIBUTOR_H__ */
