#include <boost/algorithm/string.hpp>
#include <iostream>
#include <map>
#include <list>
#include <vector>

extern "C" {
#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <stdio.h>
}

namespace PowerdnsMrubyBackend
{

#define POWERDNS_MRUBY_BACKEND_DEFINE_METHOD_MRB_GET_REQUEST(method_suffix)    \
  mrb_value mrb_request_get_##method_suffix(mrb_state *mrb, mrb_value self);   \
  mrb_value mrb_request_get_##method_suffix(mrb_state *mrb, mrb_value self)    \
  {                                                                            \
    mrb_value r;                                                               \
    r = mrb_obj_iv_get(mrb, (struct RObject *)mrb->object_class,               \
                       mrb_intern_lit(mrb, "__ requestMap __"));               \
    if (mrb_nil_p(r)) {                                                        \
      r = mrb_hash_new(mrb);                                                   \
      mrb_obj_iv_set(mrb, (struct RObject *)mrb->object_class,                 \
                     mrb_intern_lit(mrb, "__ requestMap __"), r);              \
    }                                                                          \
    return mrb_hash_get(mrb, r, mrb_str_new_cstr(mrb, #method_suffix));        \
  }

POWERDNS_MRUBY_BACKEND_DEFINE_METHOD_MRB_GET_REQUEST(remote_addr);
POWERDNS_MRUBY_BACKEND_DEFINE_METHOD_MRB_GET_REQUEST(type);
POWERDNS_MRUBY_BACKEND_DEFINE_METHOD_MRB_GET_REQUEST(domain);

void set_request(mrb_state *mrb, std::map<std::string, std::string> r)
{
  mrb_value rhsh = mrb_hash_new(mrb);
  const std::map<std::string, std::string>::const_iterator itEnd = r.end();
  for (std::map<std::string, std::string>::const_iterator it = r.begin();
       it != itEnd; ++it) {
    mrb_hash_set(mrb, rhsh, mrb_str_new_cstr(mrb, it->first.c_str()),
                 mrb_str_new_cstr(mrb, it->second.c_str()));
  }
  mrb_obj_iv_set(mrb, (struct RObject *)mrb->object_class,
                 mrb_intern_lit(mrb, "__ requestMap __"), rhsh);
}

std::vector<std::map<std::string, std::string> >
get_answer_records(mrb_state *mrb)
{

  int len;
  std::vector<std::map<std::string, std::string> > answerRecordsList;
  answerRecordsList.clear();

  mrb_value mrb_pdns_answer_records;
  mrb_pdns_answer_records =
      mrb_obj_iv_get(mrb, (struct RObject *)mrb->object_class,
                     mrb_intern_lit(mrb, "__ mrb_pdns_answer_records __"));

  if (!mrb_array_p(mrb_pdns_answer_records)) {
    return answerRecordsList;
  }

  len = RARRAY_LEN(mrb_pdns_answer_records);
  // printf( "DEBUG: answer_records.length=%d\n", len );

  if (len > 4096) {
    printf("DEBUG: opps! hit a bug!\n");
    return answerRecordsList;
  }

  answerRecordsList.clear();

  std::map<std::string, std::string> record;
  for (int i = 0; i < len; ++i) {
    mrb_value val = mrb_ary_ref(mrb, mrb_pdns_answer_records, i);
    if (mrb_hash_p(val)) {
      mrb_value keys = mrb_hash_keys(mrb, val);
      int klen = RARRAY_LEN(keys);
      // printf( "DEBUG: answer_records[%d] has %d keys\n", i, klen );

      record.clear();

      // printf( "DEBUG:" );
      for (int ki = 0; ki < klen; ++ki) {
        mrb_value rkey = mrb_ary_ref(mrb, keys, ki);
        mrb_value rval = mrb_hash_get(mrb, val, rkey);
        // printf("[%d]%s=>%s ", ki, mrb_str_to_cstr(mrb,
        // rkey),mrb_str_to_cstr(mrb, rval));

        std::string hkey = std::string(mrb_str_to_cstr(mrb, rkey));
        record[hkey] = std::string(mrb_str_to_cstr(mrb, rval));
      }
      // printf( "\n" );
      answerRecordsList.push_back(record);
    }
  }

  return answerRecordsList;
}

// mruby functions definition
mrb_value mrb_get_module_info(mrb_state *mrb, mrb_value self)
{
  return mrb_str_new_lit(mrb, __TIMESTAMP__);
}

mrb_value mrb_get_answer_records(mrb_state *mrb, mrb_value self)
{

  mrb_value mrb_pdns_answer_records;
  mrb_pdns_answer_records =
      mrb_obj_iv_get(mrb, (struct RObject *)mrb->object_class,
                     mrb_intern_lit(mrb, "__ mrb_pdns_answer_records __"));

  return mrb_pdns_answer_records;
}

void install_mrb_class(mrb_state *mrb)
{

  mrb_value mrb_pdns_answer_records = mrb_ary_new(mrb);
  mrb_obj_iv_set(mrb, (struct RObject *)mrb->object_class,
                 mrb_intern_lit(mrb, "__ mrb_pdns_answer_records __"),
                 mrb_pdns_answer_records);

  struct RClass *rclass = mrb_define_class(mrb, "Powerdns", mrb->object_class);
  struct RClass *req_rclass =
      mrb_define_class_under(mrb, rclass, "Request", mrb->object_class);
  mrb_define_class_method(mrb, rclass, "module_version", mrb_get_module_info,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "answer", mrb_get_answer_records,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, req_rclass, "remote_addr",
                          mrb_request_get_remote_addr, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, req_rclass, "type", mrb_request_get_type,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, req_rclass, "domain", mrb_request_get_domain,
                          MRB_ARGS_NONE());
}
}
