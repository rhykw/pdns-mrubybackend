#include "pdns/utility.hh"
#include "pdns/dnsbackend.hh"
#include "pdns/dns.hh"
#include "pdns/dnsbackend.hh"
#include "pdns/dnspacket.hh"
#include "pdns/pdnsexception.hh"
#include "pdns/logger.hh"
#include "pdns/version.hh"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <map>
#include <exception>
#include <string>

extern "C" {
#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/variable.h>
#include <mruby/hash.h>
#include <mruby/error.h>
#include <unistd.h>
}

namespace PowerdnsMrubyBackend
{
void set_request(mrb_state *, map<string, string>);
std::vector<std::map<std::string, std::string> >
get_answer_records(mrb_state *);
void install_mrb_class(mrb_state *);
}

class MrubyBackendCommonException
{
};

/* FIRST PART */
class MrubyBackend : public DNSBackend
{
public:
  MrubyBackend(const string &suffix = "")
  {
    setArgPrefix("mruby" + suffix);
    d_ri_cnt = 0;
    mrb = mrb_open();
  }

  void init(void)
  {

    string mrb_filename = getArg("filename");
    init_code = getArg("initcode");
    lookup_code = getArg("lookupcode");

    PowerdnsMrubyBackend::install_mrb_class(mrb);
    mrb_gc_arena_restore(mrb, 0);
    mrbc = mrbc_context_new(mrb);

    if (!mrb_filename.empty() && mrb_filename != "") {
      mrbc_filename(mrb, mrbc, mrb_filename.c_str());

      FILE *fp = fopen(mrb_filename.c_str(), "r");
      if (fp == NULL) {
        fprintf(stderr, "Cannot open %s\n", mrb_filename.c_str());
      } else {
        mrb_load_file_cxt(mrb, fp, mrbc);
        fclose(fp);
        if (mrb->exc) {
          mrb_print_error(mrb);
          mrb->exc = 0;
        }
      }
      fp = NULL;
    }

    code_execute(init_code);
  }

  bool list(const string &target, int id, bool include_disabled)
  {
    return false; // we don't support AXFR
  }

  void lookup(const QType &type, const string &qdomain, DNSPacket *p,
              int zoneId)
  {
    map<string, string> the_request;

    the_request["remote_addr"] = p->getRemote();
    the_request["type"] = type.getName();
    the_request["domain"] = qdomain;

    PowerdnsMrubyBackend::set_request(mrb, the_request);

    if (!code_execute(lookup_code)) {
      mrb_gc_arena_restore(mrb, 0);
      return;
    }
    d_records = PowerdnsMrubyBackend::get_answer_records(mrb);
    mrb_gc_arena_restore(mrb, 0);

    // printf( "DEBUG: d_records.size()=%d\n",d_records.size() );
    d_ri = d_records.begin();
  }

  bool code_execute(string str)
  {
    mrb_value v = mrb_load_string_cxt(mrb, str.c_str(), mrbc);
    if (mrb->exc != 0) {
      int pid = (int)Utility::getpid();
      mrb_value exc = mrb_obj_value(mrb->exc);

      mrb_value backtrace = mrb_get_backtrace(mrb);
      mrb_value inspect = mrb_inspect(mrb, exc);

      L << Logger::Error << "mruby code execution failed: "
        << std::string(mrb_str_to_cstr(mrb, mrb_inspect(mrb, backtrace)))
        << endl;
      L << Logger::Error << std::string(mrb_str_to_cstr(mrb, inspect)) << endl;
      // printf("DEBUG[%d]:
      // ##################################################\n",pid);
      // printf("DEBUG[%d]: %s\n",pid,mrb_str_to_cstr(mrb, mrb_inspect(mrb,
      // backtrace)));
      // printf("DEBUG[%d]: %s\n",pid,mrb_str_to_cstr(mrb, inspect));
      // printf("DEBUG[%d]:
      // ##################################################\n",pid);

      mrb->exc = 0;
      return false;
    }
    return true;
  }

  bool get(DNSResourceRecord &rr)
  {
    if (d_records.size() == 0 || d_ri == d_records.end())
      return false;

    std::map<std::string, std::string> recordMap = *d_ri;
    DNSResourceRecord record;
    record.ttl = 120;
    record.auth = 1;
    record.qtype = QType(QType::chartocode(recordMap["type"].c_str()));
    record.content = recordMap["content"];
    record.qname = recordMap["name"];
    rr = record;

    d_ri++;
    if (d_ri == d_records.end())
      d_records.clear();

    // DEBUG
    // printf("DEBUG[%d]:
    // d_ri_cnt=%d,qname=%s,qtype=%s,ttl=%d,auth=%d,content=%s",Utility::getpid(),d_ri_cnt++,rr.qname.c_str(),rr.qtype.getName().c_str(),rr.ttl,rr.auth,rr.content.c_str());
    // printf("\n");
    return true;
  }

private:
  int d_ri_cnt;
  std::vector<std::map<std::string, std::string> > d_records;
  std::vector<std::map<std::string, std::string> >::const_iterator d_ri;

  mrb_state *mrb;
  mrbc_context *mrbc;

  string init_code;
  string lookup_code;
};

/* SECOND PART */

class MrubyFactory : public BackendFactory
{
public:
  MrubyFactory() : BackendFactory("mruby")
  {
  }
  void declareArguments(const string &suffix = "")
  {
    declare(suffix, "filename", "Filename of the script for mruby backend", "");
    declare(suffix, "initcode", "Init Code", "");
    declare(suffix, "lookupcode", "Lookup Code", "");
  }
  DNSBackend *make(const string &suffix = "")
  {
    MrubyBackend *backend = new MrubyBackend(suffix);
    L << Logger::Debug << "[mrubybackend] New backend has made." << endl;
    backend->init();
    return backend;
  }
};

/* THIRD PART */

class MrubyLoader
{
public:
  MrubyLoader()
  {
    BackendMakers().report(new MrubyFactory);
    L << Logger::Info
      << "[mrubybackend] This is the mruby backend version " VERSION
         " (" __DATE__ ", " __TIME__ ") reporting" << endl;
  }
};

static MrubyLoader mrubyLoader;
