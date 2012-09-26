#include <string.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/bigarray.h>
#include <caml/camlidlruntime.h>

#include "gstreamer_inc.h"

/***** Helper C functions *****/

GstElement* parse_launch(const gchar *pipeline_description) {
  GError *err = NULL;
  GstElement *e;
  e = gst_parse_launch(pipeline_description, &err);
  if (err != NULL) {
    value s = caml_copy_string(err->message);
    if (e) { gst_object_unref(e); }
    g_error_free(err);
    caml_raise_with_arg(*caml_named_value("gst_exn_gerror"), s);
  }
  return e;
};


/***** Stubs that cannot be written directly in IDL. *****/

void camlidl_ml2c_gstreamer_idl_pGstElement(value _v1, pGstElement * _c2, camlidl_ctx _ctx);

value caml_app_sink_pull_buffer(value as){
  GstBuffer *gstbuf;
  GstElement *e;
  struct camlidl_ctx_struct _ctxs = { CAMLIDL_TRANSIENT, NULL };
  camlidl_ctx _ctx = &_ctxs;
  camlidl_ml2c_gstreamer_idl_pGstElement(as, &e, _ctx);

  caml_enter_blocking_section();
  gstbuf = gst_app_sink_pull_buffer(GST_APP_SINK(e));
  if (!gstbuf) {
    caml_leave_blocking_section();
    caml_raise_constant(*caml_named_value("gst_exn_failure"));
  }
  intnat len = gstbuf->size;
  char *data = malloc(len);
  memcpy(data, gstbuf->data, len);
  gst_buffer_unref (gstbuf);
  caml_leave_blocking_section();

  value ba = caml_ba_alloc(CAML_BA_MANAGED | CAML_BA_C_LAYOUT | CAML_BA_UINT8, 1, data, &len);
  return ba;
}

value caml_app_sink_pull_buffer_string(value as){
  GstBuffer *gstbuf;
  GstElement *e;
  struct camlidl_ctx_struct _ctxs = { CAMLIDL_TRANSIENT, NULL };
  camlidl_ctx _ctx = &_ctxs;
  camlidl_ml2c_gstreamer_idl_pGstElement(as, &e, _ctx);

  caml_enter_blocking_section();
  gstbuf = gst_app_sink_pull_buffer(GST_APP_SINK(e));
  if (!gstbuf) {
    caml_leave_blocking_section();
    caml_raise_constant(*caml_named_value("gst_exn_failure"));
  }
  intnat len = gstbuf->size;
  caml_leave_blocking_section();

  value s = caml_alloc_string(len);
  memcpy(String_val(s), gstbuf->data, len);
  gst_buffer_unref (gstbuf);
  return s;
}

value caml_app_src_push_buffer_string(value as, value buf){
  int buflen = caml_string_length(buf);
  GstBuffer *gstbuf = gst_buffer_new_and_alloc(buflen);
  memcpy(GST_BUFFER_DATA(gstbuf), String_val(buf), buflen);
  GstFlowReturn ret;
  GstElement *e;
  struct camlidl_ctx_struct _ctxs = { CAMLIDL_TRANSIENT, NULL };
  camlidl_ctx _ctx = &_ctxs;
  camlidl_ml2c_gstreamer_idl_pGstElement(as, &e, _ctx);

  caml_enter_blocking_section();
  ret = gst_app_src_push_buffer(GST_APP_SRC(e), gstbuf);
  if (ret != GST_FLOW_OK) {
    caml_leave_blocking_section();
    caml_raise_constant(*caml_named_value("gst_exn_failure"));
  }
  caml_leave_blocking_section();

  return Val_unit;
}
