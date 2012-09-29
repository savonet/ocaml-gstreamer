#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/bigarray.h>
#include <caml/camlidlruntime.h>
#include <caml/threads.h>

#include <string.h>
#include <assert.h>

#include <gst/gst.h>
#include <gst/gstclock.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

CAMLprim value ocaml_gstreamer_init(value _argv)
{
  CAMLparam1(_argv);
  char **argv = NULL;
  int argc = 0;
  int len, i;

  if (Is_block(_argv))
    {
      _argv = Field(_argv, 0);
      argc = Wosize_val(_argv);
      argv = malloc(argc*sizeof(char*));
      for(i = 0; i < argc; i++)
        {
          len = caml_string_length(Field(_argv,i));
          argv[i] = malloc(len);
          memcpy(argv[i], String_val(Field(_argv,i)), len);
        }
    }

  caml_release_runtime_system();
  gst_init(&argc, &argv);
  for(i = 0; i < argc; i++)
    free(argv[i]);
  free(argv);
  caml_acquire_runtime_system();

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_deinit(value unit)
{
  CAMLparam0();

  caml_release_runtime_system();
  gst_deinit();
  caml_acquire_runtime_system();

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_version(value unit)
{
  CAMLparam0();
  CAMLlocal1(ans);

  unsigned int major, minor, micro, nano;
  gst_version(&major, &minor, &micro, &nano);

  ans = caml_alloc_tuple(4);
  Store_field(ans,0,Val_int(major));
  Store_field(ans,1,Val_int(minor));
  Store_field(ans,2,Val_int(micro));
  Store_field(ans,3,Val_int(nano));

  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_version_string(value unit)
{
  CAMLparam0();
  CAMLreturn(caml_copy_string(gst_version_string()));
}

/***** Element ******/

#define Element_val(v) (*(GstElement**)Data_custom_val(v))

static void finalize_element(value v)
{
  GstElement *e = Element_val(v);
  gst_object_unref(e);
}

static struct custom_operations element_ops =
  {
    "ocaml_gstreamer_element",
    finalize_element,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_element(GstElement *e)
{
  if (!e) caml_raise_constant(*caml_named_value("gstreamer_exn_error"));
  value ans = caml_alloc_custom(&element_ops, sizeof(GstElement*), 0, 1);
  Element_val(ans) = e;
  return ans;
}

CAMLprim value ocaml_gstreamer_element_set_property_string(value e, value l, value v)
{
  CAMLparam3(e,l,v);
  g_object_set (Element_val(e), String_val(l), String_val(v), NULL);
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_element_set_property_int(value e, value l, value v)
{
  CAMLparam3(e,l,v);
  g_object_set (Element_val(e), String_val(l), Int_val(v), NULL);
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_element_set_property_bool(value e, value l, value v)
{
  CAMLparam3(e,l,v);
  g_object_set (Element_val(e), String_val(l), Bool_val(v), NULL);
  CAMLreturn(Val_unit);
}

static GstState state_of_val(value v)
{
  switch(Int_val(v))
    {
    case 0:
      return GST_STATE_VOID_PENDING;

    case 1:
      return GST_STATE_NULL;

    case 2:
      return GST_STATE_READY;

    case 3:
      return GST_STATE_PAUSED;

    case 4:
      return GST_STATE_PLAYING;

    default:
      assert(0);
    }
}

static value value_of_state_change_return(GstStateChangeReturn ret)
{
  switch(ret)
    {
    case GST_STATE_CHANGE_FAILURE:
      /* TODO: raise an error */
      assert(0);

    case GST_STATE_CHANGE_SUCCESS:
      return Val_int(0);

    case GST_STATE_CHANGE_ASYNC:
      return Val_int(1);

    case GST_STATE_CHANGE_NO_PREROLL:
      return Val_int(2);

    default:
      assert(0);
    }
}

CAMLprim value ocaml_gstreamer_element_set_state(value _e, value _s)
{
  CAMLparam2(_e, _s);
  GstElement *e = Element_val(_e);
  GstState s = state_of_val(_s);
  GstStateChangeReturn ret;

  caml_release_runtime_system();
  ret = gst_element_set_state(e, s);
  caml_acquire_runtime_system();

  CAMLreturn(value_of_state_change_return(ret));
}

CAMLprim value ocaml_gstreamer_element_link(value _src, value _dst)
{
  CAMLparam2(_src, _dst);
  GstElement *src = Element_val(_src);
  GstElement *dst = Element_val(_dst);
  gboolean ret;

  caml_release_runtime_system();
  ret = gst_element_link(src, dst);
  caml_acquire_runtime_system();

  assert(ret);
  CAMLreturn(Val_unit);
}

/***** Element factory *****/

CAMLprim value ocaml_gstreamer_element_factory_make(value factname, value name)
{
  CAMLparam2(factname, name);
  CAMLlocal1(ans);
  GstElement *e;

  e = gst_element_factory_make(String_val(factname), String_val(name));
  ans = value_of_element(e);

  CAMLreturn(ans);
}

/***** Bin ******/

#define Bin_val(v) GST_BIN(Element_val(v))

CAMLprim value ocaml_gstreamer_bin_add(value _bin, value _e)
{
  CAMLparam2(_bin, _e);
  GstBin *bin = Bin_val(_bin);
  GstElement *e = Element_val(_e);
  gboolean ret;

  caml_release_runtime_system();
  /* TODO: we have to ref here and have the bin unref it children at finalize!
     Namely, the bin should not have references to deallocated childs. */
  //gst_object_ref(e);
  ret = gst_bin_add(bin, e);
  caml_acquire_runtime_system();

  assert(ret);

  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_bin_get_by_name(value _bin, value _name)
{
  CAMLparam2(_bin, _name);
  CAMLlocal1(ans);
  GstBin *bin = Bin_val(_bin);
  GstElement *e;

  e = gst_bin_get_by_name(bin, String_val(_name));

  CAMLreturn(value_of_element(e));
}

/***** Pipeline *****/

CAMLprim value ocaml_gstreamer_pipeline_create(value s)
{
  CAMLparam1(s);
  CAMLlocal1(ans);
  GstElement *e;

  e = gst_pipeline_new(String_val(s));

  ans = value_of_element(e);
  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_pipeline_parse_launch(value s)
{
  CAMLparam1(s);
  CAMLlocal1(ans);
  GError *err = NULL;
  GstElement *e;

  e = gst_parse_launch(String_val(s), &err);
  if (err != NULL)
    {
      value s = caml_copy_string(err->message);
      if (e) { gst_object_unref(e); }
      g_error_free(err);
      caml_raise_with_arg(*caml_named_value("gst_exn_gerror"), s);
    }
  ans = value_of_element(e);

  CAMLreturn(ans);
}

/***** Appsrc *****/

typedef struct {
  GstAppSrc *appsrc;
  value need_data_cb; // Callback function
  gulong need_data_hid; // Callback handler ID
} appsrc;

#define Appsrc_val(v) (*(appsrc**)Data_custom_val(v))

static void disconnect_need_data(appsrc *as)
{
  if(as->need_data_cb)
    {
      caml_remove_global_root(&as->need_data_cb);
      as->need_data_cb = 0;
    }
  if(as->need_data_hid)
    {
      g_signal_handler_disconnect(as->appsrc, as->need_data_hid);
      as->need_data_hid = 0;
    }
}

static void finalize_appsrc(value v)
{
  appsrc *as = Appsrc_val(v);
  disconnect_need_data(as);
  free(as);
}

static struct custom_operations appsrc_ops =
  {
    "ocaml_gstreamer_appsrc",
    finalize_appsrc,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
  };

static value value_of_appsrc(GstAppSrc *e)
{
  value ans = caml_alloc_custom(&appsrc_ops, sizeof(GstAppSrc*), 0, 1);
  appsrc *as = malloc(sizeof(appsrc));
  as->appsrc = e;
  as->need_data_cb = 0;
  as->need_data_hid = 0;
  Appsrc_val(ans) = as;
  return ans;
}

CAMLprim value ocaml_gstreamer_appsrc_of_element(value _e)
{
  CAMLparam1(_e);
  GstElement *e = Element_val(_e);
  CAMLreturn(value_of_appsrc(GST_APP_SRC(e)));
}

CAMLprim value ocaml_gstreamer_appsrc_to_element(value _as)
{
  CAMLparam1(_as);
  appsrc *as = Appsrc_val(_as);
  CAMLreturn(value_of_element(GST_ELEMENT(as->appsrc)));
}

CAMLprim value ocaml_gstreamer_appsrc_push_buffer_string(value _as, value _buf)
{
  CAMLparam2(_as, _buf);
  int buflen = caml_string_length(_buf);
  appsrc *as = Appsrc_val(_as);
  GstBuffer *gstbuf = gst_buffer_new_and_alloc(buflen);
  GstFlowReturn ret;

  memcpy(GST_BUFFER_DATA(gstbuf), String_val(_buf), buflen);

  caml_release_runtime_system();
  ret = gst_app_src_push_buffer(as->appsrc, gstbuf);
  caml_acquire_runtime_system();

  //TODO: raise
  assert(ret == GST_FLOW_OK);

  CAMLreturn(Val_unit);
}

static void appsrc_need_data_cb(GstAppSrc *gas, guint length, gpointer user_data)
{
  appsrc *as = (appsrc*)user_data;

  caml_c_thread_register();
  caml_acquire_runtime_system();
  caml_callback(as->need_data_cb, Val_int(length));
  caml_release_runtime_system();
  caml_c_thread_unregister();
}

CAMLprim value ocaml_gstreamer_appsrc_connect_need_data(value _as, value f)
{
  CAMLparam2(_as, f);
  appsrc *as = Appsrc_val(_as);
  disconnect_need_data(as);

  caml_register_global_root(&as->need_data_cb);

  caml_release_runtime_system();
  as->need_data_cb = f;
  as->need_data_hid = g_signal_connect(as->appsrc, "need-data", G_CALLBACK(appsrc_need_data_cb), as);
  caml_acquire_runtime_system();

  assert(as->need_data_hid);
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_gstreamer_appsrc_end_of_stream(value _as)
{
  CAMLparam1(_as);
  appsrc *as = Appsrc_val(_as);
  GstFlowReturn ret;

  caml_release_runtime_system();
  g_signal_emit_by_name(as->appsrc, "end-of-stream", &ret);
  caml_acquire_runtime_system();

  //TODO: raise
  assert(ret == GST_FLOW_OK);

  CAMLreturn(Val_unit);
}

/***** Appsink *****/

#define Appsink_val(v) (GST_APP_SINK(Element_val(v)))

CAMLprim value ocaml_gstreamer_appsink_pull_buffer(value _as)
{
  CAMLparam1(_as);
  CAMLlocal1(ans);
  GstAppSink *as = Appsink_val(_as);
  GstBuffer *gstbuf;
  intnat len;

  caml_release_runtime_system();
  gstbuf = gst_app_sink_pull_buffer(as);
  caml_acquire_runtime_system();

  if (!gstbuf)
    caml_raise_constant(*caml_named_value("gstreamer_exn_error"));
  len = gstbuf->size;
  ans = caml_ba_alloc(CAML_BA_C_LAYOUT | CAML_BA_UINT8, 1, NULL, &len);
  memcpy(Caml_ba_data_val(ans), gstbuf->data, len);
  gst_buffer_unref(gstbuf);

  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_appsink_pull_buffer_string(value _as)
{
  CAMLparam1(_as);
  GstAppSink *as = Appsink_val(_as);
  GstBuffer *gstbuf;
  intnat len;

  caml_release_runtime_system();
  gstbuf = gst_app_sink_pull_buffer(as);
  caml_acquire_runtime_system();

  if (!gstbuf)
    caml_raise_constant(*caml_named_value("gstreamer_exn_error"));
  len = gstbuf->size;
  value ans = caml_alloc_string(len);
  memcpy(String_val(ans), gstbuf->data, len);
  gst_buffer_unref(gstbuf);
  CAMLreturn(ans);
}

CAMLprim value ocaml_gstreamer_appsink_is_eos(value _as)
{
  CAMLparam1(_as);
  GstAppSink *as = Appsink_val(_as);
  gboolean ret;

  caml_release_runtime_system();
  ret = gst_app_sink_is_eos(as);
  caml_acquire_runtime_system();

  CAMLreturn(Val_bool(ret));
}
